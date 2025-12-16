#define SOY_LIB_IMPLEMENTATION
#include "soylib.h"

#define SOY_TRACKER_MAX_INST 0xFF
#define SOY_TRACKER_MAX_LEN 512
#define SOY_TRACKER_MAX_DIR 500
#define SOY_TRACKER_MAX_PATS 0xFF
#define SOY_TRACKER_MAX_UNDOS 0xFF
#define SOY_TRACKER_MAX_NODES 0xFF
#define TRACKER_MIN_HEIGHT 132
#define TRACKER_ICON_SIZE 14

#define CHANNEL_BUTTONS 8
#define SOY_TRACKER_BUTTONS 16
#define SOY_TRACKER_VISUALINTS 16

cString global_nodeString[] = {
  "Note",
  "Octave",
  "Instrument",
  "Instrument",
  "Volume",
  "Volume",
  "Effect",
  "Effect",
  "Param",
  "Param"
};

const int8* global_noteStr[] = {
  "C-", "Db", "D-", "Eb", "E-", "F-", "Gb", "G-", "Ab", "A-", "Bb", "B-"
};

const int32 nodeSteps14[] = { 4, 6, 8, 9, 11, 12, 14, 15, 17, 18 };
const int32 nodeSteps11[] = { 2, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
const int32* global_nodeSteps = nodeSteps14;

typedef enum enum_soyNodeState {
  TNODE_EMPTY,
  TNODE_FADE,
  TNODE_NOTE,
  TNODE_INST,
  TNODE_VOLUME,
  TNODE_EFFECT,
  TNODE_EFFPAR,
  TNODE_END
} soyNodeState;

typedef enum enum_soyTrackerState {
  EDIT,
  CONFIG,
  FILES,
  SAMPLE,
  INSTRUMENT,
  MIXER
} soyTrackerState;

typedef enum enum_soyTrackerPopup {
  NOPOPUP,
  SAVE,
  LOAD
} soyTrackerPopup;

typedef struct tag_soyTrackerNode {

  uint8 note;
  uint8 octave;
  uint8 instrument;
  uint8 volume;
 
  uint8 effect;
  uint8 effectParam;
  uint8 number;
  uint8 flags;

} soyTrackerNode;

typedef struct tag_loopPoint {
  uint32 track;
  uint32 node;
} loopPoint;

typedef struct tag_soyTrackerChannel {
  soyTrackerNode nodes[SOY_TRACKER_MAX_NODES];
  soyButton buttons[CHANNEL_BUTTONS];
  int32 visibleLen;
  int32 timescale;
  int32 guiHeaderHeight;
  int32 guiChannelWidth;
  sflag enabled;
  sflag update;
  sflag empty;
  sflag fade;
  uint8 fadeAmt;
  int32 ID;
  soyRect rect;
  cString infoStr;
} soyTrackerChannel;

#define SOY_TRACKER_MAX_LOOP 16 

typedef struct tag_nodeSelection {
  int32 pattern;
  int32 channelStart;
  int32 channelEnd;
  int32 cursorStart;
  int32 cursorEnd;
  int32 paramStart;
  int32 paramEnd;
  sflag lockMouse;
  sflag lockKey;
} nodeSelection;

typedef struct tag_soyTrackerPattern {
  soyTrackerChannel channels[SOY_MIXER_CHANNELS];
  int32 channelLen;
  int32 channelCount;
  soyTrackerNode copyNode;
  nodeSelection selection;
  real32 bpm;
  soyRect rect;
  sflag used;
  int32 ID;
} soyTrackerPattern;

typedef struct tag_trackerClipboard {
  uint32 dataCount;
  uint32 nodeCount;
  uint32 chanCount;
  uint32 maxSize;
  nodeSelection selection;
  soyTrackerNode** copyData;
} trackerClipboard;

typedef struct tag_soyTracker {
  soyVirtualMemory patternMemory;
  soyVirtualMemory clipboardMemory;
  int32 patternIndices[SOY_TRACKER_MAX_PATS];
  int32 trackLen;
  int32 patternCount;
  int32 curPattern;
  int32 curChannel;
  int32 trackCursor;
  uint32 playCursor;
  uint32 editCursor;
  int32 nodeCursor;
  int32 globalOctave;
  int32 globalTranspose;
  int32 numSamples;
  int32 loopCounter;
  int32 loopState;
  real32 timer;
  sflag globalShiftInput;
  sflag globalControlInput;
  sflag playing;
  sflag showBg;
  soyRect rect;
  soyRect toolbarRect;
  soyRect controlRect;
  soyRect trackRect;
  soyRect infoRect;
  soyTrackerState state;
  soyTrackerPopup popupState;
  soyAppHandle appHandle;
  int8 lastSaveDirBuffer[SOY_TRACKER_MAX_DIR];
  soyButton buttons[SOY_TRACKER_VISUALINTS];
  trackerClipboard clipboard;
  int8 emptyGlyph;
} soyTracker;

/* globals */
soyTracker global_tracker;
sflag global_lockInput = FALSE;
sflag global_fontChanged = FALSE;
soySound global_samples[0xFF];
uint32 global_keyMap[0xFF];
uint32 global_pianoMap[0xFF];
int32 global_bpm = 120;

/********************************************/
/* @constructors                            */
/********************************************/

sflag soy_tracker_init_clipboard(soyVirtualMemory* memory, trackerClipboard* cb) {
  cb->copyData = ((soyTrackerNode**)memory->memory);
  cb->maxSize = (memory->size/sizeof(soyTrackerNode*));
  cb->dataCount = 0;
  cb->chanCount = 0;
  cb->nodeCount = 0;
  return TRUE;
}

sflag soy_tracker_reset_clipboard(trackerClipboard* cb, soyTracker* tracker) {
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  soyTrackerPattern* curPat = &patterns[tracker->curPattern];
  soyTrackerNode* curNode = &curPat->channels[tracker->curChannel].nodes[tracker->editCursor];
  nodeSelection sel = curPat->selection;
  cb->selection = sel;
  cb->dataCount = 1;
  cb->nodeCount = 1;
  cb->chanCount = 1;
  cb->copyData[0] = curNode;
  return TRUE;
}

sflag soy_tracker_reset_selection(nodeSelection* s,
    int32 curs, int32 param, int32 chan, int32 pat) {
  s->cursorStart = curs;
  s->cursorEnd = curs;
  s->channelStart = chan;
  s->channelEnd = chan;
  s->paramStart = param;
  s->paramEnd = param;
  s->pattern = pat;
  s->lockMouse = FALSE;
  return TRUE;
}

soyTrackerNode soy_tracker_node(soyNote note, uint8 instrument) {
  soyTrackerNode ret;
  ret.note = note;
  ret.instrument = instrument;
  ret.volume = 0;
  ret.effect = 0;
  ret.number = 0;
  ret.octave = 0;
  ret.effectParam = 0;
  ret.flags = 0;
  SOY_bitSet(ret.flags, TNODE_EMPTY);
  return ret;
}

sflag soy_tracker_node_end(soyTrackerNode* curNode) {
  SOY_bitSet(curNode->flags, TNODE_END);
  return TRUE;
}

sflag soy_tracker_node_fade(soyTrackerNode* curNode) {
  SOY_bitSet(curNode->flags, TNODE_FADE);
  return TRUE;
}

sflag soy_tracker_node_clear(soyTrackerNode* curNode) {
  curNode->effect = 0;
  curNode->instrument = 0;
  curNode->note = 0;
  curNode->octave = 0;
  curNode->volume = 0;
  curNode->effectParam = 0;
  curNode->flags = 0;
  SOY_bitSet(curNode->flags, TNODE_EMPTY);
  return TRUE;
}

sflag soy_tracker_channel_init_nodes(soyTrackerChannel* chan) {
  int32 j;
  for(j = 0; j < SOY_TRACKER_MAX_NODES; j++) {
    chan->nodes[j] = soy_tracker_node(0, 0);
    chan->nodes[j].number = j;
  }
  return TRUE;
}

sflag soy_tracker_channel_init(soyAppHandle app, int32 width, int32 headHeight,
    soyTrackerChannel* chan, uint32 index) {
  uint32 yStep = (app->draw.font.grid-app->draw.font.globalKerning)*app->draw.font.scale;
  int32 i = 0;
  chan->enabled = TRUE;
  chan->timescale = 4;
  chan->empty = TRUE;
  chan->update = TRUE;
  chan->guiChannelWidth = width;
  chan->guiHeaderHeight = headHeight;
  chan->rect = SOY_rectSimple("Channel", 0, 0, yStep*width+4, 0);
  chan->ID = index;
  chan->visibleLen = chan->rect.width+2;
  for(i = 0; i < CHANNEL_BUTTONS; i++) {
    chan->buttons[i] = SOY_buttonMicro("cbutton", 0, 0, NULL);
  }
  SOY_rectUnsetArmable(&chan->rect);
  soy_tracker_channel_init_nodes(chan);
  return FALSE;
}

sflag soy_tracker_channel_resize(soyAppHandle app, int32 width, soyTrackerChannel* chan) {
  uint32 yStep = (app->draw.font.grid-app->draw.font.globalKerning)*app->draw.font.scale;
  chan->rect = SOY_rectSimple("Channel", 0, 0, yStep*width+4, 0);
  chan->visibleLen = chan->rect.width+2;
  SOY_rectUnsetArmable(&chan->rect);
  return TRUE;
}

sflag soy_tracker_channel_mute(soyAppHandle app, soyTrackerChannel* channel) {
  channel->update = FALSE;
  SOY_soundStop(SOY_soundGet(app, channel->ID));
  return TRUE;
}

sflag soy_tracker_channel_unmute(soyTrackerChannel* channel) {
  channel->update = TRUE;
  return TRUE;
}

sflag soy_tracker_pattern_init(soyTrackerPattern* p, soyAppHandle app, soyTracker* tracker, uint32 id) {
  int32 i;
  p->channelCount = SOY_MIXER_CHANNELS;
  p->rect = tracker->rect;
  p->bpm = 120;
  p->channelLen = 64;
  p->copyNode = soy_tracker_node(0,0);
  p->used = FALSE;
  p->ID = id;
  p->selection.channelStart = 0;
  p->selection.channelEnd = 0;
  p->selection.cursorStart = 0;
  p->selection.cursorEnd = 0;
  p->selection.paramStart = 0;
  p->selection.paramEnd = 0;
  tracker->curChannel = 0;
  for(i = 0; i < p->channelCount; i++) {
    soy_tracker_channel_init(app, 20, 5, &p->channels[i], i);
  }
  return TRUE;
}
sflag soy_tracker_pattern_resize(soyTrackerPattern* p, soyAppHandle app, soyTracker* tracker) {
  int32 i;
  for(i = 0; i < p->channelCount; i++) {
    soy_tracker_channel_resize(app, 20, &p->channels[i]);
  }
  soy_tracker_reset_selection(&p->selection,
      tracker->editCursor, tracker->nodeCursor, tracker->curChannel, tracker->curPattern);
  return tracker != NULL;
}

/********************************************/
/* @saving                                  */
/********************************************/

sflag soy_tracker_load_instruments(soyAppHandle app, soyTracker* tracker, cString dir) {
  soyListFileData lfd;
  uint32 numFiles = 0;
  int8 dirBuf[511];
  cString curString = NULL;
  uint8 i = 0;
  soyVirtualMemory* loadMemory = &app->memory.audioLoadMemory;
  soyVirtualMemory* sampleMemory = &app->memory.audioSampleMemory;
  if(tracker == NULL) { return FALSE; }
  if(!SOY_listFilesInDir(app, dir, "*.raw", SOY_listFiles, &lfd)) {
    SLOG("No instruments found\n");
    for(i = 0; i < SOY_TRACKER_MAX_INST; i++) {
      SOY_soundInit(&global_samples[i]);  
    }
    return TRUE;
  }
  numFiles = lfd.count;
  if(numFiles >= SOY_MAX_FILES) { numFiles = SOY_MAX_FILES-1; }
  SOY_memoryReset(sampleMemory);
  SOY_memoryReset(loadMemory);
  for(i = 0; i < SOY_TRACKER_MAX_INST; i++) {
    if(i < numFiles) {
      sprintf(dirBuf, "%s\\%s", dir, lfd.filenames[i]);
      if(!SOY_loadSoundRaw(app, "sample", &global_samples[i], dirBuf, 1, 16, 48000)) {
        SLOG1("Error loading file: %s", curString);
        global_samples[i].name = NULL;
      }
    } else {
      SOY_soundInit(&global_samples[i]);
    }
  }
  tracker->numSamples = numFiles;
  return TRUE;
}

sflag soy_tracker_load_state(soyTracker* tracker, cString name, uint32 state) {
  int32 ret;
  soyFile* saveFile;
  int8 strBuf[512];
  sprintf(strBuf, "data/%s_%d.soy", name, state);
  saveFile = SOY_openFile(strBuf, "rb");
  if(saveFile == NULL) { return FALSE; }
  SLOG("Loading tracker state\n");
  ret = fread(tracker, sizeof(soyTracker), 1, saveFile);
  ret = fread(tracker->patternMemory.memory, tracker->patternMemory.size, 1, saveFile);
  SLOG("Tracker state loaded  \n");
  SOY_closeFile(saveFile);
  return ret;
}

sflag soy_tracker_save_state(soyTracker* tracker, cString name, uint32 state) {
  int32 ret;
  soyFile* saveFile;
  int8 strBuf[512];
  sprintf(strBuf, "data/%s_%d.soy", name, state);
  saveFile = SOY_openFile(strBuf, "wb");
  SLOG("Saving tracker state\n");
  if(saveFile == NULL) { return FALSE; }
  ret = fwrite(tracker, sizeof(soyTracker), 1, saveFile);
  ret = fwrite(tracker->patternMemory.memory,
      tracker->patternMemory.size, 1, saveFile);
  SLOG("Tracker state saved\n");
  SOY_closeFile(saveFile);
  return ret;
}

soyTextBox global_saveTextBox;
soyButton global_fileDialogButtons[8];
sflag global_cancelPopup = FALSE;
int8 global_activeDirBuffer[1024];
int32 global_activeDirBufferIndex = 0;

sflag soy_tracker_textbox_func(int8* text, uint32 textLength) {
  if(text == NULL || textLength == 0) { return TRUE; }  
  return TRUE;
}

sflag soy_tracker_init_save_system(soyTracker* tracker) {
  global_saveTextBox = SOY_textBox(tracker->appHandle, "savebox", 0, 0,
      64, 1, soy_tracker_textbox_func);
  global_fileDialogButtons[0] =
    SOY_button(tracker->appHandle, "Cancel", 0, 0, SOY_commandFind(tracker->appHandle, "TR_cancelpopup"));
  global_fileDialogButtons[1] =
    SOY_button(tracker->appHandle, "Confirm", 0, 0, SOY_commandFind(tracker->appHandle, "TR_confpopup"));
  sprintf(global_activeDirBuffer, ".");
  return TRUE;
}

sflag global_popupLockMouse = FALSE;
int32 global_popupScroll = 0;
int32 global_popupPrevScroll = 0;
int32 global_popupCurScroll = 0;

sflag soy_tracker_save_file(soyAppHandle app, soyTracker* tracker, cString filename) {
  int32 i, j, k;
  int8 saveBuf[SOY_TRACKER_MAX_DIR];
  soyFile* file = NULL;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  soyTrackerPattern* curPatt = NULL;
  soyTrackerChannel* curChan = NULL;
  if(app == NULL || tracker == NULL || filename == NULL) { return FALSE; }
  if(SOY_stringMatch(filename, ".stt") || SOY_stringMatch(filename, ".SST")) {
    sprintf(saveBuf, "%s", filename);
  } else {
    sprintf(saveBuf, "%s.stt", filename);
  }
  file = SOY_openFile(saveBuf, "w");
  if(file == NULL) { return FALSE; }
  fprintf(file, "STT\n");
  fprintf(file, "LEN %d\n", tracker->trackLen);
  fprintf(file, "SEQ ");
  for(i = 0; i < tracker->trackLen; i++) {
    fprintf(file, "%d ", tracker->patternIndices[i]);
  }
  fprintf(file, "\nDAT\n");
  for(i = 0; i < SOY_TRACKER_MAX_PATS; i++) {
    curPatt = &patterns[i];
    if(curPatt->used) {
      fprintf(file, "PAT %d\n", i);
      fprintf(file, "IDX %d\n", curPatt->ID);
      for(j = 0; j < curPatt->channelCount; j++) {
        curChan = &curPatt->channels[j];
        if(!curChan->empty) {
          for(k = 0; k < curPatt->channelLen; k++) {
            if(!SOY_bitCheck(curChan->nodes[k].flags, TNODE_EMPTY)) {
              fprintf(file, "%02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                curChan->ID,
                curChan->nodes[k].number,
                curChan->nodes[k].note,
                curChan->nodes[k].octave,
                curChan->nodes[k].instrument,
                curChan->nodes[k].volume,
                curChan->nodes[k].effect,
                curChan->nodes[k].effectParam,
                curChan->nodes[k].flags
              );
            }
          }
        }
      }
    }
    fprintf(file, "\n");
  }
  fprintf(file, "\nEND");
  SOY_closeFile(file);
  return TRUE;
}

sflag soy_tracker_load_file(soyAppHandle app, soyTracker* tracker, cString filename) {
  soyFile* file;
  int32 i;
  int32 numBuf;
  int32 curPat;
  int32 numTracks;
  int8 strBuf[128];
  uint32 parseNode[9];
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  soyTrackerPattern* curPatHandle = NULL;
  soyTrackerChannel* curChannelHandle = NULL;
  soyTrackerNode* curNodeHandle = NULL;
  if(app == NULL || tracker == NULL || filename == NULL) { return FALSE; }
  if(!SOY_stringMatchEnd(filename, ".stt")) { return FALSE; }
  file = fopen(filename, "r");
  if(file == NULL) { return FALSE; }
  if(fscanf(file, "%s", strBuf) == EOF) { return FALSE; }
  if(!SOY_stringMatch(strBuf, "STT")) { fclose(file); return FALSE; }
  SLOG("Found STT format file\n");
  if(fscanf(file, "%s %d", strBuf, &numBuf) == EOF) { return FALSE; }
  if(!SOY_stringMatch(strBuf, "LEN")) { fclose(file); return FALSE; }
  numTracks = numBuf;
  SLOG1("Number of tracks: %d\n", numTracks);
  if(fscanf(file, "%s", strBuf) == EOF) { return FALSE; }
  if(!SOY_stringMatch(strBuf, "SEQ")) { fclose(file); return FALSE; }
  tracker->trackLen = numTracks;
  for(i = 0; i < numTracks; i++) {
    if(fscanf(file, "%d ", &numBuf) == EOF) { fclose(file); return FALSE; }
    tracker->patternIndices[i] = numBuf;
  }
  if(fscanf(file, "%s", strBuf) == EOF) { return FALSE; }
  if(!SOY_stringMatch(strBuf, "DAT")) { fclose(file); return FALSE; }
  SLOG("Found data section\n");
  while(fscanf(file, "%s %d", strBuf, &numBuf) != EOF) {
    SLOG("Found data block\n");
    if(!SOY_stringMatch(strBuf, "PAT")) { fclose(file); return FALSE; }
    if(fscanf(file, "%s %d", strBuf, &numBuf) == EOF) { return FALSE; }
    if(!SOY_stringMatch(strBuf, "IDX")) { fclose(file); return FALSE; }
    curPat = numBuf;
    if(curPat > SOY_TRACKER_MAX_PATS) { curPat = 0; }
    curPatHandle = &patterns[curPat];
    if(curPatHandle == NULL){ fclose(file); return FALSE; }
    soy_tracker_pattern_init(curPatHandle, app, tracker, numBuf);
    while(fscanf(file, "%02X %02X %02X %02X %02X %02X %02X %02X %02X",
        &parseNode[0],
        &parseNode[1],
        &parseNode[2],
        &parseNode[3],
        &parseNode[4],
        &parseNode[5],
        &parseNode[6],
        &parseNode[7],
        &parseNode[8]
        ) == 9) {
      SLOG9("%02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
        parseNode[0],
        parseNode[1],
        parseNode[2],
        parseNode[3],
        parseNode[4],
        parseNode[5],
        parseNode[6],
        parseNode[7],
        parseNode[8]
      );
      curChannelHandle = &curPatHandle->channels[parseNode[0]];
      if(curChannelHandle == NULL){ fclose(file); return FALSE; }
      curNodeHandle = &curPatHandle->channels[parseNode[0]].nodes[parseNode[1]]; 
      if(curNodeHandle == NULL){ fclose(file); return FALSE; }
      curNodeHandle->note         = parseNode[2];
      curNodeHandle->octave       = parseNode[3];
      curNodeHandle->instrument   = parseNode[4];
      curNodeHandle->volume       = parseNode[5];
      curNodeHandle->effect       = parseNode[6];
      curNodeHandle->effectParam  = parseNode[7];
      curNodeHandle->flags        = parseNode[8];
    }
  }
  SOY_closeFile(file);
  return TRUE;
}

sflag soy_tracker_file_explorer_func(soyAppHandle app, soyTracker* tracker, soyTrackerPopup type) {
  int32 i;
  soyListFileData lfd;
  soyButton curRect;
  int32 diff;
  int8 newDirBuf[SOY_TRACKER_MAX_DIR];
  int8 shortString[SOY_TRACKER_MAX_DIR];
  int32 yStep = app->draw.font.grid*app->draw.font.scale+3;
  int32 xStep = (app->draw.font.grid-app->draw.font.globalKerning)*app->draw.font.scale;
  int32 popupX = SOY_width(app)/2-global_saveTextBox.rect.width/2;
  int32 popupY = SOY_height(app)/8;
  int32 popupWidth = global_saveTextBox.rect.width+32;
  int32 popupHeight = SOY_height(app)-(SOY_height(app)/8)*2;
  int32 alignLeft = popupX;
  int32 alignRight = alignLeft+popupWidth-32;
  int32 alignTop = popupY+global_saveTextBox.rect.height + 2;
  int32 alignBot = popupY+popupHeight-global_saveTextBox.rect.height-2;
  soyFontHandle font = &app->draw.font;
  int32 buttonCount = (((global_saveTextBox.rect.y-7)-alignTop)/yStep);
  int32 sameCheck = 0;
  int32 strLen = 0;
  int32 drawLen = 0;
  int32 drawIndex = 0;
  int32 startIndex = 0;
  sflag exitTrue = FALSE;
  int32 centerAlign = alignLeft + (alignRight-alignLeft)/2;
  sflag rightFormat = FALSE;
  sflag hasName = FALSE;
  sflag noExt = TRUE;
  sflag isFolder = FALSE;
  sflag isSame = FALSE;
  int32 overwriteTextWidth = SOY_drawTextLen("OVERWRITE!", &app->draw.font);
  SOY_buttonResize(&global_fileDialogButtons[0],
      alignLeft, alignBot+(yStep/2)-5,
      SOY_drawTextLen(global_fileDialogButtons[0].rect.label, &app->draw.font), yStep);
  SOY_buttonResize(&global_fileDialogButtons[1],
      alignRight-global_fileDialogButtons[1].rect.width, alignBot+(yStep/2)-5,
      SOY_drawTextLen(global_fileDialogButtons[1].rect.label, &app->draw.font), yStep);
  SOY_buttonUpdate(app, &global_fileDialogButtons[0]);
  SOY_buttonUpdate(app, &global_fileDialogButtons[1]);
  SOY_drawResetPaletteTint(app);
  SOY_drawGUIPanel(app, alignLeft-16, alignTop-16, popupWidth, popupHeight);
  SOY_textBoxMove(&global_saveTextBox, alignLeft, alignBot-global_saveTextBox.rect.height-1);
  SOY_textBoxResize(&global_saveTextBox,
      soy_min(soy_max((int32)SOY_width(app)/((font->grid-font->globalKerning)*font->scale)-16, 16), 64), 1);
  SOY_textBoxUpdate(app, &global_saveTextBox);
  SOY_drawTextBox(app, &global_saveTextBox);
  SOY_drawGradient(app, alignLeft, alignTop, alignRight, global_saveTextBox.rect.y-4,
      SOY_drawGetPaletteColour(app, SOY_GUI_GRADENDINACTIVE),
      SOY_drawGetPaletteColour(app, SOY_GUI_GRADSTARTINACTIVE), 'v');
  SOY_drawGUIBorderInner(app, alignLeft, alignTop, alignRight, global_saveTextBox.rect.y-4);
  SOY_listFilesInDir(app, tracker->lastSaveDirBuffer, "*.*", SOY_listFiles, &lfd);
  if(lfd.count < 1) {
    SOY_currentDir(SOY_TRACKER_MAX_DIR, tracker->lastSaveDirBuffer);
  }
  SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
  startIndex = global_popupScroll;
  drawIndex = 0;
  drawLen = startIndex + buttonCount;
  if((uint32)drawLen > lfd.count) {
    drawLen = lfd.count;
  }
  for(i = startIndex; i < drawLen; i++) {
    strLen = SOY_stringLen(lfd.filenames[i]);
    diff = strLen - ((global_saveTextBox.rect.width-(24))/xStep);
    if(diff > 0) {
      sprintf(shortString, "[...%s]", lfd.filenames[i]+diff+5);
      curRect = SOY_button(app, shortString, alignLeft+2, alignTop+yStep*drawIndex+2, NULL);
    } else {
      curRect = SOY_button(app, lfd.filenames[i], alignLeft+2, alignTop+yStep*drawIndex+2, NULL);
    }
    SOY_rectUnsetArmable(&curRect.rect);
    SOY_buttonUpdate(app, &curRect);
    if(SOY_stringMatch(lfd.filenames[i], ".soy")) {
      SOY_drawSetPaletteTint(app, 0.5f, 0.9f, 0.6f);
    } else if(lfd.isDir[i]) {
      if(lfd.isHidden[i]) {
        SOY_drawSetPaletteTint(app, 0.7f, 0.7f, 0.7f);
      } else {
        SOY_drawSetPaletteTint(app, 0.8f, 0.9f, 0.9f);
      }
    } else if(lfd.isHidden[i]) {
        SOY_drawSetPaletteTint(app, 0.5f, 0.5f, 0.5f);
    } else {
        SOY_drawSetPaletteTint(app, 0.8f, 0.9f, 0.8f);
    }
    SOY_drawButton(app, &curRect, 2, 1);
    SOY_drawGUIPanel(app,
          curRect.rect.x+curRect.rect.width+curRect.rect.height/4+1,
          curRect.rect.y+curRect.rect.height/4+1,
          curRect.rect.height/2,
          curRect.rect.height/2);
    if(lfd.isDir[i]) {
      SOY_drawGUIPanel(app,
          curRect.rect.x+curRect.rect.width+curRect.rect.height/4-1,
          curRect.rect.y+curRect.rect.height/4-1,
          curRect.rect.height/2,
          curRect.rect.height/2);
    }
    SOY_drawResetPaletteTint(app);
    if(SOY_rectPressedL(&curRect.rect) && !global_popupLockMouse) {
      if(lfd.isDir[i] && !lfd.isHidden[i]) {
        if(SOY_stringCompare(lfd.filenames[i], "..") == 0) {
          sprintf(newDirBuf, "%s\\..\\.", tracker->lastSaveDirBuffer);
          SOY_fileDir(newDirBuf, SOY_TRACKER_MAX_DIR, tracker->lastSaveDirBuffer);       
        } else if(SOY_stringCompare(lfd.filenames[i], ".") == 0) {
          SOY_currentDir(SOY_TRACKER_MAX_DIR, tracker->lastSaveDirBuffer);
        } else {
          sprintf(newDirBuf, "%s\\%s", tracker->lastSaveDirBuffer, lfd.filenames[i]);
          sprintf(tracker->lastSaveDirBuffer, "%s", newDirBuf);
        }
      } else if (!lfd.isHidden[i]) {
        SOY_textBoxPaste(&global_saveTextBox, lfd.filenames[i]);
      }
    }
    drawIndex++;
  }

  global_popupCurScroll = SOY_mouseScroll(app);
  if(global_popupCurScroll > global_popupPrevScroll) {
    global_popupScroll++;
  } else if(global_popupCurScroll < global_popupPrevScroll) {
    global_popupScroll--;
  }
  global_popupPrevScroll = global_popupCurScroll;

  if(global_popupScroll < 0) { global_popupScroll = 0; }
  if(global_popupScroll > (int32)lfd.count-1) { global_popupScroll = (int32)lfd.count-1; }
  if(SOY_mouseLeft(app)) {
    global_popupLockMouse = TRUE;
  } else {
    global_popupLockMouse = FALSE;
  }
  isSame = FALSE;
  isFolder = FALSE;
  for(sameCheck = 0; (int32)sameCheck < (int32)lfd.count; sameCheck++) {
    if(SOY_stringCompare(SOY_textBoxGetText(&global_saveTextBox), lfd.filenames[sameCheck]) == 0) {
      isSame = TRUE;
      if(lfd.isDir[sameCheck]) {
         isFolder = TRUE;
      }
    }
  }
  noExt = FALSE;
  rightFormat = FALSE;
  hasName = FALSE; 
  if(global_saveTextBox.back > 0) {
    hasName = TRUE;
  }
  if(!SOY_stringMatch(SOY_textBoxGetText(&global_saveTextBox), ".") && sameCheck > 1) {
    noExt = TRUE;
  }
    
  if(SOY_stringMatchEnd(SOY_textBoxGetText(&global_saveTextBox), ".stt")) {
    rightFormat = TRUE; 
  }
  
  if(SOY_stringCompare(SOY_textBoxGetText(&global_saveTextBox), ".stt") == 0) {
    hasName = FALSE;
  }
  if(hasName && noExt && !isFolder) {
    SOY_drawSetColour(app, 192, 128, 0, 255);
    SOY_drawSetPaletteTint(app, 1.0f, 0.8f, 0.5f);
  } else if(hasName && rightFormat && !isFolder && !isSame) {
    SOY_drawSetColour(app, 0, 160, 0, 255);
    SOY_drawSetPaletteTint(app, 0.5f, 1.0f, 0.5f);
  } else {
    SOY_drawSetColour(app, 255, 0, 0, 255);
    SOY_drawSetPaletteTint(app, 1.0f, 0.5f, 0.5f);
  } 
  SOY_drawRectF(app, centerAlign-overwriteTextWidth, alignBot+(yStep/2)-5,
        centerAlign+overwriteTextWidth, alignBot+(yStep/2+yStep)-4);
  SOY_drawGUIRectOuter(app, centerAlign-overwriteTextWidth, alignBot+(yStep/2-5),
        centerAlign+overwriteTextWidth, alignBot+(yStep/2+yStep)-4);
  SOY_drawResetPaletteTint(app);
  SOY_drawSetColour(app, 255, 255, 255, 255);
  
  if(!hasName) {
    SOY_drawText(app,
      centerAlign-overwriteTextWidth/2,
      alignBot+yStep/2-2, "  EMPTY!  ", &app->draw.font);
  } else if(noExt) {
    if(isFolder) {
      SOY_drawText(app,
        centerAlign-overwriteTextWidth/2,
        alignBot+yStep/2-2, "DIRECTORY!", &app->draw.font);
    } else {
      SOY_drawText(app,
        centerAlign-overwriteTextWidth/2,
        alignBot+yStep/2-2, " NO EXT. !", &app->draw.font);
    }
  } else if((rightFormat || noExt) && isSame) {
    SOY_drawText(app,
      centerAlign-overwriteTextWidth/2,
      alignBot+yStep/2-2, "OVERWRITE!", &app->draw.font);
  } else if(!rightFormat) {
    SOY_drawText(app,
      centerAlign-overwriteTextWidth/2,
      alignBot+yStep/2-2, " INVALID! ", &app->draw.font);
  } else {
    SOY_drawText(app,
      centerAlign-overwriteTextWidth/2,
      alignBot+yStep/2-2, " CORRECT! ", &app->draw.font);
  }
  
  SOY_drawButton(app, &global_fileDialogButtons[0], 2, 2);
  SOY_drawButton(app, &global_fileDialogButtons[1], 2, 2);
  SOY_drawText(app, alignLeft, alignTop-yStep+4, tracker->lastSaveDirBuffer, &app->draw.font);
  if((SOY_rectReleased(&global_fileDialogButtons[1].rect) || SOY_keyPressed(app, SOY_RETURN)) &&
  (rightFormat)) {
    switch(type) {
      case(SAVE): {
        SLOG1("Saving file: \"%s\"\n", SOY_textBoxGetText(&global_saveTextBox));
        exitTrue = soy_tracker_save_file(app, tracker, SOY_textBoxGetText(&global_saveTextBox));
        break;
      }
      case(LOAD): {
        SLOG1("Loading file: \"%s\"\n", SOY_textBoxGetText(&global_saveTextBox));
        exitTrue = soy_tracker_load_file(app, tracker, SOY_textBoxGetText(&global_saveTextBox));
        break;
      }
      default: { break; }
    }
  } else if(SOY_rectReleased(&global_fileDialogButtons[0].rect) || SOY_keyPressed(app, SOY_ESC)) {
    exitTrue = TRUE;
  } else {
    exitTrue = FALSE;
  }
  if(exitTrue) {
    tracker->popupState = NOPOPUP;
  }
  return TRUE;
}

/********************************************/
/* @node                                    */
/********************************************/

real64 soy_tracker_fade_linear(real64 x) {
  return x;
}

sflag soy_tracker_play_node(soyApp* app, uint32 channel, soyTrackerNode* node) {
  soySound* soundCopy = NULL;
  if(global_samples[node->instrument].name == NULL) { SLOG("Null instrument\n"); return FALSE; }
  SOY_soundToChannel(app, global_samples[node->instrument], channel);
  soundCopy = SOY_soundGet(app, channel);
  SOY_soundStop(soundCopy);
  SOY_soundSetVolume(soundCopy, (real32)node->volume/255.0f);
  SOY_soundSetPitch(soundCopy, node->note+(node->octave*12));
  SOY_soundPlay(soundCopy);
  return TRUE;
}

/********************************************/
/* @update                                  */
/********************************************/

static sflag global_lockMousePress = FALSE;

sflag soy_tracker_update_channel(soyTrackerChannel* channel,
    soyAppHandle app, soyTrackerPattern* pattern, soyTracker* tracker, sflag playSound) {
  int32 i;
  soyRect curRect;
  int32 xStep = (app->draw.font.grid-app->draw.font.globalKerning)*app->draw.font.scale;
  int32 yStep = app->draw.font.grid*app->draw.font.scale;
  int32 yPad = yStep*channel->guiHeaderHeight;
  int32 maxH = 0;
  int32 cy = 0;
  int32 dy = 0;
  int32 nVCh = pattern->rect.width/channel->visibleLen;
  int32 startIndex = 0;
  sflag empty = channel->empty;
  soyTrackerNode* curNode = NULL;
  channel->rect.y = pattern->rect.y+2;
  channel->rect.height = pattern->rect.height-4;
  maxH = channel->rect.height-12-yPad;
  if(maxH <= yStep) { maxH = yStep; }
  dy = maxH/yStep;
  startIndex = 0;
  if(channel->enabled == FALSE) { return FALSE; }
  if(nVCh <= 0) { nVCh = 1; }
  SOY_rectMoveRel(
      &channel->rect,
      &pattern->rect,
      2+(channel->ID-(nVCh*(tracker->curChannel/nVCh)))*channel->visibleLen, 2);
  SOY_rectUpdate(app, &channel->rect);
  channel->infoStr = global_nodeString[tracker->nodeCursor];
  if(dy == 0) { startIndex = 0;
  } else { startIndex = (tracker->playCursor/dy)*dy; }
  if(playSound && !empty) {
    curNode = &channel->nodes[tracker->playCursor];
    if(SOY_bitCheck(curNode->flags, TNODE_NOTE)) {
      if(SOY_soundGet(app, channel->ID)) { 
        SOY_soundSetPitch(SOY_soundGet(app, channel->ID),
        curNode->note+(curNode->octave*12));
      }
    }
    if(SOY_bitCheck(curNode->flags, TNODE_VOLUME)) {
      if(SOY_soundGet(app, channel->ID)) {
        SOY_soundSetVolume(SOY_soundGet(app, channel->ID), (real32)curNode->volume/255.0f); }
    }

    if(SOY_bitCheck(curNode->flags, TNODE_END)) {
      SOY_soundStop(SOY_soundGet(app, channel->ID));
      return TRUE;
    }

    if(SOY_bitCheck(curNode->flags, TNODE_FADE)) {
      SOY_soundFade(SOY_soundGet(app, channel->ID), soy_tracker_fade_linear(curNode->effectParam/255.0));
    }

    if(SOY_bitCheck(curNode->flags, TNODE_INST)) {
      soy_tracker_play_node(app, channel->ID, curNode);
    }
    
    if(SOY_bitCheck(curNode->flags, TNODE_EFFECT)) {
      switch(curNode->effect) {
        case(0x0A): {
          tracker->playCursor = curNode->effectParam;
          tracker->loopCounter++;
          break;
        }
        default: { break; }
      }

      if((curNode->effect-(curNode->effect&0xF0)) >= 0x0C && (curNode->effect-(curNode->effect&0xF0)) < 0x0D) {
        if(tracker->loopCounter >= (curNode->effect>>4)) {
          tracker->playCursor = curNode->effectParam;
          tracker->loopCounter = 0;
        }
      }

    }
  }
  
  for(i = 0; i < CHANNEL_BUTTONS; i++) {
    SOY_buttonMove(&channel->buttons[i],
        channel->rect.x+4+(i)*(SPR_MICRO_SIZE+2),
        channel->rect.y+yPad-1);
    if(tracker->popupState == NOPOPUP) {
      SOY_buttonUpdate(app, &channel->buttons[i]);
    }
    if(SOY_rectHover(&channel->buttons[i].rect) && channel->buttons[i].rect.label) {
      channel->infoStr = channel->buttons[i].rect.label;
    }
  }
  
  if(SOY_rectPressedL(&channel->rect) && tracker->popupState == NOPOPUP &&
      channel->rect.x+channel->rect.width < pattern->rect.x+pattern->rect.width-2) {
      tracker->curChannel = channel->ID;
  }

  for(i = startIndex; i < startIndex+dy; i++) {
    if(i >= pattern->channelLen) { return TRUE; }
    curNode = &channel->nodes[i];
    curRect = SOY_rectSimple("node", 0, 0, channel->rect.width-4, yStep);
    SOY_rectUnsetArmable(&curRect);
    SOY_rectMoveRel(&curRect, &channel->rect, 2, yPad+8+cy);
    SOY_rectUpdate(app, &curRect);
    if(!global_lockInput && tracker->popupState == NOPOPUP) {
      if(SOY_rectPressedL(&curRect) && !global_lockMousePress) {
        tracker->editCursor = i;
        if(SOY_mouseX(app) > curRect.x-2 + xStep*global_nodeSteps[8]) {
          tracker->nodeCursor = 8;
        } else if(SOY_mouseX(app) > curRect.x-2 + xStep*global_nodeSteps[6]) {
          tracker->nodeCursor = 6;
        } else if(SOY_mouseX(app) > curRect.x-2 + xStep*global_nodeSteps[4]) {
          tracker->nodeCursor = 4;
        } else if(SOY_mouseX(app) > curRect.x-2 + xStep*global_nodeSteps[2]) {
          tracker->nodeCursor = 2;
        } else if(SOY_mouseX(app) > curRect.x-2 + xStep*global_nodeSteps[1]) {
          tracker->nodeCursor = 1;
        } else {
          tracker->nodeCursor = 0;
        }
        global_lockMousePress = TRUE;
      }
      if(!SOY_rectPressedL(&curRect)) {
        global_lockMousePress = FALSE;
      }
    }
    cy+=yStep;
  }
  return TRUE;
}

sflag soy_tracker_node_inherit(soyTrackerNode* parent, soyTrackerNode* child) {
  if(parent == NULL || child == NULL) { return FALSE; }
  child->flags = parent->flags;
  child->note = parent->note;
  child->octave = parent->octave;
  child->instrument = parent->instrument;
  child->volume = parent->volume;
  child->effect = parent->effect;
  child->effectParam = parent->effectParam;
  return TRUE;
}

sflag soy_tracker_check_if_node_empty(soyTrackerNode* curNode) {
  if(
    SOY_bitCheck(curNode->flags, TNODE_INST)    ||
    SOY_bitCheck(curNode->flags, TNODE_NOTE)    ||
    SOY_bitCheck(curNode->flags, TNODE_VOLUME)  ||
    SOY_bitCheck(curNode->flags, TNODE_EFFECT)  ||
    SOY_bitCheck(curNode->flags, TNODE_EFFPAR)  ||
    SOY_bitCheck(curNode->flags, TNODE_END)     ||
    SOY_bitCheck(curNode->flags, TNODE_FADE)) { 
    SOY_bitClear(curNode->flags, TNODE_EMPTY);
  } else {
    SOY_bitSet(curNode->flags, TNODE_EMPTY);
  }
  return TRUE;
}

sflag soy_tracker_parse_node_input(soyTrackerNode* curNode,
    soyTrackerPattern* pattern, soyTracker* tracker, uint32 lastKeyN) {
  uint32 lastKey;
  if(lastKeyN < 0x40) {
        lastKey = lastKeyN - 0x30;
      } else if( lastKeyN < 0x60) {
        lastKey = lastKeyN - 0x41+10;
      } else {
        lastKey = lastKeyN - 0x60;
      }  
  switch(tracker->nodeCursor) {
      case(0): {
        if((global_keyMap[lastKeyN]) == 0) { break; }
        soy_tracker_node_inherit(&pattern->copyNode, curNode);
        curNode->note = (global_keyMap[lastKeyN]-4)%12;
        curNode->octave = (global_keyMap[lastKeyN]-4)/12 - 3 + tracker->globalOctave;
        pattern->copyNode.note = curNode->note;
        pattern->copyNode.octave = curNode->octave;
        pattern->copyNode.flags = curNode->flags;
        SOY_bitSet(curNode->flags, TNODE_NOTE);
        tracker->editCursor++;
        break;
      }
      case(1): {
        if(lastKey > 9) { break; }
        soy_tracker_node_inherit(&pattern->copyNode, curNode);
        curNode->octave = lastKey;
        pattern->copyNode.octave = curNode->octave;
        pattern->copyNode.flags = curNode->flags;
        SOY_bitSet(curNode->flags, TNODE_NOTE);
        tracker->editCursor++;
        break;
      }
      case(2): {
        if(SOY_keyIsHex(lastKeyN)) {
          curNode->instrument |= 0xF0;
          curNode->instrument ^= 0xF0;
          curNode->instrument += (lastKey)<<4;
          pattern->copyNode.instrument = curNode->instrument;
          SOY_bitSet(curNode->flags, TNODE_INST);
          SOY_bitSet(pattern->copyNode.flags, TNODE_INST);
          tracker->nodeCursor++;
        }
        break;
      }
      case(3): {
        if(SOY_keyIsHex(lastKeyN)) {
          curNode->instrument |= 0x0F;
          curNode->instrument ^= 0x0F;
          curNode->instrument += (lastKey);
          pattern->copyNode.instrument = curNode->instrument;
          SOY_bitSet(curNode->flags, TNODE_INST);
          SOY_bitSet(pattern->copyNode.flags, TNODE_INST);
          tracker->editCursor++;
          tracker->nodeCursor--;
        }
        break;
      }
      case(4): {
        if(SOY_keyIsHex(lastKeyN)) {
          curNode->volume |= 0xF0;
          curNode->volume ^= 0xF0;
          curNode->volume += (lastKey<<4);
          pattern->copyNode.volume = curNode->volume;
          SOY_bitSet(curNode->flags, TNODE_VOLUME);
          SOY_bitSet(pattern->copyNode.flags, TNODE_VOLUME);
          tracker->nodeCursor++;
        }
        break;
      }
      case(5): {
      if(SOY_keyIsHex(lastKeyN)) {
          curNode->volume |= 0x0F;
          curNode->volume ^= 0x0F;
          curNode->volume += (lastKey);
          pattern->copyNode.volume = curNode->volume;
          SOY_bitSet(curNode->flags, TNODE_VOLUME);
          SOY_bitSet(pattern->copyNode.flags, TNODE_VOLUME);
          tracker->editCursor++;
          tracker->nodeCursor--;
        }
        break;
      }
      case(6): {
        if(SOY_keyIsHex(lastKeyN)) {
          curNode->effect |= 0xF0;
          curNode->effect ^= 0xF0;
          curNode->effect += (lastKey<<4);
          pattern->copyNode.effect = curNode->effect;
          SOY_bitSet(curNode->flags, TNODE_EFFECT);
          SOY_bitSet(pattern->copyNode.flags, TNODE_EFFECT);
          tracker->nodeCursor++;
        }
        break;
      }
      case(7): {
        if(SOY_keyIsHex(lastKeyN)) {
          curNode->effect |= 0x0F;
          curNode->effect ^= 0x0F;
          curNode->effect += (lastKey);
          pattern->copyNode.effect = curNode->effect;
          SOY_bitSet(curNode->flags, TNODE_EFFECT);
          SOY_bitSet(pattern->copyNode.flags, TNODE_EFFECT);
          tracker->editCursor++;
          tracker->nodeCursor--;
        }
        break;
      }
      case(8): {
        if(SOY_keyIsHex(lastKeyN)) {
          curNode->effectParam |= 0xF0;
          curNode->effectParam ^= 0xF0;
          curNode->effectParam += (lastKey<<4);
          pattern->copyNode.effectParam = curNode->effectParam;
          SOY_bitSet(curNode->flags, TNODE_EFFPAR);
          SOY_bitSet(pattern->copyNode.flags, TNODE_EFFPAR);
          tracker->nodeCursor++;
        }
        break;
      }
      case(9): {
        if(SOY_keyIsHex(lastKeyN)) {
          curNode->effectParam |= 0x0F;
          curNode->effectParam ^= 0x0F;
          curNode->effectParam += (lastKey);
          pattern->copyNode.effectParam = curNode->effectParam;
          SOY_bitSet(curNode->flags, TNODE_EFFPAR);
          SOY_bitSet(pattern->copyNode.flags, TNODE_EFFPAR);
          tracker->editCursor++;
          tracker->nodeCursor--;
        }
        break;
      }
      default: { break; }
    }
  return TRUE;
}

sflag soy_tracker_is_in_selection(nodeSelection selection, int32 channel, int32 node) {
  return
  (((channel >= selection.channelStart && channel <= selection.channelEnd)) ||
  ((channel <= selection.channelStart && channel >= selection.channelEnd))) &&
  (((node >= selection.cursorStart && node <= selection.cursorEnd)) ||
  ((node <= selection.cursorStart && node >= selection.cursorEnd)));
}

sflag soy_tracker_copy_to_clipboard(trackerClipboard* cb, soyTracker* tracker) {
  int32 i, j;
  int32 nS, cS;
  soyTrackerChannel* curChan;
  soyTrackerNode* curNode;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  soyTrackerPattern* curPat = &patterns[tracker->curPattern];
  nodeSelection sel = curPat->selection;
  int32 nodeCount = soy_abs(sel.cursorEnd - sel.cursorStart);
  int32 chanCount = soy_abs(sel.channelEnd - sel.channelStart);
  int32 chanStart = soy_min(sel.channelStart, sel.channelEnd);
  int32 nodeStart = soy_min(sel.cursorStart, sel.cursorEnd);
  cb->selection = sel;
  cb->dataCount = 0;
  cb->nodeCount = nodeCount+1;
  cb->chanCount = chanCount+1;
  cS = chanStart;
  for(i = 0; i < chanCount+1; i++) {
    curChan = &curPat->channels[cS++];
    nS = nodeStart; 
    for(j = 0; j < nodeCount+1; j++) {
      curNode = &curChan->nodes[nS++];
      if((uint32)j+i*(nodeCount+1) < (uint32)tracker->clipboardMemory.size/sizeof(soyTrackerNode*)) {
        cb->copyData[j+i*(nodeCount+1)] = curNode;
        cb->dataCount++;
      }
    }
  }
  return TRUE;
}

sflag soy_tracker_is_note_key(uint32 key) {
  return global_keyMap[key];
}

sflag soy_tracker_update_clipboard(soyAppHandle app, trackerClipboard* cb, soyTracker* tracker) {
  uint32 i, j;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  soyTrackerPattern* curPat = &patterns[tracker->curPattern];
  soyTrackerNode* curNode = &curPat->channels[tracker->curChannel].nodes[tracker->editCursor];
  if(!global_lockInput) {
    if(SOY_keyIsAlphanumeric(SOY_keyLast(app)) && (soy_tracker_is_note_key(SOY_keyLast(app)))
      && !tracker->globalShiftInput) {
      for(i = 0; i < cb->dataCount; i++)  {
        curNode = cb->copyData[i];
        soy_tracker_parse_node_input(curNode, curPat, tracker, SOY_keyLast(app));
        soy_tracker_check_if_node_empty(curNode);
      }
      tracker->editCursor = (curNode->number+1)%curPat->channelLen;
      if(SOY_bitCheck(curNode->flags, TNODE_INST) && SOY_bitCheck(curNode->flags, TNODE_NOTE)) {
        soy_tracker_play_node(app, curPat->channels[tracker->curChannel].ID, curNode);
      }
      curPat->channels[tracker->curChannel].empty = TRUE;
      for(j = 0; j < (uint32)curPat->channelLen; j++) {
        if(!SOY_bitCheck(curPat->channels[tracker->curChannel].nodes[j].flags, TNODE_EMPTY)) {
          curPat->channels[tracker->curChannel].empty = FALSE;
        }
      }
    }
  }
  return TRUE;
}

sflag soy_tracker_set_clipboard(soyTracker* tracker, sflag parseNode) {
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  soyTrackerPattern* curPat = &patterns[tracker->curPattern];
  if(!tracker->globalShiftInput) {
    if(parseNode) {
      soy_tracker_update_clipboard(tracker->appHandle, &tracker->clipboard, tracker);
    }
    soy_tracker_reset_selection(&curPat->selection,
      tracker->editCursor,
      tracker->nodeCursor,
      tracker->curChannel,
      tracker->curPattern);
    soy_tracker_copy_to_clipboard(&tracker->clipboard, tracker);
  }
  return TRUE;
}

sflag soy_tracker_update_selection(soyAppHandle app,
    nodeSelection* selection, soyTracker* tracker) {
  if(SOY_keyPressed(app, SOY_SHIFT)) {
    if(!selection->lockKey) {
      selection->cursorStart = tracker->editCursor;
      selection->paramStart = tracker->nodeCursor;
      selection->channelStart = tracker->curChannel;
      selection->lockKey = TRUE;
    }
  } else {
    if(selection->lockKey || selection->lockMouse) {
      soy_tracker_copy_to_clipboard(&tracker->clipboard, tracker);
    }
    selection->lockKey = FALSE;
    selection->lockMouse = FALSE;
    soy_tracker_reset_selection(selection,
      tracker->editCursor,
      tracker->nodeCursor,
      tracker->curChannel,
      tracker->curPattern);
  }
  if(SOY_mouseLeft(app)) {
    selection->lockMouse = TRUE;
  } else {
    selection->lockMouse = FALSE;
  }
  selection->cursorEnd = tracker->editCursor;
  selection->paramEnd = tracker->nodeCursor;
  selection->channelEnd = tracker->curChannel;
  return TRUE;
}

sflag soy_tracker_update_pattern(soyAppHandle app, soyTrackerPattern* pattern, sflag playSound,
    soyTracker* tracker) {
  int32 i;
  SOY_rectResizeRel(&pattern->rect, &tracker->rect, 2,
      tracker->toolbarRect.height+4, 4, tracker->toolbarRect.height+6);
  SOY_rectUpdate(app, &pattern->rect);
  pattern->bpm = global_bpm;
  if(SOY_keyPressedAny(app) && SOY_keyIsAlphanumeric(SOY_keyLast(app)) &&
      !tracker->globalShiftInput) {
    soy_tracker_set_clipboard(tracker, TRUE);
  }
  for(i = 0; i < pattern->channelCount; i++) {
    if(!soy_tracker_update_channel(&pattern->channels[i], app, pattern, tracker, playSound)) {
      SLOG("Error!\n");
    }
  }
  pattern->used = FALSE;
  for(i = 0; i < tracker->trackLen; i++) {
    if((int32)tracker->patternIndices[i] == (int32)pattern->ID) {
      pattern->used = TRUE;
    }
  }
  return TRUE;
}

sflag soy_tracker_apply_sizing(soyAppHandle app, soyTracker* tracker) {
  sflag firstStage = FALSE;
  sflag secondStage = FALSE;
  sflag thirdStage = FALSE;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  uint32 chanWidth = patterns[tracker->curPattern].channels[0].rect.width+4;
  uint32 buttonHeight = tracker->buttons[0].rect.height;
  uint32 totalPad = 6;
  uint32 stagePad = buttonHeight+2;
  SOY_rectResizeRel(&tracker->rect, SOY_windowRect(app), 16, 32, 32, 46);
  SOY_rectMoveRel(&tracker->toolbarRect, &tracker->rect, 2, 2);
  SOY_rectResizeRelWidth(&tracker->toolbarRect, &tracker->rect, 4);
  firstStage = ((int32)tracker->toolbarRect.width < (int32)chanWidth*8);
  secondStage = ((int32)tracker->toolbarRect.width < (int32)chanWidth*3); 
  thirdStage = ((int32)tracker->toolbarRect.width < (int32)chanWidth*2);
  if(thirdStage) {
    SOY_rectResizeHeight(&tracker->toolbarRect, 8*(stagePad)+totalPad);
  } else if(secondStage) {
    SOY_rectResizeHeight(&tracker->toolbarRect, 7*(stagePad)+totalPad);
  } else if(firstStage) {
    SOY_rectResizeHeight(&tracker->toolbarRect, 6*(stagePad)+totalPad);
  } else {
    SOY_rectResizeHeight(&tracker->toolbarRect, 5*(stagePad)+totalPad);
  }
  
  SOY_rectMoveRel(&tracker->infoRect, &tracker->toolbarRect, 2, 2);
  SOY_rectResizeWidth(&tracker->infoRect, tracker->toolbarRect.width-4);
  SOY_rectResizeHeight(&tracker->infoRect, stagePad+12);

  SOY_rectMoveRel(&tracker->controlRect, &tracker->toolbarRect, 2, tracker->infoRect.height+4);
  SOY_rectResizeWidth(&tracker->controlRect, tracker->toolbarRect.width-4);
  SOY_rectResizeHeight(&tracker->controlRect,
      (thirdStage) ? stagePad*4+2 : (secondStage) ? stagePad*3+2 : (firstStage) ? stagePad*2+2 : stagePad+2);
  
  SOY_rectMoveRel(&tracker->trackRect, &tracker->toolbarRect, 2,
      tracker->controlRect.height+tracker->infoRect.height+6);
  SOY_rectResizeWidth(&tracker->trackRect,
      tracker->toolbarRect.width-4);
  SOY_rectResizeHeight(&tracker->trackRect, stagePad*2+2);
  
  SOY_rectUpdate(app, &tracker->rect);
  SOY_rectUpdate(app, &tracker->toolbarRect);
  SOY_rectUpdate(app, &tracker->controlRect);
  SOY_rectUpdate(app, &tracker->trackRect);
  SOY_rectUpdate(app, &tracker->infoRect);
  return TRUE;
}

sflag soy_tracker_update(soyAppHandle app, soyTracker* tracker) {
  sflag playSound = FALSE; 
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  soy_tracker_apply_sizing(app, tracker);
  if(SOY_keyPressed(app, SOY_SHIFT)) {
    tracker->globalShiftInput = TRUE;
  } else {
    tracker->globalShiftInput = FALSE;
  }
  if(SOY_keyPressed(app, SOY_CONTROL)) {
    tracker->globalControlInput = TRUE;
  } else {
    tracker->globalControlInput = FALSE;
  }
  if(tracker->playing) {
    tracker->timer+=SOY_appTimestep(app);
    if(tracker->timer > 60.0f/(patterns[tracker->curPattern].bpm*4)) {
      tracker->timer = 0.0f;
      tracker->playCursor++;
      if((int32)tracker->playCursor >= (int32)patterns[tracker->curPattern].channelLen) {
        tracker->trackCursor++;
        tracker->trackCursor%=tracker->trackLen;
        tracker->curPattern = tracker->patternIndices[tracker->trackCursor];
      }
      tracker->playCursor%=patterns[tracker->curPattern].channelLen;
      playSound = TRUE;
    }
  }
  soy_tracker_update_selection(app, &patterns[tracker->curPattern].selection, tracker);
  soy_tracker_update_pattern(app, &patterns[tracker->curPattern], playSound, tracker);
  return TRUE;
}

sflag soy_tracker_visualizer_update(soyAppHandle app, soyTrackerChannel* channel) {
  if(channel == NULL || app == NULL) { return FALSE; }
  return TRUE;
}

int32 global_controlButtonScroll = 0;
int32 global_controlButtonScrollCur = 0;
int32 global_controlButtonScrollPrev = 0;

sflag soy_tracker_update_controls(soyAppHandle app, soyTracker* tracker) {
  int32 i;
  uint32 drawWidth = 0;
  uint32 drawHeight = 0;
  soyButton* curB;
  
  for(i = 0; i < SOY_TRACKER_BUTTONS; i++) {
    if(tracker->buttons[i+global_controlButtonScroll].type != SOY_BUTTON_NULL) {
      curB = &tracker->buttons[i+global_controlButtonScroll];
      if(global_fontChanged) {
        SOY_buttonResize(curB, curB->rect.x, curB->rect.y,
          SOY_drawTextLen(curB->rect.label, &app->draw.font), app->draw.font.grid*app->draw.font.scale+2);
      }

      if((int32)drawWidth > (int32)tracker->controlRect.width-(int32)curB->rect.width-4) {
        drawWidth = 0;
        drawHeight += curB->rect.height+2;
      }
      SOY_buttonMove(curB,
          tracker->controlRect.x+2+drawWidth, 
          tracker->controlRect.y+2+drawHeight);
      if((int32)drawHeight+(int32)curB->rect.height < (int32)tracker->controlRect.height) {
        SOY_buttonUpdate(app, curB);        
        drawWidth+= curB->rect.width+2;
      }
    }
  }

  if(tracker->playing) {
    SOY_rectSelect(&tracker->buttons[0].rect);
  }

  if(SOY_rectHover(&tracker->controlRect) && SOY_mouseScrolled(app)) {
    global_controlButtonScrollCur = SOY_mouseScroll(app);
    if(global_controlButtonScrollCur > global_controlButtonScrollPrev) {
      global_controlButtonScroll++;
    } else if(global_controlButtonScrollCur < global_controlButtonScrollPrev) {
      global_controlButtonScroll--;
    }
    global_controlButtonScrollPrev = global_controlButtonScrollCur;
    if(global_controlButtonScroll < 0) { global_controlButtonScroll = 0; }
    if(global_controlButtonScroll > 12) {
      global_controlButtonScroll = 12;
    }
  }
  return TRUE;
}

/********************************************/
/* @resize                                  */
/********************************************/

sflag soy_tracker_resize(soyAppHandle app, soyTracker* tracker) {
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  int32 i = 0;
  soy_tracker_apply_sizing(app, tracker);
  for(i = 0; i < tracker->patternCount; i++) {
    soy_tracker_update_pattern(app, &patterns[i], FALSE, tracker);
  }
  return TRUE;
}

/********************************************/
/* @commands                                */
/********************************************/

sflag commandModeSwitch(soyCommandHandle command, void* args) {
  int8* arg = (int8*)args;
  if(global_lockInput) { return FALSE; }
  switch(arg[0]) {
    case('e'): {
      global_tracker.state = EDIT;
      break;
    }
    case('s'): {
      global_tracker.state = INSTRUMENT;
      break;
    }
    case('c'): {
      global_tracker.state = CONFIG;
      break;
    }   
    default: { break; }
  }
  return command->enabled;
}

sflag commandPlay(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  if(global_lockInput) { return FALSE; }
  tracker->playing = !tracker->playing;
  return command->enabled;
}

sflag commandStop(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  if(global_lockInput) { return FALSE; }
  tracker->playing = FALSE;
  tracker->playCursor = 0;
  tracker->timer = 0;
  return command->enabled;
}

sflag commandRedo(soyCommandHandle command, void* args) {
  if(global_lockInput || args == NULL) { return FALSE; }
  return command->enabled;
}

sflag commandUndo(soyCommandHandle command, void* args) {
  if(global_lockInput || args == NULL) { return FALSE; }
  return command->enabled;
}

sflag commandSave(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  tracker->popupState = SAVE;
  return command->enabled;
}

sflag commandLoad(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  tracker->popupState = LOAD;
  return command->enabled;
}

sflag commandBgToggle(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  tracker->showBg = !tracker->showBg;
  return command->enabled;
}

sflag commandChannelInc(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  if(tracker->curChannel < patterns->channelCount-1) {
    tracker->curChannel++;
  } 
  return command->enabled;
}

sflag commandChannelDec(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  if(tracker->curChannel > 0) {
    tracker->curChannel--;
  } 
  return command->enabled;
}

sflag commandTrackInc(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  if(tracker->trackCursor < tracker->trackLen-1) {
    tracker->trackCursor++;
    tracker->curPattern = tracker->patternIndices[tracker->trackCursor];
    soy_tracker_update_pattern(tracker->appHandle, &patterns[tracker->curPattern], FALSE, tracker);
  } 
  return command->enabled;
}

sflag commandTrackDec(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  if(tracker->trackCursor > 0) {
    tracker->trackCursor--;
    tracker->curPattern = tracker->patternIndices[tracker->trackCursor];
    soy_tracker_update_pattern(tracker->appHandle, &patterns[tracker->curPattern], FALSE, tracker);
  } 
  return command->enabled;
}

sflag commandPatternInc(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  if(tracker->curPattern < tracker->patternCount-1) {
    tracker->curPattern++;
    tracker->patternIndices[tracker->trackCursor] = tracker->curPattern;
    soy_tracker_update_pattern(tracker->appHandle, &patterns[tracker->curPattern], FALSE, tracker);
  } 
  return command->enabled;
}

sflag commandPatternDec(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  if(tracker->curPattern > 0) {
    tracker->curPattern--;
    tracker->patternIndices[tracker->trackCursor] = tracker->curPattern;
    soy_tracker_update_pattern(tracker->appHandle, &patterns[tracker->curPattern], FALSE, tracker);
  } 
  return command->enabled;
}

sflag commandDebugDraw(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  tracker->appHandle->draw.drawParams.debugDraw = 
  !tracker->appHandle->draw.drawParams.debugDraw;
  return command->enabled;
}

sflag commandSpriteGui(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  tracker->appHandle->draw.spriteGUI = 
  !tracker->appHandle->draw.spriteGUI;
  return command->enabled;
}

static uint32 global_selectedFont = 0;
sflag commandFontSelect(soyCommandHandle command, void* args) {
  int32 i;
  soyTracker* tr = (soyTracker*)args;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tr->patternMemory);
  if(tr == NULL) { return FALSE; }
  global_selectedFont++;
  SOY_fontSelect(tr->appHandle, global_selectedFont%3);
  global_fontChanged = TRUE;
  for(i = 0; i < tr->patternCount; i++) {
    soy_tracker_pattern_resize(&patterns[i], tr->appHandle, tr);
  }
  return command->enabled;
}

sflag commandCursor(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  int32 curChanId = tracker->curChannel;
  soyTrackerChannel* curChannel = &patterns->channels[tracker->curChannel];
  if(global_lockInput) { return FALSE; }
  switch(command->keys[0]) {
    case(SOY_LEFT): {
      tracker->nodeCursor--;
      break;
    }
    case(SOY_RIGHT): {
      tracker->nodeCursor++;
      break;
    }
    case(SOY_UP): {
      if(tracker->globalControlInput) {
        tracker->editCursor--;
        while(tracker->editCursor%curChannel->timescale) {
          tracker->editCursor--;
        } 
      } else {
        tracker->editCursor--;
      }
      break;
    }
    case(SOY_DOWN): {
      if(tracker->globalControlInput) {
        tracker->editCursor++;
        while(tracker->editCursor%curChannel->timescale) {
          tracker->editCursor++;
        }
      } else {
        tracker->editCursor++;
      }
      break;
    }
    case(SOY_PGUP): {
      if(!tracker->globalControlInput) {                  
        tracker->editCursor = 0;
      }
      break;
    }
    case(SOY_PGDOWN): {
      if(!tracker->globalControlInput) {                  
        tracker->editCursor = patterns[tracker->curPattern].channelLen-1;
      }
      break;
    }
    default: { break; }
  }

  if(tracker->nodeCursor > 9) {
    if(curChanId < patterns[tracker->curPattern].channelCount-1) {
      tracker->nodeCursor = 0;
      curChanId++;
    }
  }
  if(tracker->nodeCursor < 0) {
    if(curChanId > 0) {
      tracker->nodeCursor = 9;
      curChanId--;
    }
  }
  tracker->curChannel = curChanId;
  if(tracker->nodeCursor < 0) { tracker->nodeCursor = 0; }
  if(tracker->nodeCursor > 9) { tracker->nodeCursor = 9; }
  tracker->editCursor%=patterns[tracker->curPattern].channelLen;
  if(!tracker->globalShiftInput) {
    soy_tracker_set_clipboard(tracker, FALSE);
  }
  return command->enabled;
}

sflag commandNodeInput(soyCommandHandle command, void* args) {
  soyTracker* tracker = (soyTracker*)args;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  soyTrackerPattern* curPat = &patterns[tracker->curPattern];
  soyTrackerNode* curNode = &curPat->channels[tracker->curChannel].nodes[tracker->editCursor];
  if(global_lockInput) { return FALSE; }
  if((tracker->popupState != NOPOPUP)) { return FALSE; }
  switch(command->keys[0]) {
    case(SOY_DELETE): {
      switch(tracker->nodeCursor) {
        case(0): { SOY_bitClear(curNode->flags, TNODE_NOTE); break; }
        case(1): { SOY_bitClear(curNode->flags, TNODE_NOTE); break; }
        case(2): { SOY_bitClear(curNode->flags, TNODE_INST); break; }
        case(3): { SOY_bitClear(curNode->flags, TNODE_INST); break; }
        case(4): { SOY_bitClear(curNode->flags, TNODE_VOLUME); break; }
        case(5): { SOY_bitClear(curNode->flags, TNODE_VOLUME); break; }
        case(6): { SOY_bitClear(curNode->flags, TNODE_EFFECT); break; }
        case(7): { SOY_bitClear(curNode->flags, TNODE_EFFECT); break; }
        case(8): { SOY_bitClear(curNode->flags, TNODE_EFFPAR); break; }
        case(9): { SOY_bitClear(curNode->flags, TNODE_EFFPAR); break; }
        default: { break; }
      }
      
      if(!tracker->globalControlInput) {
        tracker->editCursor++;
      }

      break;
    }
    case(SOY_END): {
      if(tracker->globalShiftInput) {
        soy_tracker_node_fade(curNode);
      } else {
        soy_tracker_node_end(curNode);
      }
      tracker->editCursor++;
      break;
    }
    case(SOY_BACKSPACE): {
      if(!tracker->globalShiftInput) {
        tracker->editCursor--;
      }
      if(tracker->globalShiftInput) {
        tracker->editCursor++;
      }
      soy_tracker_node_clear(curNode);
      break;
    }
    case(SOY_RETURN): {
      soy_tracker_play_node(tracker->appHandle, curPat->channels[tracker->curChannel].ID, curNode);
      if(tracker->globalShiftInput) {
        tracker->editCursor++;
      }
      break;
    }
    default: { break; }
  }
  tracker->editCursor%=patterns[tracker->curPattern].channelLen;
  soy_tracker_check_if_node_empty(curNode);
  soy_tracker_set_clipboard(tracker, FALSE);
  return TRUE;
}

/********************************************/
/* @init                                    */
/********************************************/

sflag soy_tracker_init(soyApp* app, soyTracker* tracker) {
  soyTrackerPattern* patterns;
  uint32 btns;
  uint32 i;
  tracker->emptyGlyph = '-';
  tracker->state = EDIT;
  tracker->curPattern = 0;
  tracker->playing = FALSE;
  tracker->playCursor = 0;
  tracker->editCursor = 0;
  tracker->nodeCursor = 0;
  tracker->timer = 0.0f;
  tracker->appHandle = app;
  tracker->globalOctave = 3;
  tracker->globalShiftInput = FALSE;
  tracker->globalControlInput = FALSE;
  tracker->popupState = NOPOPUP;
  tracker->patternCount = 0;
  tracker->loopCounter = 0;
  tracker->loopState = 0;
  tracker->rect = SOY_rectSimple("TR_k", 16, 32, app->frameBuffer.width-16, app->frameBuffer.height-42);
  tracker->showBg = FALSE;
  SOY_memoryEnslave(app, &tracker->patternMemory, "trackerMem",  0xFF*sizeof(soyTrackerPattern));
  SOY_memoryEnslave(app, &tracker->clipboardMemory, "clipboardMem",  Megabytes(8));
  SOY_currentDir(SOY_TRACKER_MAX_DIR-1, tracker->lastSaveDirBuffer);
  soy_tracker_init_clipboard(&tracker->clipboardMemory, &tracker->clipboard);
  soy_tracker_reset_clipboard(&tracker->clipboard, tracker);
  patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  
  if(!soy_tracker_load_state(tracker, "tracker_state", 0)) {
    for(i = 0; i < tracker->patternMemory.size/sizeof(soyTrackerPattern); i++) {
      soy_tracker_pattern_init(&patterns[i], app, tracker, i);
      tracker->patternCount++;
    }
  }
  
  SOY_windowSetMin(app, (patterns[0].channels[0].rect.width)+32+10, 350);
  tracker->trackLen = 8;
  tracker->trackCursor = 0;
  tracker->toolbarRect = SOY_rectSimple("Toolbar", 0, 0, tracker->rect.width, 100);
  tracker->controlRect = SOY_rectSimple("Control", 0, 0, 100, tracker->toolbarRect.height);
  tracker->trackRect = SOY_rectSimple("Track", 0, 0, 100, tracker->toolbarRect.height);
  tracker->infoRect = SOY_rectSimple("Info", 0, 0, 100, tracker->toolbarRect.height);
  for(i = 0; i < SOY_TRACKER_MAX_PATS; i++) {
    tracker->patternIndices[i] = 0;
  }
  for(i = 0; i < SOY_TRACKER_BUTTONS; i++) {
    tracker->buttons[i].type = SOY_BUTTON_NULL;
  }
  SOY_commandAdd(app, commandCursor, tracker, SOY_LEFT, TRUE, 14, 2, "TR_left");
  SOY_commandAdd(app, commandCursor, tracker, SOY_RIGHT, TRUE, 14, 2, "TR_right");
  SOY_commandAdd(app, commandCursor, tracker, SOY_UP, TRUE, 14, 2, "TR_up");
  SOY_commandAdd(app, commandCursor, tracker, SOY_DOWN, TRUE, 14, 2, "TR_down");
  SOY_commandAdd(app, commandCursor, tracker, SOY_PGUP, TRUE, 0, 0, "TR_pageup");
  SOY_commandAdd(app, commandCursor, tracker, SOY_PGDOWN, TRUE, 0, 0, "TR_pagedown");
  SOY_commandAdd(app, commandModeSwitch, "t", SOY_TAB, TRUE, 0, 0, "TR_tab");
  SOY_commandAdd(app, commandNodeInput, tracker, SOY_BACKSPACE, TRUE, 14, 2, "TR_bspc");
  SOY_commandAdd(app, commandNodeInput, tracker, SOY_DELETE, TRUE, 14, 2, "TR_del");
  SOY_commandAdd(app, commandNodeInput, tracker, SOY_END, TRUE, 14, 2, "TR_end");
  SOY_commandAdd(app, commandNodeInput, tracker, SOY_RETURN, TRUE, 14, 2, "TR_ret");
  SOY_commandAdd(app, commandNodeInput, tracker, SOY_RETURN, TRUE, 0, 0, "TR_confpopup");
  SOY_commandAdd(app, commandNodeInput, tracker, SOY_ESC, TRUE, 0, 0, "TR_cancelpopup");
  SOY_commandAddDuo(app, commandFontSelect, tracker, SOY_CONTROL, SOY_F5, TRUE, 0, 0, "TR_fontselect");
  SOY_commandAddDuo(app, commandBgToggle, tracker, SOY_CONTROL, SOY_F6, TRUE, 0, 0, "TR_togglebg");
  SOY_commandAddDuo(app, commandSpriteGui, tracker, SOY_CONTROL, SOY_F7, TRUE, 0, 0, "TR_spritegui");
  SOY_commandAddDuo(app, commandDebugDraw, tracker, SOY_CONTROL, SOY_F8, TRUE, 0, 0, "TR_debugdraw");
  
  btns = 0;
  tracker->buttons[btns++] = SOY_buttonBlank(app, " Play ",
    SOY_commandAdd(app, commandPlay, tracker, SOY_SPACE, TRUE, 0, 0, "TR_play"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, " Stop ",
    SOY_commandAddDuo(app, commandStop, tracker, SOY_CONTROL, SOY_SPACE, TRUE, 0, 0, "TR_stop"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, " Edit ", 
    SOY_commandAddDuo(app, commandModeSwitch, "e", SOY_CONTROL, SOY_I, TRUE, 0, 0, "TR_edit"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, "Instr.", 
    SOY_commandAddDuo(app, commandModeSwitch, "s", SOY_CONTROL, SOY_O, TRUE, 0, 0, "TR_instr"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, "Config", 
    SOY_commandAddDuo(app, commandModeSwitch, "c", SOY_CONTROL, SOY_P, TRUE, 0, 0, "TR_config"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, " Undo ", 
    SOY_commandAddDuo(app, commandRedo, tracker, SOY_CONTROL, SOY_Y, TRUE, 0, 0, "TR_undo"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, " Redo ", 
    SOY_commandAddDuo(app, commandUndo, tracker, SOY_CONTROL, SOY_Z, TRUE, 0, 0, "TR_redo"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, " Save ", 
    SOY_commandAddDuo(app, commandSave, tracker, SOY_CONTROL, SOY_S, TRUE, 0, 0, "TR_save"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, " Load ", 
    SOY_commandAddDuo(app, commandLoad, tracker, SOY_CONTROL, SOY_L, TRUE, 0, 0, "TR_load"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, "Chan--", 
    SOY_commandAddDuo(app, commandChannelDec, tracker, SOY_CONTROL, SOY_LEFT, TRUE, 14, 2, "TR_chandec"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, "Chan++", 
    SOY_commandAddDuo(app, commandChannelInc, tracker, SOY_CONTROL, SOY_RIGHT, TRUE, 14, 2, "TR_chaninc"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, "Patt--", 
    SOY_commandAddDuo(app, commandPatternDec, tracker, SOY_CONTROL, SOY_PGDOWN, TRUE, 14, 2, "TR_patdec"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, "Patt++", 
    SOY_commandAddDuo(app, commandPatternInc, tracker, SOY_CONTROL, SOY_PGUP, TRUE, 14, 2, "TR_patinc"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, "Trac--", 
    SOY_commandAddDuo(app, commandTrackDec, tracker, SOY_CONTROL, SOY_INSERT, TRUE, 14, 2, "TR_trackdec"));
  tracker->buttons[btns++] = SOY_buttonBlank(app, "Trac++", 
    SOY_commandAddDuo(app, commandTrackInc, tracker, SOY_CONTROL, SOY_HOME, TRUE, 14, 2, "TR_trackinc"));
  SOY_commandDefrag(app);
  return TRUE;
}

/********************************************/
/* @render                                  */
/********************************************/

cString endGlyphs = "==";
cString fadeGlyphs = "~~";

real32 prevSample = 0;
sflag soy_tracker_draw_visualizer(soyAppHandle app, soyTracker* tracker, soyTrackerChannel* channel) {
  int32 yStep = app->draw.font.grid*app->draw.font.scale;
  int32 i;
  real32 curSample = 0;
  real32 cursorPos = 0;
  soySoundHandle ls = SOY_soundGet(app, channel->ID);
  real32 x = channel->rect.x+5;
  real32 y = channel->rect.y+8+yStep*2;
  real32 timeLines = (480000.0f*10.0f)/(real32)ls->length;
  uint32 visWidth = channel->rect.width-10;
  uint32 visHeight = yStep+yStep/2;
  if(timeLines < 1) { timeLines = 1.0f; }
  SOY_drawGUI(app, x, y, visWidth, visHeight, SOY_GUI_DARKEST, TRUE, 7);
  if(ls == NULL || ls->length == 0) { return FALSE; }
  for(i = 0; i < channel->rect.width-12; i++) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARK);
    if(timeLines == 0) { return FALSE; }
    if(i%((uint32)timeLines) == 0) {
      SOY_drawLine(app,
        x+i,
        y+2,
        x+i,
        y+visHeight-2);
    }
    curSample = (real32) *(ls->dataStartA+(uint32)((real32)i/(channel->rect.width-12)*ls->length));
    curSample/=(uint32)(32786.0f);
    curSample*= visHeight/2;
    SOY_drawSetPaletteColour(app, SOY_GUI_LIGHT);
    SOY_drawLine(app, x+i, y+(int32)prevSample+visHeight/2, x+i+1, y+(int32)curSample+visHeight/2);
    prevSample = curSample;
  }
  cursorPos = (ls->cursor/(real32)ls->length)*(channel->rect.width-12);
  SOY_drawSetPaletteColour(app, SOY_GUI_LIGHTEST);
  SOY_drawLine(app, x+cursorPos+1, y+4, x+cursorPos+1, y+visHeight-4);
  return tracker->playCursor;
}

sflag soy_tracker_draw_channel(soyAppHandle app,
    soyTracker* tracker, soyTrackerChannel* channel, soyTrackerPattern* pattern,
    int32 channelLen, sflag curChannel) {
  int32 i;
  int32 xStep = (app->draw.font.grid-app->draw.font.globalKerning)*app->draw.font.scale;
  int32 yStep = app->draw.font.grid*app->draw.font.scale;
  int32 yPad = yStep*channel->guiHeaderHeight;
  int32 maxH = channel->rect.height-12-yPad;
  int32 dy = 0;
  int32 cy = 0;
  int32 bW = channel->buttons[CHANNEL_BUTTONS-1].rect.x+SPR_MICRO_SIZE;
  int32 addY = 0;
  int8 emptyStr[3];
  int8 linStr[4];
  int8 volStr[3];
  int8 notStr[3];
  int8 octStr[2];
  int8 insStr[3];
  int8 effStr[3];
  int8 parStr[3];
  int8 wholeStr[16];
  int8 cursorString[64];
  int8 chanString[16];
  sflag rectInSelection = FALSE;
  sflag highlight = FALSE;
  sflag rectInClipboard = FALSE;
  soyTrackerNode* curNode = NULL;
  soyRect curRect = SOY_rectSimple("node", 0, 0, channel->rect.width-4, yStep);
  soyTrackerPattern* curPat = pattern;
  if(tracker == NULL) { return FALSE; }
  if(curPat == NULL) { return FALSE; }
  if(channel->rect.x+channel->rect.width+4 > tracker->rect.x+tracker->rect.width) { return FALSE; }
  if(maxH < yStep) {
    maxH = yStep;
  }
  dy = maxH/(yStep);
  addY = dy;
  if(dy > channelLen) { dy = channelLen; }

  SOY_drawSetPaletteColour(app, SOY_GUI_DARK);
  SOY_drawLine(app, bW+4,
                    channel->rect.y+channel->guiHeaderHeight*yStep+SPR_MICRO_SIZE-3,
                    channel->rect.x+channel->rect.width-4,
                    channel->rect.y+channel->guiHeaderHeight*yStep+SPR_MICRO_SIZE-3);
  SOY_drawLine(app, bW+4,
                    channel->rect.y+channel->guiHeaderHeight*yStep+SPR_MICRO_SIZE-5,
                    channel->rect.x+channel->rect.width-4,
                    channel->rect.y+channel->guiHeaderHeight*yStep+SPR_MICRO_SIZE-5);
  SOY_drawLine(app, bW+4,
                    channel->rect.y+channel->guiHeaderHeight*yStep+SPR_MICRO_SIZE-7,
                    channel->rect.x+channel->rect.width-4,
                    channel->rect.y+channel->guiHeaderHeight*yStep+SPR_MICRO_SIZE-7);
  if(curChannel) {
    SOY_drawGradient(app,
        channel->rect.x+4,
        channel->rect.y+4,
        channel->rect.x+channel->rect.width-3,
        channel->rect.y+channel->guiHeaderHeight*yStep-4,
        SOY_drawGetPaletteColour(app, SOY_GUI_GRADENDACTIVE),
        SOY_drawGetPaletteColour(app, SOY_GUI_GRADSTARTACTIVE), 'v');
    sprintf(cursorString, "[%s]", channel->infoStr);
    sprintf(chanString, "%s %d", channel->rect.label, channel->ID);
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
  } else {
    SOY_drawGradient(app,
        channel->rect.x+4,
        channel->rect.y+4,
        channel->rect.x+channel->rect.width-3,
        channel->rect.y+channel->guiHeaderHeight*yStep-4,
        SOY_drawGetPaletteColour(app, SOY_GUI_GRADENDINACTIVE),
        SOY_drawGetPaletteColour(app, SOY_GUI_GRADSTARTINACTIVE), 'v');
    sprintf(cursorString, "[%s]", " ");
    sprintf(chanString, "%s %d", channel->rect.label, channel->ID);
    SOY_drawSetPaletteColour(app, SOY_GUI_TEXTINACTIVE);
  }

  SOY_drawGUIRectInner(app,
      channel->rect.x+3,
      channel->rect.y+3,
      channel->rect.x+channel->rect.width-3,
      channel->rect.y+channel->guiHeaderHeight*yStep-4);

  if(curChannel) {
    SOY_drawGUIRectInner(app, channel->rect.x, channel->rect.y,
      channel->rect.x+channel->rect.width,
      channel->rect.y+channel->rect.height);
  } else {
     SOY_drawGUIRectOuter(app, channel->rect.x, channel->rect.y,
      channel->rect.x+channel->rect.width,
      channel->rect.y+channel->rect.height);
  }
  soy_tracker_draw_visualizer(app, tracker, channel);
  SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
  SOY_drawTextB(app, channel->rect.x+5, channel->rect.y+5, chanString, SOY_appGetFont(app));
  SOY_drawText(app, channel->rect.x+5, channel->rect.y+5+yStep, cursorString, SOY_appGetFont(app));

  for(i = 0; i < CHANNEL_BUTTONS; i++) {
    SOY_drawButtonMicro(app, &channel->buttons[i],
        (SOY_rectPressedL(&channel->buttons[i].rect) ? (i%8+8) : (i%8)), SPR_MICRO_SUS);
  }

  if(dy <= 0) { return FALSE; }
  for(i = 0; i < dy; i++) {
    addY = dy*((tracker->editCursor)/dy);
    if(addY+i < channelLen) {
      curNode = &channel->nodes[addY+i];
      curRect = SOY_rectSimple("node", 0, 0, channel->rect.width-4, yStep);
      SOY_rectUnsetArmable(&curRect);
      SOY_rectMoveRel(&curRect, &channel->rect, 2, yPad+8+cy);
      SOY_rectUpdate(app, &curRect);
      cy+=yStep;
       
      rectInSelection = soy_tracker_is_in_selection(curPat->selection,
          channel->ID, curNode->number);
      
      rectInClipboard = soy_tracker_is_in_selection(tracker->clipboard.selection,
          channel->ID, curNode->number);
      
      highlight = (((int32)i+addY == (int32)tracker->playCursor || SOY_rectHover(&curRect)) &&
         tracker->popupState == NOPOPUP);
      
      if(app->draw.spriteGUI) {
        SOY_drawGUI(app, curRect.x+2, curRect.y+1,
          curRect.width-4,
          curRect.height-1,
          (highlight ? SOY_GUI_LIGHT : SOY_GUI_NEUTRAL), TRUE,
          (highlight ? 5 : 13));
      } else {
        if(highlight || rectInClipboard || rectInSelection) {
          if(rectInClipboard) {
            SOY_drawSetPaletteColour(app, SOY_GUI_GRADENDACTIVE);
          } else if(rectInSelection) {
            SOY_drawSetPaletteColour(app, SOY_GUI_LIGHT);
          } else {
            SOY_drawSetPaletteColour(app, SOY_GUI_LIGHT);
          }
          SOY_drawRectF(app, curRect.x+2, curRect.y+1,
          curRect.x+2+curRect.width-4, curRect.y+1+curRect.height-1);
        }
        SOY_drawGUIRectInner(app, curRect.x+2, curRect.y+1,
          curRect.x+2+curRect.width-4, curRect.y+1+curRect.height-1);
      }

      if(highlight || rectInSelection) {
        SOY_drawResetPaletteTint(app);
      }

      if(curChannel && curNode->number == tracker->editCursor) {
        SOY_drawGUI(app,
            curRect.x+xStep*global_nodeSteps[tracker->nodeCursor]-1,
            curRect.y+2,
            xStep+1,
            curRect.height-3, SOY_GUI_BACKGROUND, TRUE, 3);
      }

      sprintf(emptyStr, "%c%c", tracker->emptyGlyph, tracker->emptyGlyph);
      
      if(SOY_bitCheck(curNode->flags, TNODE_FADE)) {
          sprintf(notStr, "%s", fadeGlyphs);
          sprintf(octStr, "%c", fadeGlyphs[0]);
          sprintf(volStr, "%s", fadeGlyphs);
          sprintf(insStr, "%s", fadeGlyphs);
          sprintf(effStr, "%s", fadeGlyphs);
          (SOY_bitCheck(curNode->flags, TNODE_EFFPAR)) ?
          sprintf(parStr, "%02X", curNode->effectParam) : sprintf(parStr, "%s", fadeGlyphs);
      } else if(SOY_bitCheck(curNode->flags, TNODE_END)) {
          sprintf(notStr, "%s", endGlyphs);
          sprintf(octStr, "%c", endGlyphs[0]);
          sprintf(volStr, "%s", endGlyphs);
          sprintf(insStr, "%s", endGlyphs);
          sprintf(effStr, "%s", endGlyphs);
          sprintf(parStr, "%s", endGlyphs);
      } else {
        if(SOY_bitCheck(curNode->flags, TNODE_NOTE)) {
          sprintf(notStr, "%s", global_noteStr[curNode->note]);
          sprintf(octStr, "%d", curNode->octave);
        } else {
          sprintf(notStr, "%s", emptyStr);
          sprintf(octStr, "%c", tracker->emptyGlyph);
        }
        (SOY_bitCheck(curNode->flags, TNODE_INST)) ?
          sprintf(insStr, "%02X", curNode->instrument) : sprintf(insStr, "%s", emptyStr);
        (SOY_bitCheck(curNode->flags, TNODE_VOLUME)) ?
          sprintf(volStr, "%02X", curNode->volume) : sprintf(volStr, "%s", emptyStr);
        (SOY_bitCheck(curNode->flags, TNODE_EFFECT)) ?
          sprintf(effStr, "%02X", curNode->effect) : sprintf(effStr, "%s", emptyStr);
        (SOY_bitCheck(curNode->flags, TNODE_EFFPAR)) ?
          sprintf(parStr, "%02X", curNode->effectParam) : sprintf(parStr, "%s", emptyStr);
      }
      sprintf(linStr, "%02X", curNode->number);
      sprintf(wholeStr, "%s %s%s %s %s %s %s", linStr, notStr, octStr, insStr, volStr, effStr, parStr);
      SOY_drawResetPaletteTint(app);
      SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
      if(curNode->number%channel->timescale == 0) {
        SOY_drawTextB(app, curRect.x+xStep, curRect.y+2, wholeStr, SOY_appGetFont(app));
      } else {
        SOY_drawText(app, curRect.x+xStep, curRect.y+2, wholeStr, SOY_appGetFont(app));
      }
    }
  }

  return TRUE;
}

sflag soy_tracker_draw_pattern(soyAppHandle app, soyTracker* tracker,
    soyTrackerPattern* pattern) {
  int32 i;
  SOY_drawGUIRectOuter(app, pattern->rect.x, pattern->rect.y,
      pattern->rect.x+pattern->rect.width,
      pattern->rect.y+pattern->rect.height);
  SOY_drawSetExtents(app,
      pattern->rect.x, pattern->rect.y,
      pattern->rect.x + pattern->rect.width-1,
      pattern->rect.y + pattern->rect.height-1);
  for(i = 0; i < (int32)pattern->channelCount; i++) {
    if(pattern->channels[i].enabled &&
        pattern->channels[i].rect.x+pattern->channels[i].rect.width <
        pattern->rect.width+pattern->rect.x &&
        pattern->channels[i].rect.x > 0) {
      soy_tracker_draw_channel(app, tracker,
          &pattern->channels[i], pattern,
          pattern->channelLen, (tracker->curChannel == i));
    }
  }
  SOY_drawResetExtents(app);
  return TRUE;
}

int32 global_trackEditorScroll = 0;
int32 global_trackEditorScrollPrev = 0;
int32 global_trackEditorScrollCur = 0;
sflag global_trackEditorBlinker = FALSE;
sflag global_trackEditorMouseLock = FALSE;

cString SPR_TRIANGLE_N = 
"0001000"
"0011100"
"0111110"
"1111111"
"0000000"
"0000000"
"0000000";

cString SPR_TRIANGLE_S = 
"0000000"
"0000000"
"0000000"
"1111111"
"0111110"
"0011100"
"0001000";

cString SPR_TRIANGLE_W = 
"0001000"
"0011000"
"0111000"
"1111000"
"0111000"
"0011000"
"0001000";

cString SPR_TRIANGLE_E = 
"0001000"
"0001100"
"0001110"
"0001111"
"0001110"
"0001100"
"0001000";

sflag soy_tracker_draw_track_editor(soyAppHandle app, soyTracker* tracker) {
  int32 i;
  int8 strBuf[8];
  int32 alignLeft = tracker->trackRect.x;
  int32 alignRight = tracker->trackRect.x+tracker->trackRect.width;
  int32 alignTop = tracker->trackRect.y;
  int32 alignBot = tracker->trackRect.y+tracker->trackRect.height;
  int32 drawIndex = 0;
  int32 segments = 0;
  real32 trackCursorPos = 0.0f;
  real32 trackEndPos = 0.0f;
  real32 trackLineWidth = 0.0f;
  real32 tintCol = 0.0f;
  uint32 trackCursorPosI = 0;
  uint32 trackCursorPosE = 0;
  soyButton curButton = SOY_button(app, "00", 0, 0, NULL);
  int32 yStep = curButton.rect.height+2;
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  segments = ((alignRight-4)-(alignLeft+4)-1)/(curButton.rect.width+1);
  SOY_drawGUIRectOuter(app, alignLeft, alignTop, alignRight, alignBot);
  SOY_drawSetPaletteColour(app, SOY_GUI_BACKGROUND);
  SOY_drawGUI(app, alignLeft+4, alignTop+4,
      tracker->trackRect.width-8,
      yStep, SOY_GUI_BACKGROUND, TRUE, 7);
  SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
  if(tracker->playCursor%8 < 4) { global_trackEditorBlinker = FALSE;
  } else { global_trackEditorBlinker = TRUE; }
  if(!tracker->playing) { global_trackEditorBlinker = TRUE; }
  for(i = 0; i < segments; i++) {
    sprintf(strBuf, "%02X",
        tracker->patternIndices[soy_min(i+global_trackEditorScroll,
        SOY_TRACKER_MAX_PATS-1)]);
    tintCol = SOY_PI*((real32)(i+global_trackEditorScroll)/SOY_TRACKER_MAX_PATS);
    SOY_drawResetPaletteTint(app);
    if((tracker->trackCursor) == i+global_trackEditorScroll && global_trackEditorBlinker) {
      SOY_drawSetPaletteEmission(app,
        1.0f-(cos(tintCol+SOY_PI)+1.0)/2.0f,
        1.0f-(sin(tintCol)+1.0)/2.0f,
        1.0f-(cos(tintCol)+1.0)/2.0f);
    } else if(i+global_trackEditorScroll < tracker->trackLen) {
      SOY_drawSetPaletteTint(app,
        1.0f-(cos(tintCol+SOY_PI)+1.0)/6.0f,
        1.0f-(sin(tintCol)+1.0)/6.0f,
        1.0f-(cos(tintCol)+1.0)/6.0f);
    }

    if(i+global_trackEditorScroll < SOY_TRACKER_MAX_PATS) {
      curButton = SOY_button(app, strBuf, 0, 0, NULL);
      SOY_rectUnsetArmable(&curButton.rect);
      SOY_buttonMove(&curButton, alignLeft+5+drawIndex*(curButton.rect.width+1), alignTop+5);
      SOY_buttonUpdate(app, &curButton);
      if(SOY_rectPressedL(&curButton.rect)) {
        if(!global_trackEditorMouseLock && i+global_trackEditorScroll < tracker->trackLen) {
          tracker->curPattern =  
            tracker->patternIndices[soy_min(i+global_trackEditorScroll, tracker->patternCount-1)];
            tracker->playCursor = 0;
            tracker->editCursor = 0;
            tracker->trackCursor =  i+global_trackEditorScroll;
        }
        SOY_drawSetPaletteEmission(app,
          1.0f-(cos(tintCol+SOY_PI)+1.0)*0.8f,
          1.0f-(sin(tintCol)+1.0)*0.8f,
          1.0f-(cos(tintCol)+1.0)*0.8f);
      }
      SOY_drawButton(app, &curButton, 2, 2);
      drawIndex++;  
    }
  }
  SOY_drawResetPaletteTint(app);
  if(global_trackEditorScroll > 0) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    SOY_drawMask(app,tracker->rect.x-8, tracker->trackRect.y+7, SPR_TRIANGLE_W, 7);
  }

  trackLineWidth = ((alignRight-4) - (alignLeft+4));
  trackCursorPos = 
    ((real32)tracker->playCursor+
     soy_max(0, soy_min(segments, (tracker->trackCursor-global_trackEditorScroll)))*
     patterns[tracker->curChannel].channelLen)/
    ((real32)patterns[tracker->curPattern].channelLen)/
    ((real32)trackLineWidth/(curButton.rect.width+1));
  trackEndPos = 
    (soy_max(0, soy_min(segments, (tracker->trackLen-global_trackEditorScroll)))*
     patterns[tracker->curChannel].channelLen)/
    ((real32)patterns[tracker->curPattern].channelLen)/
    ((real32)trackLineWidth/(curButton.rect.width+1));

  trackCursorPos = soy_lerp(0.0f, 1.0f, trackCursorPos);
  trackEndPos = soy_lerp(0.0f, 1.0f, trackEndPos);
  trackCursorPosI = (uint32)(trackCursorPos*trackLineWidth);
  trackCursorPosE = (uint32)(trackEndPos*trackLineWidth);

  SOY_drawSetPaletteColour(app, SOY_GUI_DARK);
  SOY_drawLine(app, alignLeft+4, alignBot-8, alignRight-4, alignBot-8);
  SOY_drawSetPaletteColour(app, SOY_GUI_LIGHT);
  SOY_drawLine(app, alignLeft+4, alignBot-8+1, alignRight-4, alignBot-8+1);
  
  SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
  SOY_drawMask(app,alignLeft+4+trackCursorPosI-2,alignBot-8+1, SPR_TRIANGLE_N, 7);
  SOY_drawMask(app,alignLeft+4+trackCursorPosE-2,alignBot-16+1, SPR_TRIANGLE_S, 7);

  if(SOY_rectHover(&tracker->trackRect) && SOY_mouseScrolled(app)) {
    global_trackEditorScrollCur = SOY_mouseScroll(app);
    if(global_trackEditorScrollCur > global_trackEditorScrollPrev) {
      global_trackEditorScroll++;
    } else if(global_trackEditorScrollCur < global_trackEditorScrollPrev) {
      global_trackEditorScroll--;
    }
    global_trackEditorScrollPrev = global_trackEditorScrollCur;
    if(global_trackEditorScroll < 0) { global_trackEditorScroll = 0; }
    if(global_trackEditorScroll > SOY_TRACKER_MAX_PATS) {
      global_trackEditorScroll = SOY_TRACKER_MAX_PATS;
    }
    if(SOY_mouseLeft(app)) {
      global_trackEditorMouseLock = TRUE;
    } else {
      global_trackEditorMouseLock = FALSE;
    }
  }
  return TRUE;
}

#define NUM_INFOSTRINGS 6;

cString info_longStr = 
      "Copy:%s%X-%02X-%02X-%02X-%02X "
      "Pattern:%02X Channel:%02X Cursor:%02X "
      "Octave:%02X Transpose:%02X";
cString info_shortStr = 
      "%s%X%02X%02X%02X%02X "
      "P:%02X C:%02X c:%02X "
      "O:%02X T:%02X";

sflag global_infoMouseLock = FALSE;
sflag global_infoStringToggle = FALSE;

sflag soy_tracker_draw_info(soyAppHandle app, soyTracker* tracker) {
  int32 alignLeft = tracker->infoRect.x;
  int32 alignTop = tracker->infoRect.y;
  int32 alignRight = tracker->infoRect.x+tracker->infoRect.width;
  int32 alignBot = tracker->infoRect.y+tracker->infoRect.height;
  int32 dx = 0;
  int32 dy = 0;
  int8 strBuf[128];
  soyFont* font = &app->draw.font;
  int32 xSteps = (alignRight-alignLeft)/((font->grid-font->globalKerning)*font->scale);
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  soyTrackerPattern* curPat = &patterns[tracker->curPattern];
  
  SOY_drawGUIRectInner(app, alignLeft+2, alignTop+2, alignRight-2, alignBot-2);
  SOY_drawSetPaletteColour(app, SOY_GUI_BACKGROUND);
  SOY_drawRectF(app, alignLeft+4, alignTop+4, alignRight-4, alignBot-4);
  SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);


  if(SOY_rectPressedL(&tracker->infoRect)) {
    if(!global_infoMouseLock) {
      global_infoStringToggle = !global_infoStringToggle; 
    }
  }

  if(SOY_mouseLeft(app)) {
    global_infoMouseLock = TRUE;
  } else {
    global_infoMouseLock = FALSE;
  }

  sprintf(strBuf, 
    (global_infoStringToggle) ?
    info_longStr : info_shortStr,
    global_noteStr[curPat->copyNode.note],
    curPat->copyNode.octave,
    curPat->copyNode.instrument,
    curPat->copyNode.volume,
    curPat->copyNode.effect,
    curPat->copyNode.effectParam,
    tracker->curPattern,
    tracker->curChannel,
    tracker->editCursor,
    tracker->globalOctave,
    tracker->globalTranspose
  );

  strBuf[soy_min(127, xSteps-2)] = 0;
  SOY_drawText(app, alignLeft+6+dx, alignTop+8+dy, strBuf, &app->draw.font);
  return TRUE;
}

sflag soy_tracker_draw_controls(soyAppHandle app, soyTracker* tracker) {
  int32 i;
  SOY_drawGUIRectOuter(app, tracker->controlRect.x, tracker->controlRect.y,
      tracker->controlRect.x+tracker->controlRect.width,
      tracker->controlRect.y+tracker->controlRect.height);
  for(i = 0; i < SOY_TRACKER_BUTTONS; i++) {
    if(tracker->buttons[i+global_controlButtonScroll].rect.y+
        tracker->buttons[i+global_controlButtonScroll].rect.height < 
        tracker->controlRect.y+tracker->controlRect.height) {
      switch(tracker->buttons[i+global_controlButtonScroll].type) {
        case(SOY_BUTTON_NORMAL): {
          SOY_drawButton(app, &tracker->buttons[i+global_controlButtonScroll], 2, 2);        
          break;
        }
        case(SOY_BUTTON_IMAGE): {
          break;
        }
        case(SOY_BUTTON_MICRO): {
          break;
        }
        default: { break; }
      }  
    }
  }
  if(global_controlButtonScroll > 0) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    SOY_drawMask(app,tracker->rect.x-8, tracker->controlRect.y+7, SPR_TRIANGLE_W, 7);
  }
  return TRUE;
}

sflag soy_tracker_draw_edit(soyAppHandle app, soyTracker* tracker) {
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  SOY_rectSelect(&tracker->buttons[2].rect);
  soy_tracker_draw_pattern(app, tracker, &patterns[tracker->curPattern]);
  soy_tracker_draw_info(app, tracker);
  soy_tracker_draw_track_editor(app, tracker);
  soy_tracker_draw_controls(app, tracker);
  return TRUE;
}

sflag soy_tracker_draw_instrument_editor(soyAppHandle app, soyTracker* tracker) {
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  SOY_rectSelect(&tracker->buttons[3].rect);
  soy_tracker_draw_pattern(app, tracker, &patterns[tracker->curPattern]);
  soy_tracker_draw_info(app, tracker);
  soy_tracker_draw_controls(app, tracker);
  return TRUE;
}

sflag soy_tracker_draw_config(soyAppHandle app, soyTracker* tracker) {
  soyTrackerPattern* patterns = SOY_memoryType(soyTrackerPattern, tracker->patternMemory);
  SOY_rectSelect(&tracker->buttons[4].rect);
  soy_tracker_draw_pattern(app, tracker, &patterns[tracker->curPattern]);
  soy_tracker_draw_info(app, tracker);
  soy_tracker_draw_controls(app, tracker);
  return TRUE;
}

soyBitmap bgBitmap;
sflag bgInit = FALSE;

sflag soy_tracker_draw(soyAppHandle app, soyTracker* tracker) {
  SOY_drawResetExtents(app); 
  if(!tracker->showBg) {
    SOY_drawSetPaletteColour(app, SOY_GUI_NEUTRAL);
    SOY_drawClear(app);
  } else if(!bgInit) {
    bgBitmap.imageSize = 0;
    SOY_loadImageBmpF(app, &bgBitmap, "assets/images/bg.bmp", "rgba", FALSE, TRUE);
    bgInit = TRUE;
  } else if(bgBitmap.imageSize) {
    SOY_drawBackground(app, &bgBitmap);
  } else {
    SOY_drawSetPaletteColour(app, SOY_GUI_NEUTRAL);
    SOY_drawClear(app);
  }
  
  switch(tracker->state) {
    case(EDIT): {
      soy_tracker_draw_edit(app, tracker);
      break;
    }
    case(CONFIG): {
      soy_tracker_draw_config(app, tracker);
      break;
    }
    case(INSTRUMENT): {
      soy_tracker_draw_instrument_editor(app, tracker);
      break;
    }
    default: { break; }
  }

  SOY_drawGUIRectOuter(app, tracker->rect.x, tracker->rect.y, tracker->rect.x+tracker->rect.width,
      tracker->rect.y+tracker->rect.height);
  SOY_drawGUIRectOuter(app, tracker->toolbarRect.x, tracker->toolbarRect.y,
      tracker->toolbarRect.x+tracker->toolbarRect.width,
      tracker->toolbarRect.y+tracker->toolbarRect.height);
  if(tracker->popupState != NOPOPUP) {
    soy_tracker_file_explorer_func(app, tracker, tracker->popupState);
  }
  return TRUE;
}

/********************************************/
/* @keyboard                                */
/********************************************/

int32 global_pianoConfigCounter = 36;
sflag global_pianoConfigLock = FALSE;
uint8 flags[] = { 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0 };

sflag soy_tracker_calibrate_keyboard(soyAppHandle app, soyTracker* tracker, uint32 index) {
  int32 i, j;
  int32 px;
  int32 py;
  int32 pw;
  int32 ph;
  int32 stepX;
  int32 stepY;
  soyFile* keymapFile = NULL;
  int8 strBuf[128];
  int8 strBufWarn[128];
  int8 strBufTitle[256];
  int32 strWidth = 0;
  int32 strTitleWidth = 0;
  int32 strWarnWidth = 0;
  int8 pianoChar[2] = { 0, 0 };
  pw = 10;
  ph = 96;
  px = tracker->rect.x+tracker->rect.width/2-18*(pw+4);
  py = tracker->rect.y+32+tracker->rect.height/2;
  stepX = 0;
  stepY = 0;

  if(global_pianoConfigCounter-36 == 0) {
    keymapFile = SOY_openFile("data/tracker_keymap.soy", "r");
    if(keymapFile != NULL) {
      j = 0;
      while(fscanf(keymapFile, "%u", &global_keyMap[j]) != EOF) {
        j++;
      }
      global_pianoConfigCounter = 100;
      SLOG("Found previous piano keymap\n");
      SOY_closeFile(keymapFile);
      return TRUE;
    }
  }

  sprintf(strBufWarn, "KEYBOARD MAP FILE NOT FOUND");
  sprintf(strBufTitle, "perhaps you have started the program\n"
  "for the first time or deleted the\n\"tracker_keymap.soy\" file in \"./data/\".");
  sprintf(strBuf, "Map keys to virtual piano: %d/36",
      global_pianoConfigCounter-36);
  strWidth = SOY_drawTextLen(strBuf, &app->draw.font);
  strWarnWidth = SOY_drawTextLen(strBufWarn, &app->draw.font);
  strTitleWidth = strWidth;
  SOY_drawPattern(app, "0x", 1, 0, 0, app->frameBuffer.width, app->frameBuffer.height);
  SOY_drawGUIPanel(app, tracker->rect.x + 32, tracker->rect.x + 32,
      tracker->rect.width-64, tracker->rect.height-64);
  SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
  SOY_drawTextB(app, tracker->rect.x+tracker->rect.width/2-strWarnWidth/2,
      tracker->rect.y+tracker->rect.height/2-144,
      strBufWarn, SOY_appGetFont(app));
  SOY_drawText(app, tracker->rect.x+tracker->rect.width/2-strTitleWidth/2,
      tracker->rect.y+tracker->rect.height/2-128,
      strBufTitle, SOY_appGetFont(app));
  SOY_drawText(app, tracker->rect.x+tracker->rect.width/2-strWidth/2,
      tracker->rect.y+tracker->rect.height/2-64,
      strBuf, SOY_appGetFont(app));
  if(SOY_keyPressedAny(app)) {
    if(SOY_keyPressed(app, SOY_BACKSPACE)) {
      if(!global_pianoConfigLock) {
        if(global_pianoConfigCounter > 36) {
          global_pianoConfigCounter--;
          global_pianoMap[global_pianoConfigCounter-36] = 0;
          global_keyMap[SOY_keyLast(app)] = 0;
        }
        global_pianoConfigLock = TRUE;
      }
    }
    if(SOY_keyIsAlphanumeric(SOY_keyLast(app))) {
      if(!global_pianoConfigLock) {
        global_pianoMap[global_pianoConfigCounter-36] = SOY_keyLast(app);
        global_keyMap[SOY_keyLast(app)] = global_pianoConfigCounter;
        global_pianoConfigCounter++;
        global_pianoConfigLock = TRUE;
      }
    }
  } else {
    global_pianoConfigLock = FALSE;
  }

  if(global_pianoConfigCounter-36 >= 36) {
    keymapFile = SOY_openFile("data/tracker_keymap.soy", "wb");
    if(keymapFile == NULL) { return FALSE; }
    j = 0;
    while(j < 0xFF) {
      fprintf(keymapFile, "%d\n", global_keyMap[j]);
      j++;
    }
    fclose(keymapFile);
    global_pianoConfigCounter = 100;
  }

  for(i = 0; i < 36; i++) {
    pianoChar[0] = global_pianoMap[i];
    stepY = (flags[(i%12)]) ? -32 : 0;
    if(flags[i%12]) {
      SOY_drawSetColour(app, 0, 0, 0, 255);
      if(i == global_pianoConfigCounter-36) { SOY_drawSetColour(app, 0, 0, 255, 255); }
      SOY_drawRectF(app, px+stepX, py+stepY, px+stepX+pw, py+stepY+ph);
      SOY_drawGUIRectOuter(app, px+stepX, py+stepY, px+stepX+pw, py+stepY+ph);
      SOY_drawSetColour(app, 255, 255, 255, 255);
      SOY_drawText(app, px+stepX+2, py+ph+stepY-14, pianoChar, SOY_appGetFont(app));
    } else {
      SOY_drawSetColour(app, 255, 255, 255, 255);
      if(i == global_pianoConfigCounter-36) { SOY_drawSetColour(app, 0, 0, 255, 255); }
      SOY_drawRectF(app, px+stepX, py+stepY, px+stepX+pw, py+stepY+ph);
      SOY_drawGUIRectOuter(app, px+stepX, py+stepY, px+stepX+pw, py+stepY+ph);
      SOY_drawSetColour(app, 0, 0, 0, 255);
      SOY_drawText(app, px+stepX+2, py+ph+stepY-14, pianoChar, SOY_appGetFont(app));
    }
    stepX += pw+4;
  }

  return index;
}

/********************************************/
/* @module                                  */
/********************************************/

sflag soy_module_construct(soyApp* app) {
  int32 i = 0;
  if(!app) { return FALSE; }
  /*
  soy_tracker_load_instruments(app, &global_tracker, "assets\\audio\\instruments");
  */
  soy_tracker_init(app, &global_tracker);
  soy_tracker_init_save_system(&global_tracker);
  global_pianoConfigCounter = 36;
  for(i = 0; i < 0xFF; i++) {
    global_keyMap[i] = 0;
  }

  SLOG1("Module active: %s\n", "Tracker");

  return TRUE;
}

sflag soy_module_update(soyApp* app, soyModule* module) {
  if(!app) { return FALSE; }
  soy_tracker_update(app, &global_tracker);
  soy_tracker_update_controls(app, &global_tracker);
  global_fontChanged = FALSE;
  return TRUE;
}

uint32 global_renderFrames = 0;

sflag soy_module_render(soyApp* app, soyModule* module) {
  if(!app) { return FALSE; }
  soy_tracker_draw(app, &global_tracker);
  printf("NIgerrerere\n");
  if(global_pianoConfigCounter <= 36+36) {
    soy_tracker_calibrate_keyboard(app, &global_tracker, 0);
  }
  global_renderFrames++;
  return TRUE;
}

sflag soy_module_resize(soyApp* app) {
  if(!app) { return FALSE; }
  soy_tracker_resize(app, &global_tracker);
  soy_tracker_update_controls(app, &global_tracker);
  soy_tracker_draw(app, &global_tracker);
  return TRUE;
}
  
sflag soy_module_destroy(soyApp* app) {
  if(!app) { return FALSE; }
  SLOG("Deleting module\n");
  /*
  soy_tracker_save_state(&global_tracker, "tracker_state", 0);
  */
  SOY_commandRemove(SOY_commandFind(app, "TR_play"));
  SOY_commandRemove(SOY_commandFind(app, "TR_stop"));
  SOY_commandRemove(SOY_commandFind(app, "TR_left"));
  SOY_commandRemove(SOY_commandFind(app, "TR_right"));
  SOY_commandRemove(SOY_commandFind(app, "TR_up"));
  SOY_commandRemove(SOY_commandFind(app, "TR_down"));
  SOY_commandRemove(SOY_commandFind(app, "TR_pageup"));
  SOY_commandRemove(SOY_commandFind(app, "TR_pagedown"));
  SOY_commandRemove(SOY_commandFind(app, "TR_tab"));
  SOY_commandRemove(SOY_commandFind(app, "TR_bspc"));
  SOY_commandRemove(SOY_commandFind(app, "TR_del"));
  SOY_commandRemove(SOY_commandFind(app, "TR_end"));
  SOY_commandRemove(SOY_commandFind(app, "TR_ret"));
  SOY_commandRemove(SOY_commandFind(app, "TR_undo"));
  SOY_commandRemove(SOY_commandFind(app, "TR_redo"));
  SOY_commandRemove(SOY_commandFind(app, "TR_edit"));
  SOY_commandRemove(SOY_commandFind(app, "TR_visual"));
  SOY_commandRemove(SOY_commandFind(app, "TR_config"));
  SOY_commandRemove(SOY_commandFind(app, "TR_instr"));
  SOY_commandRemove(SOY_commandFind(app, "TR_save"));
  SOY_commandRemove(SOY_commandFind(app, "TR_load"));
  SOY_commandRemove(SOY_commandFind(app, "TR_confpopup"));
  SOY_commandRemove(SOY_commandFind(app, "TR_cancelpopup"));
  SOY_commandRemove(SOY_commandFind(app, "TR_fontselect"));
  SOY_commandRemove(SOY_commandFind(app, "TR_debugdraw"));
  SOY_commandRemove(SOY_commandFind(app, "TR_chaninc"));
  SOY_commandRemove(SOY_commandFind(app, "TR_chandec"));
  SOY_commandRemove(SOY_commandFind(app, "TR_patinc"));
  SOY_commandRemove(SOY_commandFind(app, "TR_patdec"));
  SOY_commandRemove(SOY_commandFind(app, "TR_trackinc"));
  SOY_commandRemove(SOY_commandFind(app, "TR_trackdec"));
  SOY_commandRemove(SOY_commandFind(app, "TR_togglebg"));
  SOY_commandRemove(SOY_commandFind(app, "TR_spritegui"));
  SOY_freeImageBmp(&bgBitmap);
  SOY_memoryFree(app, &global_tracker.patternMemory);
  SOY_memoryFree(app, &global_tracker.clipboardMemory);
  return TRUE;
}

