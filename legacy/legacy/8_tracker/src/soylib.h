#ifndef SOY_LIB_H
#define SOY_LIB_H

#define SOY_LIB_IMPLEMENTATION

/********************************************/
/*   oooo               O        O          */
/*  O                   O     O  O          */
/*  O                   O        O          */
/*   oooo    oo   O  O  O     O  Oooo       */
/*       O  O  O  O O   O     O  O   O      */
/*       O  O  O  Oo    O     O  O   O      */
/*   oooo    oo   O     Oooo  O  Oooo  1.10 */
/*                O                         */
/*                                          */
/*  Graphics, Audio and Input library for   */
/*  Interactive Applications                */
/*                                          */
/********************************************/

/*******************************************/
/*       Copyright (c) 2021 CTech          */
/*         All rights reserved.            */
/*******************************************/

#define SOY_NOENTRY int DllMainCRTStartup() { return 0; }
#define _CRT_SECURE_NO_WARNINGS

#ifndef SOY_LIB_IMPLEMENTATION
/********************************************/
/* @dumb                                    */
/* SoyLib Dummy Interface                   */
/********************************************/
typedef void* soyApp;
typedef int sflag;
#define FALSE 0
#define TRUE 1
#define SLOG(x) while(0) {}
#else
/********************************************/
/* @platform                                */
/* SoyLib Platform Interface                */
/********************************************/

#ifdef _WIN32 /* Windows Implementation */
#include "soylib_win32.h"

/* for documentation generator */
#define _docstart /* find items to doc from here */
#define _docheader /* define header on next line */
#define _docend /* find items to doc up to here */
#define _doclink

typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef __m128i int128;
typedef __m128d real128;

typedef UINT8 uint8;
typedef UINT16 uint16;
typedef UINT32 uint32;
typedef UINT64 uint64;

typedef double real64;
typedef float real32;

typedef FILE soyFile;
typedef FILETIME soyFileTime;

typedef const int8* cString;

typedef struct tag_soyWindow  {
  soyWindowWin32 nativeWin;
} soyWindow;

typedef struct tag_soyThread {
  uint32 ID;
  soyThreadWin32 nativeThread;
} soyThread;

#define soy_bit_set(x, whence) ((x) |= (1<<(whence)))
#define soy_bit_clear(x, whence) ((x) &= ~(1<<(whence)))
#define soy_bit_flip(x, whence) ((x) ^= (1<<(whence)))
#define soy_bit_check(x, whence) ((x) & (1<<(whence)))

#define soy_alloc(x, y) soy_win32_alloc(x, y)
#define soy_free(x) soy_win32_free(x)
#define soy_res_x() soy_win32_desktop_res_x()
#define soy_res_y() soy_win32_desktop_res_y()

size_t Kilobytes(size_t size){ return 1024*(size_t)size; }
size_t Megabytes(size_t size){ return 1024*Kilobytes(size); }
size_t Gigabytes(size_t size){ return 1024*Megabytes(size); }
size_t Terabytes(size_t size){ return 1024*Gigabytes(size); }

#define soy_file_time_equal(a, b) soy_win32_file_time_compare_equal(a, b)

soyFileTime soy_file_last_write_time(int8* file) {
  return soy_win32_file_get_last_write(file);
}

soyFile* soy_file_open(cString file, cString args) {
  return fopen(file, args);
}

sflag soy_file_close(soyFile* file) {
  return fclose(file);
}

sflag soy_file_delete(cString file) {
  soy_win32_delete_file(file);
  return TRUE;
}

sflag soy_file_copy(cString from, cString to, sflag bailIfExists) {
  soy_win32_copy_file(from, to, bailIfExists);
  return TRUE;
}

sflag soy_list_dir(soyHandle appHandle,
    cString dir, cString fmt, pfSoyWin32FileListFunc func, soyHandle args) {
  return soy_win32_file_list_dir(appHandle, dir, fmt, func, args);
}

sflag soy_file_dir(cString filename, uint32 bufLen, int8* buffer) {
  return soy_win32_get_dir_string(filename, bufLen, buffer);
}

sflag soy_current_dir(uint32 bufLen, int8* buffer) {
  return soy_win32_get_current_dir(bufLen, buffer);
}

sflag soy_window_is_focused(soyWindow* win) {
  return soy_win32_is_focused(&win->nativeWin);
}

sflag soy_window_set_resize_func(soyWindow* win, pfSoyWinResizeFunc resizeFunc) {
  win->nativeWin.eraseFunc = resizeFunc;
  return TRUE;
}

sflag soy_window_create(soyHandle app,
    soyWindow* win, int32 width, int32 height, uint32 res, cString title) {
  int32 resX = soy_win32_desktop_res_x();
  int32 resY = soy_win32_desktop_res_y();
  soy_win32_create_window(app, &win->nativeWin,
      resX/2-width/2, resY/2-height/2, width, height, res, title);
  SLOG3("Created %d x %d window: \"%s\"\n", width, height, title);
  return TRUE;
}

sflag soy_window_gl_create(soyHandle app,
    soyWindow* win, int32 width, int32 height, uint32 res, cString title) {
  int32 resX = soy_win32_desktop_res_x();
  int32 resY = soy_win32_desktop_res_y();
  soy_win32_create_window_gl(app, &win->nativeWin,
      resX/2-width/2, resY/2-height/2, width, height, res, title);
  SLOG3("Created %d x %d openGL window: \"%s\"\n", width, height, title);
  return TRUE;
}

sflag soy_window_minimize(soyWindow* window) {
  soy_win32_minimize(&window->nativeWin);
  return TRUE;
}

sflag soy_window_unlock_resize(soyWindow* window) {
  window->nativeWin.lockResize = FALSE;
  return TRUE;
}

sflag soy_window_lock_resize(soyWindow* window) {
  window->nativeWin.lockResize = TRUE;
  return TRUE;
}

sflag soy_window_set_min(soyWindow* window, int32 width, int32 height) {
  return soy_win32_set_window_min(&window->nativeWin, width, height);
}

sflag soy_window_resize(soyWindow* window, int32 x, int32 y, int32 width, int32 height) {
  soy_win32_set_window_pos(&window->nativeWin, x, y, width, height);
  return TRUE;
}

sflag soy_window_restore(soyWindow* window) {
  soy_win32_set_window_pos_prev(&window->nativeWin);
  return TRUE;
}

sflag soy_window_stop(soyWindow* window) {
  soy_win32_close(&window->nativeWin);
  return TRUE;
}

sflag soy_window_destroy(soyWindow* window) {
  soy_win32_destroy_window(&window->nativeWin);
  return TRUE;
}

sflag soy_poll_events(soyWindow* window) {
  soy_win32_poll_events(&window->nativeWin);
  return TRUE;
}

sflag soy_should_quit(soyWindow* window) {
  return soy_win32_should_quit(&window->nativeWin);
}

sflag soy_blit_pixels(soyWindow* win, void* memory, int32 memWidth, int32 memHeight) {
  soy_win32_blit_pixels(&win->nativeWin, memory, memWidth, memHeight);
  return TRUE;
}

uint32 soy_any_key_pressed(soyWindow* win) {
  return soy_win32_any_key_pressed(&win->nativeWin);
}

sflag soy_key_pressed(soyWindow* win, uint32 key) {
  return soy_win32_key_pressed(&win->nativeWin, key);
}

sflag soy_key_pressed_async(uint32 key) {
  return soy_win32_async_key_state(key);
}

uint32 soy_last_key(soyWindow* win) {
  return soy_win32_last_key(&win->nativeWin);
}

uint8 soy_key_to_ascii(int32 scancode) {
  return soy_win32_key_to_ascii(scancode);
}

sflag soy_key_pressed_count(soyWindow* window) {
  return soy_win32_key_pressed_count(&window->nativeWin);
}

int32 soy_win_pos_x(soyWindow* win) { return win->nativeWin.posX; }
int32 soy_win_pos_y(soyWindow* win) { return win->nativeWin.posY; }
int32 soy_win_width(soyWindow* win) { return win->nativeWin.width; }
int32 soy_win_height(soyWindow* win) { return win->nativeWin.height; }
int32 soy_mouse_desktop_x() { return soy_win32_desktop_mouse_x(); }
int32 soy_mouse_desktop_y() { return soy_win32_desktop_mouse_y(); }

int32 soy_mouse_x(soyWindow* win) {
  return win->nativeWin.lastMousePosX/win->nativeWin.pixelResolution;
}

int32 soy_mouse_y(soyWindow* win) {
  return win->nativeWin.lastMousePosY/win->nativeWin.pixelResolution;
}

sflag soy_mouse_pressed(soyWindow* win, mouseCode mc) {
  return soy_win32_mouse_pressed(&win->nativeWin, mc);
}

int32 soy_mouse_scrolled(soyWindow* win) {
  return soy_win32_mouse_scrolled(&win->nativeWin)*soy_win32_mouse_scroll_dir(&win->nativeWin);
}

int32 soy_mouse_scroll(soyWindow* win) {
  return soy_win32_mouse_scroll(&win->nativeWin);
}

sflag soy_mouse_moved(soyWindow* win) {
  return soy_win32_mouse_moved(&win->nativeWin);
}

int32 soy_get_clipboard(int8* buffer, int32 buffersize) {
  return soy_win32_get_clipboard(buffer, buffersize);
}

int32 soy_get_local_time(timeScale ts) {
  return soy_win32_get_local_time(ts);
}

int32 soy_get_time(timeScale ts) {
  return soy_win32_get_system_time(ts);
}

sflag soy_thread_free(soyThread* thr) {
  soy_win32_thread_free(&thr->nativeThread);
  return TRUE;
}

sflag soy_thread_create(soyThread* thr, pfThreadFunc func, void** args) {
  soy_win32_thread_create(&thr->nativeThread, func, args);
  return TRUE;
}

sflag soy_audio_create_internal(soyWindow* window, soySoundWin32* sound,
    uint32 samplesPerSecond) {
  soy_win32_dsound_create(&window->nativeWin,
      sound, samplesPerSecond, 8000*sizeof(int16)*2);
  return TRUE;
}

uint32 soy_audio_buffer_size_internal(soySoundWin32* sound) {
  return sound->secondaryBufferSize;
}

#define soy_audio_update_internal(ss, sf, sp) soy_win32_dsound_update(ss, sf, sp)
#define soy_audio_play_internal(ss) soy_win32_dsound_play(&ss->nativeAudio)
#define soy_audio_stop_internal(ss) soy_win32_dsound_stop(&ss->nativeAudio)

#endif /* Windows Implementation */



/********************************************/
/* @GL                                      */
/* SoyLib openGL Interface                  */
/********************************************/

real32 triangleVerts[] = {
  -1.0f, -1.0f, 0.0f,
  1.0f, -1.0f, 0.0f, 
  0.0f,  1.0f, 0.0f 
};

real32 quadVerts[] = {
  -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
  1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
  
  -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
  -1.0f, 1.0f, 0.0f, 0.0f, 0.0f
};

typedef enum enum_shaderType {
  SOY_VERTEX_SHADER,
  SOY_FRAGMENT_SHADER
} shaderType;

typedef struct tag_soyGLShader {
  uint32 program;
  uint32 vertID;
  uint32 fragID;
} soyGLShader;

typedef struct tag_soyGLTexture {
  uint32 ID;
  uint8* data;
} soyGLTexture;

typedef uint32 SOY_VAO;
typedef uint32 SOY_VBO;
typedef soyGLShader* soyShaderHandle;

#define SOY_MAX_SHADER 4096

int8 soy_glSourceBuffer[SOY_MAX_SHADER];

const int8* srcPtrs[1] = {
  soy_glSourceBuffer,
};

sflag soy_gl_set_viewport(int32 x, int32 y, int32 width, int32 height) {
  glViewport(x, y, width, height);
  return TRUE;
}

sflag soy_gl_gen_textures(GLuint num, GLuint* texture) {
  glGenTextures(num, texture);
  return TRUE;
}

sflag soy_gl_bind_texture(GLenum type, GLuint texture) {
  glBindTexture(type, texture);
  return TRUE;
}

sflag soy_gl_tex_image_2d(
    GLenum target,
    GLint level,
    GLint iFormat,
    GLsizei width,
    GLsizei height,
    GLint border,
    GLenum format,
    GLenum type,
    const void* data) {
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(target,
      level, iFormat,
      width, height, border,
      format, type, data);
  return TRUE;
}

sflag soy_gl_gen_mipmaps(GLenum type) {
  glGenerateMipmap(type);
  return TRUE;
}

sflag soy_gl_active_texture(GLenum texture) {
  glActiveTexture(texture);
  return TRUE;
}

sflag soy_gl_shader_load(cString filename, uint32* shader, GLenum type) {
  int8 curChar;  
  int8 infoLog[512];
  int32 shaderRet = 0;
  int32 i = 0;
  soyFile* file = soy_file_open(filename, "r");
  if(file == NULL) { return FALSE; }  
  SLOG1("Loading shader \"%s\": \n", filename);
  while(fscanf(file, "%c", &curChar) != EOF) {
    soy_glSourceBuffer[i] = curChar;
    if(i < SOY_MAX_SHADER-1) { i++; }
    printf("%c", curChar);
  }
  soy_glSourceBuffer[i] = 0;
  *shader = glCreateShader(type);
  if(!shader || *shader == 0) { SLOG("Error creating shader\n"); }
  GLCHECK();
  glShaderSource(*shader, 1, &srcPtrs[0], NULL);
  GLCHECK();
  glCompileShader(*shader);
  GLCHECK();
  glGetShaderiv(*shader, GL_COMPILE_STATUS, &shaderRet);
  GLCHECK();
  if(!shaderRet) {
    glGetShaderInfoLog(*shader, 512, NULL, infoLog);
    GLCHECK();
    SLOG1("Shader compilation faliure: %s\n", infoLog);
  }
  soy_file_close(file);
  return TRUE;
}

sflag soy_gl_shader_create(cString filenameVert, cString filenameFrag, soyShaderHandle shader) {
  int8 infoLog[512];
  int32 glReturn = 0;
  int32 glError = 0;
  shader->program = glCreateProgram();
  GLCHECK();
  if(!shader->program) { SLOG("Error creating shader program\n"); }
  soy_gl_shader_load(filenameVert, &shader->vertID, GL_VERTEX_SHADER);
  GLCHECK();
  soy_gl_shader_load(filenameFrag, &shader->fragID, GL_FRAGMENT_SHADER);
  GLCHECK();
  glAttachShader(shader->program, shader->fragID);
  GLCHECK();
  glAttachShader(shader->program, shader->vertID);
  GLCHECK();
  glLinkProgram(shader->program);
  if(shader->program == GL_INVALID_VALUE) { SLOG("Shader linking error\n"); };
  
  GLCHECK();
  glGetProgramiv(shader->program, GL_LINK_STATUS, &glReturn);
  GLCHECK();
  if(!glReturn || glError) {
    SLOG1("Shader create error: %d\n", glError);
    glGetProgramInfoLog(shader->program, 512, NULL, infoLog);
    SLOG1("%s\n", infoLog);
  }
  glUseProgram(0);
  
  glDetachShader(shader->program, shader->vertID);
  glDetachShader(shader->program, shader->fragID);

  glDeleteShader(shader->vertID);
  GLCHECK();
  glDeleteShader(shader->fragID);
  GLCHECK();
  return TRUE;
}

sflag soy_gl_shader_use(soyShaderHandle shader) {
  glUseProgram(shader->program);
  return TRUE;
}

sflag soy_gl_shader_set_bool(soyShaderHandle shader, cString label, sflag value) {
  glUniform1i(glGetUniformLocation(shader->program, label), value);
  return TRUE;
}

sflag soy_gl_shader_set_int(soyShaderHandle shader, cString label, int32 value) {
  glUniform1i(glGetUniformLocation(shader->program, label), value);
  return TRUE;
}

sflag soy_gl_shader_set_float(soyShaderHandle shader, cString label, real32 value) {
  glUniform1f(glGetUniformLocation(shader->program, label), value);
  return TRUE;
}

sflag soy_gl_gen_buffers(GLsizei i, GLuint* buffer) {
  glGenBuffers(i, buffer);
  return TRUE;
}

sflag soy_gl_gen_vertex_arrays(GLsizei i, GLuint* buffer) {
  glGenVertexArrays(i, buffer);
  return TRUE;
}

sflag soy_gl_bind_vertex_array(GLuint arrayID) {
  glBindVertexArray(arrayID);
  return TRUE;
}

sflag soy_gl_bind_buffer(GLenum target, GLuint bufferID) {
  glBindBuffer(target, bufferID);
  return TRUE;
}

sflag soy_gl_buffer_data(GLenum target, GLsizei size, const void* verts) {
  glBufferData(target, size, verts, GL_STATIC_DRAW);
  return TRUE;
}

sflag soy_gl_draw_arrays(GLenum mode, GLint startIndex, GLsizei vertCount) {
  glDrawArrays(mode, startIndex, vertCount);
  return TRUE;
}

sflag soy_gl_vertex_attrib_pointer(GLuint index,
  GLint amt, GLenum type, GLboolean norm, GLsizei size, const GLvoid* offset) {
  glVertexAttribPointer(index, amt, type, norm, size, offset);
  return TRUE;
}

sflag soy_gl_vertex_attrib_enable(GLuint index) {
  glEnableVertexAttribArray(index);
  return TRUE;
}

/********************************************/
/* @time                                    */
/* SoyLib Time Management                   */
/********************************************/

typedef struct tag_soyTiming {
  LARGE_INTEGER lastCounter;
  LARGE_INTEGER workCounter;
  LARGE_INTEGER endCounter;
  uint32 sleepMs;
  uint32 frameCounter;
  real64 workDelta;
  real64 frameDelta;
  real64 msPerFrame;
  real64 timestep;
} soyTiming;

#define soy_sleep(ms) soy_win32_sleep(ms)

#define soy_time_set_scheduler(ms) soy_win32_time_set_scheduler_ms(ms)
#define soy_time_reset_scheduler(ms) soy_win32_time_reset_scheduler_ms(ms)
#define soy_time_delta_internal(begin, end) soy_win32_time_delta(begin, end)

LARGE_INTEGER soy_time_get_clock() {
  return soy_win32_time_get_clock();
}

sflag soy_time_init(soyTiming* t, real64 timestep) {
  soy_win32_time_init();
  t->workDelta = 0.0;
  t->frameDelta = 0.0;
  t->msPerFrame = 0.0;
  t->frameCounter = 0;
  t->workCounter = soy_time_get_clock();
  t->endCounter = soy_time_get_clock();
  t->lastCounter = soy_time_get_clock();
  t->sleepMs = 0;
  t->timestep = timestep;
  return TRUE;
}

sflag soy_time_measure_from(soyTiming* t) {
  t->workCounter = soy_time_get_clock();
  t->endCounter = t->workCounter;
  return TRUE;
}

sflag soy_time_measure_to(soyTiming* t) {
  t->endCounter = soy_time_get_clock();
  t->msPerFrame = 1000.0*soy_time_delta_internal(t->workCounter, t->endCounter);
  t->frameCounter++;
  return TRUE;
}

sflag soy_time_update(soyTiming* t) {
  t->workCounter = soy_time_get_clock();
  t->workDelta = soy_time_delta_internal(t->lastCounter, t->workCounter);
  t->frameDelta = t->workDelta;
  if(t->frameDelta < t->timestep) {
    t->sleepMs = (uint32)(1000.0*(t->timestep-t->frameDelta));
    if(t->sleepMs > 0) {
      soy_sleep(t->sleepMs);
    }
    while(t->frameDelta < 0) {
      t->frameDelta = soy_time_delta_internal(t->lastCounter, soy_time_get_clock());
    }
  } else {
    /* Missed target frame rate */
    SLOG1("(Missed frame %d)\n", t->frameCounter);
  }
  t->endCounter = soy_time_get_clock();
  t->msPerFrame = 1000.0*soy_time_delta_internal(t->lastCounter, t->endCounter);
  t->lastCounter = t->endCounter;
  t->frameCounter++;
  return TRUE;
}

real64 soy_time_delta(soyTiming* timing) {
  return timing->msPerFrame;
}

/********************************************/
/* @memory                                  */
/* SoyLib Memory Management                 */
/********************************************/

typedef struct tag_soyVirtualMemory {
  uint8* memory;
  uint32 size;
  uint32 back;
  uint32 front;
  uint32 step;
  cString label;
} soyVirtualMemory;

typedef struct tag_soyMemoryPartition {
  uint8* memory;
  uint32 back;
  uint32 size;
  cString label;
} soyMemoryPartition;

typedef struct tag_soyReferenceManager {
  soyVirtualMemory** references;
  uint32 referenceCount;
  uint32 memSize;
  uint32 globalMemoryAllocationSize;
} soyReferenceManager;

sflag soy_memory_add_reference(soyReferenceManager* man, soyVirtualMemory* refMem) {
  if(man->referenceCount < man->memSize) {
    man->references[man->referenceCount] = refMem;
    man->referenceCount++;
    man->globalMemoryAllocationSize+=refMem->size;
  } else {
    SLOG("Refernce counter memory exceeded\n");
  }
  return TRUE;
}

sflag soy_memory_remove_reference(soyReferenceManager* man, soyVirtualMemory* refMem, uint32 memSize) {
  uint32 i;
  for(i = 0; i < man->memSize; i++) {
    if(man->references[i] == refMem) {
      man->references[i] = NULL;
      man->referenceCount--;
      man->globalMemoryAllocationSize-=memSize;
      return TRUE;
    }
  }
  return FALSE;
}

#define SOY_MEMORY_MAX_REFERENCES 4096

sflag soy_memory_create(soyReferenceManager* man) {
  man->references = soy_alloc(NULL, SOY_MEMORY_MAX_REFERENCES*sizeof(soyVirtualMemory*));
  man->referenceCount = 0;
  man->memSize = SOY_MEMORY_MAX_REFERENCES*sizeof(soyVirtualMemory*);
  man->globalMemoryAllocationSize = 0;
  return TRUE;
}

sflag soy_memory_destroy(soyReferenceManager* man) {
  soy_free(man->references);
  man->referenceCount = 0;
  man->memSize = 0;
  return TRUE;
}

sflag soy_memory_enslave(soyReferenceManager* man,
    soyVirtualMemory* mem, uint32 size, uint32 stride, const char* label) {
  mem->memory = soy_alloc(NULL, size*stride);
  if(mem->memory == NULL){ return FALSE; }
  mem->size = size*stride;
  mem->back = 0;
  mem->front = 0;
  mem->step = stride;
  mem->label = label;
  soy_memory_add_reference(man, mem);
  return TRUE;
}

soyMemoryPartition soy_memory_partition(soyVirtualMemory* memory, uint32 size, cString label) {
  soyMemoryPartition part;
  if(memory->back+size+1 < memory->size) {
    part.memory = memory->memory+memory->back;
    part.size = size;
    part.label = label;
    part.back = 0;
    memory->back+=size;
    memory->memory[memory->back] = 0;
    memory->back++;
  } else {
    part.memory = NULL;
    part.size = 0;
    part.label = "<NULL>";
    part.back = 0;
    SLOG1("Error creating memory partition: %s\n", label);
  }
  return part;
}

sflag soy_memory_partition_reset(soyMemoryPartition* part) {
  part->back = 0;
  return TRUE;
}

sflag soy_memory_free(soyReferenceManager* man,
    soyVirtualMemory* mem) {
  uint32 size = mem->size;
  if(soy_free(mem->memory)) {
    mem->size = 0;
    mem->back = 0;
    mem->front = 0;
    mem->step = 0;
    soy_memory_remove_reference(man, mem, size);
    return TRUE;
  } else {
    SLOG("Error freeing memory\n");
  }
  return FALSE;
}

sflag soy_memory_clear(soyVirtualMemory* mem, int8 val) {
  uint32 i;
  for(i = 0; i < mem->size; i++) {
    mem->memory[i] = val;
  }
  return TRUE;
}

sflag soy_memory_print_state(soyReferenceManager* memMan) {
  SLOG2("Allocated memory: %f, References: %d\n",
      (real64)memMan->globalMemoryAllocationSize/Megabytes(1), memMan->referenceCount);
  return TRUE;
}

#define soy_memory_of_type(T, mem) ((T*)mem->memory)
#define soy_memory_push(T, mem, item) ((T*)mem->memory)[mem->back++] = item
#define soy_memory_pop(mem) mem->back--;
#define soy_memory_reset(mem) mem->back = 0; mem->front = 0

/********************************************/
/* @string                                  */
/* SoyLib String                            */
/********************************************/
#define SOY_MAX_CSTRING 512

/* length until #tokIndex token */
uint32 soy_cstring_lentok(cString str, int8 token, int32 tokIndex) {
	uint32 index = 0;
	if(str == NULL) { return 0; }
	while(index < SOY_MAX_CSTRING && (str[index] != '\0' && str[index] != token && tokIndex >= 0)) {
		if(str[index] == token) {
			tokIndex--;
		}
		index++;
	}
	if(index == SOY_MAX_CSTRING) {
		index = 0;
	}
	return index;
}

uint32 soy_cstring_len(cString str) {
	uint32 index = 0;
	if(str == NULL) { return 0; }
    while(index < SOY_MAX_CSTRING && str[index] != '\0') {
		index++;
	}
	if(index == SOY_MAX_CSTRING) {
		index = 0;
    }
	return index;
}

uint32 soy_cstring_lenb(cString str, uint32 start, uint32 strLen) {
	uint32 index = 0;
    uint32 bIndex = start;
    SLOG("Getting reverse length\n");
	if(str == NULL) { return 0; }
	while(index < SOY_MAX_CSTRING && str[strLen-(bIndex%strLen)] != '\n') {
        index++;
        bIndex++;
	}

    if(index == SOY_MAX_CSTRING) {
		SLOG("String too long\n");
        index = 0;
	}
	return index;
}

uint32 soy_cstring_compare(cString strA, cString strB) {
	uint32 index = 0;
    uint32 dif = 0;
    uint32 lenA = soy_cstring_len(strA);
    uint32 lenB = soy_cstring_len(strB);
    if(strA == NULL || strB == NULL) { return 1; }
    if(lenA != lenB) { return -1; }
	while(index < lenA) {
      if(strA[index] != strB[index]) {
        dif++;
      }
      index++;
	}
	return dif;
}

uint32 soy_cstring_compare_to(cString strA, cString strB, uint32 len) {
	uint32 index = 0;
    uint32 dif = 0;
    uint32 lenA = soy_cstring_len(strA);
    uint32 lenB = soy_cstring_len(strB);
	if(strA == NULL || strB == NULL) { return 0; }
	while(index < lenA && index < len && index < lenB) {
      if(strA[index] != strB[index]) {
        dif++;
      }
      index++;
	}
	return dif;
}

sflag soy_cstring_copy(cString orig, int8* buffer, uint32 startIndex, uint32 bufSize) {
	uint32 i;
	uint32 lenOrig;
	lenOrig = soy_cstring_len(orig);
	if(orig == NULL || buffer == NULL || startIndex >= lenOrig) {
		return FALSE;
	}
	i = startIndex;
	while(orig[i] != '\0' && i-startIndex < bufSize-1) {
		buffer[i-startIndex] = orig[i];
		i++;
	}
	buffer[i-startIndex] = '\0';
	return TRUE;
}

sflag soy_cstring_cat(int8* dest, int32 destSize, cString src) {
	int32 lenDest= 0;
	int32 lenSrc = 0;
	int32 index = 0;
	lenDest = soy_cstring_len(dest);
	lenSrc = soy_cstring_len(src);
	if(dest == NULL || src == NULL) { return 0; }
	if(lenDest+lenSrc < destSize) {
		for(index = 0; index < lenSrc; index++) {
			dest[lenDest+index] = src[index];
		}
		dest[index] = '\0';
	} else {
		SLOG("Destination buffer too small\n");
		return FALSE;
	}
	return TRUE;
}

/********************************************/
/* @file                                    */
/* SoyLib File Interface                    */
/********************************************/

uint32 soy_file_size(cString file) {
  soyFile* f = fopen(file, "r");
  fseek(f, 0, SEEK_END);
  return ftell(f);
}

/********************************************/
/* @dll                                     */
/* SoyLib Dynamic Library Interface         */
/********************************************/

typedef struct tag_soyProcess {
  pfSoyFunc proc;
  cString label;
} soyProcess;

typedef struct tag_soyLibrary {
  cString name;
  soyLibraryWin32 lib;
  soyFileTime filetime;
  soyFileTime filetimePrev;
} soyLibrary;

sflag soy_library_load(soyLibrary* sl, cString file) {
  sl->name = file;
  sl->filetime = soy_file_last_write_time((char*)file);
  sl->filetimePrev = sl->filetime;
  return soy_win32_load_library(&sl->lib, file);
}

sflag soy_library_free(soyLibrary* sl) {
  return soy_win32_free_library(&sl->lib);
}

soyProcess soy_library_load_proc(soyLibrary* sl, cString label) {
  soyProcess ret;
  ret.proc = (pfSoyFunc)soy_win32_library_load_function(&sl->lib, label);
  ret.label = label;
  return ret;
}

/********************************************/
/* @module                                  */
/* SoyLib Module Interface                  */
/********************************************/

typedef struct tag_soyModule {
  soyLibrary lib;
  soyProcess constFunc;
  soyProcess updateFunc;
  soyProcess renderFunc;
  soyProcess resizeFunc;
  soyProcess deleteFunc;
  sflag enabled;
  cString label;
  cString filepath;
} soyModule;

typedef soyModule* soyModuleHandle;

sflag soy_module_create(soyModuleHandle sm, cString file, cString name, void* args) {
  int8 tempStr[64];
  sprintf(tempStr, "temp_%s.dll", name);
  soy_file_copy(file, tempStr, FALSE);
  if(!soy_library_load(&sm->lib, tempStr)) {
    SLOG1("Error loading library %s\n", file);
    sm->enabled = FALSE;
    return FALSE;
  }
  sm->constFunc = soy_library_load_proc(&sm->lib, "soy_module_construct");
  sm->renderFunc = soy_library_load_proc(&sm->lib, "soy_module_render");
  sm->resizeFunc = soy_library_load_proc(&sm->lib, "soy_module_resize");
  sm->updateFunc = soy_library_load_proc(&sm->lib, "soy_module_update");
  sm->deleteFunc = soy_library_load_proc(&sm->lib, "soy_module_destroy");
  sm->label = name;
  sm->filepath = file;
  sm->enabled = TRUE;
  if(sm->constFunc.proc && sm->renderFunc.proc && sm->updateFunc.proc && sm->deleteFunc.proc) {
    sm->constFunc.proc(args);
    SLOG1("Module initialized: %s\n", file);
    return TRUE;
  }
  SLOG1("Error initializing module %s\n", file);
  return FALSE;
}

sflag soy_module_free(soyModuleHandle sm, void* args) {
  if(sm->enabled == FALSE) { return FALSE; }
  sm->deleteFunc.proc(args);
  soy_library_free(&sm->lib);
  sm->enabled = FALSE;
  return TRUE;
}

sflag soy_module_reload_if_changed(soyModuleHandle mod, soyHandle app) {
  mod->lib.filetime = soy_file_last_write_time((char*)mod->filepath);
  if(!soy_file_time_equal(mod->lib.filetime, mod->lib.filetimePrev) &&
      mod->enabled &&
      soy_file_size(mod->filepath)) {
    SLOG1("Reloading library: %s\n", mod->label);
    soy_module_free(mod, app);
    SLOG("Freed previous instance\n");
    soy_module_create(mod, mod->filepath, mod->label, app);
    mod->lib.filetimePrev = mod->lib.filetime;
  } else {
    mod->lib.filetimePrev = mod->lib.filetime;
    return FALSE;
  }
  return TRUE;
}

sflag soy_module_update_func(soyHandle app, soyModuleHandle mod) {
  if(mod->updateFunc.proc && mod->enabled) {
    mod->updateFunc.proc(app);
  }
  return TRUE;
}

sflag soy_module_render_func(soyHandle app, soyModuleHandle mod) {
  if(mod->renderFunc.proc && mod->enabled) {
    mod->renderFunc.proc(app);
  }
  return TRUE;
}

sflag soy_module_resize_func(soyHandle app, soyModuleHandle mod) {
  if(mod->resizeFunc.proc && mod->enabled) {
    mod->resizeFunc.proc(app);
  }
  return TRUE;
}

/********************************************/
/* @command                                 */
/* SoyLib Commands                          */
/********************************************/

#define SOY_COMMAND_MAX_KEYS 8

typedef struct tag_soyCommandManager {
  soyVirtualMemory* memory;
  uint32 numCommands;
} soyCommandManager;

typedef struct tag_soyCommand soyCommand;
typedef soyCommand* soyCommandHandle;

typedef sflag (*pfSoyCommandAction) (soyCommandHandle command, void* args);

struct tag_soyCommand {
  cString name;
  uint8 keys[SOY_COMMAND_MAX_KEYS];
  uint8 keyCount;
  pfSoyCommandAction action;
  sflag enabled;
  sflag toggle;
  sflag lock;
  sflag force;
  uint32 timesPressed;
  sflag restrictInput;
  int32 repeat;
  int32 timer;
  uint32 delay;
  void* args;
  soyCommandHandle prev;
  soyCommandHandle next;
  sflag overWrite;
};

soyCommand soy_command_null() {
  soyCommand ret;
  ret.overWrite = TRUE;
  ret.enabled = FALSE;
  ret.name = NULL;
  return ret;
}

soyCommandHandle soy_command_find_by_name(soyCommandManager* man, cString label) {
  soyCommandHandle coms = soy_memory_of_type(soyCommand, man->memory);
  uint32 i;
  for(i = 0; i < man->memory->back; i++) {
    if(soy_cstring_compare(label, coms[i].name) == 0) {
      return &coms[i];
    }
  }
  return NULL;
}
sflag soy_command_manager_init(soyCommandManager* manager, soyVirtualMemory* mem) {
  uint32 i;
  soyCommandHandle coms = soy_memory_of_type(soyCommand, mem);
  manager->memory = mem;
  for(i = 0; i < manager->memory->size/sizeof(soyCommand); i++) {
    coms[i] = soy_command_null();
  }
  return TRUE;
}

soyCommandHandle soy_command_manager_push(soyCommandManager* manager, soyCommand command) {
  soyCommandHandle mem = soy_memory_of_type(soyCommand, manager->memory);
  soyCommandHandle ret = NULL;
  uint32 i;
  if((ret = soy_command_find_by_name(manager, command.name)) != NULL) {
    SLOG1("Command named %s already exists you fucking buffoon\n", command.name);
    return ret;
  }
  if(manager->memory->back*sizeof(soyCommand) < manager->memory->size) {
    ret = &mem[manager->memory->back];
    mem[manager->memory->back++] = command;
  } else {
    SLOG("Not enough memory for command\n");
    for(i = 0; i < manager->memory->size/sizeof(soyCommand); i++) {
      if(mem[i].overWrite) {
        mem[i] = command;
      }
    }
  }
  return ret;
}

sflag soy_command_manager_defragment(soyCommandManager* manager) {
  uint32 i, j;
  soyCommandHandle coms = soy_memory_of_type(soyCommand, manager->memory);
  for(i = 0; i < manager->memory->size/sizeof(soyCommand); i++) {
    if(coms[i].overWrite) {
      j = 0;
      while(i+j < manager->memory->size/sizeof(soyCommand) && coms[i+j].overWrite) {
        j++;
      }
      coms[i] = coms[i+j];
      coms[i+j].overWrite = TRUE;
    }
  }
  return TRUE;
}

soyCommandHandle soy_command_manager_pop(soyCommandManager* manager) {
  soyCommandHandle ret = NULL;
  soyCommandHandle mem = soy_memory_of_type(soyCommand, manager->memory);
  if(manager->memory->back*sizeof(soyCommand) > 0) {
    manager->memory->back--;
    ret = &mem[manager->memory->back];
  } else {
    SLOG("Command buffer underflow\n");
  }
  return ret;
}

sflag soy_command_push_key(soyCommandHandle command, uint8 key) {
  if(key == 0) { command->keyCount = 0; return TRUE; }
  if(command->keyCount < SOY_COMMAND_MAX_KEYS) {
    command->keys[command->keyCount++] = key;
  } else {
    SLOG1("Maximum keys is %d\n", SOY_COMMAND_MAX_KEYS);
  }
  return TRUE;
}

sflag soy_command_pop_key(soyCommandHandle command) {
  if(command->keyCount > 0) {
    command->keys[--command->keyCount] = 0;
  } else {
    SLOG("No keys left\n");
  }
  return TRUE;
}

sflag soy_command_set_args(soyCommandHandle command, void* args) {
  command->args = args;
  return TRUE;
}

soyCommand soy_command(pfSoyCommandAction action,
    void* args, uint8 key, sflag toggle, uint32 t, uint32 d, const char* name) {
  soyCommand ret;
  ret.name = name;
  ret.keyCount = 0;
  ret.timer = t;
  ret.repeat = t;
  ret.force = FALSE;
  if(action) {
    ret.action = action;
  } else {
    ret.action = NULL;
  }
  ret.overWrite = FALSE;
  ret.delay = d;
  ret.lock = FALSE;
  ret.toggle = toggle;
  ret.args = args;
  ret.enabled = TRUE;
  ret.next = NULL;
  ret.prev = NULL;
  ret.timesPressed = 0;
  ret.restrictInput = FALSE;
  soy_command_push_key(&ret, key);
  return ret;
}

sflag soy_command_restrict_input(soyCommandHandle command, sflag value) {
  command->restrictInput = value;
  return TRUE;
}

sflag soy_command_link_prev(soyCommandHandle src, soyCommandHandle prev) {
  src->prev = prev;
  return TRUE;
}

sflag soy_command_link_next(soyCommandHandle src, soyCommandHandle next) {
  src->next = next;
  return TRUE;
}

sflag soy_key_is_alphanumeric(uint32 key) {
  return ((key >= 0x30 && key <= 0x5A) ||
      key == SOY_SPACE ||
      key == SOY_OEM_PLUS ||
      key == SOY_OEM_COMMA ||
      key == SOY_OEM_MINUS ||
      key == SOY_OEM_PERIOD ||
      key == SOY_OEM_PERIOD ||
      key == SOY_OEM_COMMA ||
      (key >= 0x60 && key <= 0x6F)
      );
}

sflag soy_key_is_hex(uint32 key) {
  return ((key >= 0x30 && key <= 0x46) ||
      key == SOY_SUBTRACT ||
      key == SOY_OEM_MINUS ||
      (key >= 0x60 && key <= 0x6F)
      );
}

sflag soy_command_force_fire(soyCommandHandle command) {
  if(command) {
    if(command->action) {
      command->action(command, command->args);
      command->timesPressed++;
    }
  }
  return TRUE;
}

sflag soy_command_update(soyCommandHandle command, soyWindow* window) {
  uint32 i;
  uint32 a = 0;
  if(!soy_window_is_focused(window)) { return FALSE; }
  if(command->overWrite) { return FALSE; }
  if(command->keyCount == 0) { return FALSE; }
  if(command->action == NULL) { SLOG("Command has no action assigned\n"); return FALSE; }
  if(command->restrictInput) {
    if(soy_key_pressed_count(window) != command->keyCount) { return FALSE; }
  }
  if(command->force) {
    command->action(command, command->args);
    command->timesPressed++;
    command->force = FALSE;
    if(command->next) { soy_command_update(command->next, window); }
    return TRUE;
  }
  for(i = 0; i < command->keyCount; i++) {
    if(soy_key_pressed_async(command->keys[i]) && soy_window_is_focused(window)) {
      a++;
    }
  }
  if(command->repeat) { command->timer--;
    if(command->timer < 0) { command->timer = command->delay; command->lock = FALSE; }
  }
  if(a == command->keyCount) { 
      if(!command->lock) {
      if(command->repeat) {
        command->action(command, command->args);
        command->timesPressed++;
      } else { command->action(command, command->args); command->timesPressed++; }
    }
    if(command->toggle) { command->lock = TRUE; }
  } else { command->timer = command->repeat; command->lock = FALSE; }
  if(command->next) { soy_command_update(command->next, window); }
  return TRUE;
}


sflag soy_command_manager_reset(soyCommandManager* man) {
  man->memory->back = 0;
  return TRUE;
}

uint32 soy_command_count(soyCommandHandle command) {
  return command->timesPressed;
}

sflag soy_command_manager_update(soyCommandManager* man, soyWindow* window) {
  uint32 i;
  soyCommandHandle coms = soy_memory_of_type(soyCommand, man->memory);
  for(i = 0; i < man->memory->back; i++) {
    if(coms[i].enabled && !coms[i].overWrite) {
      soy_command_update(&coms[i], window);
    }
  }
  return TRUE;
}

/********************************************/
/* @rect                                    */
/* SoyLib Command Rect                      */
/********************************************/

typedef enum enum_soyRectState {
  SOY_RECT_IDLE,
  SOY_RECT_HOVER,
  SOY_RECT_PRESSR,
  SOY_RECT_PRESSL,
  SOY_RECT_SELECTED,
  SOY_RECT_RELEASED
} soyRectState;

typedef enum enum_soyRectInternalState {
  SOY_IRECT_LOCKMOUSE,
  SOY_IRECT_LOCKED,
  SOY_IRECT_ARMED,
  SOY_IRECT_ARMABLE
} soyRectInternalState;

typedef struct tag_soyRect soyRect;
typedef soyRect* soyRectHandle;

struct tag_soyRect {
  int32 x;
  int32 y;
  int32 width;
  int32 height;
  soyRectState state;
  soyCommandHandle lcommand;
  soyCommandHandle rcommand;
  soyRectHandle prev;
  soyRectInternalState internalState;
  cString label;
};

sflag soy_rect_check_collision(soyRect rect, int32 x, int32 y) {
  return (x > rect.x && x < rect.x+rect.width && y > rect.y && y < rect.y+rect.height);
}

soyRect soy_rect(cString label, int32 x, int32 y,
    int32 width, int32 height, soyCommandHandle lcommand, soyCommandHandle rcommand, soyRectHandle prev) {
  soyRect res;
  res.width = width;
  res.height = height;
  res.x = x;
  res.y = y;
  res.state = SOY_RECT_IDLE;
  res.lcommand = lcommand;
  res.rcommand = rcommand;
  res.label = label;
  res.internalState = 0;
  res.prev = prev;
  soy_bit_set(res.internalState, SOY_IRECT_ARMABLE);
  soy_bit_set(res.internalState, SOY_IRECT_LOCKMOUSE);
  return res;
}

sflag soy_rect_resize(soyRectHandle rect, int32 x, int32 y, uint32 width, uint32 height) {
  rect->width = width;
  rect->height = height;
  rect->x = x;
  rect->y = y;
  return TRUE;
}

sflag soy_rect_select(soyRectHandle rect) {
  soy_bit_set(rect->state, SOY_RECT_SELECTED);
  return TRUE;
}

sflag soy_rect_deselect(soyRectHandle rect) {
  soy_bit_clear(rect->state, SOY_RECT_SELECTED);
  return TRUE;
}

sflag soy_rect_set_armable(soyRectHandle rect) {
  soy_bit_set(rect->internalState, SOY_IRECT_ARMABLE);
  return TRUE;
}

sflag soy_rect_unset_armable(soyRectHandle rect) {
  rect->internalState ^= SOY_IRECT_ARMABLE;
  soy_bit_clear(rect->internalState, SOY_IRECT_ARMABLE);
  return TRUE;
}

sflag soy_rect_update(soyRectHandle rect,
    int32 mx, int32 my, sflag mPressedL, sflag mPressedR) {
  soyRectState savedState = rect->state;
  sflag isColliding = soy_rect_check_collision(*rect, mx, my);
  soy_bit_clear(rect->state, SOY_RECT_RELEASED);
  soy_bit_clear(rect->state, SOY_RECT_SELECTED);
  
  if(mPressedR) {
    soy_bit_set(rect->state, SOY_RECT_PRESSR);
  } else {
    soy_bit_clear(rect->state, SOY_RECT_PRESSR);
  }
  if(mPressedL) {
    soy_bit_set(rect->state, SOY_RECT_PRESSL);
  } else {
    soy_bit_clear(rect->state, SOY_RECT_PRESSL);
  }
  
  
  if(!soy_bit_check(rect->internalState, SOY_IRECT_ARMABLE)) {
    if(isColliding) {
      soy_bit_set(rect->state, SOY_RECT_HOVER);
    } else {
      soy_bit_clear(rect->state, SOY_RECT_HOVER);
    }
  } else {
    if(!mPressedR && !mPressedL) {
      if(soy_rect_check_collision(*rect, mx, my)) {  
        soy_bit_set(rect->state, SOY_RECT_HOVER);
      } else {
        if(!mPressedR && !mPressedL) {
          soy_bit_clear(rect->state, SOY_RECT_HOVER);
        }
      }
    }
  }
  
  if(!soy_bit_check(rect->state, SOY_RECT_PRESSL) &&
    (soy_bit_check(savedState, SOY_RECT_PRESSL) &&
     soy_bit_check(savedState, SOY_RECT_HOVER))) {
    soy_bit_set(rect->state, SOY_RECT_RELEASED);
  }

  if(!mPressedL && !mPressedR) {
    if(soy_bit_check(rect->internalState, SOY_IRECT_LOCKED)) {
      soy_bit_clear(rect->internalState, SOY_IRECT_LOCKED);
    }
  }

  if(soy_bit_check(rect->state, SOY_RECT_RELEASED) && isColliding) {
    if(!(soy_bit_check(rect->internalState, SOY_IRECT_LOCKED))) {
      soy_command_force_fire(rect->lcommand);
      soy_bit_set(rect->internalState, SOY_IRECT_LOCKED);
    }
  }
  
  if(soy_bit_check(rect->state, SOY_RECT_RELEASED) && isColliding) {
    if(!(soy_bit_check(rect->internalState, SOY_IRECT_LOCKED))) {
      soy_command_force_fire(rect->rcommand);
      soy_bit_set(rect->internalState, SOY_IRECT_LOCKED);
    }
  }
  return TRUE;
}

/********************************************/
/* @canvas                                  */
/* SoyLib Canvas                            */
/********************************************/

typedef struct tag_soyCanvas {
  const char* name;
  int32 size;
  int32 width;
  int32 height;
  int32 stride;
  void* mem;
} soyCanvas;

sflag soy_canvas_create(soyVirtualMemory* mem, soyCanvas* canvas, int32 width, int32 height, int32 stride,
    cString name) {
  uint32 i = 0;
  if(mem == NULL) { return FALSE; }
  canvas->mem = mem->memory;
  canvas->width = width;
  canvas->height = height;
  canvas->stride = stride;
  canvas->size = mem->size;
  canvas->name = name;
  for(i = 0; i < mem->size; i++) {
	((uint8*)canvas->mem)[i] = 0;
  }
  SLOG3("Created %d x %d canvas: \"%s\"\n", width, height, name);
  return TRUE;
}

sflag soy_canvas_resize(soyCanvas* canvas, int32 width, int32 height, int32 stride) {
  if(width*height*stride >= canvas->size ) { return FALSE; }
  canvas->width = width;
  canvas->height = height;
  canvas->stride = stride;
  return TRUE;
}

/********************************************/
/* @util                                    */
/* SoyLib Utilities                         */
/********************************************/

typedef struct tag_soyPixel32 {
  uint8 r;
  uint8 g;
  uint8 b;
  uint8 a;
} soyPixel32;

typedef struct tag_soyLabel {
  cString pStr;
  uint32 index;
} soyLabel;

soyLabel soy_label(cString name, uint32 index) {
  soyLabel ret;
  ret.pStr = name;
  ret.index = index;
  return ret;
}

soyPixel32 soy_pixel32(uint8 r, uint8 g, uint8 b, uint8 a) {
  soyPixel32 ret;
  ret.r = r;
  ret.g = g;
  ret.b = b;
  ret.a = a;
  return ret;
}

sflag soy_swap_real32(real32* a, real32* b) {
	real32 c = *a;
    *a = *b;
	*b = c;
	return TRUE;
}

sflag soy_swap_real64(real64* a, real64* b) {
	real64 c = *a;
	*a = *b;
	*b = c;
	return TRUE;
}

sflag soy_swap_int32(int32* a, int32* b) {
	int32 c = *a;
	*a = *b;
	*b = c;
	return TRUE;
}

sflag soy_swap_int8(int8* a, int8* b) {
	int8 c = *a;
	*a = *b;
	*b = c;
	return TRUE;
}

#define STR(x) #x
#define VSTR(x) STR(x)

#define SOYSTR_IMPL(x) #x
#define SOYSTR(x) SOYSTR_IMPL(x)

#define SOYCAT_IMPL(a, b) a##b
#define SOYCAT(a, b) SOYCAT_IMPL(a,b)

/********************************************/
/* @sound                                   */
/* SoyLib Sound Interface                   */
/********************************************/

#define SOY_MIXER_CHANNELS 32
typedef enum enum_channelFormat {
  MONO,
  STEREO
} channelFormat;

typedef enum enum_soyNote {
  Ab0, A0, Bb0, B0,
  C1, Db1, D1, Eb1, E1, F1, Gb1, G1, Ab1, A1, Bb1, B1,
  C2, Db2, D2, Eb2, E2, F2, Gb2, G2, Ab2, A2, Bb2, B2,
  C3, Db3, D3, Eb3, E3, F3, Gb3, G3, Ab3, A3, Bb3, B3,
  C4, Db4, D4, Eb4, E4, F4, Gb4, G4, Ab4, A4, Bb4, B4,
  C5, Db5, D5, Eb5, E5, F5, Gb5, G5, Ab5, A5, Bb5, B5,
  C6, Db6, D6, Eb6, E6, F6, Gb6, G6, Ab6, A6, Bb6, B6,
  C7, Db7, D7, Eb7, E7, F7, Gb7, G7, Ab7, A7, Bb7, B7,
  C8, Db8, D8, Eb8, E8, F8, Gb8, G8, Ab8, A8, Bb8, B8,
  C9, Db9, D9, Eb9, E9, F9, Gb9, G9, Ab9, A9, Bb9, B9,
  EMPTY
} soyNote;

typedef struct tag_soySound {
  const char* name;
  int16* dataStartA;
  int16* dataStartB;
  uint32 length;
  sflag playing;
  sflag loop;
  sflag fade;
  uint32 loopStart;
  uint32 loopEnd;
  uint32 bitRate;
  uint32 samplingRate;
  real64 volume;
  real64 fadeAmt;
  real64 pitch;
  real64 timer;
  real64 maxAmp;
  real64 cursor;
  real64 curSample;
} soySound;

typedef soySound* soySoundHandle;

typedef struct tag_soyAudioMixer {
  int16 curSampleL;
  int16 curSampleR;
  int16 mainVolume;
  uint32 sampleCount;
  soySound channels[SOY_MIXER_CHANNELS];
} soyAudioMixer;

typedef struct tag_soyAudio  {
  soySoundWin32 nativeAudio;
  soyAudioMixer mixer;
  channelFormat channels;
  soyVirtualMemory* sampleMemory;
  soyVirtualMemory* loadMemory;
} soyAudio;

typedef soyAudio* soyAudioHandle;

typedef struct tag_sampleParams {
  real32 timestep;
  soyAudioHandle sound;
  sflag isStereo;
} sampleParams;

#define MAX_MIX_AMP 32786*4

int16 soy_sound_mix(int16 sampleA, int16 sampleB) {
  int32 mix = 0;
  int32 a = (int32)sampleA*2;
  int32 b = (int32)sampleB*2;
  /* clip signal */
  mix = a+b;
  if(mix >= MAX_MIX_AMP-1) { mix = MAX_MIX_AMP-1; }
  if(mix <= -MAX_MIX_AMP+1) { mix = -MAX_MIX_AMP+1; }

  return (int16)(mix/2);
}

real64 soy_sound_pitch(int32 note) {
  return pow(2.0, (real64)(note-49)/12.0)*440.0;
}

sflag soy_sound_load_mono(soyVirtualMemory* loadMem,
    soyVirtualMemory* mem, soySoundHandle sample, cString filename,
    uint32 bitrate, uint32 samplingRate) {
  FILE* file;
  uint32 fLen;
  uint32 i;
  uint32 stride = bitrate/8;
  int32 maxAmp;
  file = soy_file_open(filename, "rb");
  if(file == NULL) { SLOG1("File was null: %s\n", filename); return FALSE; }
  if(bitrate != 16) { SLOG("Only 16bit quality supported\n"); return FALSE; }
  fseek(file, SEEK_SET, SEEK_END);
  fLen = ftell(file);
  rewind(file);
  if(fLen*2 + mem->back > mem->size) {
    soy_file_close(file); SLOG1("File too large: %s", filename); return FALSE; }
  if(fLen*2 > loadMem->size) {
    soy_file_close(file); SLOG1("File too large to load: %s\n", filename); return FALSE; }
  fread(loadMem->memory, fLen, 1, file);
  loadMem->back = fLen;
  sample->dataStartA = (int16*)&(mem->memory[mem->back]);
  sample->dataStartB = (int16*)&(mem->memory[mem->back+fLen]);
  sample->maxAmp = 0;
  maxAmp = 0;
  for(i = 0; i < fLen; i++) {
    mem->memory[mem->back] = loadMem->memory[i];
    mem->memory[mem->back+fLen] = loadMem->memory[i];
    mem->back++;
  }
  mem->back+=fLen;
  sample->maxAmp = (real32)maxAmp/(32786.0f/2.0f);
  sample->samplingRate = samplingRate;
  sample->bitRate = bitrate;
  sample->cursor = 0;
  sample->length = fLen/stride;
  sample->pitch = soy_sound_pitch(40);
  soy_file_close(file);
  return TRUE;
}

sflag soy_sound_init(soySoundHandle sample) {
  sample->length = 0;
  sample->cursor = 0;
  sample->dataStartA = NULL;
  sample->dataStartB = NULL;
  sample->loop = FALSE;
  sample->playing = FALSE;
  sample->loopStart = 0;
  sample->timer = 0.0f;
  sample->name = NULL;
  sample->loopEnd = 0;
  sample->volume = 0;
  sample->pitch = 0;
  sample->bitRate = 0;
  sample->fadeAmt = 0;
  sample->fade = FALSE;
  return TRUE;
}

sflag soy_sound_create(soyVirtualMemory* loadMem, cString name,
    soyVirtualMemory* mem, soySoundHandle sample, cString filename, uint32 channels,
    uint32 bits, uint32 samplingRate) {
  sflag ret = FALSE;
  sample->length = 0;
  sample->cursor = 0;
  sample->dataStartA = NULL;
  sample->dataStartB = NULL;
  sample->loop = FALSE;
  sample->playing = FALSE;
  sample->loopStart = 0;
  sample->timer = 0.0f;
  sample->name = name;
  sample->volume = 0;
  sample->pitch = 0;
  sample->bitRate = 0;
  sample->fadeAmt = 0;
  sample->fade = FALSE;

  if(samplingRate != 48000) { SLOG("Only 48000Hz samples supported\n"); return FALSE; }
  switch(channels) {
    case(1): { ret = soy_sound_load_mono(loadMem, mem, sample, filename, bits, samplingRate);  break; }
    default: { SLOG("Only mono samples supported\n"); return FALSE; }
  }
  sample->loopEnd = sample->length;
  return ret;
}

sflag soy_sound_play(soySoundHandle sample) {
  if(!sample) { SLOG("No sample found\n"); return FALSE; }
  sample->cursor = 0;
  sample->timer = 0.0f;
  sample->fade = FALSE;
  sample->playing = TRUE;
  return TRUE;
}

sflag soy_sound_set_pitch(soySoundHandle sample, soyNote note) {
  if(!sample) { SLOG("No sample found\n"); return FALSE; }
  sample->pitch = soy_sound_pitch(note)/(soy_sound_pitch(40));
  return TRUE;
}

sflag soy_sound_set_volume(soySoundHandle sample, real32 volume) {
  if(!sample) { SLOG("No sample found\n"); return FALSE; }
  sample->volume = volume;
  return TRUE;
}

sflag soy_sound_fade(soySoundHandle sample, real64 amount) {
  if(!sample) { return FALSE; }
  sample->fadeAmt = amount;
  sample->fade = TRUE;
  return FALSE;
}

sflag soy_sound_pause(soySoundHandle sample) {
  if(!sample) { SLOG("No sample found\n"); return FALSE; }
  sample->playing = FALSE;
  return TRUE;
}

sflag soy_sound_stop(soySoundHandle sample) {
  if(!sample) { SLOG("No sample found\n"); return FALSE; }
  sample->playing = FALSE;
  sample->cursor = 0;
  sample->timer = 0.0f;
  return TRUE;
}

sflag soy_audio_init_mixer(soyAudioMixer* mixer) {
  uint32 i = 0;
  while(i < SOY_MIXER_CHANNELS) {
    mixer->channels[i].dataStartA = NULL;
    mixer->channels[i].dataStartB = NULL;
    mixer->channels[i].length = 0;
    i++;
  }
  mixer->curSampleL = 0;
  mixer->curSampleR = 0;
  mixer->mainVolume = 16000;
  mixer->sampleCount = 0;
  return TRUE;
}

sflag soy_mixer_set_sample(soyAudioMixer* mixer, soySound sample, uint8 channel) {
  if(sample.name == NULL) { SLOG("No sample found\n"); return FALSE; }
  if(channel < SOY_MIXER_CHANNELS){
    if(mixer->channels[channel].length == 0) {
      mixer->sampleCount++;
    }
    mixer->channels[channel] = sample;
  }
  return TRUE;
}

sflag soy_mixer_update(soyAudioMixer* mixer) {
  uint32 i = 0;
  uint32 j = 0;
  int16 mixTempL = 0;
  int16 mixTempR = 0;
  int16 mixFinalL = 0;
  int16 mixFinalR = 0;
  soySound mixQueue[SOY_MIXER_CHANNELS];
  while(i < SOY_MIXER_CHANNELS) {
    if(mixer->channels[i].length) {
      if(mixer->channels[i].fade) {
        mixer->channels[i].volume *= (1.0 - (mixer->channels[i].fadeAmt/1000.0));
      }
      if(mixer->channels[i].volume < 0.0) {
        mixer->channels[i].fade = FALSE;
      }
      mixQueue[j] = mixer->channels[i];
      j++;
    }
    i++;
  }

  if(j == 1) {
    mixFinalL = (int16)((real64)*(mixQueue[0].dataStartA+(uint32)mixQueue[0].cursor)*mixQueue[0].volume);
    mixFinalR = (int16)((real64)*(mixQueue[0].dataStartB+(uint32)mixQueue[0].cursor)*mixQueue[0].volume);
  } else {
    while(j > 0) {
      j--;
      mixFinalL = soy_sound_mix(((real64)*(mixQueue[j].dataStartA+(uint32)mixQueue[j].cursor)*
            mixQueue[j].volume/2.0), mixTempL);
      mixTempL = mixFinalL;
      mixFinalR = soy_sound_mix(((real64)*(mixQueue[j].dataStartB+(uint32)mixQueue[j].cursor)*
            mixQueue[j].volume/2.0), mixTempR);
      mixTempR = mixFinalR;
    }
  }

  mixer->curSampleL = mixFinalL;
  mixer->curSampleR = mixFinalR;
  return TRUE;
}

sflag soy_audio_load_raw(soyAudioHandle sound, cString name,
    soySoundHandle sample, const char* filename, uint32 channels, uint32 bits, uint32 samplingRate) {
  sample->name = filename;
  if(sound->sampleMemory == NULL) {
    SLOG("Sample memory not found\n");
    return FALSE;
  }
  if(soy_sound_create(sound->loadMemory, name,
        sound->sampleMemory, sample, filename, channels, bits, samplingRate)) {
    return TRUE;
  } else {
    soy_sound_init(sample);
    SLOG("Error!\n");
    return FALSE;
  }
}

sflag soy_audio_add_sound(soyAudioHandle sound, soySound sample, uint16 channel) {
  if(sample.name == NULL) { SLOG("No sample found\n"); return FALSE; }
  if(channel < SOY_MIXER_CHANNELS) {
    if(sample.length == 0) {
      SLOG("Invalid sample!\n");
      return FALSE;
    }
    soy_mixer_set_sample(&sound->mixer, sample, channel);
  } else {
      SLOG("Sample index out of range!\n");
  }

  return TRUE;
}

sflag soy_audio_remove_sound(soyAudioHandle sound, uint16 channel) {
  if(channel < SOY_MIXER_CHANNELS) {
    sound->mixer.channels[channel].length = 0;
  } else {
    SLOG1("Channel out of range (%d)\n", SOY_MIXER_CHANNELS);
  }
  return TRUE;
}

sflag soy_audio_create(soyWindow* win, soyVirtualMemory* loadMemory,
    soyVirtualMemory* sampleMemory, soyAudioHandle sound) {
  soy_audio_create_internal(win, &sound->nativeAudio, 48000);
  soy_audio_init_mixer(&sound->mixer);
  sound->sampleMemory = sampleMemory;
  sound->loadMemory = loadMemory;
  sound->channels = STEREO;
  return TRUE;
}

sflag soy_audio_play(soyAudioHandle sound) {
  soy_audio_play_internal(sound);
  return TRUE;
}

sflag soy_audio_stop(soyAudioHandle sound) {
  soy_audio_stop_internal(sound);
  return TRUE;
}

static real64 global_audioCursor = 0.0;
sflag soy_mixer_advance_samples(real32 timestep, soyAudioMixer* mixer) {
  uint32 i;
  soySound* curSound;
  for(i = 0; i < SOY_MIXER_CHANNELS; i++) {
    curSound = &mixer->channels[i];
    if(curSound != NULL && curSound->playing && curSound->name != NULL) {
      curSound->cursor+=curSound->pitch;
      if(curSound->cursor >= curSound->length) {
        soy_sound_stop(curSound);
      }
    }
  }
  global_audioCursor+=(real64)timestep;
  return TRUE;
}

soySampleResult sample_func(void* params) {
  sampleParams* p = (sampleParams*)params;
  soyAudioHandle sound = p->sound;
  soySampleResult sr;
  sr.left = 0;
  sr.right = 0;
  if(sound->channels == STEREO) {
    soy_mixer_update(&sound->mixer);
    soy_mixer_advance_samples(p->timestep, &sound->mixer);
    sr.left = sound->mixer.curSampleL;
    sr.right = sound->mixer.curSampleR;
  }
  return sr;
}

sflag soy_audio_update(real32 timestep, soyAudioHandle sound) {
  sampleParams sp;
  sp.timestep = timestep;
  sp.sound = sound;
  sp.isStereo = TRUE;
  soy_audio_update_internal(&sound->nativeAudio, sample_func, &sp);
  return TRUE;
}

/********************************************/
/* @math                                    */
/* SoyLib Math                              */
/********************************************/

real64 soy_fabs(real64 a) { return (a < 0.0) ? (a*-1.0) : (a); }
uint32 soy_abs(int32 a) { return (a < 0) ? (a*-1) : (a); }

#define soy_max(a, b) ((a > b) ? a : b)
#define soy_min(a, b) ((a < b) ? a : b)

#define SOY_PI 3.141592653
#define SOY_2PI 6.283185307
#define SOY_EULER 2.71828

#define SOY_INT_PERCISION 8
#define  SOY_FFT_REALSIZE 8

typedef struct tag_soyComplex {
  real64 re;
  real64 im;
} soyComplex;

typedef struct tag_vec2f {
  real32 x;
  real32 y;
} vec2f;

typedef struct tag_texCoord {
  real32 u;
  real32 v;
  real32 w;
} texCoord;

typedef struct tag_vec3f {
  real32 x;
  real32 y;
  real32 z;
} vec3f;

typedef struct tag_vec4f {
  real32 x;
  real32 y;
  real32 z;
  real32 w;
} vec4f;

typedef struct tag_vec2i {
  int32 x;
  int32 y;
} vec2i;

typedef struct tag_vec3i {
  int32 x;
  int32 y;
  int32 z;
} vec3i;

typedef struct tag_vec4i {
  int32 x;
  int32 y;
  int32 z;
  int32 w;
} vec4i;

typedef struct tag_mat2 {
  real32 m[2][2];
} mat2;

typedef struct tag_mat3 {
  real32 m[3][3];
} mat3;

typedef struct tag_mat4f {
  real32 m[4][4];
} mat4f;

typedef struct tag_soyTransform {
  vec3f position;
  vec3f rotation;
  vec3f scale;
} soyTransform;

soyTransform soy_transform(
    real32 x, real32 y, real32 z, real32 rx, real32 ry, real32 rz, real32 sx, real32 sy, real32 sz) {
  soyTransform t;
  t.position.x = x;
  t.position.y = y;
  t.position.z = z;
  t.rotation.x = rx;
  t.rotation.y = ry;
  t.rotation.z = rz;
  t.scale.x = sx;
  t.scale.y = sy;
  t.scale.z = sz;
  return t;
}

sflag soy_vec3f_equal(vec3f a, vec3f b) {
  if(a.x == b.x && a.y == b.y && a.z == b.z) {
    return TRUE;
  }
  return FALSE;
}
int32 soy_vec3i_equal(vec3i a, vec3i b) {
  if(a.x == b.x && a.y == b.y && a.z == b.z) {
    return 1;
  }
  return 0;
}

int32 soy_texCoord_equal(texCoord a, texCoord b) {
  if(a.u == b.u && a.v == b.v && a.w == b.w) {
    return 1;
  }
  return 0;
}

int32 soy_vec4f_equal(vec4f a, vec4f b) {
  if(a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w) {
    return 1;
  }
  return 0;
}

texCoord soy_uv(real32 u, real32 v, real32 w) {
  texCoord res;
  res.u = u;
  res.v = v;
  res.w = w;
  return res;
}

texCoord soy_texCoord(real32 u, real32 v, real32 w) {
  texCoord res;
  res.u = u;
  res.v = v;
  res.w = w;
  return res;
}

vec3f soy_vec3f(real32 x, real32 y, real32 z) {
  vec3f res;
  res.x = x;
  res.y = y;
  res.z = z;
  return res;
}

vec4f soy_vec4f(real32 x, real32 y, real32 z, real32 w) {
  vec4f res;
  res.x = x;
  res.y = y;
  res.z = z;
  res.w = w;
  return res;
}

vec4i soy_vec4i(int32 x, int32 y, int32 z, int32 w) {
  vec4i res;
  res.x = x;
  res.y = y;
  res.z = z;
  res.w = w;
  return res;
}

vec4f soy_vec4f_add(vec4f v1, vec4f v2) {
  vec4f res;
  res.x = v1.x + v2.x;
  res.y = v1.y + v2.y;
  res.z = v1.z + v2.z;
  return res;
}

vec4f soy_vec4f_subtract(vec4f v1, vec4f v2) {
  vec4f res;
  res.x = v1.x - v2.x;
  res.y = v1.y - v2.y;
  res.z = v1.z - v2.z;
  return res;
}

vec4f soy_vec4f_multiply(vec4f v1, vec4f v2) {
  vec4f res;
  res.x = v1.x * v2.x;
  res.y = v1.y * v2.y;
  res.z = v1.z * v2.z;
  return res;
}

vec4f soy_vec4f_multiply_scalar(vec4f v, real32 scalar) {
  vec4f res;
  res.x = v.x * scalar;
  res.y = v.y * scalar;
  res.z = v.z * scalar;
  return res;
}

vec4f soy_vec4f_divide(vec4f v1, vec4f v2) {
  vec4f res;
  res.x = v1.x / v2.x;
  res.y = v1.y / v2.y;
  res.z = v1.z / v2.z;
  return res;
}

vec4f soy_vec4f_divide_scalar(vec4f v1, real32 scalar) {
  vec4f res = soy_vec4f(0.0f, 0.0f, 0.0f, 0.0f);
  if(scalar != 0) {
    res.x = v1.x / scalar;
    res.y = v1.y / scalar;
    res.z = v1.z / scalar;
  }
  return res;
}

vec4f soy_vec4f_cross_product(vec4f v1, vec4f v2) {
  vec4f res;
  res.x = v1.y * v2.z - v1.z * v2.y;
  res.y = v1.z * v2.x - v1.x * v2.z;
  res.z = v1.x * v2.y - v1.y * v2.x;
  return res;
}

vec4f soy_vec4f_invert(vec4f v) {
  vec4f res;
  res.x = v.x*-1.0f;
  res.y = v.y*-1.0f;
  res.z = v.z*-1.0f;
  return res;
}

real32 soy_distance_between_points(real32 x1, real32 y1, real32 x2, real32 y2) {
  return (sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)));
}

real32 soy_vec4f_dot_product(vec4f v1, vec4f v2) {
  return (v1.x*v2.x+v1.y*v2.y+v1.z*v2.z);
}

real32 soy_vec4f_length(vec4f v) {
  return sqrt(soy_vec4f_dot_product(v, v));
}

vec4f soy_vec4f_normalize(vec4f v) {
  vec4f res;
  real32 len = soy_vec4f_length(v);
  res.x = v.x / len;
  res.y = v.y / len;
  res.z = v.z / len;
  return res;
}

mat4f soy_mat4f() {
  mat4f m;
  m.m[0][0] = 0.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f; m.m[0][3] = 0.0f;
  m.m[1][0] = 0.0f; m.m[1][1] = 0.0f; m.m[1][2] = 0.0f; m.m[1][3] = 0.0f;
  m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 0.0f; m.m[2][3] = 0.0f;
  m.m[3][0] = 0.0f; m.m[3][1] = 0.0f; m.m[3][2] = 0.0f; m.m[3][3] = 0.0f;
  return m;
}

mat4f soy_mat4f_identity() {
  mat4f m;
  m.m[0][0] = 1.0f; m.m[0][1] = 0.0f; m.m[0][2] = 0.0f; m.m[0][3] = 0.0f;
  m.m[1][0] = 0.0f; m.m[1][1] = 1.0f; m.m[1][2] = 0.0f; m.m[1][3] = 0.0f;
  m.m[2][0] = 0.0f; m.m[2][1] = 0.0f; m.m[2][2] = 1.0f; m.m[2][3] = 0.0f;
  m.m[3][0] = 0.0f; m.m[3][1] = 0.0f; m.m[3][2] = 0.0f; m.m[3][3] = 1.0f;
  return m;
}

vec4f soy_mat4f_vec4f_multiply(vec4f i, mat4f m) {
  vec4f res;
  res.x = i.x*m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
  res.y = i.x*m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
  res.z = i.x*m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
  res.w = i.x*m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];
  return res;
}

mat4f soy_mat4f_multiply(mat4f m1, mat4f m2) {
  int32 i;
  int32 j;
  mat4f res = soy_mat4f_identity();
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      res.m[j][i] = m1.m[j][0] * m2.m[0][i] + m1.m[j][1] * m2.m[1][i] +
                     m1.m[j][2] * m2.m[2][i] + m1.m[j][3] * m2.m[3][i];
    }
  }
  return res;
}

mat4f soy_mat4f_projection(real32 n, real32 f, real32 fov, real32 aspect) {
  real32 fFovRad;
  real32 fNear = 0.1f;
  real32 fFar = 1000.0f;
  real32 fFov = 45.0f;
  real32 fAspectRatio = 1.0f;
  mat4f m = soy_mat4f_identity();
  if(n != 0) {fNear = n;}
  if(f != 0) {fFar = f;}
  if(fov != 0) {fFov = fov;}
  if(aspect != 0) {fAspectRatio = aspect;}
  fFovRad = 1.0f / tan(fFov*0.5f / 180.0f * SOY_PI);
  m.m[0][0] = fAspectRatio * fFovRad;
  m.m[1][1] = fFovRad;
  m.m[2][2] = fFar / (fFar - fNear);
  m.m[3][2] = (-fFar * fNear) / (fFar - fNear);
  m.m[2][3] = 1.0f;
  m.m[3][3] = 0.0f;
  return m;
}

mat4f soy_mat4f_translation(real32 x, real32 y, real32 z) {
  mat4f m = soy_mat4f_identity();
  m.m[3][0] = x;
  m.m[3][1] = y;
  m.m[3][2] = z;
  return m;
}

mat4f soy_mat4f_scaling(real32 x, real32 y, real32 z) {
  mat4f m = soy_mat4f_identity();
  m.m[0][0] = x;
  m.m[1][1] = y;
  m.m[2][2] = z;
  return m;
}

mat4f soy_mat4f_rotationX(real32 angleRad) {
  mat4f rotX = soy_mat4f();
  rotX.m[0][0] = 1.0f;
  rotX.m[1][1] = cos(angleRad);
  rotX.m[1][2] = sin(angleRad);
  rotX.m[2][1] = -sin(angleRad);
  rotX.m[2][2] = cos(angleRad);
  rotX.m[3][3] = 1.0f;
  return rotX;
}

mat4f soy_mat4f_rotationY(real32 angleRad) {
  mat4f rotY = soy_mat4f();
  rotY.m[0][0] = cos(angleRad);
  rotY.m[0][2] = -sin(angleRad);
  rotY.m[2][0] = sin(angleRad);
  rotY.m[1][1] = 1.0f;
  rotY.m[2][2] = cos(angleRad);
  rotY.m[3][3] = 1.0f;
  return rotY;
}

mat4f soy_mat4f_rotationZ(real32 angleRad) {
  mat4f rotZ = soy_mat4f();
  rotZ.m[0][0] = cos(angleRad);
  rotZ.m[0][1] = -sin(angleRad);
  rotZ.m[1][0] = sin(angleRad);
  rotZ.m[1][1] = cos(angleRad);
  rotZ.m[2][2] = 1.0f;
  rotZ.m[3][3] = 1.0f;
  return rotZ;
}

mat4f soy_mat4f_rotation(real32 x, real32 y, real32 z) {
  mat4f temp = soy_mat4f_multiply(soy_mat4f_rotationX(x), soy_mat4f_rotationY(y));
  return soy_mat4f_multiply(temp, soy_mat4f_rotationZ(z));
}

mat4f soy_mat4f_set(real32 px, real32 py, real32 pz,
    real32 rx, real32 ry, real32 rz, real32 sx, real32 sy, real32 sz) {
  mat4f scale = soy_mat4f_scaling(sx, sy, sz);
  mat4f rot = soy_mat4f_rotation(rx, ry, rz);
  mat4f trans = soy_mat4f_translation(px, py, pz);
  mat4f scaleRot = soy_mat4f_multiply(scale, rot);
  return soy_mat4f_multiply(scaleRot, trans);
}

mat4f soy_mat4f_translate(mat4f m, real32 x, real32 y, real32 z) {
  return soy_mat4f_multiply(m, soy_mat4f_translation(x,y,z));
}

mat4f soy_mat4f_rotate(mat4f m, real32 x, real32 y, real32 z) {
  return soy_mat4f_multiply(m, soy_mat4f_rotation(x,y,z));
}

mat4f soy_mat4f_scale(mat4f m, real32 x, real32 y, real32 z) {
  return soy_mat4f_multiply(m, soy_mat4f_scaling(x,y,z));
}

mat4f soy_mat4f_point_at(vec4f pos, vec4f target, vec4f up) {
  vec4f newForward;
  vec4f a, newUp, newRight;
  real32 dp;
  mat4f matrix = soy_mat4f();
  newForward = soy_vec4f_subtract(target, pos);
  newForward = soy_vec4f_normalize(newForward);
  dp = soy_vec4f_dot_product(up, newForward);
  a = soy_vec4f_multiply_scalar(newForward, dp);
  newUp = soy_vec4f_subtract(up, a);
  newUp = soy_vec4f_normalize(newUp);
  newRight = soy_vec4f_cross_product(newUp, newForward);
  matrix.m[0][0] = newRight.x;
  matrix.m[0][1] = newRight.y;
  matrix.m[0][2] = newRight.z;
  matrix.m[0][3] = 0.0f;
  matrix.m[1][0] = newUp.x;
  matrix.m[1][1] = newUp.y;
  matrix.m[1][2] = newUp.z;
  matrix.m[1][3] = 0.0f;
  matrix.m[2][0] = newForward.x;
  matrix.m[2][1] = newForward.y;
  matrix.m[2][2] = newForward.z;
  matrix.m[2][3] = 0.0f;
  matrix.m[3][0] = pos.x;
  matrix.m[3][1] = pos.y;
  matrix.m[3][2] = pos.z;
  matrix.m[3][3] = 1.0f;
  return matrix;
}

mat4f soy_mat4f_quick_inverse(mat4f m) {
  mat4f matrix;
  matrix.m[0][0] = m.m[0][0];
  matrix.m[0][1] = m.m[1][0];
  matrix.m[0][2] = m.m[2][0];
  matrix.m[0][3] = 0.0f;
  matrix.m[1][0] = m.m[0][1];
  matrix.m[1][1] = m.m[1][1];
  matrix.m[1][2] = m.m[2][1];
  matrix.m[1][3] = 0.0f;
  matrix.m[2][0] = m.m[0][2];
  matrix.m[2][1] = m.m[1][2];
  matrix.m[2][2] = m.m[2][2];
  matrix.m[2][3] = 0.0f;
  matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
  matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
  matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
  matrix.m[3][3] = 1.0f;
  return matrix;
}

mat4f soy_compute_world_matrix(soyTransform t) {
  mat4f world = soy_mat4f_identity();
  mat4f scale = soy_mat4f_scaling(t.scale.x, t.scale.y, t.scale.z);
  mat4f rot = soy_mat4f_rotation(t.rotation.x, t.rotation.y, t.rotation.z);
  mat4f trans = soy_mat4f_translation(t.position.x, t.position.y, t.position.z);
  mat4f temp = soy_mat4f_multiply(scale, rot);
  world = soy_mat4f_multiply(temp, trans);
  return world;
}

vec4f soy_projection_to_screen(vec4f vProj, int32 width, int32 height) {
  vec4f res = vProj;
  vec4f offset = soy_vec4f(1.0f, 1.0f, 0.0f, 0.0f);
  res.x *= -1.0f;
  res.y *= -1.0f;
  res = soy_vec4f_add(res, offset);
  res.x *= 0.5f * (real32)width;
  res.y *= 0.5f * (real32)height;
  return res;
}

real32 soy_lerp(real32 start, real32 end, real32 t) {
	return (1.0 - t) * start + t * end;
}

texCoord soy_uv_lerp(texCoord start, texCoord end, real32 t) {
	texCoord res;
	res.u = soy_lerp(start.u, end.u, t);
	res.v = soy_lerp(start.v, end.v, t);
	res.w = soy_lerp(start.w, end.w, t);
	return res;
}

vec4f soy_vec4f_lerp(vec4f start, vec4f end, real32 t) {
	vec4f res;
	res.x = soy_lerp(start.x, end.x, t);
	res.y = soy_lerp(start.y, end.y, t);
	res.z = soy_lerp(start.z, end.z, t);
	res.w = soy_lerp(start.w, end.w, t);
	return res;
}

soyPixel32 soy_pixel32_lerp(soyPixel32 start, soyPixel32 end, real32 t) {
	soyPixel32 res;
	res.r = (uint8)(soy_lerp((real32)start.r, (real32)end.r, t));
	res.g = (uint8)(soy_lerp((real32)start.g, (real32)end.g, t));
	res.b = (uint8)(soy_lerp((real32)start.b, (real32)end.b, t));
	res.a = (uint8)(soy_lerp((real32)start.a, (real32)end.a, t));
	return res;
}

real32 soy_vec2f_slope(vec2f v1, vec2f v2) {
  return (v1.x - v2.x) / (v1.y - v2.y);
}

real32 soy_vec3f_slope(vec3f v1, vec3f v2) {
  return (v1.x - v2.x) / (v1.y - v2.y);
}

real32 soy_vec4f_slope(vec4f v1, vec4f v2) {
  return (v1.x - v2.x) / (v1.y - v2.y);
}

real32 soy_uv_slope(texCoord v1, texCoord v2) {
  return (v1.u - v2.u) / (real32)fabs(v1.v - v2.v);
}

int32 soy_swap_vec4f(vec4f* a, vec4f* b) {
  vec4f c = *a;
  *a = *b;
  *b = c;
  return 1;
}

int32 soy_swap_uv(texCoord* a, texCoord* b) {
  texCoord c = *a;
  *a = *b;
  *b = c;
  return 1;
}

real32 soy_pow(real32 x, real32 expn) {
  real32 ret = x;
  while(expn > 0) {
    ret*=ret;
    expn--;
  }
  return ret;
}

uint32 soy_clip_ui(uint32 i, uint32 min, uint32 max) {
  if(i > max) { return max; }
  if(i < min) { return min; }
  return i;
}

int32 soy_clip_i(int32 i, int32 min, int32 max) {
  if(i > max) { return max; }
  if(i < min) { return min; }
  return i;
}

real32 soy_clip_f(real32 i, real32 min, real32 max) {
  if(i > max) { return max; }
  if(i < min) { return min; }
  return i;
}

real64 soy_clip_ff(real64 i, real64 min, real64 max) {
  if(i > max) { return max; }
  if(i < min) { return min; }
  return i;
}

vec4f soy_vec4f_alpha_blend(vec4f fg, vec4f bg) {
    return soy_vec4f(
        ((fg.w * fg.x + (1.0f - fg.w) * bg.x)),
        ((fg.w * fg.y + (1.0f - fg.w) * bg.y)),
        ((fg.w * fg.z + (1.0f - fg.w) * bg.z)),
        (fg.w+bg.w)/2.0f);
}

static int64 SOY_PERLIN_SEED = 81651324;

static int64 soy_perlin_hash[] =
                    {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                     185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                     9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                     70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                     203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                     164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                     228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                     232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                     193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                     101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                     135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                     114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};

int64 soy_noise(int64 x, int64 y) {
    int64 tmp = soy_perlin_hash[(y + SOY_PERLIN_SEED) % 256];
    return soy_perlin_hash[(tmp + x) % 256];
}

real64 soy_perlin_lerp(real64 x, real64 y, real64 s) {
    return x + s * (y-x);
}
real64 soy_perlin_lerp_s(real64 x, real64 y, real64 s) {
    return soy_perlin_lerp(x, y, s * s * (3-2*s));
}

real64 soy_noise_2d(real64 x, real64 y) {
    int64 xInt = x;
    int64 yInt = y;
    real64 xFrac = x - xInt;
    real64 yFrac = y - yInt;
    int64 s = soy_noise(xInt, yInt);
    int64 t = soy_noise(xInt+1, yInt);
    int64 u = soy_noise(xInt, yInt+1);
    int64 v = soy_noise(xInt+1, yInt+1);
    real64 low = soy_perlin_lerp_s(s, t, xFrac);
    real64 high = soy_perlin_lerp_s(u, v, xFrac);
    return soy_perlin_lerp_s(low, high, yFrac);
}

real64 soy_perlin_noise_2d(real64 x, real64 y, real64 freq, int32 depth) {
    real64 xa = x*freq;
    real64 ya = y*freq;
    real64 amp = 1.0;
    real64 fin = 0;
    real64 d = 0.0;
    int32 i;
    for(i=0; i<depth; i++) {
        d += 256 * amp;
        fin += soy_noise_2d(xa, ya) * amp;
        amp /= 2;
        xa *= 2;
        ya *= 2;
    }
    return fin/d;
}

#define soy_map(x, low, high, min, max) (((x-low)/(high-low))*(max-min)+min);

real32 soy_normal_distribution(real32 x, real32 shift, real32 falloff, uint32 power) {
  x+=shift; 
  return (real32)pow(SOY_EULER, (real64)(-(pow(x, power))*falloff));
}

/********************************************/
/* @fft                                     */
/* SoyLib FFT (by Wang Jian-Sheng)          */
/********************************************/

static sflag stockham(soyComplex x[], int n, int flag, int n2, soyComplex y[]) {
   soyComplex  *y_orig, *tmp;
   int  i, j, k, k2, Ls, r, jrs;
   int  half, m, m2;
   real64  wr, wi, tr, ti;
   y_orig = y;
   r = half = n >> 1;
   Ls = 1;
   while(r >= n2) {                              /* loops log2(n/n2) times */
      tmp = x;                           /* swap pointers, y is always old */
      x = y;                                   /* x is always for new data */
      y = tmp;
      m = 0;                        /* m runs over first half of the array */
      m2 = half;                             /* m2 for second half, n2=n/2 */
      for(j = 0; j < Ls; ++j) {
         wr = cos(SOY_PI*j/Ls);                   /* real and imaginary part */
         wi = -flag * sin(SOY_PI*j/Ls);                      /* of the omega */
         jrs = j*(r+r);
         for(k = jrs; k < jrs+r; ++k) {           /* "butterfly" operation */
            k2 = k + r;
            tr =  wr*y[k2].re - wi*y[k2].im;      /* soyComplex multiply, w*y */
            ti =  wr*y[k2].im + wi*y[k2].re;
            x[m].re = y[k].re + tr;
            x[m].im = y[k].im + ti;
            x[m2].re = y[k].re - tr;
            x[m2].im = y[k].im - ti;
            ++m;
            ++m2;
         }
      } 
      r  >>= 1;
      Ls <<= 1;
   };
   if (y != y_orig) {                     /* copy back to permanent memory */
      for(i = 0; i < n; ++i) {               /* if it is not already there */
         y[i] = x[i];               /* performed only if log2(n/n2) is odd */
      }
   }
   /*assert(Ls == n/n2);*/                        /* ensure n is a power of 2  */
   /*assert(1 == n || m2 == n);*/           /* check array index within bound  */
  if(Ls != n/n2) { return FALSE; }
  if((1 != n) || (m2 != n)) { return FALSE; }
  return TRUE;
}  

sflag soy_fft(soyVirtualMemory* mem, soyComplex* x, int n, int flag) {
   soyComplex* y;
   if(!(flag == 1) || !(flag == -1)) { return FALSE; }
   y = soy_memory_of_type(soyComplex, mem);
   if(y == NULL) { return FALSE; }
   if(!stockham(x, n, flag, 1, y)) {
      return FALSE;
   }
   return TRUE;
}

/********************************************/
/* @strutil                                 */
/* SoyLib String Utilities                  */
/********************************************/

int64 s_decimal_divs[] = {
		1,
		10,
		100,
		1000,
		10000,
		100000,
		1000000,
		10000000,
		100000000,
		1000000000,
	};

#define SOY_INT_MAX_PERCISION 9
uint32 soy_int32_decimal_size(int64 n) {
	uint32 numSize = 0;
	int32 iter = SOY_INT_MAX_PERCISION;
	int32 i = 0;
	if(n < 0) {
		n*=-1;
	}
	while(i < iter) {
		if(n/s_decimal_divs[i] < 10) {
			numSize = i+1;
			return numSize;
		}
		i++;
	}
	return numSize;
}

sflag soy_int32_to_cstring(int64 un, int8* buffer, uint32 bufferSize) {
	uint32 numSize;
	uint32 i;
	uint32 iter = SOY_INT_MAX_PERCISION;
	sflag sig = FALSE;
	int32 a;
	uint64 n = (uint64) un;
	if(un < 0) {
		n = (uint64) (un*-1);
		sig = TRUE;
	}
	numSize = soy_int32_decimal_size(n);
	if(bufferSize < numSize + (sig ? 1 : 0)) {
		SLOG("Buffer size smaller then needed\n");
		return FALSE;
	}
	i = 0;
	if(numSize > iter) { numSize = iter; }
	a = sig ? 0 : 1;
	for(i = 0; i < numSize; i++) {
		if(sig) {
			buffer[0] = '-';
		}
		switch(n/s_decimal_divs[i] - 10*(n/s_decimal_divs[i+1])) {
			case(0): buffer[numSize-i-a] = '0'; break;
			case(1): buffer[numSize-i-a] = '1'; break;
			case(2): buffer[numSize-i-a] = '2'; break;
			case(3): buffer[numSize-i-a] = '3'; break;
			case(4): buffer[numSize-i-a] = '4'; break;
			case(5): buffer[numSize-i-a] = '5'; break;
			case(6): buffer[numSize-i-a] = '6'; break;
			case(7): buffer[numSize-i-a] = '7'; break;
			case(8): buffer[numSize-i-a] = '8'; break;
			case(9): buffer[numSize-i-a] = '9'; break;
			default: buffer[numSize-i-a] = 'E'; break;
		}
	}
	buffer[i+(sig ? 1 : 0)] = '\0';
	return TRUE;
}

sflag soy_real32_to_cstring(real64 un, int8* buffer, int32 percision, uint32 bufferSize) {
	int64 wholeNum;
	int64 realNum;
	int32 nsW;
	real64 usn;
	int8 whole[SOY_INT_MAX_PERCISION];
	int8 real[SOY_INT_MAX_PERCISION];
	wholeNum = (int64)un;
	usn = un - (real64)wholeNum;
	usn -= 1.0;
	if(usn < 0.0) {
		usn*= -1.0;
	}
	realNum = (int64) ((usn)*(real64)(soy_pow(10, percision)));
	nsW = soy_int32_decimal_size(wholeNum);

	soy_int32_to_cstring(wholeNum, whole, SOY_INT_MAX_PERCISION);
	soy_int32_to_cstring(realNum, real, SOY_INT_MAX_PERCISION);

	soy_cstring_copy(whole, buffer, 0, bufferSize);
	soy_cstring_copy(".", buffer+nsW+1, 0, bufferSize);
	soy_cstring_copy(real, buffer+nsW+2 , 1, bufferSize);
	return TRUE;
}

/********************************************/
/* @bitmap                                  */
/* SoyLib Bitmap Loading                    */
/********************************************/

typedef enum enum_BMPState {
  INITIALIZED,
  UNDEFINED
} BMPState;

typedef enum enum_bmpType {
  BMP_MONO,
  BMP_256,
  BMP_16,
  BMP_24
} bmpType;

typedef struct tag_soyBitmap {
  uint64 imageSize;
  uint64 rawSize;
  int32 headerSize;
  int32 compressionMethod;
  int32 width;
  int32 height;
  int32 padding;
  int32 pixelFormat;
  uint8* bytes;
  cString name;
  BMPState state;
} soyBitmap;

typedef soyBitmap* soyBitmapHandle;

sflag soy_bitmap_free(soyBitmapHandle bitmap) {
  soy_free(bitmap->bytes);
  bitmap->bytes = NULL;
  return TRUE;
}

sflag soy_bitmap_init(soyBitmapHandle bitmap) {
  bitmap->state = FALSE;
  bitmap->bytes = NULL;
  return TRUE;
}

#define RED 0
#define GREEN 1
#define BLUE 2
#define ALPHA 3
#define NULLCOL 4

sflag soy_bitmap_import(soyVirtualMemory* lMem, soyVirtualMemory* vMem,
    soyBitmapHandle bmp, cString filepath, const int8 bmpFormat[], sflag flipV, sflag flipH) {
  FILE* file = NULL;
  uint64 fileSize = 0;
  uint64 resultSize = 0;
  uint32 parsedInt = 0;
  uint16 parsedShort = 0;
  uint32 index = 0;
  uint32 indexC = 0;
  uint32 indexH = 0;
  uint32 width = 0;
  uint32 height = 0;
  uint16 pixelFormat = 0;
  uint16 pixelFormatInternal = 0;
  uint32 headerSize = 0;
  uint32 dataStartIndex = 0;
  uint32 rawSize = 0;
  int32 padding = 0;
  int32 padInternal = 0;
  uint32 fc1, fc2, fc3, fc4;
  uint16 sc1, sc2;
  int8 compressionMethod = 0;
  int8 pixelBufer[4];
  int32 pixelIndex = 0;
  int32 bmpFormatStrLen = 0;
  soyPixel32 tempPixel;

  if(bmp == NULL) { SLOG("soyBitmap was NULL\n"); return FALSE; }

  bmp->name = filepath;
  bmp->bytes = NULL;

  while(bmpFormat[bmpFormatStrLen] != '\0') {
    if(bmpFormatStrLen > 4) { SLOG("Invalid format\n"); return FALSE; }
    bmpFormatStrLen++;
  }

  if(bmpFormatStrLen <= 0) { SLOG("Unsupported format size\n"); return FALSE; }

  for(; pixelIndex < bmpFormatStrLen; pixelIndex++) {
    switch(bmpFormat[pixelIndex]) {
      case('r'): { pixelBufer[pixelIndex] = RED; break; }
      case('g'): { pixelBufer[pixelIndex] = GREEN; break; }
      case('b'): { pixelBufer[pixelIndex] = BLUE; break; }
      case('a'): { pixelBufer[pixelIndex] = ALPHA; break; }
      case('n'): { pixelBufer[pixelIndex] = NULLCOL; break; }
      default:   { break; }
    }
  }

  file = soy_file_open(filepath, "rb");
  if(!file) { SLOG1("Could not open file: %s \n", filepath); return FALSE; }

  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  rewind(file);

  if(!lMem || fileSize >= lMem->size) {
    SLOG("Could not allocate enough memory\n");
    return FALSE;
  }

  resultSize = fread(lMem->memory, 1, fileSize, file);
  if(resultSize != fileSize) {
    SLOG1("Could not read file: %s\n", filepath);
    return FALSE;
  }
  soy_file_close(file);
  if(lMem->memory[0] == 'B' && lMem->memory[1] == 'M') {}

  fc1 = lMem->memory[2];        fc2 = lMem->memory[3] << 8;
  fc3 = lMem->memory[4] << 16;  fc4 = lMem->memory[5] << 24;
  parsedInt = fc1 | fc2 | fc3 | fc4;

  if(parsedInt != fileSize) { SLOG("Failed to parse correct filesize, aborting... \n"); return FALSE; }

  fc1 = lMem->memory[10];       fc2 = lMem->memory[11] << 8;
  fc3 = lMem->memory[12] << 16; fc4 = lMem->memory[13] << 24;
  parsedInt = fc1 | fc2 | fc3 | fc4;
  dataStartIndex = parsedInt;

  fc1 = lMem->memory[14];       fc2 = lMem->memory[15] << 8;
  fc3 = lMem->memory[16] << 16; fc4 = lMem->memory[17] << 24;
  parsedInt = fc1 | fc2 | fc3 | fc4;
  headerSize = parsedInt;

  fc1 = lMem->memory[18];       fc2 = lMem->memory[19] << 8;
  fc3 = lMem->memory[20] << 16; fc4 = lMem->memory[21] << 24;
  parsedInt = fc1 | fc2 | fc3 | fc4;
  width = parsedInt;

  fc1 = lMem->memory[22];       fc2 = lMem->memory[23] << 8;
  fc3 = lMem->memory[24] << 16; fc4 = lMem->memory[25] << 24;
  parsedInt = fc1 | fc2 | fc3 | fc4;
  height = parsedInt;

  sc1 = lMem->memory[28];       sc2 = lMem->memory[29] << 8;
  parsedShort = sc1 | sc2;
  pixelFormat = parsedShort;

  compressionMethod = lMem->memory[30];
  fc1 = lMem->memory[34];       fc2 = lMem->memory[35] << 8;
  fc3 = lMem->memory[36] << 16; fc4 = lMem->memory[37] << 24;
  parsedInt = fc1 | fc2 | fc3 | fc4;
  rawSize = parsedInt;

  padding = 0;
  while((width*3+padding)%4 != 0) {
    padding++;
  }
  index = 0;
  switch(pixelFormat) {
    case(24): { pixelFormatInternal = 3; break; }
    case(8): { pixelFormatInternal = 0; SLOG1("Format: %d not currently supported\n", pixelFormat); break; }
    case(4): { pixelFormatInternal = 0; SLOG1("Format: %d not currently supported\n", pixelFormat); break; }
    case(1): { pixelFormatInternal = 0; SLOG1("Format: %d not currently supported\n", pixelFormat); break; }
  }
  bmp->headerSize = headerSize;
  bmp->rawSize = rawSize;
  bmp->imageSize = width*height*bmpFormatStrLen;
  if(vMem->back+bmp->imageSize >= vMem->size) {
	  bmp->bytes = NULL;
      printf("Out of video memory\n");
	  return FALSE;
  }
  bmp->bytes = &vMem->memory[vMem->back];
  bmp->width = width;
  bmp->height = height;
  bmp->padding = padding;
  bmp->pixelFormat = pixelFormat;
  bmp->compressionMethod = compressionMethod;
  bmp->state = INITIALIZED;
  padInternal = 0;
  indexC = 0;
  for(index = 0; index < width*height; index++) {
    for(pixelIndex = 0; pixelIndex < 4; pixelIndex++) {
		if(pixelBufer[pixelIndex] < ALPHA) {
        bmp->bytes[(flipV) ? (index*4+pixelIndex) : (bmp->imageSize-4) - index*4+pixelIndex]
        = lMem->memory[dataStartIndex+index*pixelFormatInternal+pixelBufer[pixelIndex]+padInternal];
      } else if (pixelBufer[pixelIndex] == ALPHA) {
        bmp->bytes[(flipV) ? (index*4+pixelIndex) : (bmp->imageSize-4) - index*4+pixelIndex] = 255;
      }
	  vMem->back++;
    }
    if((index)%(width) == 0) {
      padInternal+=padding;
    }
  }
  /* flip the image horizontaly */
  if(flipH) {
	pixelIndex = 0;
	for(indexC = 0; indexC < height; indexC++) {
	  for(indexH = 0; indexH < width/2; indexH++) {
		tempPixel = ((soyPixel32*)bmp->bytes)[(indexC+1)*width - indexH-1];
		((soyPixel32*)bmp->bytes)[(indexC+1)*width-indexH-1] = ((soyPixel32*)bmp->bytes)[indexH+(indexC)*width];
		((soyPixel32*)bmp->bytes)[indexH+(indexC)*width] = tempPixel;
	  }
	}
  }
  SLOG4("Loaded bitmap: \"%s\" | %d x %d | %.2f MB \n", filepath, width, height, (real64)rawSize/1024/1024);
  return TRUE;
}

soyPixel32 soy_bitmap_sample_uv(soyBitmapHandle bmp, real32 x, real32 y) {
  int32 ix, iy;
  ix = soy_min((int32)(x*(real32)bmp->width), bmp->width-1);
  iy = soy_min((int32)(y*(real32)bmp->height), bmp->width-1);
  return ((soyPixel32*)bmp->bytes)[soy_abs(ix%bmp->width)+soy_abs(iy%bmp->height)*bmp->width];
}

soyPixel32 soy_bitmap_sample(soyBitmapHandle bmp, int32 x, int32 y) {
  return ((soyPixel32*)bmp->bytes)[(x%bmp->width)+(y%bmp->height)*bmp->width];
}

sflag soy_bitmap_create(soyVirtualMemory* lMem, soyVirtualMemory* vMem,
		soyBitmapHandle bmp, cString filepath, cString pixFormat, sflag flipV, sflag flipH) {
  sflag ret = FALSE;
  soy_bitmap_init(bmp);
  ret = soy_bitmap_import(lMem, vMem, bmp, filepath, pixFormat, flipV, flipH);
  return ret;
}

/********************************************/
/* @font                                    */
/* SoyLib Font                              */
/********************************************/

#define SOY_FONT_FRAGMENTS 0xFF

typedef struct tag_soyCharFragment {
  uint8 xCoord;
  uint8 yCoord;
  uint8 col;
  uint8 reserved;
} soyCharFragment;

typedef struct tag_soyCharInfo {
  int8 kerning;
  int8 midline;
  uint16 fragmentCount;
  soyCharFragment fragments[SOY_FONT_FRAGMENTS];
} soyCharInfo;

typedef struct tag_soyFont {
	soyBitmapHandle fontBitmapHandle;
    soyCharInfo charInfo[256];
    real32 globalKerning;
    real32 scale;
    int32 globalVertical;
    int32 grid;
} soyFont;

typedef soyFont* soyFontHandle;

sflag soy_font_set_bitmap(soyFontHandle font, soyBitmapHandle fontAtlas, int32 gridSize) {
  int32 x = 0;
  int32 y = 0;
  int32 kerningIndex = 0;
  int32 midlineIndex = 0;
  int32 subCharX = 0;
  int32 subCharY = 0;
  uint32 fragmentCount = 0;
  int32 charIndex = 0;
  sflag kCol = FALSE;
  soyPixel32 tempPixel;
  if(fontAtlas->bytes == NULL) { font->fontBitmapHandle = NULL; return FALSE; }
  kCol = FALSE;
  kerningIndex = 0;
  charIndex = 0;
  midlineIndex = 0;
  font->fontBitmapHandle = fontAtlas;
  font->grid = gridSize;
  for(y = 0; y < fontAtlas->height; y += gridSize) {
    for(x = 0; x < fontAtlas->width; x += gridSize) {
      kerningIndex = 0;
      midlineIndex = 0;
      tempPixel = ((soyPixel32*)fontAtlas->bytes)[x+kerningIndex+y*fontAtlas->width];
      kCol = FALSE;
      while(!kCol && kerningIndex < gridSize) {
        tempPixel = ((soyPixel32*)fontAtlas->bytes)[x+kerningIndex+y*fontAtlas->width];
        if(tempPixel.r == 255 && tempPixel.g == 255 && tempPixel.b == 0) {
          kCol = TRUE;
        }
        kerningIndex++;
      }
      kCol = FALSE;
      tempPixel = ((soyPixel32*)fontAtlas->bytes)[x+(y+midlineIndex)*fontAtlas->width];
      while(!kCol && kerningIndex < gridSize) {
        if(tempPixel.r == 255 && tempPixel.g == 255 && tempPixel.b == 0) {
          kCol = TRUE;
        }
        tempPixel = ((soyPixel32*)fontAtlas->bytes)[x+(y+midlineIndex)*fontAtlas->width];
        midlineIndex++;
      }
      midlineIndex-=gridSize/2;
      fragmentCount = 0;
      for(subCharY = 0; subCharY < gridSize; subCharY++) {
        for(subCharX = 0; subCharX < gridSize; subCharX++) {
          tempPixel = ((soyPixel32*)fontAtlas->bytes)[x+subCharX+(y+subCharY)*fontAtlas->width];
          if(tempPixel.r == 255 && tempPixel.g == 255 && tempPixel.b == 255) {
            if(fragmentCount < SOY_FONT_FRAGMENTS) {
              font->charInfo[charIndex].fragments[fragmentCount].xCoord = (uint8)subCharX;
              font->charInfo[charIndex].fragments[fragmentCount].yCoord = (uint8)subCharY;
              font->charInfo[charIndex].fragments[fragmentCount].col = tempPixel.r;
              font->charInfo[charIndex].fragments[fragmentCount].reserved = 0;
              fragmentCount++;
            }
          }
        }
      }
      if(charIndex < 255) {
        font->charInfo[charIndex].fragmentCount = fragmentCount;
        font->charInfo[charIndex].kerning = kerningIndex;
        font->charInfo[charIndex].midline = midlineIndex;
      }
      charIndex++;
    }
  }
  return TRUE;
}

sflag soy_font_set_scale(soyFontHandle font, real32 scale) {
  font->scale = scale;
  return TRUE;
}

sflag soy_font_set_vertical(soyFontHandle font, int32 value) {
  font->globalVertical = value;
  return TRUE;
}

sflag soy_font_set_kerning(soyFontHandle font, real32 value) {
  font->globalKerning = value;
  return TRUE;
}

sflag soy_font_set_atlas(soyFontHandle font, soyBitmapHandle fontAtlas, int32 gridSize) {
  if(font == NULL || fontAtlas == NULL) {
    return FALSE;
  }
  font->scale = 1;
  font->globalKerning = 1.0f;
  font->globalVertical = 0.0f;
  soy_font_set_bitmap(font, fontAtlas, gridSize);
  return TRUE;
}

/********************************************/
/* @draw                                    */
/* SoyLib Drawing                           */
/********************************************/

typedef struct tag_soyDrawParams {
  int32 memWidth;
  int32 memHeight;
  int32 memX;
  int32 memY;
  int32 fbWidth;
  int32 fbHeight;
  soyPixel32 curColour;
  soyPixel32 curTint;
  sflag doSubPixel;
  sflag doCorrectTexture;
  sflag debugDraw;
  void* memory;
} soyDrawParams;

sflag soy_draw_init(soyDrawParams* params, void* memory, int32 width, int32 height) {
  params->doSubPixel = TRUE;
  params->doCorrectTexture = TRUE;
  params->memory = memory;
  params->memWidth = 0;
  params->memHeight = 0;
  params->memWidth = width;
  params->memHeight = height;
  params->fbWidth = width;
  params->fbHeight = height;
  params->debugDraw = FALSE;
  params->curColour = soy_pixel32(25, 127, 248, 255);
  params->curTint = soy_pixel32(0, 0, 0, 0);
  return TRUE;
}

sflag soy_draw_update(soyDrawParams* params, int32 width, int32 height, void* memory) {
  params->memWidth = width;
  params->memHeight = height;
  params->fbWidth = width;
  params->fbHeight = height;
  params->memory = memory;
  return TRUE;
}

typedef union uni_simdPixel {
  soyPixel32 raw[4];
  int128 packed;
} simdPixel;

static simdPixel global_simdPixel;

sflag soy_draw_clear(soyDrawParams* params) {
  uint32 index = 0;
  uint32 memLen = params->memWidth*params->memHeight;
  uint32 rest = memLen%4;
  for(index = 0; index < memLen-rest; index+=4) {
    ((simdPixel*)params->memory)[index/4] = global_simdPixel;
  }
  for(; index < memLen; index++) {
    ((soyPixel32*)params->memory)[index] = params->curColour;
  }
  return TRUE;
}

sflag soy_draw_set_colour(soyDrawParams* params,
    uint8 r, uint8 g, uint8 b, uint8 a) {
  params->curColour.r = b;
  params->curColour.g = g;
  params->curColour.b = r;
  params->curColour.a = a;
  global_simdPixel.raw[0] = params->curColour;
  global_simdPixel.raw[1] = params->curColour;
  global_simdPixel.raw[2] = params->curColour;
  global_simdPixel.raw[3] = params->curColour;
  return TRUE;
}

sflag soy_draw_pixel(soyDrawParams* params, int32 x, int32 y) {
  if(x >= params->memX && y >= params->memY && x < params->memWidth && y < params->memHeight) {
	((soyPixel32*)params->memory)[x+y*params->fbWidth] = params->curColour;
  }
  return TRUE;
}

sflag soy_draw_depth_buffer(soyDrawParams* params, real32* depthBuffer, int32 x, int32 y, real32 d) {
  if(x >= params->memX && y >= params->memY && x < params->memWidth && y < params->memHeight) {
    depthBuffer[x+y*params->fbWidth] = d;
  }
  return TRUE;
}

real32 soy_draw_sample_depth_buffer(soyDrawParams* params, real32* depthBuffer, int32 x, int32 y) {
  if(x >= 0 && y >= 0 && x < params->memWidth && y < params->memHeight) {
    return depthBuffer[x+y*params->memWidth];
  }
  return TRUE;
}

sflag soy_draw_clear_depth_buffer(soyDrawParams* params, soyRect* drawRect, real32* depthBuffer) {
  int32 i, j;
  for(i = drawRect->y; i < drawRect->y+drawRect->height; i++) {
    for(j = drawRect->x; j < drawRect->x+drawRect->width; j++) {
        depthBuffer[j+i*params->memWidth] = 0.0f;
    }
  }
  return TRUE;
}

sflag soy_draw_line(soyDrawParams* params, real32 x1, real32 y1, real32 x2, real32 y2) {
  real32 dx, dy, x, y, step;
  uint32 i;
  if(params->memory == NULL) { return FALSE; }
  dx = (x2 - x1);
  dy = (y2 - y1);
  step = soy_fabs(dx) >= soy_fabs(dy) ? soy_fabs(dx) : soy_fabs(dy);
  dx /= step;
  dy /= step;
  x = x1;
  y = y1;
  i = 0;
  while(i <= soy_abs((int32)step)) {
    soy_draw_pixel(params, (int32)(x), (int32)(y));
    x += dx;
    y += dy;
    i += 1;
  }
  return TRUE;
}

sflag soy_draw_ellipse(soyDrawParams* params, int32 xm, int32 ym, int32 a, int32 b) {
  int32 x = -a;
  int32 y = 0;
  int64 e2 = (int64)b*b;
  int64 err = x*(2*e2+x)+e2;
  if(params->memory == NULL) { return FALSE; }
  do {
    soy_draw_pixel(params, xm-x, ym+y);
    soy_draw_pixel(params, xm+x, ym+y);
    soy_draw_pixel(params, xm+x, ym-y);
    soy_draw_pixel(params, xm-x, ym-y);
    e2 = 2*err;
    if(e2 >= (x*2+1)*(int64)b*b) {
      err+= (++x*2+1)*(int64)b*b;
    }
    if(e2 <= (y*2+1)*(int64)a*a) {
      err += (++y*2+1)*(int64)a*a;
    }
  } while (x <= 0);

  while(y++ < b) {
    soy_draw_pixel(params, xm, ym+y);
    soy_draw_pixel(params, xm, ym-y);
  }
  return TRUE;
}

sflag soy_draw_circle_fill(soyDrawParams* params, int32 xm, int32 ym, int32 r) {
  int32 x;
  int32 y;
  for(y = 0; y < r*2; y++) {
    for(x = 0; x < r*2; x++) {
      if(soy_distance_between_points(
            (real32)xm,
            (real32)ym,
            (real32)xm+(x-r),
            (real32)ym+(y-r)) < (real32)r) {
        soy_draw_pixel(params, xm-r+x, ym-r+y);
      }
    }
  }
  return TRUE;
}

sflag soy_draw_rect(soyDrawParams* params, int32 x1, int32 y1, int32 x2, int32 y2) {
  int32 temp;
  if(params->memory == NULL) { return FALSE; }
  if((x1 < params->memWidth*2 && x1 > -params->memWidth) ||
      (y1 < params->memHeight*2 && y1 > -params->memHeight)) {
    if(y1 > y2) {
     temp = y1;
     y1 = y2;
     y2 = temp;
    }
   if(x1 > x2) {
     temp = x1;
     x1 = x2;
     x2 = temp;
   }
   soy_draw_line(params, x1, y1, x2, y1);
   soy_draw_line(params, x1, y2, x2, y2);
   soy_draw_line(params, x1, y1, x1, y2);
   soy_draw_line(params, x2, y1, x2, y2);
  }
  return TRUE;
}

sflag soy_draw_rect_thick(soyDrawParams* params, int32 x1, int32 y1, int32 x2, int32 y2, uint32 thickness) {
  int32 temp;
  if(thickness == 0) { return TRUE; }
  if(params->memory == NULL) { return FALSE; }
  if((x1 < params->memWidth*2 && x1 > -params->memWidth) ||
      (y1 < params->memHeight*2 && y1 > -params->memHeight)) {
    if(y1 > y2) {
     temp = y1;
     y1 = y2;
     y2 = temp;
    }
   if(x1 > x2) {
     temp = x1;
     x1 = x2;
     x2 = temp;
   }
   soy_draw_line(params, x1, y1, x2, y1);
   soy_draw_line(params, x1, y2, x2, y2);
   soy_draw_line(params, x1, y1, x1, y2);
   soy_draw_line(params, x2, y1, x2, y2);
  }
  soy_draw_rect_thick(params, x1+1, y1+1, x2-1, y2-1, thickness-1);
  return TRUE;
}

sflag soy_draw_fill(soyDrawParams* params, int32 x1, int32 y1, int32 x2, int32 y2) {
  int32 i, j;
  int32 temp;
  if(params->memory == NULL) { return FALSE; }
  if(y1 > y2) {
    temp = y1;
    y1 = y2;
    y2 = temp;
  }
  for(i = y1; i < y2; i++) {
    for(j = x1; j < x2; j++) {
      soy_draw_pixel(params, j, i);
    }
  }
  return TRUE;
}

sflag soy_draw_gradient_h(soyDrawParams* params,
    int32 x1, int32 y1, int32 x2, int32 y2, soyPixel32 colA, soyPixel32 colB) {
  int32 i, j;
  vec4f colL;
  int32 temp;
  real32 t = 0.0f;
  if(params->memory == NULL) { return FALSE; }
  if(y1 > y2) {
    temp = y1;
    y1 = y2;
    y2 = temp;
  }
  for(i = y1; i < y2; i++) {
    for(j = x1; j < x2; j++) {
      colL = soy_vec4f_lerp(
              soy_vec4f((real32)colA.r, (real32)colA.g, (real32)colA.b, 255),
              soy_vec4f((real32)colB.r, (real32)colB.g, (real32)colB.b, 255), t);
      params->curColour.r = (uint8)colL.x;
      params->curColour.g = (uint8)colL.y;
      params->curColour.b = (uint8)colL.z;
      soy_draw_pixel(params, j, i);
      t = ((real32)j-(real32)x1)/((real32)x2-(real32)x1);
    }
  }
  return TRUE;
}

sflag soy_draw_gradient_v(soyDrawParams* params,
    int32 x1, int32 y1, int32 x2, int32 y2, soyPixel32 colA, soyPixel32 colB) {
  int32 i, j;
  vec4f colL;
  int32 temp;
  real32 t = 0;
  if(params->memory == NULL) { return FALSE; }
  if(y1 > y2) {
    temp = y1;
    y1 = y2;
    y2 = temp;
  }
  for(i = y1; i < y2; i++) {
    colL = soy_vec4f_lerp(
              soy_vec4f((real32)colA.r, (real32)colA.g, (real32)colA.b, 255),
              soy_vec4f((real32)colB.r, (real32)colB.g, (real32)colB.b, 255), t);
    for(j = x1; j < x2; j++) {  
      params->curColour.r = (uint8)colL.x;
      params->curColour.g = (uint8)colL.y;
      params->curColour.b = (uint8)colL.z;
      soy_draw_pixel(params, j, i);
    }
    t = (real32)(i-y1)/(real32)(y2-y1);
  }
  return TRUE;
}

sflag soy_draw_gradient_c(soyDrawParams* params,
    int32 x1, int32 y1, int32 x2, int32 y2, soyPixel32 colA, soyPixel32 colB) {
  int32 i, j;
  vec4f colL;
  int32 temp;
  real32 t = 0;
  if(params->memory == NULL) { return FALSE; }
  if(y1 > y2) {
    temp = y1;
    y1 = y2;
    y2 = temp;
  }
  for(i = y1; i < y2; i++) {
    for(j = x1; j < x2; j++) {
      colL = soy_vec4f_lerp(
              soy_vec4f((real32)colA.r, (real32)colA.g, (real32)colA.b, 255),
              soy_vec4f((real32)colB.r, (real32)colB.g, (real32)colB.b, 255), t);
      params->curColour.r = (uint8)colL.x;
      params->curColour.g = (uint8)colL.y;
      params->curColour.b = (uint8)colL.z;
      soy_draw_pixel(params, j, i);
      t = (soy_vec4f_length(soy_vec4f((real32)(i-y1)/(y2-y1)-0.5f, (real32)(j-x1)/(x2-x1)-0.5f, 0, 0)));
    }
  }
  return TRUE;
}

sflag soy_draw_fill_pattern(soyDrawParams* params,
    cString pattern, int32 shift, int32 x1, int32 y1, int32 x2, int32 y2) {
  int32 i, j;
  int32 temp;
  int32 shiftD = 0;
  uint32 strLen = soy_cstring_len(pattern);
  if(params->memory == NULL) { return FALSE; }
  if(y1 > y2) {
    temp = y1;
    y1 = y2;
    y2 = temp;
  }
  for(i = y1; i < y2; i++) {
    for(j = x1; j < x2; j++) {
      if(pattern[(j+i*params->memWidth+shiftD)%strLen] != '0') {
        soy_draw_pixel(params, j, i);
      }
    }
    shiftD+=shift;
  }
  return TRUE;
}

#define SPR_SMALL_SIZE 9
#define SPR_MICRO_SIZE 7
const int8* SPR_MICRO_PLUS =
"0000000"
"0000000"
"0001000"
"0011100"
"0001000"
"0000000"
"0000000";
const int8* SPR_MICRO_MINUS =
"0000000"
"0000000"
"0000000"
"0011100"
"0000000"
"0000000"
"0000000";
const int8* SPR_MICRO_X =
"0000000"
"0100010"
"0010100"
"0001000"
"0010100"
"0100010"
"0000000";
const int8* SPR_MICRO_CIRCLE =
"0011100"
"0111110"
"1100011"
"1100011"
"1100011"
"0111110"
"0011100";
const int8* SPR_MICRO_SUS =
"0000000"
"0011100"
"0100010"
"0111110"
"0110110"
"0110110"
"0000000";

const int8* SPR_PLUS =
"111111111"
"100000001"
"100010001"
"100010001"
"101111101"
"100010001"
"100010001"
"100000001"
"111111111";
const int8* SPR_NS =
"111111111"
"100000001"
"101011101"
"101010001"
"101111101"
"100010101"
"101110101"
"100000001"
"111111111";
const int8* SPR_MINUS =
"111111111"
"100000001"
"100000001"
"100000001"
"101111101"
"100000101"
"100000001"
"100000001"
"111111111";
const int8* SPR_DECOR =
"101101111"
"000100101"
"010011011"
"001100101"
"000000100"
"000001011"
"000001001"
"000000100"
"000000001";

sflag soy_draw_mask(soyDrawParams* params, int32 ix, int32 iy, const int8* arr, uint32 sideLen) {
  uint32 x, y;
  for(y = 0; y < sideLen; y++) {
    for(x = 0; x < sideLen; x++) {
      if(arr[x+y*sideLen] != '0') {
        soy_draw_pixel(params, ix+x, iy+y);
      }
    }
  }
  return TRUE;
}

soyPixel32 chromaColour;

sflag soy_draw_sprite(soyDrawParams* params, soyBitmapHandle bmp, int32 x, int32 y,
    int32 cellSizeX, int32 cellSizeY, int32 indexW, int32 indexH) {
  int32 i, j;
  int32 index = 0;
  soyPixel32 col = soy_pixel32(0, 0, 0, 255);
  chromaColour = soy_pixel32(255, 0, 255, 255);
  for(j = 0; j < cellSizeY; j++) {
    if(y+j > params->memHeight) { return TRUE; }
    for(i = 0; i < cellSizeX; i++) {
      if(x+i > params->memWidth) {
        break;
      }
      index = ((i+indexW*cellSizeX)%bmp->width)+(((j+indexH*cellSizeY)%bmp->height)*bmp->width);
      col = ((soyPixel32*)bmp->bytes)[index];
      if(col.b == chromaColour.r && col.g == chromaColour.g && col.r == chromaColour.b) {
      
      } else {
        soy_draw_set_colour(params, col.b, col.g, col.r, 255);
        soy_draw_pixel(params, x+i, y+j);
      }
    }
  }
  return TRUE;
}

sflag soy_draw_background(soyDrawParams* params, soyBitmapHandle bmp) {
  int32 i, j;
  soyPixel32* canvas = ((soyPixel32*)bmp->bytes);
  for(j = 0; j < params->memHeight; j++) {
    for(i = 0; i < params->memWidth; i++) {     
      params->curColour = canvas[((i)%bmp->width)+(((j)%bmp->height)*bmp->width)];
      soy_draw_pixel(params, i, j);
    }
  }
  return TRUE;
}

sflag soy_draw_bitmap32(soyDrawParams* params,
      int32 xPos, int32 yPos,
      void* src,
      int32 bmpWidth, int32 bmpHeight,
      int32 sizeWidth, int32 sizeHeight) {
	uint32 x = 0;
	uint32 y = 0;
    real64 dW, dH;
    soyPixel32 srcCol;
    if(src == NULL) { return FALSE; }
    dW = ((real64)sizeWidth/(real64)bmpWidth);
    dH = ((real64)sizeHeight/(real64)bmpHeight);
    for(y = 0; y < (uint32)bmpHeight*dH; y++) {
      for(x = 0; x < (uint32)bmpWidth*dW; x++) {
        srcCol = ((soyPixel32*)src)[(uint32)(x/dW)+(uint32)(y/dH)*bmpWidth];
        soy_draw_set_colour(params, srcCol.b, srcCol.g, srcCol.r, 255);
        soy_draw_pixel(params, x+xPos, y+yPos);
      }
    }
	return TRUE;
}

sflag soy_draw_char(soyDrawParams* params, real32 ix, real32 iy, const int8 text, soyFontHandle font) {
  int32 subX;
  int32 subY;
  uint32 canvasIndex;
  uint32 bitmapIndex;
  soyPixel32 bitmapPixel;
  int32 x = (int32)(ix+0.5f);
  int32 y = (int32)(iy+0.5f);
  for(subY = 0; subY < (int32)((real32)font->grid*font->scale); subY++) {
    for(subX = 0; subX < (int32)((real32)font->grid*font->scale); subX++) {
      canvasIndex = subX+subY*params->memWidth;
      bitmapIndex = (((uint8)text)%16)*font->grid+(int32)((real32)subX/font->scale) +
                   ((((uint8)text)/16)*font->grid+(int32)((real32)subY/font->scale)) *
                   font->fontBitmapHandle->width;
      if(bitmapIndex < (uint32)font->fontBitmapHandle->width*font->fontBitmapHandle->height) {
        bitmapPixel = ((soyPixel32*)font->fontBitmapHandle->bytes)[bitmapIndex];
        if(bitmapPixel.r == 255 && bitmapPixel.g == 255 && bitmapPixel.b == 255) {
          soy_draw_pixel(params, canvasIndex%params->memWidth+x*font->scale,
                                  canvasIndex/params->memWidth+y*font->scale);
        }
      }
    }
  }
  return TRUE;
}

uint32 soy_draw_text_len(cString text, int32 tlen, soyFontHandle font) {
  int32 strLen = 0;
  int32 index;
  int32 subX;
  int32 subY;
  uint32 canvasIndexX;
  int32 step;
  int32 scaledX;
  int32 scaledY;
  int32 canvX;
  real32 canXDiff;
  int32 nextKerning;
  int8 curChar = 0;
  int8 prevChar = 0;
  step = font->grid;
  canvX = 0;
  scaledY = (int32)((real32)step*font->scale);
  scaledX = (int32)((real32)step*font->scale);
  canXDiff = 0;
  canvasIndexX = 0;
  if(tlen < 0) {
    strLen = soy_cstring_len(text);
  } else {
    if(tlen < 1024) {
      strLen = tlen;
    } else {
      strLen = 1024-1;
    }
  }
  for(index = 0; index < strLen; index++) {
    prevChar = curChar;
    curChar = text[index];
    while(curChar == 0) {
      index++;
      prevChar = curChar;
      curChar = text[index];
      if(index >= strLen) { return FALSE; }
    }
    nextKerning = 0;
    nextKerning = font->charInfo[(uint8)prevChar].kerning;
    if(canvX != 0) {
      canXDiff-=((real32)(step-nextKerning)*font->scale)+font->globalKerning*font->scale;
    } else {
      canXDiff = 0;
    }
    for(subY = 0; subY < scaledY; subY++) {
      for(subX = 0; subX < scaledX; subX++) {
        canvasIndexX = (int32)((real32)subX+canXDiff+canvX*step*font->scale);
      }
    }
    canvX++;
  }
  return canvasIndexX;
}
sflag soy_draw_text_optimized(soyDrawParams* params,
    real32 ix, real32 iy, cString text, soyFontHandle font) {
  uint32 index = 0;
  uint32 xIndex = 0;
  uint32 yIndex = 0;
  uint32 i = 0;
  uint8 curChar = 0;
  soyCharFragment curFrag;
  uint32 strLen = soy_cstring_len(text);
  for(index = 0; index < strLen; index++) {
    curChar = text[index];
    while(curChar == '\n') {
      index++;
      yIndex++;
      xIndex = 0;
      curChar = text[index];  
    }
    if(curChar == 0) { return TRUE; }
    if(curChar == '\t') { curChar = ' '; };
    for(i = 0; i < font->charInfo[curChar].fragmentCount; i++) {
      curFrag = font->charInfo[curChar].fragments[i];
      soy_draw_pixel(params,
          (int32)(ix-0.5f)+(int32)curFrag.xCoord+xIndex*((font->grid-font->globalKerning)*font->scale),
          (int32)(iy-0.5f)+(int32)curFrag.yCoord+yIndex*((font->grid-font->globalVertical)*font->scale)
          );
    }
    xIndex++;
  }
  return TRUE;
}

sflag soy_draw_text_optimized_bold(soyDrawParams* params,
    real32 ix, real32 iy, cString text, soyFontHandle font) {
  uint32 index = 0;
  uint32 xIndex = 0;
  uint32 yIndex = 0;
  uint32 i = 0;
  uint8 curChar = 0;
  soyCharFragment curFrag;
  uint32 strLen = soy_cstring_len(text);
  for(index = 0; index < strLen; index++) {
    curChar = text[index];
    while(curChar == '\n') {
      index++;
      yIndex++;
      xIndex = 0;
      curChar = text[index];  
    }
    if(curChar == 0) { return TRUE; }
    if(curChar == '\t') { curChar = ' '; };
    for(i = 0; i < font->charInfo[curChar].fragmentCount; i++) {
      curFrag = font->charInfo[curChar].fragments[i];
      soy_draw_pixel(params,
          (int32)(ix-0.5f)+(int32)curFrag.xCoord+xIndex*((font->grid-font->globalKerning)*font->scale),
          (int32)(iy-0.5f)+(int32)curFrag.yCoord+yIndex*((font->grid-font->globalVertical)*font->scale)
          );
      soy_draw_pixel(params,
          (int32)(ix+1.0f-0.5f)+(int32)curFrag.xCoord+xIndex*((font->grid-font->globalKerning)*font->scale),
          (int32)(iy-0.5f)+(int32)curFrag.yCoord+yIndex*((font->grid-font->globalVertical)*font->scale)
          );
    }
    xIndex++;
  }
  return TRUE;
}


sflag soy_draw_text(soyDrawParams* params,
    real32 ix, real32 iy, cString text, int32 tlen, soyFontHandle font) {
  int32 strLen = 0;
  int32 index = 0;
  int32 subX = 0;
  int32 subY = 0;
  int32 bitmapIndex = 0;
  int32 scaledX = 0;
  int32 scaledY = 0;
  int32 canvX = 0;
  real32 canXDiff = 0;
  int32 nextKerning = 0;
  soyPixel32 bitmapPixel;
  int8 curChar = 0;
  int8 prevChar = 0;
  int32 x = (int32)(ix+0.5f);
  int32 y = (int32)(iy+0.5f);
  if(font == NULL) { return FALSE; }
  if(params->memory == NULL || font->fontBitmapHandle == NULL) { return FALSE; }
  scaledY = (int32)((real32)font->grid*font->scale);
  scaledX = (int32)((real32)(font->grid)*font->scale);
  
  if(tlen < 0) { strLen = soy_cstring_len(text);
  
  } else {
    if(tlen < 1024) { strLen = tlen;
    } else { strLen = 1024-1; }
  }
  for(index = 0; index < strLen; index++) {
    prevChar = curChar;
    curChar = text[index];
    while(curChar == 0) { return FALSE; }
    while(curChar == '\n') {
      canvX = 0;
      index++;
      curChar = text[index];
      y+=(font->grid-font->globalVertical)*font->scale;
    }
    if(curChar == '\t') { curChar = ' '; }
    nextKerning = 0;
    nextKerning = font->charInfo[(uint8)prevChar].kerning;
    if(canvX != 0) {
      canXDiff-=((real32)(font->grid-nextKerning)*font->scale)+font->globalKerning*font->scale;
    } else {
      canXDiff = 0;
    }

    for(subY = 0; subY < scaledY; subY++) {
      for(subX = 0; subX < scaledX; subX++) {
        bitmapIndex = 
          (((((uint8) curChar)%16)*(font->grid))+(int32)((real32)subX/font->scale)) +
          (((((uint8) curChar)>>4)*font->grid)+(int32)((real32)subY/font->scale)) *
          font->fontBitmapHandle->width;
        if(bitmapIndex < (font->fontBitmapHandle->width*font->fontBitmapHandle->height)) {
          bitmapPixel = ((soyPixel32*)font->fontBitmapHandle->bytes)[bitmapIndex];
          if(bitmapPixel.r == 255 && bitmapPixel.g == 255 && bitmapPixel.b == 255) {
            soy_draw_pixel(params,
                ((int32)((real32)subX+canXDiff+canvX*(font->grid)*font->scale))+x-1,subY+y-1);
          }
        }
      }
    }
    canvX++;
  }
  return TRUE;
}

sflag soy_draw_text_bold(soyDrawParams* params,
    real32 ix, real32 iy, cString text, int32 tlen, soyFontHandle font) {
  int32 strLen = 0;
  int32 index = 0;
  int32 subX = 0;
  int32 subY = 0;
  int32 bitmapIndex = 0;
  int32 scaledX = 0;
  int32 scaledY = 0;
  int32 canvX = 0;
  real32 canXDiff = 0;
  soyPixel32 bitmapPixel;
  int32 nextKerning = 0;
  int8 curChar = 0;
  int8 prevChar = 0;
  int32 x = (int32)(ix+0.5f);
  int32 y = (int32)(iy+0.5f);
  if(font == NULL) { return FALSE; }
  if(params->memory == NULL || font->fontBitmapHandle == NULL) { return FALSE; }
  scaledY = (int32)((real32)font->grid*font->scale);
  scaledX = (int32)((real32)(font->grid-font->globalKerning)*font->scale);
  if(tlen < 0) { strLen = soy_cstring_len(text);
  } else {
    if(tlen < 1024) { strLen = tlen;
    } else { strLen = 1024-1; }
  }
  for(index = 0; index < strLen; index++) {
    prevChar = curChar;
    curChar = text[index];
    while(curChar == 0) { return FALSE; }
    while(curChar == '\n') {
      canvX = 0;
      index++;
      curChar = text[index];
      y+=(font->grid-font->globalVertical)*font->scale;
    }
    if(curChar == '\t') { curChar = ' '; }
    nextKerning = font->charInfo[(uint8)prevChar].kerning;
    if(canvX != 0) {
      canXDiff-=((real32)(font->grid-nextKerning)*font->scale)+font->globalKerning*font->scale;
    } else {
      canXDiff = 0;
    }
    for(subY = 0; subY < scaledY; subY++) {
      for(subX = 0; subX < scaledX; subX++) {
        bitmapIndex = 
          (((((uint8)curChar)%16)*font->grid)+(int32)((real32)subX/font->scale)) +
          (((((uint8)curChar)>>4)*font->grid)+(int32)((real32)subY/font->scale)) *
          font->fontBitmapHandle->width;
        if(bitmapIndex < (font->fontBitmapHandle->width*font->fontBitmapHandle->height)) {
          bitmapPixel = ((soyPixel32*)font->fontBitmapHandle->bytes)[bitmapIndex];
          if(bitmapPixel.r == 255 && bitmapPixel.g == 255 && bitmapPixel.b == 255) {
            soy_draw_pixel(params,
                ((int32)((real32)subX+canXDiff+canvX*font->grid*font->scale))+x-1,subY+y-1);
            soy_draw_pixel(params,
                ((int32)((real32)subX+canXDiff+canvX*font->grid*font->scale))+x,subY+y-1);
          }
        }
      }
    }
    canvX++;
  }
  return TRUE;
}

sflag soy_draw_text_italic(soyDrawParams* params,
    real32 ix, real32 iy, cString text, int32 tlen, soyFontHandle font, real32 amnt) {
  int32 strLen;
  uint8 charIndex;
  uint32 bitmapSize;
  int32 index;
  int32 subX;
  int32 subY;
  uint32 bmpWidth;
  uint32 bitmapIndex;
  int32 step;
  int32 scaledX;
  int32 scaledY;
  int32 bitmapX;
  int32 bitmapY;
  real32 bmpSubX;
  real32 bmpSubY;
  int32 canvX;
  real32 canXDiff;
  int32 nextKerning;
  soyPixel32 bitmapPixel;
  int8 curChar = 0;
  int8 prevChar = 0;
  int32 x = (int32)(ix+0.5f);
  int32 y = (int32)(iy+0.5f);
  real32 yAcc = 0;
  real32 maxYAcc = 0;
  if(font == NULL) { return FALSE; }
  if(params->memory == NULL || font->fontBitmapHandle == NULL) { return FALSE; }
  bmpWidth = font->fontBitmapHandle->width;
  bitmapSize = font->fontBitmapHandle->width*font->fontBitmapHandle->height;
  step = font->grid;
  canvX = 0;
  scaledY = (int32)((real32)step*font->scale);
  scaledX = (int32)((real32)step*font->scale);
  canXDiff = 0;
  strLen = 0;
  if(tlen < 0) {
    strLen = soy_cstring_len(text);
  } else {
    if(tlen < 1024) {
      strLen = tlen;
    } else {
      strLen = 1024-1;
    }
  }
  for(index = 0; index < strLen; index++) {
    prevChar = curChar;
    curChar = text[index];

    while(curChar == 0) {
      index++;
      prevChar = curChar;
      curChar = text[index];
      if(index >= strLen) { return FALSE; }
    }
    charIndex = (uint8) curChar;
    bitmapX = (charIndex%16)*step;
    bitmapY = (charIndex/16)*step;
    nextKerning = 0;
    nextKerning = font->charInfo[(uint8)prevChar].kerning;
    if(canvX != 0) {
      canXDiff-=((real32)(step-nextKerning)*font->scale)+font->globalKerning*font->scale;
    } else {
      canXDiff = 0;
    }
    maxYAcc = amnt*scaledY;
    yAcc = 0;
    for(subY = 0; subY < scaledY; subY++) {
      for(subX = 0; subX < scaledX; subX++) {
        bmpSubX = (real32)subX/font->scale;
        bmpSubY = (real32)subY/font->scale;
        bitmapIndex = bitmapX+(int32)bmpSubX + (bitmapY+(int32)bmpSubY) * bmpWidth;
        if(bitmapIndex < bitmapSize) {
          bitmapPixel = ((soyPixel32*)font->fontBitmapHandle->bytes)[bitmapIndex];
          if(bitmapPixel.r == 255 && bitmapPixel.g == 255 && bitmapPixel.b == 255) {
            soy_draw_pixel(params,
                ((int32)((real32)subX+canXDiff+canvX*step*font->scale))+x+(int32)(maxYAcc-yAcc), subY+y-1);
          }
        }
      }
      yAcc+=amnt;
    }
    canvX++;
  }
  return TRUE;
}

sflag soy_draw_triangle_solid(soyDrawParams* params, real32* depthBuffer,
                                real32 x1, real32 y1, real32 x2, real32 y2, real32 x3, real32 y3,
                                real32 w1, real32 w2, real32 w3, real32 r, real32 g, real32 b) {
  real32 dx1 = 0, dy1 = 0; real32 dw1 = 0;
  real32 dx2 = 0, dy2 = 0; real32 dw2 = 0;
  real32 texW = 0;
  real32 stepDAX = 0, stepDBX = 0, stepDW1 = 0, stepDW2 = 0;
  real32 tStep = 0; real32 t = 0;
  real32 texSW = 0, texEW = 0;
  real32 depth;
  int32 ax = 0, bx = 0;
  int32 i = 0, j = 0;
  soyPixel32 curCol = params->curColour;
  if(y2 < y1) {
    soy_swap_real32(&x1, &x2); soy_swap_real32(&y1, &y2); soy_swap_real32(&w1, &w2);
  }
  if(y3 < y1) {
    soy_swap_real32(&x1, &x3); soy_swap_real32(&y1, &y3); soy_swap_real32(&w1, &w3);
  }
  if(y3 < y2) {
    soy_swap_real32(&x2, &x3); soy_swap_real32(&y2, &y3); soy_swap_real32(&w2, &w3);
  }
  dx1 = x2 - x1; dy1 = y2 - y1; dw1 = w2 - w1;
  dx2 = x3 - x1; dy2 = y3 - y1; dw2 = w3 - w1;
  if(dy1) stepDAX = dx1 / soy_fabs(dy1);
  if(dy2) stepDBX = dx2 / soy_fabs(dy2);
  if(dy1) stepDW1 = dw1 / soy_fabs(dy1);
  if(dy2) stepDW2 = dw2 / soy_fabs(dy2);
  if(dy1) {
    for(i = (int32)y1+1; i <= (int32)y2; i++) {
      ax = (int32)(x1 + (real32)(i - y1) * stepDAX); bx = (int32)(x1 + (real32)(i - y1) * stepDBX);
      texSW = w1 + (real32)(i - y1) * stepDW1;
      texEW = w1 + (real32)(i - y1) * stepDW2;
      if(ax > bx) {
        soy_swap_int32(&ax, &bx);
        soy_swap_real32(&texSW, &texEW);
      }
      texW = texSW;
      tStep = 1.0f/((real32)(bx-ax));
      t = 0.0f;
      for(j = ax; j < bx; j++) {
        texW = (1.0f - t) * texSW + t * texEW;
        depth = soy_draw_sample_depth_buffer(params, depthBuffer, j, i);
        if(texW > depth) {
          curCol.r*=r; curCol.g*=g; curCol.b*=b;
          soy_draw_set_colour(params, curCol.b, curCol.g, curCol.r, curCol.a);
          soy_draw_pixel(params, j, i);
          soy_draw_depth_buffer(params, depthBuffer, j, i, texW);
        }
        t += tStep;
      }
    }
  }
  dx1 = x3 - x2; dy1 = y3 - y2; dw1 = w3 - w2;
  if(dy1) stepDAX = dx1 / soy_fabs(dy1); if(dy2) stepDBX = dx2 / soy_fabs(dy2);
  if(dy1) stepDW1 = dw1 / soy_fabs(dy1);
  if(dy1) {
    for(i = (int32)y2+1; i <= (int32)(y3); i++) {
      ax = (int32)(x2 + (real32)(i - y2) * stepDAX); bx = (int32)(x1 + (real32)(i - y1) * stepDBX);
      texSW = w2 + (real32)(i - y2) * stepDW1;
      texEW = w1 + (real32)(i - y1) * stepDW2;
      if(ax > bx) {
        soy_swap_int32(&ax, &bx);
        soy_swap_real32(&texSW, &texEW);
      }
      texW = texSW;
      tStep = 1.0f/((real32)(bx-ax));
      t = 0.0f;
      for(j = ax; j < bx; j++) {
        texW = (1.0f - t) * texSW + t * texEW;
        depth = soy_draw_sample_depth_buffer(params, depthBuffer, j, i);
        if(texW > depth) {
          curCol.r*=r; curCol.g*=g; curCol.b*=b;
          soy_draw_set_colour(params, curCol.b, curCol.g, curCol.r, curCol.a);
          soy_draw_pixel(params, j, i);
          soy_draw_depth_buffer(params, depthBuffer, j, i, texW);

        }
        t += tStep;
      }
    }
  }
  return TRUE;
}

sflag soy_draw_triangle_textured_int(soyDrawParams* params, soyBitmapHandle texture, real32* depthBuffer,
                                    int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3,
                                    real32 u1, real32 v1, real32 w1, real32 u2, real32 v2, real32 w2,
                                    real32 u3, real32 v3, real32 w3, real32 r, real32 g, real32 b) {
  int32 dx1 = 0, dy1 = 0; real32 du1 = 0, dv1 = 0, dw1 = 0;
  int32 dx2 = 0, dy2 = 0; real32 du2 = 0, dv2 = 0, dw2 = 0;
  real32 texU = 0, texV = 0, texW = 0;
  real32 stepDAX = 0, stepDBX = 0, stepDU1 = 0, stepDV1 = 0, stepDW1 = 0, stepDU2 = 0, stepDV2 = 0, stepDW2 = 0;
  real32 tStep = 0; real32 t = 0;
  real32 texSU = 0, texSV = 0, texSW = 0, texEU = 0, texEV = 0, texEW = 0;
  real32 depth;
  int32 ax = 0, bx = 0;
  int32 i = 0, j = 0;
  soyPixel32 curCol;
  if(y2 < y1) {
    soy_swap_int32(&x1, &x2); soy_swap_int32(&y1, &y2);
    soy_swap_real32(&u1, &u2); soy_swap_real32(&v1, &v2); soy_swap_real32(&w1, &w2);
  }
  if(y3 < y1) {
    soy_swap_int32(&x1, &x3); soy_swap_int32(&y1, &y3);
    soy_swap_real32(&u1, &u3); soy_swap_real32(&v1, &v3); soy_swap_real32(&w1, &w3);
  }
  if(y3 < y2) {
    soy_swap_int32(&x2, &x3); soy_swap_int32(&y2, &y3);
    soy_swap_real32(&u2, &u3); soy_swap_real32(&v2, &v3); soy_swap_real32(&w2, &w3);
  }
  dx1 = x2 - x1; dy1 = y2 - y1; dv1 = v2 - v1; du1 = u2 - u1; dw1 = w2 - w1;
  dx2 = x3 - x1; dy2 = y3 - y1; dv2 = v3 - v1; du2 = u3 - u1; dw2 = w3 - w1;
  if(dy1) stepDAX = dx1 / (real32)soy_abs(dy1);
  if(dy2) stepDBX = dx2 / (real32)soy_abs(dy2);
  if(dy1) stepDU1 = du1 / (real32)soy_abs(dy1);
  if(dy1) stepDV1 = dv1 / (real32)soy_abs(dy1);
  if(dy1) stepDW1 = dw1 / (real32)soy_abs(dy1);
  if(dy2) stepDU2 = du2 / (real32)soy_abs(dy2);
  if(dy2) stepDV2 = dv2 / (real32)soy_abs(dy2);
  if(dy2) stepDW2 = dw2 / (real32)soy_abs(dy2);
  if(dy1) {
    for(i = y1; i <= y2; i++) {
      ax = x1 + (real32)(i - y1) * stepDAX; bx = x1 + (real32)(i - y1) * stepDBX;
      texSU = u1 + (real32)(i - y1) * stepDU1;
      texSV = v1 + (real32)(i - y1) * stepDV1;
      texSW = w1 + (real32)(i - y1) * stepDW1;
      texEU = u1 + (real32)(i - y1) * stepDU2;
      texEV = v1 + (real32)(i - y1) * stepDV2;
      texEW = w1 + (real32)(i - y1) * stepDW2;
      if(ax > bx) {
        soy_swap_int32(&ax, &bx);
        soy_swap_real32(&texSU, &texEU); soy_swap_real32(&texSV, &texEV); soy_swap_real32(&texSW, &texEW);
      }
      texU = texSU; texV = texSV; texW = texSW;
      tStep = 1.0f/((real32)(bx-ax));
      t = 0.0f;
      for(j = ax; j < bx; j++) {
        texU = (1.0f - t) * texSU + t * texEU;
        texV = (1.0f - t) * texSV + t * texEV;
        texW = (1.0f - t) * texSW + t * texEW;

        depth = soy_draw_sample_depth_buffer(params, depthBuffer, j, i);
        if(texW > depth) {
          if(params->doCorrectTexture == TRUE) {
            curCol = soy_bitmap_sample_uv(texture, texU/texW, texV/texW);
          } else {

            curCol = soy_bitmap_sample_uv(texture, texU, texV);

          }
          curCol.r*=r; curCol.g*=g; curCol.b*=b;
          soy_draw_set_colour(params, curCol.b, curCol.g, curCol.r, curCol.a);
          soy_draw_pixel(params, j, i);

          soy_draw_depth_buffer(params, depthBuffer, j, i, texW);

        }
        t += tStep;
      }
    }
  }
  dx1 = x3 - x2; dy1 = y3 - y2; dv1 = v3 - v2; du1 = u3 - u2; dw1 = w3 - w2;
  if(dy1) stepDAX = dx1 / (real32)soy_abs(dy1); if(dy2) stepDBX = dx2 / (real32)soy_abs(dy2);
  stepDU1 = 0; stepDV1 = 0;
  if(dy1) stepDU1 = du1 / (real32)soy_abs(dy1);
  if(dy1) stepDV1 = dv1 / (real32)soy_abs(dy1);
  if(dy1) stepDW1 = dw1 / (real32)soy_abs(dy1);
  if(dy1) {
    for(i = y2+1; i <= y3; i++) {
      ax = x2 + (real32)(i - y2) * stepDAX; bx = x1 + (real32)(i - y1) * stepDBX;
      texSU = u2 + (real32)(i - y2) * stepDU1;
      texSV = v2 + (real32)(i - y2) * stepDV1;
      texSW = w2 + (real32)(i - y2) * stepDW1;
      texEU = u1 + (real32)(i - y1) * stepDU2;
      texEV = v1 + (real32)(i - y1) * stepDV2;
      texEW = w1 + (real32)(i - y1) * stepDW2;
      if(ax > bx) {
        soy_swap_int32(&ax, &bx);
        soy_swap_real32(&texSU, &texEU); soy_swap_real32(&texSV, &texEV); soy_swap_real32(&texSW, &texEW);
      }
      texU = texSU; texV = texSV; texW = texSW;
      tStep = 1.0f/((real32)(bx-ax));
      t = 0.0f;
      for(j = ax; j < bx; j++) {
        texU = (1.0f - t) * texSU + t * texEU;
        texV = (1.0f - t) * texSV + t * texEV;
        texW = (1.0f - t) * texSW + t * texEW;

        depth = soy_draw_sample_depth_buffer(params, depthBuffer, j, i);
        if(texW > depth) {
          if(params->doCorrectTexture == TRUE) {
            curCol = soy_bitmap_sample_uv(texture, texU/texW, texV/texW);
          } else {
            curCol = soy_bitmap_sample_uv(texture, texU, texV);
          }
          curCol.r*=r; curCol.g*=g; curCol.b*=b;
          soy_draw_set_colour(params, curCol.b, curCol.g, curCol.r, curCol.a);
          soy_draw_pixel(params, j, i);
          soy_draw_depth_buffer(params, depthBuffer, j, i, texW);

        }
        t += tStep;
      }
    }
  }
  return TRUE;
}

sflag soy_draw_triangle_textured(soyDrawParams* params, soyBitmapHandle texture, real32* depthBuffer,
                                real32 x1, real32 y1, real32 x2, real32 y2, real32 x3, real32 y3,
                                real32 u1, real32 v1, real32 w1, real32 u2, real32 v2, real32 w2,
                                real32 u3, real32 v3, real32 w3, real32 r, real32 g, real32 b) {
  real32 dx1 = 0, dy1 = 0; real32 du1 = 0, dv1 = 0, dw1 = 0;
  real32 dx2 = 0, dy2 = 0; real32 du2 = 0, dv2 = 0, dw2 = 0;
  real32 texU = 0, texV = 0, texW = 0;
  real32 stepDAX = 0, stepDBX = 0, stepDU1 = 0, stepDV1 = 0, stepDW1 = 0, stepDU2 = 0, stepDV2 = 0, stepDW2 = 0;
  real32 tStep = 0; real32 t = 0;
  real32 texSU = 0, texSV = 0, texSW = 0, texEU = 0, texEV = 0, texEW = 0;
  real32 depth;
  int32 ax = 0, bx = 0;
  int32 i = 0, j = 0;
  soyPixel32 curCol;
  if(y2 < y1) {
    soy_swap_real32(&x1, &x2); soy_swap_real32(&y1, &y2);
    soy_swap_real32(&u1, &u2); soy_swap_real32(&v1, &v2); soy_swap_real32(&w1, &w2);
  }
  if(y3 < y1) {
    soy_swap_real32(&x1, &x3); soy_swap_real32(&y1, &y3);
    soy_swap_real32(&u1, &u3); soy_swap_real32(&v1, &v3); soy_swap_real32(&w1, &w3);
  }
  if(y3 < y2) {
    soy_swap_real32(&x2, &x3); soy_swap_real32(&y2, &y3);
    soy_swap_real32(&u2, &u3); soy_swap_real32(&v2, &v3); soy_swap_real32(&w2, &w3);
  }
  dx1 = x2 - x1; dy1 = y2 - y1; dv1 = v2 - v1; du1 = u2 - u1; dw1 = w2 - w1;
  dx2 = x3 - x1; dy2 = y3 - y1; dv2 = v3 - v1; du2 = u3 - u1; dw2 = w3 - w1;
  if(dy1) stepDAX = dx1 / soy_fabs(dy1);
  if(dy2) stepDBX = dx2 / soy_fabs(dy2);
  if(dy1) stepDU1 = du1 / soy_fabs(dy1);
  if(dy1) stepDV1 = dv1 / soy_fabs(dy1);
  if(dy1) stepDW1 = dw1 / soy_fabs(dy1);
  if(dy2) stepDU2 = du2 / soy_fabs(dy2);
  if(dy2) stepDV2 = dv2 / soy_fabs(dy2); if(dy2) stepDW2 = dw2 / soy_fabs(dy2);
  if(dy1) {
    for(i = (int32)y1+1; i <= (int32)y2; i++) {
      ax = (int32)(x1 + (real32)(i - y1) * stepDAX); bx = (int32)(x1 + (real32)(i - y1) * stepDBX);
      texSU = u1 + (real32)(i - y1) * stepDU1;
      texSV = v1 + (real32)(i - y1) * stepDV1;
      texSW = w1 + (real32)(i - y1) * stepDW1;
      texEU = u1 + (real32)(i - y1) * stepDU2;
      texEV = v1 + (real32)(i - y1) * stepDV2;
      texEW = w1 + (real32)(i - y1) * stepDW2;
      if(ax > bx) {
        soy_swap_int32(&ax, &bx);
        soy_swap_real32(&texSU, &texEU); soy_swap_real32(&texSV, &texEV); soy_swap_real32(&texSW, &texEW);
      }
      texU = texSU; texV = texSV; texW = texSW;
      tStep = 1.0f/((real32)(bx-ax));
      t = 0.0f;
      for(j = ax; j < bx; j++) {
        texU = (1.0f - t) * texSU + t * texEU;
        texV = (1.0f - t) * texSV + t * texEV;
        texW = (1.0f - t) * texSW + t * texEW;

        depth = soy_draw_sample_depth_buffer(params, depthBuffer, j, i);
        if(texW > depth) {
          if(params->doCorrectTexture == TRUE) {
            curCol = soy_bitmap_sample_uv(texture, texU/texW, texV/texW);
          } else {
            curCol = soy_bitmap_sample_uv(texture, texU, texV);
          }
          curCol.r*=r; curCol.g*=g; curCol.b*=b;
          soy_draw_set_colour(params, curCol.b, curCol.g, curCol.r, curCol.a);
          soy_draw_pixel(params, j, i);

          soy_draw_depth_buffer(params, depthBuffer, j, i, texW);

        }

        t += tStep;
      }
    }
  }
  dx1 = x3 - x2; dy1 = y3 - y2; dv1 = v3 - v2; du1 = u3 - u2; dw1 = w3 - w2;
  if(dy1) stepDAX = dx1 / soy_fabs(dy1); if(dy2) stepDBX = dx2 / soy_fabs(dy2);
  stepDU1 = 0; stepDV1 = 0;
  if(dy1) stepDU1 = du1 / soy_fabs(dy1);
  if(dy1) stepDV1 = dv1 / soy_fabs(dy1);
  if(dy1) stepDW1 = dw1 / soy_fabs(dy1);
  if(dy1) {
    for(i = (int32)y2+1; i <= (int32)(y3); i++) {
      ax = (int32)(x2 + (real32)(i - y2) * stepDAX); bx = (int32)(x1 + (real32)(i - y1) * stepDBX);
      texSU = u2 + (real32)(i - y2) * stepDU1;
      texSV = v2 + (real32)(i - y2) * stepDV1;
      texSW = w2 + (real32)(i - y2) * stepDW1;
      texEU = u1 + (real32)(i - y1) * stepDU2;
      texEV = v1 + (real32)(i - y1) * stepDV2;
      texEW = w1 + (real32)(i - y1) * stepDW2;
      if(ax > bx) {
        soy_swap_int32(&ax, &bx);
        soy_swap_real32(&texSU, &texEU); soy_swap_real32(&texSV, &texEV); soy_swap_real32(&texSW, &texEW);
      }
      texU = texSU; texV = texSV; texW = texSW;
      tStep = 1.0f/((real32)(bx-ax));
      t = 0.0f;
      for(j = ax; j < bx; j++) {
        texU = (1.0f - t) * texSU + t * texEU;
        texV = (1.0f - t) * texSV + t * texEV;
        texW = (1.0f - t) * texSW + t * texEW;

        depth = soy_draw_sample_depth_buffer(params, depthBuffer, j, i);
        if(texW > depth) {
          if(params->doCorrectTexture == TRUE) {
            curCol = soy_bitmap_sample_uv(texture, texU/texW, texV/texW);
          } else {
            curCol = soy_bitmap_sample_uv(texture, texU, texV);
          }
          curCol.r*=r; curCol.g*=g; curCol.b*=b;
          soy_draw_set_colour(params, curCol.b, curCol.g, curCol.r, curCol.a);
          soy_draw_pixel(params, j, i);
          soy_draw_depth_buffer(params, depthBuffer, j, i, texW);

        }
        t += tStep;
      }
    }
  }
  /*
  params->drawCb(16);
  */
  return TRUE;
}

sflag soy_draw_triangle_filled(soyDrawParams* params,
    real32 x1, real32 y1, real32 x2, real32 y2, real32 x3, real32 y3) {
  real32 px1, px2;
  real32 stepX1, stepX2, stepX3, stepX4, stepX5;
  real32 dx1, dy1, dx2, dy2, dx3, dy3;
  real32 midX;
  int32 midY, botY, topY;
  int32 i;
  int32 spanX1, spanX2;
  sflag leftBend = FALSE;
  if(params->memory == NULL) { return FALSE; }
  /* order verts by y coord first */
  if(y1 > y2) { soy_swap_real32(&y1, &y2); soy_swap_real32(&x1, &x2); }
  if(y1 > y3) { soy_swap_real32(&y1, &y3); soy_swap_real32(&x1, &x3); }
  if(y2 > y3) { soy_swap_real32(&y2, &y3); soy_swap_real32(&x2, &x3); }
  topY = (int32)y1;
  midY = (int32)y2;
  botY = (int32)y3;
  midX = (x1 + ((real32)(y2 - y1) / (real32)(y3 - y1)) * (x3 - x1));
  if(x2 > midX) { leftBend = TRUE; }
  /* order verts by x coord second */
  if(leftBend) { soy_swap_real32(&x2, &x3); soy_swap_real32(&y2, &y3); }

  /* calculate slopes */
  dx1 = (x2 - x1); dy1 = (y2 - y1);
  dx2 = (x3 - x1); dy2 = (y3 - y1);
  dx3 = (x3 - x2); dy3 = (y3 - y2);
  stepX1 = dx1/dy1;
  stepX2 = dx2/dy2;
  stepX3 = dx3/dy3;

  /* draw top part */
  if(y2 >= y3) { stepX4 = stepX1; stepX5 = stepX3; } else { stepX4 = stepX3; stepX5 = stepX2; }
  px1 = x1;
  px2 = x1;
  i = topY;
  while(i < midY) {
    spanX1 = (int32)px1;
    spanX2 = (int32)px2;
    while(spanX1 < spanX2) {
      soy_draw_pixel(params, spanX1, i);
      spanX1++;
    }
    px1 += stepX1;
    px2 += stepX2;
    i++;
  }

  /* draw bottom part */
  if(y2 >= y3) {
    px2 = x3;
  }  else {
    px1 = x2;
  }
  i = midY;
  while(i < botY) {
    spanX1 = (int32)px1;
    spanX2 = (int32)px2;
    while(spanX1 < spanX2) {
      soy_draw_pixel(params, spanX1, i);
      spanX1++;
    }
    px1 += stepX4;
    px2 += stepX5;
    i++;
  }
  return TRUE;
}

/********************************************/
/* @drawgui                                 */
/* SoyLib GUI Drawing Helpers               */
/********************************************/

typedef struct tag_soyGUIPalette {
  soyPixel32 colours[32];
  soyPixel32 orig[32];
  soyFileTime ft;
  soyFileTime ftPrev;
  cString filename;
} soyGUIPalette;

typedef enum enum_guiColour {
  SOY_GUI_LIGHTEST = 0,
  SOY_GUI_LIGHT = 1,
  SOY_GUI_NEUTRAL = 2,
  SOY_GUI_DARK = 3,
  SOY_GUI_DARKEST = 4,
  SOY_GUI_GRADSTARTACTIVE = 5,
  SOY_GUI_GRADENDACTIVE = 6,
  SOY_GUI_GRADSTARTINACTIVE = 7,
  SOY_GUI_GRADENDINACTIVE = 8,
  SOY_GUI_TEXTACTIVE = 9,
  SOY_GUI_TEXTINACTIVE = 10,
  SOY_GUI_BACKGROUND = 11,
  SOY_GUI_HIGHLIGHT = 12
} guiColour;

guiColour soy_draw_convert_colour_to_index(soyPixel32 pixel) {
  return (pixel.b == 255 && pixel.r == 0 && pixel.g == 255) ? (SOY_GUI_BACKGROUND) : (4-pixel.r/63);
}

sflag soy_draw_load_gui_palette(soyGUIPalette* p, cString filename) {
  soyFile* file;
  uint32 i;
  int32 r, g, b;
  if(filename) {
    file = soy_file_open(filename, "r");
    if(file == NULL) { SLOG1("Could not open %s\n", filename); }
  } else {
    file = NULL;
  }
  if(file == NULL) {
    p->colours[SOY_GUI_LIGHTEST] = soy_pixel32(255, 255, 255, 255);
    p->colours[SOY_GUI_LIGHT] = soy_pixel32(223, 223, 223, 255);
    p->colours[SOY_GUI_NEUTRAL] = soy_pixel32(192, 192, 192, 255);
    p->colours[SOY_GUI_DARK] = soy_pixel32(128, 128, 128, 255);
    p->colours[SOY_GUI_DARKEST] = soy_pixel32(0, 0, 0, 255);
    p->colours[SOY_GUI_GRADSTARTACTIVE] = soy_pixel32(0, 0, 100, 255);
    p->colours[SOY_GUI_GRADENDACTIVE] = soy_pixel32(164, 204, 244, 255);
    p->colours[SOY_GUI_GRADSTARTINACTIVE] = soy_pixel32(128, 128, 128, 255);
    p->colours[SOY_GUI_GRADENDINACTIVE] = soy_pixel32(192, 192, 192, 255);
    p->colours[SOY_GUI_TEXTACTIVE] = soy_pixel32(0, 0, 0, 255);
    p->colours[SOY_GUI_TEXTINACTIVE] = soy_pixel32(212, 208, 200, 255);
    p->colours[SOY_GUI_BACKGROUND] = soy_pixel32(255, 255, 255, 255);
    p->colours[SOY_GUI_HIGHLIGHT] = soy_pixel32(0, 128, 216, 255);
    for(i = 0; i < 13; i++) {
      p->orig[i] = p->colours[i];
    }
    p->filename = NULL;
  } else {
    for(i = 0; i < 13; i++) {
      fscanf(file, "%d %d %d", &r, &g, &b);
      p->colours[i] = soy_pixel32(b, g, r, 255);
      p->orig[i] = p->colours[i];
    }
    p->filename = filename;
  }
  soy_file_close(file);
  return TRUE;
}

sflag soy_draw_gui_palette_reload_if_changed(soyGUIPalette* p, cString filename) {
  p->ft = soy_file_last_write_time((int8*)filename);
  if(!soy_file_time_equal(p->ft, p->ftPrev)) {
    soy_draw_load_gui_palette(p, filename);
  }
  p->ftPrev = p->ft;
  return TRUE;
}

sflag soy_draw_set_tint(soyDrawParams* p,
    real32 r, real32 g, real32 b) {
  p->curColour.r = (uint8)((real32)p->curColour.r*r);
  p->curColour.g = (uint8)((real32)p->curColour.g*g);
  p->curColour.b = (uint8)((real32)p->curColour.b*b);
  return TRUE;
}

sflag soy_draw_set_emission(soyDrawParams* p,
    real32 r, real32 g, real32 b) {
  int32 colR;
  int32 colG;
  int32 colB;
  colR = (uint32)((real32)p->curColour.r*(1.0f+r));
  if(colR > 255) { colR = 255; }
  if(colR < 0) { colR = 0; }
  colG = (uint32)((real32)p->curColour.g*(1.0f+g));
  if(colG > 255) { colG = 255; }
  if(colG < 0) { colG = 0; }
  colB = (uint32)((real32)p->curColour.b*(1.0f+b));
  if(colB > 255) { colB = 255; }
  if(colB < 0) { colB = 0; }
  p->curColour.r = colR;
  p->curColour.g = colG;
  p->curColour.b = colB;
  return TRUE;
}

sflag soy_draw_set_gui_tint(soyGUIPalette* p,
    real32 r, real32 g, real32 b) {
  uint32 i;
  for(i = 0; i < 13; i++) {
    p->colours[i].r = (uint8)((real32)p->orig[i].r*r);
    p->colours[i].g = (uint8)((real32)p->orig[i].g*g);
    p->colours[i].b = (uint8)((real32)p->orig[i].b*b);
  }
  return TRUE;
}

sflag soy_draw_set_gui_emission(soyGUIPalette* p,
    real32 r, real32 g, real32 b) {
  uint32 i;
  int32 colR;
  int32 colG;
  int32 colB;
  for(i = 0; i < 13; i++) {
    colR = (uint32)((real32)p->orig[i].r*(1.0f+r));
    if(colR > 255) { colR = 255; }
    if(colR < 0) { colR = 0; }
    colG = (uint32)((real32)p->orig[i].g*(1.0f+g));
    if(colG > 255) { colG = 255; }
    if(colG < 0) { colG = 0; }
    colB = (uint32)((real32)p->orig[i].b*(1.0f+b));
    if(colB > 255) { colB = 255; }
    if(colB < 0) { colB = 0; }
    p->colours[i].r = colR;
    p->colours[i].g = colG;
    p->colours[i].b = colB;
  }
  return TRUE;
}


sflag soy_draw_reset_gui_tint(soyGUIPalette* p) {
  uint32 i;
  for(i = 0; i < 13; i++) {
    p->colours[i].r = p->orig[i].r;
    p->colours[i].g = p->orig[i].g;
    p->colours[i].b = p->orig[i].b;
  }
  return TRUE;
}

sflag soy_draw_gui_border_outer(soyDrawParams* params,
    int32 x1, int32 y1, int32 x2, int32 y2, soyGUIPalette p) {
  int32 width = 0;
  int32 height = 0;
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  width = x2 - x1;
  height = y2 - y1;
  params->curColour = p.colours[SOY_GUI_LIGHTEST];
  soy_draw_line(params, x1+1, y1+1, x1+width-1, y1+1);
  soy_draw_line(params, x1+1, y1+1, x1+1, y1+height-1);
  params->curColour = p.colours[SOY_GUI_DARK];
  soy_draw_line(params, x1+1, y1+height-1, x1+width-1, y1+height-1);
  soy_draw_line(params, x1+width-1, y1+1, x1+width-1, y1+height-1);
  params->curColour = p.colours[SOY_GUI_LIGHT];
  soy_draw_line(params, x1, y1, x1+width, y1);
  soy_draw_line(params, x1, y1, x1, y1+height);
  params->curColour = p.colours[SOY_GUI_DARKEST];
  soy_draw_line(params, x1, y1+height, x1+width, y1+height);
  soy_draw_line(params, x1+width, y1, x1+width, y1+height);
  return TRUE;
}

sflag soy_draw_gui_border_inner(soyDrawParams* params,
    int32 x1, int32 y1, int32 x2, int32 y2, soyGUIPalette p) {
  int32 width = 0;
  int32 height = 0;
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  width = x2 - x1;
  height = y2 - y1;
  params->curColour = p.colours[SOY_GUI_DARKEST];
  soy_draw_line(params, x1+1, y1+1, x1+width-1, y1+1);
  soy_draw_line(params, x1+1, y1+1, x1+1, y1+height-1);
  params->curColour = p.colours[SOY_GUI_LIGHT];
  soy_draw_line(params, x1+1, y1+height-1, x1+width-1, y1+height-1);
  soy_draw_line(params, x1+width-1, y1+1, x1+width-1, y1+height-1);
  params->curColour = p.colours[SOY_GUI_DARK];
  soy_draw_line(params, x1, y1, x1+width, y1);
  soy_draw_line(params, x1, y1, x1, y1+height);
  params->curColour = p.colours[SOY_GUI_LIGHTEST];
  soy_draw_line(params, x1, y1+height, x1+width, y1+height);
  soy_draw_line(params, x1+width, y1, x1+width, y1+height);
  return TRUE;
}

sflag soy_draw_gui_rect_outer(soyDrawParams* params,
    int32 x1, int32 y1, int32 x2, int32 y2, soyGUIPalette p) {
  int32 width = 0;
  int32 height = 0;
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  width = x2 - x1;
  height = y2 - y1;
  params->curColour = p.colours[SOY_GUI_LIGHTEST];
  soy_draw_line(params, x1, y1, x1+width, y1);
  soy_draw_line(params, x1, y1, x1, y1+height);
  params->curColour = p.colours[SOY_GUI_DARKEST];
  soy_draw_line(params, x1, y1+height, x1+width, y1+height);
  soy_draw_line(params, x1+width, y1, x1+width, y1+height);
  return TRUE;
}

sflag soy_draw_gui_separator_top(soyDrawParams* params,
    int32 x1, int32 y1, int32 x2, int32 y2, soyGUIPalette p) {
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  params->curColour = p.colours[SOY_GUI_LIGHT];
  soy_draw_line(params, x1, y1, x2, y2);
  params->curColour = p.colours[SOY_GUI_DARK];
  soy_draw_line(params, x1, y1+1, x2, y2+1);
  return TRUE;
}

sflag soy_draw_gui_separator_bottom(soyDrawParams* params,
    int32 x1, int32 y1, int32 x2, int32 y2, soyGUIPalette p) {
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  params->curColour = p.colours[SOY_GUI_DARK];
  soy_draw_line(params, x1, y1, x2, y2);
  params->curColour = p.colours[SOY_GUI_LIGHT];
  soy_draw_line(params, x1, y1+1, x2, y2+1);
  return TRUE;
}

sflag soy_draw_gui_rect_inner(soyDrawParams* params,
    int32 x1, int32 y1, int32 x2, int32 y2, soyGUIPalette p) {
  int32 width = 0;
  int32 height = 0;
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  if(x2 < x1) { soy_swap_int32(&x2, &x1); }
  width = x2 - x1;
  height = y2 - y1;
  params->curColour = p.colours[SOY_GUI_DARK];
  soy_draw_line(params, x1, y1, x1+width, y1);
  soy_draw_line(params, x1, y1, x1, y1+height);
  params->curColour = p.colours[SOY_GUI_LIGHTEST];
  soy_draw_line(params, x1, y1+height, x1+width, y1+height);
  soy_draw_line(params, x1+width, y1, x1+width, y1+height);
  return TRUE;
}

sflag soy_draw_gui_panel(soyDrawParams* params,
    int32 x, int32 y, int32 width, int32 height, soyGUIPalette p) {
  soyPixel32 prevCol = params->curColour;
  params->curColour = p.colours[SOY_GUI_NEUTRAL];
  soy_draw_fill(params, x, y, x+width, y+height);
  soy_draw_gui_border_outer(params, x, y, x+width, y+height, p);
  params->curColour = prevCol;
  return TRUE;
}

sflag soy_draw_gui_panel_hover(soyDrawParams* params,
    int32 x, int32 y, int32 width, int32 height, soyGUIPalette p) {
  soyPixel32 prevCol = params->curColour;
  params->curColour = p.colours[SOY_GUI_LIGHT];
  soy_draw_fill(params, x, y, x+width, y+height);
  soy_draw_gui_border_outer(params, x, y, x+width, y+height, p);
  params->curColour = prevCol;
  return TRUE;
}

sflag soy_draw_gui_panel_press(soyDrawParams* params,
    int32 x, int32 y, int32 width, int32 height, soyGUIPalette p) {
  soyPixel32 prevCol = params->curColour;
  params->curColour = p.colours[SOY_GUI_DARK];
  soy_draw_fill(params, x, y, x+width, y+height);
  soy_draw_gui_border_inner(params, x, y, x+width, y+height, p);
  params->curColour = prevCol;
  return TRUE;
}

sflag soy_draw_gui_canvas(soyDrawParams* params,
    int32 x, int32 y, int32 width, int32 height, soyGUIPalette p) {
  soyPixel32 prevCol = params->curColour;
  params->curColour = p.colours[SOY_GUI_BACKGROUND];
  soy_draw_fill(params, x, y, x+width, y+height);
  soy_draw_gui_border_inner(params, x, y, x+width, y+height, p);
  params->curColour = prevCol;
  return TRUE;
}

/********************************************/
/* @3D                                      */
/* SoyLib 3D Interface                      */
/********************************************/

typedef struct tag_soyTri {
  vec4f v1, v2, v3;
  vec4f n1, n2, n3;
  vec4f c1, c2, c3;
  texCoord t1, t2, t3;
} soyTri;

typedef struct tag_soyMesh {
  cString name;
  int32 triCount;
  int32 triCacheCount;
  sflag initialized;
  soyTri* tris;
  soyTri* triCache;
} soyMesh;

typedef struct tag_soyModel {
  cString filename;
  int32 ID;
  int32 fileSize;
  sflag alive;
  soyMesh mesh;
} soyModel;

typedef soyModel* soyModelHandle;

typedef struct tag_soyCamera {
  vec3f position;
  vec3f rotation;
  mat4f mat;
  int32 width;
  int32 height;
  real32 fieldOfView;
  real32 aspectRatio;
  real32 nearClip;
  real32 farClip;
} soyCamera;

static int32 soy3D_global_triCount = 0;

typedef enum enum_soyRenderMode {
  SOY_3D_TEXTURED = 0x0001,
  SOY_3D_LINES = 0x0010,
  SOY_3D_SOLID = 0x0100
} soyRenderMode;

typedef struct tag_soyTriQueue {
  soyVirtualMemory* triMem;
  uint32 back;
  uint32 front;
} soyTriQueue;

typedef struct tag_soy3DParams {
  soyCamera mainCamera;
  soyRenderMode mode;
  soyTriQueue triQueue;
  soyRect drawRect;
} soy3DParams;


soyTri soy_tri(real32 x, real32 y, real32 z) {
  soyTri tri;
  tri.v1 = soy_vec4f(x, y+0.5f, z, 1.0f);
  tri.v2 = soy_vec4f(x-0.5f, y-0.5f, z, 1.0f);
  tri.v3 = soy_vec4f(x+0.5f, y-0.5f, z, 1.0f);
  tri.n1 = soy_vec4f(0.0f, 0.0f, 1.0f, 1.0f);
  tri.n2 = soy_vec4f(0.0f, 0.0f, 1.0f, 1.0f);
  tri.n3 = soy_vec4f(0.0f, 0.0f, 1.0f, 1.0f);
  tri.t1 = soy_texCoord(0.5f, 0.5f, 1.0f);
  tri.t2 = soy_texCoord(0.0f, 1.0f, 1.0f);
  tri.t3 = soy_texCoord(1.0f, 0.0f, 1.0f);
  tri.c1 = soy_vec4f(0, 0, 0, 0);
  tri.c2 = soy_vec4f(0, 0, 0, 0);
  tri.c3 = soy_vec4f(0, 0, 0, 0);
  return tri;
}

sflag soy_tri_queue_init(soyTriQueue* q) {
  q->front = 0;
  q->back = 0;
  return TRUE;
}

sflag soy_tri_enqueue(soyTriQueue* q, soyTri* tri) {
  soyTri* tris = soy_memory_of_type(soyTri, q->triMem);
  if(q->back*sizeof(soyTri) >= q->triMem->size) { SLOG("Out of tri memory\n"); return FALSE; }
  tris[q->back] = *tri;
  q->back++;
  return TRUE;
}

soyTri soy_tri_dequeue(soyTriQueue* q) {
  soyTri* tris = soy_memory_of_type(soyTri, q->triMem);
  if(q->front >= q->back) { q->front = q->back-1; }
  return tris[q->front++];
}

soyCamera soy_3d_camera(int32 width, int32 height) {
  soyCamera camera;
  camera.nearClip = 0.1f;
  camera.farClip = 1000.0f;
  camera.fieldOfView = 45.0f;
  camera.width = width;
  camera.height = height;
  camera.aspectRatio = (real32)height/(real32)width;
  camera.position = soy_vec3f(0,0,0);
  camera.rotation = soy_vec3f(0,0,0);
  camera.mat = soy_mat4f_projection(camera.nearClip, camera.farClip, camera.fieldOfView, camera.aspectRatio);
  return camera;
}

sflag soy_3d_init(soy3DParams* params,
    soyVirtualMemory* cacheMem, uint32 width, uint32 height) {
  params->triQueue.triMem = cacheMem;
  params->mainCamera = soy_3d_camera(width, height);
  params->mode = SOY_3D_TEXTURED;
  params->drawRect = soy_rect("draw3d", 0, 0, width, height, NULL, NULL, NULL);
  return TRUE;
}

sflag soy_3d_change_mode(soy3DParams* params, soyRenderMode mode) {
  params->mode = mode;
  return TRUE;
}

sflag soy_3d_free_mesh(soyMesh* m) {
  soy_free(m->tris);
  soy_free(m->triCache);
  m->triCount = 0;
  m->name = "[deleted]";
  m->initialized = FALSE;
  return TRUE;
}

sflag soy_3d_import_obj(soyVirtualMemory* loadMem, soyVirtualMemory* meshMem,
    soyMesh* destMesh, cString filename) {
  FILE* file = NULL;
  int8 lineHeader[512];
  int32 res;
  int32 vertCount = 0;
  int32 uvCount = 0;
  int32 normalCount = 0;
  int32 triCount = 0;
  int32 i;
  vec4f* tempVerts = NULL;
  texCoord* tempUVs = NULL;
  vec4f* tempNormals = NULL;
  vec3i* tempVertIndices = NULL;
  vec3i* tempNormalIndices = NULL;
  vec3i* tempUVIndices = NULL;
  destMesh->initialized = FALSE;

  file = soy_file_open(filename, "r");

  if(file == NULL) {
    SLOG1("Couldn't find file %s\n", filename);
    return FALSE;
  }

  while(TRUE) {
    res = fscanf(file, "%s", lineHeader);
    if(res == EOF) {
      break;
    }
    if(strcmp(lineHeader, "v") == 0) {
      vertCount++;
    } else if(strcmp(lineHeader, "vt") == 0) {
      uvCount++;
    } else if(strcmp(lineHeader, "vn") == 0) {
      normalCount++;
    } else if(strcmp(lineHeader, "f") == 0) {
      triCount++;
    }
  }
  rewind(file);
  loadMem->back = 0;
  destMesh->triCount = triCount;
  destMesh->triCacheCount = triCount;
  if(meshMem->back + triCount*sizeof(soyTri)*2 > meshMem->size) {
    SLOG("Not enough memory to store model\n");
    return FALSE;
  }

  if(triCount*sizeof(vec3i)*3 +
      vertCount*sizeof(vec4f) +
      normalCount*sizeof(vec4f) +
      uvCount*sizeof(texCoord) > loadMem->size) {
    SLOG("Not enough memory to load model\n");
    return FALSE;
  }

  destMesh->tris = (soyTri*)(meshMem->memory+meshMem->back);
  meshMem->back += triCount*sizeof(soyTri);
  destMesh->triCache = (soyTri*)(meshMem->memory+meshMem->back);
  meshMem->back += triCount*sizeof(soyTri);

  tempVerts = (vec4f*)(loadMem->memory+loadMem->back);
  loadMem->back+=sizeof(vec4f)*vertCount;
  tempVertIndices = (vec3i*)(loadMem->memory+loadMem->back);
  loadMem->back+=sizeof(vec3i)*triCount;

  if(normalCount) {
    tempNormals = (vec4f*)(loadMem->memory+loadMem->back);
    loadMem->back+=sizeof(vec4f)*normalCount;
    tempNormalIndices = (vec3i*)(loadMem->memory+loadMem->back);
    loadMem->back+=sizeof(vec3i)*triCount;
  }
  if(uvCount) {
    tempUVs = (texCoord*)(loadMem->memory+loadMem->back);
    loadMem->back+=sizeof(texCoord)*uvCount;
    tempUVIndices = (vec3i*)(loadMem->memory+loadMem->back);
    loadMem->back+=sizeof(vec3i)*triCount;
  }
  vertCount = 0;
  uvCount = 0;
  normalCount = 0;
  triCount = 0;

  while(TRUE) {
    res = fscanf(file, "%s", lineHeader);
    if(res == EOF) {
      break;
    }
    if(strcmp(lineHeader, "v") == 0) {
      fscanf(file, "%f %f %f\n", &tempVerts[vertCount].x, &tempVerts[vertCount].y, &tempVerts[vertCount].z);
      vertCount++;
    } else if(strcmp(lineHeader, "vt") == 0) {
      fscanf(file, "%f %f\n", &tempUVs[uvCount].u, &tempUVs[uvCount].v);
      uvCount++;
    } else if(strcmp(lineHeader, "vn") == 0) {
      fscanf(file, "%f %f %f\n",
          &tempNormals[normalCount].x,
          &tempNormals[normalCount].y,
          &tempNormals[normalCount].z);
      normalCount++;
    } else if(strcmp(lineHeader, "f") == 0) {
      if( uvCount > 0 && normalCount > 0 ) {
        fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
          &tempVertIndices[triCount].x,
          &tempUVIndices[triCount].x,
          &tempNormalIndices[triCount].x,

          &tempVertIndices[triCount].y,
          &tempUVIndices[triCount].y,
          &tempNormalIndices[triCount].y,

          &tempVertIndices[triCount].z,
          &tempUVIndices[triCount].z,
          &tempNormalIndices[triCount].z
          );
      } else if(uvCount == 0 && normalCount > 0) {
        fscanf(file, "%d//%d %d//%d %d//%d\n",
          &tempVertIndices[triCount].x,
          &tempNormalIndices[triCount].x,

          &tempVertIndices[triCount].y,
          &tempNormalIndices[triCount].y,

          &tempVertIndices[triCount].z,
          &tempNormalIndices[triCount].z
          );
      } else if(uvCount > 0 && normalCount == 0) {
         fscanf(file, "%d/%d %d/%d %d/%d\n",
          &tempVertIndices[triCount].x,
          &tempUVIndices[triCount].x,

          &tempVertIndices[triCount].y,
          &tempUVIndices[triCount].y,

          &tempVertIndices[triCount].z,
          &tempUVIndices[triCount].z
          );
      } else if(uvCount == 0 && normalCount == 0) {
         fscanf(file, "%d %d %d\n",
          &tempVertIndices[triCount].x,
          &tempVertIndices[triCount].y,
          &tempVertIndices[triCount].z
          );
      }
      triCount++;
    }
  }

  /* decrement each index because n- */
  for(i = 0; i < triCount; i++) {
    destMesh->tris[i].v1 = tempVerts[tempVertIndices[i].x-1];
    destMesh->tris[i].v2 = tempVerts[tempVertIndices[i].y-1];
    destMesh->tris[i].v3 = tempVerts[tempVertIndices[i].z-1];
    destMesh->tris[i].c1 = soy_vec4f(255, 0, 0, 255);
    destMesh->tris[i].c2 = soy_vec4f(0, 255, 0, 255);
    destMesh->tris[i].c3 = soy_vec4f(0, 0, 255, 255);
    if(normalCount) {
      destMesh->tris[i].n1 = tempNormals[tempNormalIndices[i].x-1];
      destMesh->tris[i].n2 = tempNormals[tempNormalIndices[i].y-1];
      destMesh->tris[i].n3 = tempNormals[tempNormalIndices[i].z-1];
    }
    if(uvCount) {
      destMesh->tris[i].t1 = tempUVs[tempUVIndices[i].x-1];
      destMesh->tris[i].t2 = tempUVs[tempUVIndices[i].y-1];
      destMesh->tris[i].t3 = tempUVs[tempUVIndices[i].z-1];
    }
  }
  destMesh->initialized = TRUE;
  soy_file_close(file);
  SLOG1("Loaded asset: %s\n", filename);
  return TRUE;
}

static int64 modelIDs = 0;

sflag soy_3d_create_model(soyModelHandle model, soyVirtualMemory* loadMem, soyVirtualMemory* meshMem,
    cString filename, cString name) {
  cString defaultName = "Null";
  FILE* file = NULL;
  int32 filesize = 0;
  modelIDs++;
  model->ID = modelIDs;
  if(model == NULL) {
    return FALSE;
  }
  if(filename != NULL) {
    soy_3d_import_obj(loadMem, meshMem, &model->mesh, filename);
  } else {
    SLOG1("Could not create model: %s\n", filename);
    return FALSE;
  }
  if(name == NULL) {
    name = defaultName;
  } else {
    model->mesh.name = name;
  }
  file = soy_file_open(filename, "r");
  fseek(file, 0L, SEEK_END);
  filesize = ftell(file);
  soy_file_close(file);
  model->alive = TRUE;
  model->fileSize = filesize;

  return TRUE;
}

sflag soy_3d_free_model(soyModelHandle model) {
  soy_3d_free_mesh(&model->mesh);
  model->alive = FALSE;
  return TRUE;
}
/* returns point where line interescts plane */
vec4f soy_vec4f_intersect_plane(vec4f point, vec4f normal, vec4f lineStart, vec4f lineEnd, real32* t) {
  vec4f line, intersect;
  real32 tf = *t;
  vec4f pNorm = soy_vec4f_normalize(normal);
  real32 planeDp = -soy_vec4f_dot_product(pNorm, point);
  real32 ad = soy_vec4f_dot_product(lineStart, pNorm);
  real32 bd = soy_vec4f_dot_product(lineEnd, pNorm);
  tf = (-planeDp - ad) / (bd - ad);
  *t = tf;
  line = soy_vec4f_subtract(lineEnd, lineStart);
  intersect = soy_vec4f_multiply_scalar(line, tf);
  return soy_vec4f_add(lineStart, intersect);
}

real32 soy_point_to_plane_dist(vec4f pNormal, vec4f pPoint, vec4f p) {
  return (pNormal.x * p.x + pNormal.y * p.y + pNormal.z * p.z - soy_vec4f_dot_product(pNormal, pPoint));
}

int32 soy_triangle_clip_against_plane(vec4f pPoint, vec4f pNormal, soyTri* inTri,
                                       soyTri* out1, soyTri* out2) {
  int32 insidePointCount = 0;
  int32 outsidePointCount = 0;
  int32 insideTexCount = 0;
  int32 outsideTexCount = 0;
  real32 d1, d2, d3;
  real32 t = 0;
  vec4f* insidePoints[3];
  vec4f* outsidePoints[3];
  texCoord* insideTex[3];
  texCoord* outsideTex[3];
  vec4f pNorm = soy_vec4f_normalize(pNormal);

  *out1 = *inTri;
  *out2 = *inTri;

  d1 = soy_point_to_plane_dist(pNorm, pPoint, inTri->v1);
  d2 = soy_point_to_plane_dist(pNorm, pPoint, inTri->v2);
  d3 = soy_point_to_plane_dist(pNorm, pPoint, inTri->v3);

  if(d1 >= 0) {
    insidePoints[insidePointCount++] = &inTri->v1;
    insideTex[insideTexCount++] = &inTri->t1;
  } else {
    outsidePoints[outsidePointCount++] = &inTri->v1;
    outsideTex[outsideTexCount++] = &inTri->t1;
  }

  if(d2 >= 0) {
    insidePoints[insidePointCount++] = &inTri->v2;
    insideTex[insideTexCount++] = &inTri->t2;
  } else {
    outsidePoints[outsidePointCount++] = &inTri->v2;
    outsideTex[outsideTexCount++] = &inTri->t2;
  }

  if(d3 >= 0) {
    insidePoints[insidePointCount++] = &inTri->v3;
    insideTex[insideTexCount++] = &inTri->t3;
  } else {
    outsidePoints[outsidePointCount++] = &inTri->v3;
    outsideTex[outsideTexCount++] = &inTri->t3;
  }

  if(insidePointCount == 0) { return 0; }
  if(insidePointCount == 3) { *out1 = *inTri; return 1; }

  if(insidePointCount == 1 && outsidePointCount == 2) {
    *out1 = *inTri;
    out1->v1 = *insidePoints[0];
    out1->t1 = *insideTex[0];
    out1->v2 = soy_vec4f_intersect_plane(pPoint, pNorm, *insidePoints[0], *outsidePoints[0], &t);
    out1->t2.u = t * (outsideTex[0]->u - insideTex[0]->u) + insideTex[0]->u;
    out1->t2.v = t * (outsideTex[0]->v - insideTex[0]->v) + insideTex[0]->v;
    out1->t2.w = t * (outsideTex[0]->w - insideTex[0]->w) + insideTex[0]->w;

    out1->v3 = soy_vec4f_intersect_plane(pPoint, pNorm, *insidePoints[0], *outsidePoints[1], &t);
    out1->t3.u = t * (outsideTex[1]->u - insideTex[0]->u) + insideTex[0]->u;
    out1->t3.v = t * (outsideTex[1]->v - insideTex[0]->v) + insideTex[0]->v;
    out1->t3.w = t * (outsideTex[1]->w - insideTex[0]->w) + insideTex[0]->w;
    return 1;
  }

  if(insidePointCount == 2 && outsidePointCount == 1) {
    *out1 = *inTri;
    *out2 = *inTri;
    out1->v1 = *insidePoints[0];
    out1->v2 = *insidePoints[1];
    out1->t1 = *insideTex[0];
    out1->t2 = *insideTex[1];

    out1->v3 = soy_vec4f_intersect_plane(pPoint, pNorm, *insidePoints[0], *outsidePoints[0], &t);
    out1->t3.u = t * (outsideTex[0]->u - insideTex[0]->u) + insideTex[0]->u;
    out1->t3.v = t * (outsideTex[0]->v - insideTex[0]->v) + insideTex[0]->v;
    out1->t3.w = t * (outsideTex[0]->w - insideTex[0]->w) + insideTex[0]->w;

    out2->v1 = *insidePoints[1];
    out2->t1 = *insideTex[1];
    out2->v2 = out1->v3;
    out2->t2 = out1->t3;

    out2->v3 = soy_vec4f_intersect_plane(pPoint, pNorm, *insidePoints[1], *outsidePoints[0], &t);
    out2->t3.u = t * (outsideTex[0]->u - insideTex[1]->u) + insideTex[1]->u;
    out2->t3.v = t * (outsideTex[0]->v - insideTex[1]->v) + insideTex[1]->v;
    out2->t3.w = t * (outsideTex[0]->w - insideTex[1]->w) + insideTex[1]->w;
    return 2;
  }
  return 0;
}

_docstart
_docheader
/* Notes: */
/* TODO: tri queue works but may be fucked, keep check on this (soy_3d_draw_verts_textured) */
_docend

sflag soy_3d_draw_verts_lines(soyDrawParams* params, soy3DParams* dParams3d,
    soyMesh* mesh, soyCamera* cam, soyTransform t) {
  int32 i, j, k, l;
  vec4f vCam;
  vec4f line1, line2;
  mat4f worldMatrix;
  mat4f projectionMatrix;
  vec4f vSun;
  vec4f normal;
  vec4f vCamRay;
  real32 sunDP = 0;
  int32 clippedTris = 0;
  soyTri clipped[2];
  int32 origTriCount = 0;
  int32 clipTriCount = 0;
  int32 newTriCount = 0;
  int32 addTriCount = 0;
  soyTri testTri;
  soyTri origTri;
  soyTri worldTri;
  soyTri projTri;
  soyTri finalTri;
  vCam.x = 0.0f;
  vCam.y = 0.0f;
  vCam.z = 0.0f;
  vCam.w = 1.0f;
  vSun.x = 0.9f;
  vSun.y = 0.6f;
  vSun.z = -0.9f;
  vSun.w = 1.0f;
  worldMatrix = soy_compute_world_matrix(t);
  projectionMatrix = soy_mat4f_projection(cam->nearClip, cam->farClip, cam->fieldOfView, cam->aspectRatio);
  origTriCount = mesh->triCount;
  for(i = 0; i < origTriCount; i++ ) {
    origTri = mesh->tris[i];
    worldTri = origTri;
    worldTri.v1 = soy_mat4f_vec4f_multiply(origTri.v1, worldMatrix);
    worldTri.v2 = soy_mat4f_vec4f_multiply(origTri.v2, worldMatrix);
    worldTri.v3 = soy_mat4f_vec4f_multiply(origTri.v3, worldMatrix);
    vCamRay = soy_vec4f_subtract(worldTri.v1, vCam);
    line1 = soy_vec4f_subtract(worldTri.v2, worldTri.v1);
    line2 = soy_vec4f_subtract(worldTri.v3, worldTri.v1);
    normal = soy_vec4f_cross_product(line1, line2);
    normal = soy_vec4f_normalize(normal);
    vCamRay = soy_vec4f_subtract(worldTri.v1, vCam);
    if(soy_vec4f_dot_product(normal, vCamRay) < 0.0f) {
      vSun = soy_vec4f_normalize(vSun);
      sunDP = soy_max(0.1f, soy_vec4f_dot_product(vSun, normal));
      worldTri.c1.x = sunDP;
      worldTri.c1.y = sunDP;
      worldTri.c1.z = sunDP;
      clippedTris = soy_triangle_clip_against_plane(soy_vec4f(0.0f, 0.0f, 0.2f, 0.0f),
                                                    soy_vec4f(0.0f, 0.0f, 1.0f, 0.0f),
                                                    &worldTri, &clipped[0], &clipped[1]);
      for(k = 0; k < clippedTris; k++) {
        projTri = clipped[k];
        projTri.v1 = soy_mat4f_vec4f_multiply(projTri.v1, projectionMatrix);
        projTri.v2 = soy_mat4f_vec4f_multiply(projTri.v2, projectionMatrix);
        projTri.v3 = soy_mat4f_vec4f_multiply(projTri.v3, projectionMatrix);
        projTri.v1 = soy_vec4f_divide_scalar(projTri.v1, projTri.v1.w);
        projTri.v2 = soy_vec4f_divide_scalar(projTri.v2, projTri.v2.w);
        projTri.v3 = soy_vec4f_divide_scalar(projTri.v3, projTri.v3.w);
        projTri.v1 = soy_projection_to_screen(projTri.v1, cam->width, cam->height);
        projTri.v2 = soy_projection_to_screen(projTri.v2, cam->width, cam->height);
        projTri.v3 = soy_projection_to_screen(projTri.v3, cam->width, cam->height);
        finalTri = projTri;

        if(clipTriCount >= mesh->triCacheCount) {
            SLOG("Error tri cache memory exceeded!\n");
            return FALSE;
        }
        mesh->triCache[clipTriCount] = finalTri;
        clipTriCount++;
      }
    }
  }
  /*
  for(i = 1; i < clipTriCount; i++) {
    k = i;
    while(soy_min(soy_min(mesh->triCache[k].t1.w, mesh->triCache[k].t2.w), mesh->triCache[k].t3.w) >
          soy_min(soy_min(mesh->triCache[k-1].t1.w, mesh->triCache[k-1].t2.w), mesh->triCache[k-1].t3.w) &&
          k > 1) {
      tempTri = mesh->triCache[k];
      mesh->triCache[k] = mesh->triCache[k-1];
      mesh->triCache[k-1] = tempTri;
      k--;
    }
  }
  */
  for(i = 0; i < clipTriCount; i++) {
    soy_tri_queue_init(&dParams3d->triQueue);
    soy_tri_enqueue(&dParams3d->triQueue, &mesh->triCache[i]);
    testTri = mesh->triCache[i];
    newTriCount = 1;
    for(k = 0; k < 4; k++) {
      addTriCount = 0;
      while(newTriCount > 0) {
        testTri = soy_tri_dequeue(&dParams3d->triQueue);
        newTriCount--;
        switch(k) {
          case(0): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f(0.0f, 1.0f, 0.0f, 0.0f),
                  soy_vec4f(0.0f, 1.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
          }
          case(1): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f(0.0f, (real32)params->memHeight-1.0f, 0.0f, 0.0f),
                  soy_vec4f(0.0f, -1.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
         }

         case(2): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f(1.0f, 0.0f, 0.0f, 0.0f),
                  soy_vec4f(1.0f, 0.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
          }
         case(3): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f((real32)params->memWidth-1.0f, 0.0f, 0.0f, 0.0f),
                  soy_vec4f(-1.0f, 0.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
          }
        }
        for(j = 0; j < addTriCount; j++) {
          soy_tri_enqueue(&dParams3d->triQueue, &clipped[j]);
        }
      }
      newTriCount = dParams3d->triQueue.back-dParams3d->triQueue.front;
    }

    for(l = 0; l < newTriCount; l++) {
      testTri = soy_tri_dequeue(&dParams3d->triQueue);
      soy_draw_line(params, testTri.v1.x, testTri.v1.y, testTri.v2.x, testTri.v2.y);
      soy_draw_line(params, testTri.v2.x, testTri.v2.y, testTri.v3.x, testTri.v3.y);
      soy_draw_line(params, testTri.v2.x, testTri.v2.y, testTri.v1.x, testTri.v1.y);
      soy3D_global_triCount++;
    }
  }
  return TRUE;
}

sflag soy_3d_draw_verts_filled(soyDrawParams* params, soy3DParams* dParams3d,
    soyMesh* mesh, soyCamera* cam, real32* depthBuffer, soyTransform t) {
  int32 i, j, k, l;
  vec4f vCam;
  vec4f line1, line2;
  mat4f worldMatrix;
  mat4f projectionMatrix;
  vec4f vSun;
  vec4f normal;
  vec4f vCamRay;
  real32 sunDP = 0;
  int32 clippedTris = 0;
  soyTri clipped[2];
  int32 origTriCount = 0;
  int32 clipTriCount = 0;
  int32 newTriCount = 0;
  int32 addTriCount = 0;
  soyTri testTri;
  soyTri origTri;
  soyTri worldTri;
  soyTri projTri;
  soyTri finalTri;
  vCam.x = 0.0f;
  vCam.y = 0.0f;
  vCam.z = 0.0f;
  vCam.w = 1.0f;
  vSun.x = 0.9f;
  vSun.y = 0.6f;
  vSun.z = -0.9f;
  vSun.w = 1.0f;
  worldMatrix = soy_compute_world_matrix(t);
  projectionMatrix = soy_mat4f_projection(cam->nearClip, cam->farClip, cam->fieldOfView, cam->aspectRatio);
  origTriCount = mesh->triCount;
  for(i = 0; i < origTriCount; i++ ) {
    origTri = mesh->tris[i];
    worldTri = origTri;
    worldTri.v1 = soy_mat4f_vec4f_multiply(origTri.v1, worldMatrix);
    worldTri.v2 = soy_mat4f_vec4f_multiply(origTri.v2, worldMatrix);
    worldTri.v3 = soy_mat4f_vec4f_multiply(origTri.v3, worldMatrix);
    vCamRay = soy_vec4f_subtract(worldTri.v1, vCam);
    line1 = soy_vec4f_subtract(worldTri.v2, worldTri.v1);
    line2 = soy_vec4f_subtract(worldTri.v3, worldTri.v1);
    normal = soy_vec4f_cross_product(line1, line2);
    normal = soy_vec4f_normalize(normal);
    vCamRay = soy_vec4f_subtract(worldTri.v1, vCam);
    if(soy_vec4f_dot_product(normal, vCamRay) < 0.0f) {
      vSun = soy_vec4f_normalize(vSun);
      sunDP = soy_max(0.1f, soy_vec4f_dot_product(vSun, normal));
      worldTri.c1.x = sunDP;
      worldTri.c1.y = sunDP;
      worldTri.c1.z = sunDP;
      clippedTris = soy_triangle_clip_against_plane(soy_vec4f(0.0f, 0.0f, 0.2f, 0.0f),
                                                    soy_vec4f(0.0f, 0.0f, 1.0f, 0.0f),
                                                    &worldTri, &clipped[0], &clipped[1]);
      for(k = 0; k < clippedTris; k++) {
        projTri = clipped[k];
        projTri.v1 = soy_mat4f_vec4f_multiply(projTri.v1, projectionMatrix);
        projTri.v2 = soy_mat4f_vec4f_multiply(projTri.v2, projectionMatrix);
        projTri.v3 = soy_mat4f_vec4f_multiply(projTri.v3, projectionMatrix);
        projTri.t1.w = 1.0f / projTri.v1.w;
        projTri.t2.w = 1.0f / projTri.v2.w;
        projTri.t3.w = 1.0f / projTri.v3.w;
        projTri.v1 = soy_vec4f_divide_scalar(projTri.v1, projTri.v1.w);
        projTri.v2 = soy_vec4f_divide_scalar(projTri.v2, projTri.v2.w);
        projTri.v3 = soy_vec4f_divide_scalar(projTri.v3, projTri.v3.w);
        projTri.v1 = soy_projection_to_screen(projTri.v1, cam->width, cam->height);
        projTri.v2 = soy_projection_to_screen(projTri.v2, cam->width, cam->height);
        projTri.v3 = soy_projection_to_screen(projTri.v3, cam->width, cam->height);
        finalTri = projTri;

        if(clipTriCount >= mesh->triCacheCount) {
            SLOG("Error tri cache memory exceeded!\n");
            return FALSE;
        }
        mesh->triCache[clipTriCount] = finalTri;
        clipTriCount++;
      }
    }
  }
  /*
  for(i = 1; i < clipTriCount; i++) {
    k = i;
    while(soy_min(soy_min(mesh->triCache[k].t1.w, mesh->triCache[k].t2.w), mesh->triCache[k].t3.w) >
          soy_min(soy_min(mesh->triCache[k-1].t1.w, mesh->triCache[k-1].t2.w), mesh->triCache[k-1].t3.w) &&
          k > 1) {
      tempTri = mesh->triCache[k];
      mesh->triCache[k] = mesh->triCache[k-1];
      mesh->triCache[k-1] = tempTri;
      k--;
    }
  }
  */
  for(i = 0; i < clipTriCount; i++) {
    soy_tri_queue_init(&dParams3d->triQueue);
    soy_tri_enqueue(&dParams3d->triQueue, &mesh->triCache[i]);
    testTri = mesh->triCache[i];
    newTriCount = 1;
    for(k = 0; k < 4; k++) {
      addTriCount = 0;
      while(newTriCount > 0) {
        testTri = soy_tri_dequeue(&dParams3d->triQueue);
        newTriCount--;
        switch(k) {
          case(0): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f(0.0f, 1.0f, 0.0f, 0.0f),
                  soy_vec4f(0.0f, 1.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
          }
          case(1): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f(0.0f, (real32)params->memHeight-1.0f, 0.0f, 0.0f),
                  soy_vec4f(0.0f, -1.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
         }

         case(2): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f(1.0f, 0.0f, 0.0f, 0.0f),
                  soy_vec4f(1.0f, 0.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
          }
         case(3): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f((real32)params->memWidth-1.0f, 0.0f, 0.0f, 0.0f),
                  soy_vec4f(-1.0f, 0.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
          }
        }
        for(j = 0; j < addTriCount; j++) {
          soy_tri_enqueue(&dParams3d->triQueue, &clipped[j]);
        }
      }
      newTriCount = dParams3d->triQueue.back-dParams3d->triQueue.front;
    }

    for(l = 0; l < newTriCount; l++) {
      testTri = soy_tri_dequeue(&dParams3d->triQueue);
      finalTri = testTri;
      soy_draw_set_colour(params, 255,255,255,255);

      soy_draw_triangle_solid(params, depthBuffer,
        finalTri.v1.x, finalTri.v1.y, finalTri.v2.x, finalTri.v2.y, finalTri.v3.x, finalTri.v3.y,
        finalTri.t1.w, finalTri.t2.w,
        finalTri.t3.w, finalTri.c1.x, finalTri.c1.y, finalTri.c1.z);

      soy3D_global_triCount++;
    }
  }
  return TRUE;
}

sflag soy_3d_draw_verts_textured(soyDrawParams* params, soy3DParams* dParams3d,
    soyMesh* mesh, soyBitmapHandle texture, soyCamera* cam, real32* depthBuffer, soyTransform t) {
  int32 i, j, k, l;
  vec4f vCam;
  vec4f line1, line2;
  mat4f worldMatrix;
  mat4f projectionMatrix;
  vec4f vSun;
  vec4f normal;
  vec4f vCamRay;
  real32 sunDP = 0;
  int32 clippedTris = 0;
  soyTri clipped[2];
  int32 origTriCount = 0;
  int32 clipTriCount = 0;
  int32 newTriCount = 0;
  int32 addTriCount = 0;
  soyTri testTri;
  soyTri origTri;
  soyTri worldTri;
  soyTri projTri;
  soyTri finalTri;
  vCam.x = 0.0f;
  vCam.y = 0.0f;
  vCam.z = 0.0f;
  vCam.w = 1.0f;
  vSun.x = 0.9f;
  vSun.y = 0.6f;
  vSun.z = -0.9f;
  vSun.w = 1.0f;  
  worldMatrix = soy_compute_world_matrix(t);
  projectionMatrix = soy_mat4f_projection(cam->nearClip, cam->farClip, cam->fieldOfView, cam->aspectRatio);
  origTriCount = mesh->triCount;
  for(i = 0; i < origTriCount; i++ ) {
    origTri = mesh->tris[i];
    worldTri = origTri;
    worldTri.v1 = soy_mat4f_vec4f_multiply(origTri.v1, worldMatrix);
    worldTri.v2 = soy_mat4f_vec4f_multiply(origTri.v2, worldMatrix);
    worldTri.v3 = soy_mat4f_vec4f_multiply(origTri.v3, worldMatrix);
    vCamRay = soy_vec4f_subtract(worldTri.v1, vCam);
    line1 = soy_vec4f_subtract(worldTri.v2, worldTri.v1);
    line2 = soy_vec4f_subtract(worldTri.v3, worldTri.v1);
    normal = soy_vec4f_cross_product(line1, line2);
    normal = soy_vec4f_normalize(normal);
    vCamRay = soy_vec4f_subtract(worldTri.v1, vCam);
    if(soy_vec4f_dot_product(normal, vCamRay) < 0.0f) {
      vSun = soy_vec4f_normalize(vSun);
      sunDP = soy_max(0.1f, soy_vec4f_dot_product(vSun, normal));
      worldTri.c1.x = sunDP;
      worldTri.c1.y = sunDP;
      worldTri.c1.z = sunDP;
      clippedTris = soy_triangle_clip_against_plane(soy_vec4f(0.0f, 0.0f, 0.2f, 0.0f),
                                                    soy_vec4f(0.0f, 0.0f, 1.0f, 0.0f),
                                                    &worldTri, &clipped[0], &clipped[1]);
      for(k = 0; k < clippedTris; k++) {
        projTri = clipped[k];
        projTri.v1 = soy_mat4f_vec4f_multiply(projTri.v1, projectionMatrix);
        projTri.v2 = soy_mat4f_vec4f_multiply(projTri.v2, projectionMatrix);
        projTri.v3 = soy_mat4f_vec4f_multiply(projTri.v3, projectionMatrix);
        if(params->doCorrectTexture == TRUE) {
          projTri.t1.u /= projTri.v1.w;
          projTri.t2.u /= projTri.v2.w;
          projTri.t3.u /= projTri.v3.w;
          projTri.t1.v /= projTri.v1.w;
          projTri.t2.v /= projTri.v2.w;
          projTri.t3.v /= projTri.v3.w;
        }
        projTri.t1.w = 1.0f / projTri.v1.w;
        projTri.t2.w = 1.0f / projTri.v2.w;
        projTri.t3.w = 1.0f / projTri.v3.w;
        projTri.v1 = soy_vec4f_divide_scalar(projTri.v1, projTri.v1.w);
        projTri.v2 = soy_vec4f_divide_scalar(projTri.v2, projTri.v2.w);
        projTri.v3 = soy_vec4f_divide_scalar(projTri.v3, projTri.v3.w);
        projTri.v1 = soy_projection_to_screen(projTri.v1, cam->width, cam->height);
        projTri.v2 = soy_projection_to_screen(projTri.v2, cam->width, cam->height);
        projTri.v3 = soy_projection_to_screen(projTri.v3, cam->width, cam->height);
        finalTri = projTri;

        if(clipTriCount >= mesh->triCacheCount) {
            SLOG("Error tri cache memory exceeded!\n");
            return FALSE;
        }
        mesh->triCache[clipTriCount] = finalTri;
        clipTriCount++;
      }
    }
  }
  /*
  for(i = 1; i < clipTriCount; i++) {
    k = i;
    while(soy_min(soy_min(mesh->triCache[k].t1.w, mesh->triCache[k].t2.w), mesh->triCache[k].t3.w) >
          soy_min(soy_min(mesh->triCache[k-1].t1.w, mesh->triCache[k-1].t2.w), mesh->triCache[k-1].t3.w) &&
          k > 1) {
      tempTri = mesh->triCache[k];
      mesh->triCache[k] = mesh->triCache[k-1];
      mesh->triCache[k-1] = tempTri;
      k--;
    }
  }
  */
  for(i = 0; i < clipTriCount; i++) {
    soy_tri_queue_init(&dParams3d->triQueue);
    soy_tri_enqueue(&dParams3d->triQueue, &mesh->triCache[i]);
    testTri = mesh->triCache[i];
    newTriCount = 1;
    for(k = 0; k < 4; k++) {
      addTriCount = 0;
      while(newTriCount > 0) {
        testTri = soy_tri_dequeue(&dParams3d->triQueue);
        newTriCount--;
        switch(k) {
          case(0): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f(0.0f, 1.0f, 0.0f, 0.0f),
                  soy_vec4f(0.0f, 1.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
          }
          case(1): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f(0.0f, (real32)params->memHeight-1.0f, 0.0f, 0.0f),
                  soy_vec4f(0.0f, -1.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
         }

         case(2): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f(1.0f, 0.0f, 0.0f, 0.0f),
                  soy_vec4f(1.0f, 0.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
          }
         case(3): {
              addTriCount = soy_triangle_clip_against_plane(
                  soy_vec4f((real32)params->memWidth-1.0f, 0.0f, 0.0f, 0.0f),
                  soy_vec4f(-1.0f, 0.0f, 0.0f, 0.0f),
                  &testTri, &clipped[0], &clipped[1]);
              break;
          }
        }
        for(j = 0; j < addTriCount; j++) {
          soy_tri_enqueue(&dParams3d->triQueue, &clipped[j]);
        }
      }
      newTriCount = dParams3d->triQueue.back-dParams3d->triQueue.front;
    }

    for(l = 0; l < newTriCount; l++) {
      testTri = soy_tri_dequeue(&dParams3d->triQueue);
      finalTri = testTri;
      soy_draw_set_colour(params, 255,255,255,255);

      if(params->doSubPixel) {
        soy_draw_triangle_textured(params, texture, depthBuffer,
          finalTri.v1.x, finalTri.v1.y, finalTri.v2.x, finalTri.v2.y, finalTri.v3.x, finalTri.v3.y,
          finalTri.t1.u, finalTri.t1.v, finalTri.t1.w, finalTri.t2.u, finalTri.t2.v, finalTri.t2.w,
          finalTri.t3.u, finalTri.t3.v, finalTri.t3.w, finalTri.c1.x, finalTri.c1.y, finalTri.c1.z);
      } else {
        soy_draw_triangle_textured_int(params, texture, depthBuffer,
          (int32)finalTri.v1.x, (int32)finalTri.v1.y,
          (int32)finalTri.v2.x, (int32)finalTri.v2.y,
          (int32)finalTri.v3.x, (int32)finalTri.v3.y,
          finalTri.t1.u, finalTri.t1.v, finalTri.t1.w, finalTri.t2.u, finalTri.t2.v, finalTri.t2.w,
          finalTri.t3.u, finalTri.t3.v, finalTri.t3.w, finalTri.c1.x, finalTri.c1.y, finalTri.c1.z);
      }
      soy3D_global_triCount++;
    }
  }
  return TRUE;
}

sflag soy_3d_draw_model(soyDrawParams* dp, soy3DParams* dp3, real32* depthBuffer,
    soyModelHandle model, soyBitmapHandle texture, soyTransform t) {
  switch(dp3->mode) {
    case(SOY_3D_TEXTURED): {
      soy_3d_draw_verts_textured(
      dp,
      dp3,
      &model->mesh,
      texture,
      &dp3->mainCamera,
      depthBuffer,
      t);
      break;
    }

    case(SOY_3D_LINES): {
      soy_3d_draw_verts_lines(dp, dp3, &model->mesh, &dp3->mainCamera, t);
      break;
    }

    case(SOY_3D_SOLID): {
      soy_3d_draw_verts_filled(dp, dp3, &model->mesh, &dp3->mainCamera, depthBuffer, t);
      break;
    }
    default: { break; }
  }
  return TRUE;
}

/********************************************/
/* @textbox                                 */
/* SoyLib Textbox                           */
/********************************************/

typedef sflag (*pfSoyTextBoxFunc) (int8* text, uint32 textLength);

typedef struct tag_soyTextBox {
  soyRect rect;
  soyFontHandle font;
  int32 cursorPosX;
  int32 cursorPosY;
  int32 charsW;
  int32 charsH;
  uint8* mem;
  int32 back;
  int32 memSize;
  sflag lockCursor;
  sflag enabled;
  sflag selected;
  pfSoyTextBoxFunc boxFunc;
  uint32 textBoxFrames;
  uint32 textBoxCursorSpeed;
} 
soyTextBox;

typedef soyTextBox* soyTextBoxHandle;

soyTextBox soy_textbox(cString name,
    int32 x, int32 y, uint32 charsW, uint32 charsH,
    uint8* mem, uint32 memSize, soyFontHandle font, soyRectHandle prevRect, pfSoyTextBoxFunc func) {
  soyTextBox ret;
  if(font != NULL) {
      ret.rect = soy_rect(name, x, y,
      (charsW+1)*(font->grid-font->globalKerning)*font->scale,
      (charsH)*(font->grid*font->scale)+(font->grid*font->scale/2), NULL, NULL, prevRect);
  } else {
    ret.rect = soy_rect(name, 6, 6, 6, 6, NULL, NULL, NULL);
  }
  ret.textBoxFrames = 0;
  ret.textBoxCursorSpeed = 64;
  ret.mem = mem;
  ret.font = font;
  ret.memSize = memSize;
  ret.cursorPosX = 0;
  ret.cursorPosY = 0;
  ret.charsW = charsW;
  ret.charsH = charsH;
  ret.back = 0;
  ret.enabled = TRUE;
  ret.selected = TRUE;
  ret.lockCursor = FALSE;
  ret.boxFunc = func;
  return ret;
}

sflag soy_textbox_flush(soyTextBoxHandle box, sflag clear) {
  int32 i;
  uint32 texLen;
  texLen = box->back;
  if(box->boxFunc != NULL) {
    box->boxFunc((int8*)box->mem, texLen);
  }
  if(clear) {
    box->cursorPosX = 0;
    box->cursorPosY = 0;
    for(i = 0; i < box->memSize; i++) {
      box->mem[i] = '0';
    }
    box->back = 0;
  }
  box->selected = FALSE;
  return TRUE;
}

sflag soy_textbox_clear(soyTextBoxHandle box) {
  int32 i;
  box->cursorPosX = 0;
  box->cursorPosY = 0;
  for(i = 0; i < box->memSize; i++) {
    box->mem[i] = '0';
  }
  box->back = 0;
  return TRUE;
}

sflag soy_textbox_putchar(soyTextBoxHandle box, int8 c) {
  if(box->cursorPosX > box->charsW*box->charsH-2 || box->cursorPosX < 0) { return FALSE; }
  box->mem[box->cursorPosX] = c;
  box->mem[box->cursorPosX+1] = 0;
  box->cursorPosX++;
  box->back++;
  return TRUE;
}

sflag soy_textbox_paste(soyTextBoxHandle box, cString c) {
  int32 i;
  int32 len = soy_cstring_len(c);
  if(len > box->charsW) { len = box->charsW-1; }
  for(i = 0; i < len; i++) {
    soy_textbox_putchar(box, c[i]);
  }
  return TRUE;
}

sflag soy_textbox_remchar(soyTextBoxHandle box) {
  if(box->cursorPosX > 0) {
    box->cursorPosX--;
    box->back--;
    box->mem[box->cursorPosX] = 0;
  }
  return FALSE;
}

static sflag global_consolePasteToggle = FALSE;

sflag soy_textbox_process_input(
    soyWindow* win,
    soyVirtualMemory* clipboardMemory,
    soyTextBoxHandle box) {
  int32 clipBDex;
  int32 clipBLen;
  uint8 asciiKey;
  if(soy_key_pressed(win, SOY_CONTROL) && soy_key_pressed(win, SOY_V)) {
    if(!global_consolePasteToggle) {
      SLOG("Pasting\n");
      clipBDex = 0;
      clipBLen = soy_get_clipboard((char*)clipboardMemory->memory, clipboardMemory->size);
      while(clipBDex < clipBLen && clipBDex < (int)box->memSize) {
        soy_textbox_putchar(box, clipboardMemory->memory[clipBDex]);
        clipBDex++;
      }
      global_consolePasteToggle = TRUE;
    }
  } else {
    global_consolePasteToggle = FALSE;
  }

  if(soy_any_key_pressed(win)) {
    asciiKey = soy_key_to_ascii(soy_last_key(win));
      if(soy_last_key(win) == SOY_BACKSPACE) {
        soy_textbox_remchar(box);
      } else if(soy_last_key(win) == SOY_RETURN) {
        soy_textbox_flush(box, FALSE);
      } else if(asciiKey >= 2*16 && asciiKey < 8*16) {
        soy_textbox_putchar(box, soy_key_to_ascii(soy_last_key(win)));
      }
  }
  return TRUE;
}

sflag soy_textbox_resize(soyTextBoxHandle box, int32 x, int32 y, uint32 charsW, uint32 charsH) {
  soyFontHandle font = box->font;
  if(font != NULL) {
      soy_rect_resize(&box->rect, x, y,
      (charsW+1)*(font->grid-font->globalKerning)*font->scale,
      (charsH)*(font->grid*font->scale)+(font->grid*font->scale/2));
  }
  box->charsW = charsW;
  box->charsH = charsH;
  box->cursorPosX = soy_min((int32)box->cursorPosX, (int32)charsW*(int32)charsH);
  box->back = soy_min((int32)box->back, (int32)charsW*(int32)charsH);
  return TRUE;
}

sflag soy_textbox_update(soyWindow* win, soyVirtualMemory* clipboardMemory, soyTextBoxHandle box) {
  if(box->selected) {
    soy_textbox_process_input(win, clipboardMemory, box);
  }
  if(soy_rect_check_collision(box->rect, soy_mouse_x(win), soy_mouse_y(win))) {
    if(soy_mouse_pressed(win, SOY_MOUSE_LEFT)) {
      box->selected = TRUE;
    }
  } else {
    if(soy_mouse_pressed(win, SOY_MOUSE_LEFT)) {
      box->selected = FALSE;
    }
  }

  return TRUE;
}

sflag soy_draw_textbox(soyDrawParams* params, soyTextBoxHandle box, soyGUIPalette p) {
  int32 i;
  uint32 cx = 0;
  uint32 cy = 0;
  uint32 fs = (box->font->grid - box->font->globalKerning)*box->font->scale;
  uint32 fh = (box->font->grid)*box->font->scale;
  if(box->selected) {
    soy_draw_set_gui_emission(&p, 0.2f, 0.2f, 0.2f);
  }
  soy_draw_gui_panel_press(params,
      box->rect.x, box->rect.y,
      box->rect.width, box->rect.height, p);
    
  params->curColour = p.colours[SOY_GUI_TEXTACTIVE];
  for(i = 0; i < box->back+1; i++) {
    if((i) % box->charsW == 0) {
      cx = 0;
      cy+=fh;
    }
    if(i < box->back) {
      soy_draw_char(params, box->rect.x+cx+fs/2-1, box->rect.y+cy-fh+fh/4+1, box->mem[i], box->font);
    }
    if(i == box->cursorPosX &&
              box->selected &&
              box->textBoxFrames%box->textBoxCursorSpeed < box->textBoxCursorSpeed/2) {
      soy_draw_char(params, box->rect.x+cx+fs/2-1, box->rect.y+cy-fh+fh/4+1, '_', box->font);
    }
    cx+=fs;
  }
  box->textBoxFrames++;
  return TRUE;
}




/********************************************/
/* @button                                  */
/* SoyLib Button                            */
/********************************************/

typedef enum enum_soyButtonType {
  SOY_BUTTON_NULL,
  SOY_BUTTON_NORMAL,
  SOY_BUTTON_MICRO,
  SOY_BUTTON_IMAGE
} soyButtonType;

typedef struct tag_soyButton {
  soyRect rect;
  soyFontHandle font;
  uint32 strLen;
  soyButtonType type;
  sflag pressAnim;
  sflag hoverAnim;
} soyButton;

typedef soyButton* soyButtonHandle;

soyButton soy_button(cString name, soyFontHandle font, int32 x, int32 y, soyCommandHandle command) {
  soyButton ret;
  uint32 strLen = soy_cstring_len(name);
  uint32 strWidth = soy_draw_text_len(name, strLen, font);
  ret.pressAnim = TRUE;
  ret.hoverAnim = FALSE;
  ret.font = font;
  ret.type = SOY_BUTTON_NORMAL;
  ret.strLen = strLen;
  ret.rect = soy_rect(name, x, y, strWidth,
      font->grid*font->scale+2, command, NULL, NULL);
  return ret;
}

soyButton soy_button_img(cString name,
    int32 x, int32 y, uint32 width, uint32 height, soyCommandHandle command) {
  soyButton ret;
  ret.pressAnim = TRUE;
  ret.hoverAnim = FALSE;
  ret.strLen = 0;
  ret.font = NULL;
  ret.type = SOY_BUTTON_IMAGE;
  ret.rect = soy_rect(name, x, y, width, height, command, NULL, NULL);
  return ret;
}

soyButton soy_button_spr(cString name,
    int32 x, int32 y, uint32 width, uint32 height, soyCommandHandle command) {
  soyButton ret;
  ret.pressAnim = TRUE;
  ret.hoverAnim = FALSE;
  ret.strLen = 0;
  ret.font = NULL;
  ret.type = SOY_BUTTON_MICRO;
  ret.rect = soy_rect(name, x, y, width, height, command, NULL, NULL);
  return ret;
}

sflag soy_button_update(soyWindow* win, soyButtonHandle button) {
  soy_rect_update(&button->rect,
      soy_mouse_x(win), soy_mouse_y(win),
      soy_mouse_pressed(win, SOY_MOUSE_LEFT),
      soy_mouse_pressed(win, SOY_MOUSE_RIGHT));
  return TRUE;
}

sflag soy_button_enable_press_anim(soyButtonHandle b) {
  b->pressAnim = TRUE;
  return TRUE;
}

sflag soy_button_enable_hover_anim(soyButtonHandle b) {
  b->hoverAnim = TRUE;
  return TRUE;
}

sflag soy_button_disable_press_anim(soyButtonHandle b) {
  b->pressAnim = FALSE;
  return TRUE;
}

sflag soy_button_disable_hover_anim(soyButtonHandle b) {
  b->hoverAnim = FALSE;
  return TRUE;
}

sflag soy_button_draw(soyDrawParams* params, soyGUIPalette p, soyButtonHandle button, int32 padX, int32 padY) {
  uint32 dx = padX;
  uint32 dy = padY;
  int32 hAn = ((button->hoverAnim) ? -1 : 0);
  int32 pAn = ((button->pressAnim) ? 1 : 0);
  params->curColour =  
      (soy_bit_check(button->rect.state, SOY_RECT_SELECTED) ||
      (soy_bit_check(button->rect.state, SOY_RECT_HOVER) &&
       soy_bit_check(button->rect.state, SOY_RECT_PRESSL))) ? 
      p.colours[SOY_GUI_LIGHT] : p.colours[SOY_GUI_NEUTRAL];
  soy_draw_fill(params, button->rect.x, button->rect.y,
        button->rect.x+button->rect.width, button->rect.y+button->rect.height);
  if(!soy_bit_check(button->rect.state, SOY_RECT_HOVER) &&
      !(soy_bit_check(button->rect.state, SOY_RECT_SELECTED))) {
    soy_draw_gui_rect_outer(params, button->rect.x, button->rect.y,
        button->rect.x+button->rect.width, button->rect.y+button->rect.height, p);
    if(button->font == NULL) { return TRUE; } 
    params->curColour = p.colours[SOY_GUI_TEXTACTIVE];
    soy_draw_text(params, button->rect.x+dx, button->rect.y+dy,
        button->rect.label, button->strLen,
        button->font);
  } else if((soy_bit_check(button->rect.state, SOY_RECT_PRESSL) &&
             soy_bit_check(button->rect.state, SOY_RECT_HOVER)) ||
        soy_bit_check(button->rect.state, SOY_RECT_SELECTED)) {
    soy_draw_gui_rect_inner(params, button->rect.x, button->rect.y,
        button->rect.x+button->rect.width, button->rect.y+button->rect.height, p);
    if(button->font == NULL) { return TRUE; }
    params->curColour = p.colours[SOY_GUI_TEXTACTIVE];
    soy_draw_text(params, button->rect.x+dx+pAn, button->rect.y+dy+pAn,
        button->rect.label, button->strLen,
        button->font);
  } else {
    soy_draw_gui_rect_outer(params, button->rect.x+hAn, button->rect.y+hAn,
        button->rect.x+button->rect.width+hAn, button->rect.y+button->rect.height+hAn, p);
    if(button->font == NULL) { return TRUE; }
    params->curColour = p.colours[SOY_GUI_TEXTACTIVE];
    soy_draw_text(params, button->rect.x+dx+hAn, button->rect.y+dy+hAn,
        button->rect.label, button->strLen,
        button->font);
  }
  return TRUE;
}

sflag soy_button_micro_draw(soyDrawParams* params, soyGUIPalette p, cString mask,
    soyButtonHandle button) {
  uint32 hAn = (button->hoverAnim) ? -1 : 0;
  params->curColour =  p.colours[SOY_GUI_NEUTRAL];
  soy_draw_fill(params, button->rect.x, button->rect.y,
        button->rect.x+button->rect.width, button->rect.y+button->rect.height);
  if(!soy_bit_check(button->rect.state, SOY_RECT_HOVER) &&
      !(soy_bit_check(button->rect.state, SOY_RECT_SELECTED))) {
    soy_draw_gui_rect_outer(params, button->rect.x, button->rect.y,
        button->rect.x+button->rect.width, button->rect.y+button->rect.height, p);
    params->curColour = p.colours[SOY_GUI_GRADSTARTINACTIVE];
    soy_draw_mask(params, button->rect.x, button->rect.y, mask, button->rect.width); 
  } else if((soy_bit_check(button->rect.state, SOY_RECT_PRESSL) &&
             soy_bit_check(button->rect.state, SOY_RECT_HOVER)) ||
        soy_bit_check(button->rect.state, SOY_RECT_SELECTED)) { 
    soy_draw_gui_rect_inner(params, button->rect.x+1, button->rect.y+1,
        button->rect.x+button->rect.width+1, button->rect.y+button->rect.height+1, p);
    params->curColour = p.colours[SOY_GUI_GRADSTARTINACTIVE];
    soy_draw_mask(params, button->rect.x+1, button->rect.y+1, mask, button->rect.width); 
  } else {
    soy_draw_gui_rect_outer(params, button->rect.x+hAn, button->rect.y+hAn,
        button->rect.x+button->rect.width+hAn, button->rect.y+button->rect.height+hAn, p);
    params->curColour = p.colours[SOY_GUI_GRADSTARTINACTIVE];
    soy_draw_mask(params, button->rect.x+hAn, button->rect.y+hAn, mask, button->rect.width); 
  }
  return TRUE;
}

sflag soy_draw_gui_sprite(soyDrawParams* params, soyBitmapHandle bmp, int32 x, int32 y,
    uint32 cellSize, uint32 indexW, uint32 indexH, soyGUIPalette p) {
  uint32 i, j;
  soyPixel32 col;
  for(j = 0; j < cellSize; j++) {
    for(i = 0; i < cellSize; i++) {
      col = ((soyPixel32*)bmp->bytes)[i+indexW*cellSize+(j+indexH*cellSize)*bmp->width];
      if(col.r < 32 && col.g < 32 && col.b < 32) {
        params->curColour = p.colours[SOY_GUI_DARKEST];
      } else if(col.r < 64) {
        params->curColour = p.colours[SOY_GUI_DARKEST];
      } else if(col.r < 96) {
        params->curColour = p.colours[SOY_GUI_DARK];
      } else if(col.r < 128) {
        params->curColour = p.colours[SOY_GUI_DARK];
      } else if(col.r < 160) {
        params->curColour = p.colours[SOY_GUI_NEUTRAL];
      } else if(col.r < 160) {
        params->curColour = p.colours[SOY_GUI_NEUTRAL];
      } else if(col.r < 192) {
        params->curColour = p.colours[SOY_GUI_LIGHT];
      } else if(col.r < 224) {
        params->curColour = p.colours[SOY_GUI_LIGHT];
      } else if(col.r < 255) {
        params->curColour = p.colours[SOY_GUI_LIGHTEST];
      } else {
        params->curColour = p.colours[SOY_GUI_LIGHTEST];
      }
      if(col.r != 0 && col.g != 0 && col.b != 0) {
        soy_draw_pixel(params, x+i, y+j);
      }
    }
  }
  return TRUE;
}

/********************************************/
/* @app                                     */
/* SoyLib Application Abstraction           */
/********************************************/
typedef struct tag_soyApp soyApp;

typedef struct tag_soyDraw {
  soyDrawParams drawParams;
  soy3DParams draw3DParams;
  soyGUIPalette guiPalette;
  soyBitmap normalFontAtlas;
  soyBitmap smallFontAtlas;
  soyBitmap bigFontAtlas;
  soyBitmap guiAtlas;
  soyFont font;
  soyFont normalFont;
  soyFont smallFont;
  soyFont bigFont;
  sflag spriteGUI;
} soyDraw;

typedef enum enum_soyAppState {
  SOY_DONTSTOP,
  SOY_STOP,
  SOY_PAUSED,
  SOY_ERROR
} soyAppState;

typedef struct tag_soyAppMemory {
  soyVirtualMemory imageLoadMemory;
  soyVirtualMemory imageMemory;
  soyVirtualMemory meshLoadMemory;
  soyVirtualMemory meshMemory;
  soyVirtualMemory framebufferMemory;
  soyVirtualMemory depthbufferMemory;
  soyVirtualMemory clipboardMemory;
  soyVirtualMemory audioSampleMemory;
  soyVirtualMemory audioLoadMemory;
  soyVirtualMemory guiMemory;
  soyVirtualMemory commandMemory;
  soyVirtualMemory triCacheMemory;
  soyVirtualMemory moduleMemory;
  soyVirtualMemory textBoxMem;
} soyAppMemory;


struct tag_soyApp {
  cString name;
  soyWindow window;
  soyAppState state;
  soyAppMemory memory;
  soyReferenceManager memMan;
  soyCommandManager comMan;
  soyAudio audio;
  soyCanvas frameBuffer;
  soyCanvas depthBuffer;
  soyDraw draw;
  soyRect windowRect;
  soyTiming timing;
  soyTiming userTiming;
  uint32 pixelResolution;
  pfSoyWinResizeFunc userResize;
  sflag fullscreened;
  sflag renderStep;
  sflag updateStep;
  sflag hardwareAccelaration;
  uint32 renderFrames;
  uint32 updateFrames;
  uint32 inputFrames;
};

typedef soyApp* soyAppHandle;

sflag soy_resize_func(soyHandle app, int32 width, int32 height) {
  soyAppHandle global_soyApp = (soyAppHandle)app;
  soy_canvas_resize(
      &global_soyApp->frameBuffer,
      width/global_soyApp->pixelResolution,
      height/global_soyApp->pixelResolution,
      sizeof(soyPixel32));

  soy_canvas_resize(
      &global_soyApp->depthBuffer,
      width/global_soyApp->pixelResolution,
      height/global_soyApp->pixelResolution,
      sizeof(soyPixel32));

  soy_draw_update(&global_soyApp->draw.drawParams,
      global_soyApp->frameBuffer.width,
      global_soyApp->frameBuffer.height, global_soyApp->frameBuffer.mem);

  if(global_soyApp->userResize) {
    global_soyApp->userResize(global_soyApp,
        global_soyApp->frameBuffer.width, global_soyApp->frameBuffer.height);
  }

  global_soyApp->windowRect = soy_rect("window",
      0,
      0,
      global_soyApp->frameBuffer.width, global_soyApp->frameBuffer.height, NULL, NULL, NULL);

  if(global_soyApp->hardwareAccelaration) {
    SwapBuffers(global_soyApp->window.nativeWin.deviceContext);
  } else {
    soy_blit_pixels(&global_soyApp->window, global_soyApp->frameBuffer.mem,
      global_soyApp->frameBuffer.width, global_soyApp->frameBuffer.height);
  }
  return TRUE;
}

sflag soy_set_resize(soyAppHandle app, pfSoyWinResizeFunc resize) {
  app->userResize = resize;
  return TRUE;
}

sflag soy_app_create(soyAppHandle app, uint32 width, uint32 height, uint32 res, cString title,
    sflag hardwareAccelaration) {
  soy_memory_create(&app->memMan);
  soy_memory_enslave(&app->memMan, &app->memory.imageLoadMemory, Megabytes(8), sizeof(char), "bmpload");
  soy_memory_enslave(&app->memMan, &app->memory.imageMemory, Megabytes(8), sizeof(char), "bmpstore");
  soy_memory_enslave(&app->memMan, &app->memory.meshLoadMemory, Megabytes(8), sizeof(char), "meshload");
  soy_memory_enslave(&app->memMan, &app->memory.meshMemory, Megabytes(8), sizeof(char), "meshstore");
  soy_memory_enslave(&app->memMan, &app->memory.framebufferMemory, Megabytes(16), sizeof(char), "framebuffer");
  soy_memory_enslave(&app->memMan, &app->memory.depthbufferMemory, Megabytes(16), sizeof(char), "depthbuffer");
  soy_memory_enslave(&app->memMan, &app->memory.clipboardMemory, Megabytes(8), sizeof(char), "clipboard");
  soy_memory_enslave(&app->memMan, &app->memory.audioSampleMemory, Megabytes(128), sizeof(char), "samplemem");
  soy_memory_enslave(&app->memMan, &app->memory.audioLoadMemory, Megabytes(16), sizeof(char), "audioload");
  soy_memory_enslave(&app->memMan, &app->memory.guiMemory, Megabytes(8), sizeof(char), "guimem");
  soy_memory_enslave(&app->memMan, &app->memory.commandMemory, Megabytes(2), sizeof(char), "commands");
  soy_memory_enslave(&app->memMan, &app->memory.triCacheMemory, Megabytes(16), sizeof(char), "tricache");
  soy_memory_enslave(&app->memMan, &app->memory.moduleMemory, Megabytes(8), sizeof(char), "modmem");
  soy_memory_enslave(&app->memMan, &app->memory.textBoxMem, Megabytes(8), sizeof(char), "textBoxMem");
  app->hardwareAccelaration = hardwareAccelaration;
  app->pixelResolution = res;
  app->name = title;
  app->userResize = NULL;
  app->fullscreened = FALSE;
  app->window.nativeWin.focused = FALSE;
  app->draw.spriteGUI = FALSE;
  app->renderStep = 1;
  app->updateStep = 1;
  app->renderFrames = 0;
  app->updateFrames = 0;
  app->inputFrames = 0;
  soy_canvas_create(
      &app->memory.framebufferMemory,
      &app->frameBuffer, width/res, height/res, sizeof(soyPixel32), "Framebuffer");
  soy_canvas_create(
      &app->memory.depthbufferMemory,
      &app->depthBuffer, width/res, height/res, sizeof(real32), "Depthbuffer");
  soy_command_manager_init(&app->comMan, &app->memory.commandMemory);
  if(hardwareAccelaration) {
    soy_window_gl_create((soyHandle)app, &app->window, width, height, res, title);
  } else {
    soy_window_create((soyHandle)app, &app->window, width, height, res, title);
  }

  soy_window_set_resize_func(&app->window, soy_resize_func);
  soy_time_init(&app->userTiming, 1.0 / 120.0);
  soy_time_init(&app->timing, 1.0 / 120.0);
  soy_time_set_scheduler(1);
  soy_draw_init(&app->draw.drawParams, app->frameBuffer.mem, app->frameBuffer.width, app->frameBuffer.height);
  soy_3d_init(&app->draw.draw3DParams, &app->memory.triCacheMemory,
      app->frameBuffer.width, app->frameBuffer.height);
  soy_draw_load_gui_palette(&app->draw.guiPalette, "assets/palettes/win2k.soy");
  soy_bitmap_create(
      &app->memory.imageLoadMemory,
      &app->memory.imageMemory,
      &app->draw.normalFontAtlas, "assets/fonts/MS_14.bmp", "rgba", FALSE, TRUE);
  soy_bitmap_create(
      &app->memory.imageLoadMemory,
      &app->memory.imageMemory,
      &app->draw.bigFontAtlas, "assets/fonts/CONSTANTINE_14.bmp", "rgba", FALSE, TRUE); 
  soy_bitmap_create(
      &app->memory.imageLoadMemory,
      &app->memory.imageMemory,
      &app->draw.smallFontAtlas, "assets/fonts/AUGUST_11.bmp", "rgba", FALSE, TRUE);
  if(!soy_bitmap_create(
      &app->memory.imageLoadMemory,
      &app->memory.imageMemory,
      &app->draw.guiAtlas, "assets/gui/default.bmp", "rgba", FALSE, TRUE)) {
    SLOG("No gui atlas found!\n");
      app->draw.spriteGUI = FALSE;
  }
  soy_font_set_atlas(&app->draw.normalFont, &app->draw.normalFontAtlas, 14);
  soy_font_set_kerning(&app->draw.normalFont, 7);
  soy_font_set_atlas(&app->draw.smallFont, &app->draw.smallFontAtlas, 11);
  soy_font_set_kerning(&app->draw.smallFont, 5);
  soy_font_set_atlas(&app->draw.bigFont, &app->draw.bigFontAtlas, 14);
  soy_font_set_kerning(&app->draw.bigFont, 7);
  app->draw.font = app->draw.normalFont;
  soy_audio_create(&app->window, &app->memory.audioLoadMemory,  &app->memory.audioSampleMemory, &app->audio);
  soy_audio_play(&app->audio);
  SLOG1("Soy Application \"%s\" created succesfully!\n\n", title);
  app->state = SOY_DONTSTOP;
  return TRUE;
}

sflag soy_app_update(soyAppHandle app) {
  if(app->state == SOY_STOP) { return TRUE; }
  soy_canvas_resize(
      &app->frameBuffer,
      soy_win_width(&app->window)/app->pixelResolution,
      soy_win_height(&app->window)/app->pixelResolution,
      sizeof(soyPixel32));
  soy_canvas_resize(
      &app->depthBuffer,
      soy_win_width(&app->window)/app->pixelResolution,
      soy_win_height(&app->window)/app->pixelResolution,
      sizeof(soyPixel32));
  soy_draw_update(&app->draw.drawParams,
      app->frameBuffer.width, app->frameBuffer.height, app->frameBuffer.mem);
  app->windowRect = soy_rect("window",
      0,
      0,
      app->frameBuffer.width, app->frameBuffer.height, NULL, NULL, NULL);
  soy_command_manager_update(&app->comMan, &app->window);
  soy_time_measure_from(&app->userTiming);
  return TRUE;
}

sflag soy_app_present(soyAppHandle app) {
  soy_audio_update(app->timing.timestep, &app->audio);
  if(app->hardwareAccelaration) {
    SwapBuffers(app->window.nativeWin.deviceContext);
  } else {
    soy_blit_pixels(&app->window, app->frameBuffer.mem, app->frameBuffer.width, app->frameBuffer.height);
  }
  soy_time_measure_to(&app->userTiming);
  return TRUE;
}

sflag soy_app_destroy(soyAppHandle app) {
  soy_audio_stop(&app->audio);
  soy_time_reset_scheduler(1);
  SLOG("Freeing memory...\n");
  soy_memory_free(&app->memMan, &app->memory.imageLoadMemory);
  soy_memory_free(&app->memMan, &app->memory.imageMemory);
  soy_memory_free(&app->memMan, &app->memory.meshMemory);
  soy_memory_free(&app->memMan, &app->memory.meshLoadMemory);
  soy_memory_free(&app->memMan, &app->memory.framebufferMemory);
  soy_memory_free(&app->memMan, &app->memory.depthbufferMemory);
  soy_memory_free(&app->memMan, &app->memory.clipboardMemory);
  soy_memory_free(&app->memMan, &app->memory.audioSampleMemory);
  soy_memory_free(&app->memMan, &app->memory.audioLoadMemory);
  soy_memory_free(&app->memMan, &app->memory.guiMemory);
  soy_memory_free(&app->memMan, &app->memory.commandMemory);
  soy_memory_free(&app->memMan, &app->memory.triCacheMemory);
  soy_memory_free(&app->memMan, &app->memory.moduleMemory);
  soy_memory_free(&app->memMan, &app->memory.textBoxMem);
  soy_memory_print_state(&app->memMan);
  soy_memory_destroy(&app->memMan);
  soy_window_destroy(&app->window);
  return TRUE;
}

/* @user */

#define SOY_bitSet(x, whence) ((x) |= (1<<(whence)))
#define SOY_bitClear(x, whence) ((x) &= ~(1<<(whence)))
#define SOY_bitFlip(x, whence) ((x) ^= (1<<(whence)))
#define SOY_bitCheck(x, whence) ((x) & (1<<(whence)))

_docstart
_docheader
/* SoyLib User Interface */

_docheader
/* App */

/* Initialize the 'soyApp' structure, allocate base memory. THIS MUST BE CALLED FIRST */
sflag SOY_appCreate (soyAppHandle app, uint32 w, uint32 h, uint32 res, cString title,
    sflag hardwareAccelaration) {
  soy_app_create(app, w, h, res, title, hardwareAccelaration);
  return TRUE;
}

/* Clean up the 'soyApp' structure, deallocating all memory. THIS MUST BE CALLED LAST */
sflag SOY_appDestroy (soyAppHandle app) {
  soy_app_destroy(app);
  return TRUE;
}

/* Update input, timers, handle resizing etc... */
sflag SOY_appBeginFrame (soyAppHandle app) {
  if(app->inputFrames%app->updateStep == 0) {
    soy_app_update(app);
    app->updateFrames++;
  }
  return TRUE;
}

/* Present the current frame's framebuffer and audio stream */
sflag SOY_appEndFrame (soyAppHandle app) {
  if(app->inputFrames%app->renderStep == 0) {
    soy_app_present(app);
    app->renderFrames++;
  }
  app->inputFrames++;
  return TRUE;
}

sflag SOY_appUpdateTiming(soyAppHandle app) {
  return soy_time_update(&app->timing);
}

sflag SOY_appPollInput(soyAppHandle app) {
  return soy_poll_events(&app->window);
}

sflag SOY_appSetRenderStep(soyAppHandle app, int32 step) {
  app->renderStep = step;
  return TRUE;
}

sflag SOY_appSetUpdateStep(soyAppHandle app, int32 step) {
  app->updateStep = step;
  return TRUE;
}

/* Set the app state to 'Running' */
sflag SOY_appStart (soyAppHandle app) {
  app->state = SOY_DONTSTOP;
  return TRUE;
}

/* Set the app state to 'Stop' */
sflag SOY_appStop (soyAppHandle app) {
  app->state = SOY_STOP;
  return TRUE;
}

/* Query if the application sent a quit message */
sflag SOY_appShouldQuit (soyAppHandle app) {
  return soy_should_quit(&app->window) || (app->state == SOY_STOP);
}

/* Desired timestep in ms */
real32 SOY_appTimestep (soyAppHandle app) {
  return app->timing.timestep;
}

/* Set desired timestep in ms */
sflag SOY_appSetTimestep (soyAppHandle app, uint32 timestep) {
  app->userTiming.timestep = 1.0 / timestep;
  app->timing.timestep = 1.0 / timestep;
  return TRUE;
}

/* Get a handle to the app's main font */
soyFontHandle SOY_appGetFont (soyAppHandle app) {
  return &app->draw.font;
}

_docheader
/* Window */

/* Query the framebuffer width */
uint32 SOY_width(soyAppHandle app) {
  return app->frameBuffer.width;
}

/* Query the framebuffer height */
uint32 SOY_height(soyAppHandle app) {
  return app->frameBuffer.height;
}

/* Query the window width */
uint32 SOY_windowWidth (soyAppHandle app) {
  return soy_win_width(&app->window);
}

/* Query the window height */
uint32 SOY_windowHeight (soyAppHandle app) {
  return soy_win_height(&app->window);
}

/* Minimize the application */
sflag SOY_windowMinimize (soyAppHandle app) {
  soy_window_minimize(&app->window);
  return TRUE;
}

/* Query if window is focused */
sflag SOY_windowIsFocused(soyAppHandle app) {
  return soy_window_is_focused(&app->window);
}

/* Fullscreen the application */
sflag SOY_windowFullscreen (soyAppHandle app) {
  app->fullscreened = TRUE;
  soy_window_lock_resize(&app->window);
  soy_window_resize(&app->window, 0, 0, soy_res_x(), soy_res_y());
  return TRUE;
}

/* Query if application is fullscreened */
sflag SOY_windowIsFullscreen (soyAppHandle app) {
  return app->fullscreened;
}

/* Restore the application from fullscreen */
sflag SOY_windowRestore (soyAppHandle app) {
  app->fullscreened = FALSE;
  soy_window_unlock_resize(&app->window);
  soy_window_restore(&app->window);
  return TRUE;
}

/* Set a function the window will use when resizing the window */
sflag SOY_windowSetResizeFunc (soyAppHandle app, pfSoyWinResizeFunc func) {
  soy_set_resize(app, func);
  return TRUE;
}

sflag SOY_windowSetMin(soyAppHandle app, int32 width, int32 height) {
  return soy_window_set_min(&app->window, width*app->pixelResolution, height*app->pixelResolution);
}

_docheader
/* GL */

sflag SGL_viewport(int32 x, int32 y, int32 width, int32 height) {
  return soy_gl_set_viewport(x, y, width, height);
}

sflag SGL_clearColour (real32 r, real32 g, real32 b, real32 a) {
  glClearColor(r, g, b, a);
  return TRUE;
}

sflag SGL_clear (GLbitfield mask) {
  glClear(mask);
  return TRUE;
}

sflag SGL_shaderCreate (soyShaderHandle shader, cString vertFile, cString fragFile) {
  return soy_gl_shader_create(vertFile, fragFile, shader);
}

sflag SGL_shaderUse (soyShaderHandle shader) {
  return soy_gl_shader_use(shader);
}

sflag SGL_shaderSetInt(soyShaderHandle shader, cString label, int32 val) {
  return soy_gl_shader_set_int(shader, label, val);
}

sflag SGL_shaderSetFloat(soyShaderHandle shader, cString label, real32 val) {
  return soy_gl_shader_set_float(shader, label, val);
}

sflag SGL_genBuffers(uint32 i, uint32* buffer) {
  soy_gl_gen_buffers(i, buffer);
  return TRUE;
}

sflag SGL_genVertexArrays(uint32 i, uint32* buffer) {
  soy_gl_gen_vertex_arrays(i, buffer);
  return TRUE;
}

sflag SGL_bindVertexArray(uint32 arrayID) {
  soy_gl_bind_vertex_array(arrayID);
  return TRUE;
}

sflag SGL_bindBuffer(GLenum target, uint32 bufferID) {
  soy_gl_bind_buffer(target, bufferID);
  return TRUE;
}

sflag SGL_bufferData(GLenum target, GLsizei size, void* verts) {
  soy_gl_buffer_data(target, size, verts);
  return TRUE;
}

sflag SGL_drawArrays(GLenum mode, GLint startIndex, GLsizei vertCount) {
  soy_gl_draw_arrays(mode, startIndex, vertCount);
  return TRUE;
}

sflag SGL_vertexAttribPointer(GLuint index, GLint amt, GLenum type, GLboolean norm,
    GLsizei stride, const GLvoid* offset) {
  return soy_gl_vertex_attrib_pointer(index, amt, type, norm, stride, offset);
}

sflag SGL_vertexAttribEnable(uint32 index) {
  return soy_gl_vertex_attrib_enable(index);
}

sflag SGL_genTextures(GLuint num, soyGLTexture* texture) {
  return soy_gl_gen_textures(num, &texture->ID);
}

sflag SGL_bindTexture(GLenum type, soyGLTexture* texture) {
  return soy_gl_bind_texture(type, texture->ID);
}

sflag SGL_texImage2D(
    GLenum target,
    GLint level,
    GLint iFormat,
    GLsizei width,
    GLsizei height,
    GLint border,
    GLenum format,
    GLenum type,
    const void* data) {
  return 
    soy_gl_tex_image_2d(target,
      level, iFormat, width, height, border,
      format, type, data);
}

sflag SGL_activeTexture(GLenum texture) {
  return soy_gl_active_texture(texture);
}

sflag SGL_genMipmaps(GLenum type) {
  return soy_gl_gen_mipmaps(type);
}

_docheader
/* Memory */

_docend
#define SOY_memoryType(t, m) ((t*)m.memory)
#define SOY_memoryTypeP(t, m) ((t*)m->memory)
_docstart


/* Allocate a block of memory and initialize the virtual memory struct */
sflag SOY_memoryEnslave (soyAppHandle app, soyVirtualMemory* mem, cString name, uint32 size) {
  return soy_memory_enslave(&app->memMan, mem, size, 1, name);
}

/* Deallocate the memory block previously allocated by memoryEnslave */
sflag SOY_memoryFree (soyAppHandle app, soyVirtualMemory* mem) {
  return soy_memory_free(&app->memMan, mem);
}

/* Reset the memory counters */
sflag SOY_memoryReset (soyVirtualMemory* mem) {
  mem->back = 0;
  mem->front = 0;
  return TRUE;
}

_docheader
/* Input */

/* Query if key was pressed this frame */
sflag SOY_keyPressed (soyAppHandle app, uint32 key) {
  return soy_key_pressed(&app->window, key);
}

/* Query if any key was pressed this frame */
sflag SOY_keyPressedAny (soyAppHandle app) {
  return soy_any_key_pressed(&app->window);
}

/* Query how many keys were pressed this frame */
uint32 SOY_keyPressedCount (soyAppHandle app) {
  return soy_key_pressed_count(&app->window);
}

/* Query the last pressed key */
uint32 SOY_keyLast (soyAppHandle app) {
  return soy_last_key(&app->window);
}

/* Query if key was pressed this frame asynchronously */
sflag SOY_keyPressedAsync (uint32 key) {
  return soy_key_pressed_async(key);
}

/* Query if mouse was scrolled this frame */
int32 SOY_mouseScrolled (soyAppHandle app) {
  return soy_mouse_scrolled(&app->window);
}

/* Query if mouse was moved this frame */
sflag SOY_mouseMoved (soyAppHandle app) {
  return soy_mouse_moved(&app->window);
}

/* Get the mouse scroll wheel coordinate */
int32 SOY_mouseScroll (soyAppHandle app) {
  return soy_mouse_scroll(&app->window);
}

/* Mouse horizontal position in the window */
int32 SOY_mouseX (soyAppHandle app) {
  return soy_mouse_x(&app->window);
}

/* Mouse vertical position in the window */
int32 SOY_mouseY (soyAppHandle app) {
  return soy_mouse_y(&app->window);
}

/* Mouse horizontal position on the screen */
int32 SOY_mouseScreenX () {
  return soy_mouse_desktop_x();
}

/* Mouse vertical position on the screen */
int32 SOY_mouseScreenY () {
  return soy_mouse_desktop_y();
}

/* Query if middle mouse button was pressed */
int32 SOY_mouseMiddle (soyAppHandle app) {
  return soy_mouse_pressed(&app->window, SOY_MOUSE_MIDDLE);
}

/* Query if right mouse button was pressed */
int32 SOY_mouseRight (soyAppHandle app) {
  return soy_mouse_pressed(&app->window, SOY_MOUSE_RIGHT);
}

/* Query if left mouse button was pressed */
int32 SOY_mouseLeft (soyAppHandle app) {
  return soy_mouse_pressed(&app->window, SOY_MOUSE_LEFT);
}

/* Limit the input to characters used in hexadecimal */
sflag SOY_keyIsHex (uint32 key) {
  return soy_key_is_hex(key);
}

/* Limit the input to numbers, letters and punctuation */
sflag SOY_keyIsAlphanumeric (uint32 key) {
  return soy_key_is_alphanumeric(key);
}

_docend
_docheader
/* Utilities */

real64 SOY_absf(real64 a) { return (a < 0.0) ? (a*-1.0) : (a); }
uint32 SOY_abs(int32 a) { return (a < 0) ? (a*-1) : (a); }

#define SOY_max(a, b) ((a > b) ? a : b)
#define SOY_min(a, b) ((a < b) ? a : b)
_docstart

_docheader
/* String utils */

/* Get the lenght of a NULL TERMINATED string */
uint32 SOY_stringLen (cString s) {
  return soy_cstring_len(s);
}

/* Get the lenght of a NULL TERMINATED string up to the n-th found token (single char) */
uint32 SOY_stringLenTok (cString s, int8 tok, uint32 index) {
  return soy_cstring_lentok(s, tok, index);
}

/* Compare two NULL TERMINATED strings */
uint32 SOY_stringCompare (cString s, cString f) {
  return soy_cstring_compare(s, f);
}

sflag SOY_stringMatch(cString a, cString b) {
  uint32 i, j;
  uint32 match = 0;
  uint32 lenA = SOY_stringLen(a);
  uint32 lenB = SOY_stringLen(b);
  if(lenB > lenA) { return FALSE; }
  for(i = 0; i < lenA; i++) {
    match = 0;
    for(j = 0; j < lenB; j++) {
      if(i+j < lenA) {
        if(b[j] == a[i+j]) {
          match++;
        }
      }
    }
    if(match == lenB) { return TRUE; }
  }
  return FALSE;
}

sflag SOY_stringMatchAmount(cString a, cString b, uint32 amount) {
  uint32 i, j;
  uint32 match = 0;
  uint32 lenA = SOY_stringLen(a);
  uint32 lenB = SOY_stringLen(b);
  uint32 matchCount = 0;
  if(lenB > lenA) { return FALSE; }
  for(i = 0; i < lenA; i++) {
    match = 0;
    for(j = 0; j < lenB; j++) {
      if(i+j < lenA) {
        if(b[j] == a[i+j]) {
          match++;
        }
      }
    }
    if(match == lenB) {
      match = 0;
      matchCount++;
    }
    if(matchCount == amount) {
      return TRUE;
    }
  }
  return FALSE;
}

sflag SOY_stringMatchEnd(cString a, cString b) {
  uint32 i, j;
  uint32 match = 0;
  uint32 lenA = SOY_stringLen(a);
  uint32 lenB = SOY_stringLen(b);
  if(lenB > lenA) { return FALSE; }
  for(i = 0; i < lenA; i++) {
    match = 0;
    for(j = 0; j < lenB; j++) {
      if(i+j < lenA) {
        if(b[j] == a[i+j]) {
          match++;
        }
      }
    }
    if(match == lenB) {
      if(i == lenA-lenB) {
        return TRUE;
      } else {
        return FALSE;
      }
    }
  }
  return FALSE;
}

_docend
#define SOY_toString(x) STR(x)
_docstart
 
_docheader
/* Files */

/* Wrapper around fopen */
soyFile* SOY_openFile (cString file, cString args) {
  return soy_file_open(file, args);
}

/* Wrapper around fclose */
sflag SOY_closeFile (soyFile* file) {
  return soy_file_close(file);
}

#define SOY_MAX_FILES 256

typedef struct tag_soyListFileData {
  uint32 count;
  sflag isDir[SOY_MAX_FILES];
  sflag isNormal[SOY_MAX_FILES];
  sflag isHidden[SOY_MAX_FILES];
  int8 filenames[SOY_MAX_FILES][0xFF];
} soyListFileData;

sflag SOY_listFiles(soyFileData* data, soyHandle args) {
  soyListFileData* fdata = (soyListFileData*) args;
  if(args == NULL) { SLOG("Args were null\n"); return FALSE; }
  if(data->indexInDir > SOY_MAX_FILES) { SLOG("Exceeded max files\n"); return FALSE; }
  if(data->isDir) {
    fdata->isDir[fdata->count] = TRUE;
  } else {
    fdata->isDir[fdata->count] = FALSE;
  }
  if(data->isNormal) {
    fdata->isNormal[fdata->count] = TRUE;
  } else {
    fdata->isNormal[fdata->count] = FALSE;
  }
  if(data->isHidden) {
    fdata->isHidden[fdata->count] = TRUE;
  } else {
    fdata->isHidden[fdata->count] = FALSE;
  }
  sprintf(fdata->filenames[fdata->count++], "%s", data->cName);
  return (data != NULL);
}

sflag SOY_listFilesInDir(soyAppHandle app,
    cString dir, cString fmt, pfSoyWin32FileListFunc func, soyListFileData* data) {
  data->count = 0;
  if(func == NULL) { SLOG("Error\n"); return FALSE; }
  return soy_list_dir((soyHandle)app, dir, fmt, func, (soyHandle)data);
}

sflag SOY_currentDir(uint32 bufLen, int8* buffer) {
  return soy_current_dir(bufLen, buffer);
}

sflag SOY_fileDir(cString file, uint32 bufLen, int8* buffer) {
  return soy_file_dir(file, bufLen, buffer);
}

_docheader
/* Commands */

/* Add new command with a single key */
soyCommandHandle SOY_commandAdd (soyAppHandle app,
    pfSoyCommandAction action,
    void* args,
    uint32 key,
    sflag toggle,
    uint32 repeat,
    uint32 delay,
    cString name
    ) {
  soyCommand newCom = soy_command (action, args, key, toggle, repeat, delay, name);
  return soy_command_manager_push(&app->comMan, newCom);
}

/* Add new command with multiple keys */
soyCommandHandle SOY_commandAddMulti (soyAppHandle app,
    pfSoyCommandAction action,
    void* args,
    uint8* keys,
    uint32 count,
    sflag toggle,
    uint32 repeat,
    uint32 delay,
    cString name
    ) {
  uint32 i = 0;
  soyCommand newCom = soy_command (action, args, keys[i++], toggle, repeat, delay, name);
  while(i < count) {
    soy_command_push_key(&newCom, keys[i++]);
  }
  return soy_command_manager_push (&app->comMan, newCom);
}

/* Add new command with two keys */
soyCommandHandle SOY_commandAddDuo (soyAppHandle app,
    pfSoyCommandAction action,
    void* args,
    uint32 key1,
    uint32 key2,
    sflag toggle,
    uint32 repeat,
    uint32 delay,
    cString name
    ) {
  soyCommand newCom = soy_command (action, args, key1, toggle, repeat, delay, name);
  soy_command_push_key(&newCom, key2);
  return soy_command_manager_push(&app->comMan, newCom);
}

/* Add new command with a single key */
soyCommandHandle SOY_commandAddExc (soyAppHandle app, pfSoyCommandAction action,
    void* args,
    uint32 key,
    sflag toggle,
    uint32 repeat,
    uint32 delay,
    cString name) {
  soyCommandHandle newCom = 
  SOY_commandAdd(app, action, args, key, toggle, repeat, delay, name);
  soy_command_restrict_input(newCom, TRUE);
  return newCom;
}

/* Add new command with two keys */
soyCommandHandle SOY_commandAddDuoExc (soyAppHandle app, pfSoyCommandAction action,
    void* args,
    uint32 key1,
    uint32 key2,
    sflag toggle,
    uint32 repeat,
    uint32 delay,
    cString name) {
  soyCommandHandle newCom = 
  SOY_commandAddDuo(app, action, args, key1, key2, toggle, repeat, delay, name);
  soy_command_restrict_input(newCom, TRUE);
  return newCom;
}

/* Find a command by name and retrieve it's handle */
soyCommandHandle SOY_commandFind (soyAppHandle app, const char* label) {
  return soy_command_find_by_name(&app->comMan, label);
}

/* Remove a command from the active commands list */
sflag SOY_commandRemove (soyCommandHandle command) {
  if(command == NULL) { SLOG("Command not found\n"); return FALSE; }
  command->enabled = FALSE;
  command->overWrite = TRUE;
  command->name = NULL;
  return TRUE;
}

/* Query the amount of times command has been fired */
sflag SOY_commandCount (soyCommandHandle command) {
  return soy_command_count(command);
}

/* Defragment command list memory */
sflag SOY_commandDefrag (soyAppHandle app) {
  return soy_command_manager_defragment(&app->comMan);
}
_docheader
/* Textbox */

/* Recieve a text box object */
soyTextBox SOY_textBox (soyAppHandle app, cString name,
    int32 x, int32 y, uint32 charsW, uint32 charsH, pfSoyTextBoxFunc action) {
  uint32 msize = charsW*charsH+1;
  soyTextBox ret;
  ret.mem = NULL;
  if(app->memory.textBoxMem.back+msize < app->memory.textBoxMem.size) {
    ret = soy_textbox(name, x, y, charsW, charsH,
        app->memory.textBoxMem.memory+app->memory.textBoxMem.back,
        app->memory.textBoxMem.size,
        &app->draw.font, NULL, action);
    app->memory.textBoxMem.back+=msize;
  } else {
    SLOG("Not enough memory for textbox\n");
  }
  return ret;
}

/* Update a text box object */
sflag SOY_textBoxUpdate (soyAppHandle app, soyTextBoxHandle box) {
  return soy_textbox_update(&app->window, &app->memory.clipboardMemory, box);
}

/* Update a text box object */
int8* SOY_textBoxGetText (soyTextBoxHandle box) {
  return ((int8*)box->mem);
}

/* Paste null terminated string into the textbox */
sflag SOY_textBoxPaste (soyTextBoxHandle box, cString c) {
  soy_textbox_clear(box);
  return soy_textbox_paste(box, c);
}

_docheader
/* Buttons */

/* Recieve a button object */
soyButton SOY_button (soyAppHandle app, cString name, int32 x, int32 y, soyCommandHandle command) {
  return soy_button(name, &app->draw.font, x, y, command);
}

/* Recieve an empty button object */
soyButton SOY_buttonBlank (soyAppHandle app, cString name, soyCommandHandle command) {
  return soy_button(name, &app->draw.font, 0, 0, command);
}

soyButton SOY_buttonMicro (cString name, int32 x, int32 y, soyCommandHandle command) {
  return soy_button_spr(name, x, y, SPR_MICRO_SIZE, SPR_MICRO_SIZE, command);
}

/* Move a button object */
sflag SOY_buttonMove (soyButtonHandle button, int32 x, int32 y) {
  button->rect.x = x;
  button->rect.y = y;
  return TRUE;
}

/* Resize a button object */
sflag SOY_buttonResize (soyButtonHandle button, int32 x, int32 y, uint32 width, uint32 height) {
  button->rect.x = x;
  button->rect.y = y;
  button->rect.width = width;
  button->rect.height = height;
  return TRUE;
}

/* Update a button object */
sflag SOY_buttonUpdate (soyAppHandle app, soyButtonHandle button) {
  return soy_button_update(&app->window, button);
}

_docheader
/* GUI */

/* Recieve an interactive rectangle object */
soyRect SOY_rect (cString label, int32 x, int32 y, uint32 width, uint32 height,
    soyCommandHandle lcommand, soyCommandHandle rcommand) {
  return soy_rect(label, x, y, width, height, lcommand, rcommand, NULL);
}

/* Recieve an interactive rectangle object for retards */
soyRect SOY_rectSimple (cString label, int32 x, int32 y, uint32 width, uint32 height) {
  return soy_rect(label, x, y, width, height, NULL, NULL, NULL);
}

sflag SOY_rectSetArmable(soyRectHandle rect) {
  return soy_rect_set_armable(rect);
}

sflag SOY_rectUnsetArmable(soyRectHandle rect) {
  return soy_rect_unset_armable(rect);
}

/* Update a interactive rectangle object */
sflag SOY_rectUpdate (soyAppHandle app, soyRectHandle rect) {
  return soy_rect_update(rect, SOY_mouseX(app), SOY_mouseY(app), SOY_mouseLeft(app), SOY_mouseRight(app));
}

sflag SOY_rectResize (soyRectHandle rect,
    int32 x, int32 y, uint32 W, uint32 H) {
  rect->x = x;
  rect->y = y;
  rect->width = W;
  rect->height = H;
  return TRUE;
}

sflag SOY_rectResizeWidth (soyRectHandle rect, uint32 W) {
  rect->width = W;
  return TRUE;
}

sflag SOY_rectResizeHeight (soyRectHandle rect, uint32 H) {
  rect->height = H;
  return TRUE;
}

/* Resize a interactive rectangle object relative to it's parent */
sflag SOY_rectResizeRel (soyRectHandle child, soyRectHandle parent,
    int32 padX, int32 padY, uint32 padW, uint32 padH) {
  if(child == NULL || parent == NULL) { return FALSE; }
  child->x = parent->x+padX;
  child->y = parent->y+padY;
  child->width = parent->width-padW;
  child->height = parent->height-padH;
  return TRUE;
}

/* Resize a interactive rectangle object relative to it's parent */
sflag SOY_rectResizeRelWidth (soyRectHandle child, soyRectHandle parent,
    int32 padW) {
  if(child == NULL || parent == NULL) { return FALSE; }
  child->width = parent->width-padW;
  return TRUE;
}

/* Resize a interactive rectangle object relative to it's parent */
sflag SOY_rectResizeRelHeight (soyRectHandle child, soyRectHandle parent,
    int32 padH) {
  if(child == NULL || parent == NULL) { return FALSE; }
  child->height = parent->height-padH;
  return TRUE;
}

/* Move a interactive rectangle object relative to it's parent */
sflag SOY_rectMoveRel (soyRectHandle child, soyRectHandle parent,
    int32 padX, int32 padY) {
  if(child == NULL || parent == NULL) { return FALSE; }
  child->x = parent->x+padX;
  child->y = parent->y+padY;
  return TRUE;
}

sflag SOY_rectMoveRelRight (soyRectHandle child, soyRectHandle parent,
    int32 padX, int32 padY) {
  if(child == NULL || parent == NULL) { return FALSE; }
  child->x = parent->x+parent->width-child->width-padX;
  child->y = parent->y+padY;
  return TRUE;
}

/* Get a handle to the window's draw rectangle */
soyRectHandle SOY_windowRect (soyAppHandle app) {
  return &app->windowRect;
}

/* Leftmost coordiante of rect */
int32 SOY_rectLeft (soyRectHandle rect) {
  return rect->x;
}

/* Rightmost coordiante of rect */
int32 SOY_rectRight (soyRectHandle rect) {
  return rect->x+rect->width;
}

/* Topmost coordiante of rect */
int32 SOY_rectTop (soyRectHandle rect) {
  return rect->y;
}

/* Nethermost coordiante of rect */
int32 SOY_rectBottom (soyRectHandle rect) {
  return rect->y+rect->height;
}

/* Query if the mouse is hovering above the rect */
sflag SOY_rectHover (soyRectHandle rect) {
  return soy_bit_check(rect->state, SOY_RECT_HOVER);
}

/* Query if nothing is happening with the rect */
sflag SOY_rectIdle (soyRectHandle rect) {
  return soy_bit_check(rect->state, SOY_RECT_IDLE);
}

/* Query if left mouse button was pressed on the rect */
sflag SOY_rectPressedL (soyRectHandle rect) {
  return (soy_bit_check(rect->state, SOY_RECT_PRESSL) && soy_bit_check(rect->state, SOY_RECT_HOVER));
}

/* Query if right mouse button was pressed on the rect */
sflag SOY_rectPressedR (soyRectHandle rect) {
  return (soy_bit_check(rect->state, SOY_RECT_PRESSR) && soy_bit_check(rect->state, SOY_RECT_HOVER));
}

/* Query if right mouse button was released on the rect */
sflag SOY_rectReleased (soyRectHandle rect) {
  return soy_bit_check(rect->state, SOY_RECT_RELEASED);
}

/* Query if rect is selected */
sflag SOY_rectSelected (soyRectHandle rect) {
  return soy_bit_check(rect->state, SOY_RECT_SELECTED);
}

sflag SOY_rectSelect (soyRectHandle rect) {
  return soy_rect_select(rect);
}

sflag SOY_rectDeselect (soyRectHandle rect) {
  return soy_rect_deselect(rect);
}

/* Load the gui palette from file. If file isn't found a default palette is initialized */
sflag SOY_loadGUIPalette (soyAppHandle app, cString file) {
  return soy_draw_load_gui_palette(&app->draw.guiPalette, file);
}

_docheader
/* Drawing */

/* Set the current draw extents. */
sflag SOY_drawSetExtents (soyAppHandle app, int32 x, int32 y, uint32 width, uint32 height) {
  if((uint32)width*height > (uint32)app->frameBuffer.width*app->frameBuffer.height) { return FALSE; }
  app->draw.drawParams.memWidth = width;
  app->draw.drawParams.memHeight = height;
  app->draw.drawParams.memX = x;
  app->draw.drawParams.memY = y;
  return TRUE;
}

/* Reset the current draw extents. */
sflag SOY_drawResetExtents (soyAppHandle app) {
  app->draw.drawParams.memWidth = app->frameBuffer.width;
  app->draw.drawParams.memHeight = app->frameBuffer.height;
  app->draw.drawParams.memX = 0;
  app->draw.drawParams.memY = 0;
  return TRUE;
}

sflag SOY_enableSpriteGUI(soyAppHandle app, sflag enable) {
  if(app->draw.guiAtlas.rawSize > 0) {
    app->draw.spriteGUI = enable;
  }
  return TRUE;
}

/* Set the current drawing colour */
sflag SOY_drawSetColour (soyAppHandle app, uint8 r, uint8 g, uint8 b, uint8 a) {
  return soy_draw_set_colour(&app->draw.drawParams, r, g, b, a);
}

sflag SOY_drawSetTint (soyAppHandle app, real32 r, real32 g, real32 b) {
  return soy_draw_set_tint(&app->draw.drawParams, r, g, b);
}


sflag SOY_drawSetEmission (soyAppHandle app, real32 r, real32 g, real32 b) {
  return soy_draw_set_emission(&app->draw.drawParams, r, g, b);
}

/* Set the current drawing colour as a pixel */
sflag SOY_drawSetPixel (soyAppHandle app, soyPixel32 pix) {
  app->draw.drawParams.curColour = pix;
  return TRUE;
}

/* Set the current drawing colour as a palette index */
sflag SOY_drawSetPaletteColour (soyAppHandle app, guiColour col) {
  soyPixel32 p = app->draw.guiPalette.colours[col];
  app->draw.drawParams.curColour = p;
  global_simdPixel.raw[0] = p;
  global_simdPixel.raw[1] = p;
  global_simdPixel.raw[2] = p;
  global_simdPixel.raw[3] = p;
  return TRUE;
}

/* Set the current drawing colour as a palette index */
sflag SOY_drawSetPaletteTint (soyAppHandle app, real32 r, real32 g, real32 b) {
  return soy_draw_set_gui_tint(&app->draw.guiPalette, b, g, r);
}

/* Set the current drawing colour as a palette index */
sflag SOY_drawSetPaletteEmission (soyAppHandle app, real32 r, real32 g, real32 b) {
  return soy_draw_set_gui_emission(&app->draw.guiPalette, b, g, r);
}

/* Set the current drawing colour as a palette index */
sflag SOY_drawResetPaletteTint (soyAppHandle app) {
  return soy_draw_reset_gui_tint(&app->draw.guiPalette);
}

soyPixel32 SOY_drawGetPaletteColour (soyAppHandle app, guiColour col) {
  return app->draw.guiPalette.colours[col];
}

/* Clear the screen with a solid colour */
sflag SOY_drawClear (soyAppHandle app) {
  return soy_draw_clear(&app->draw.drawParams);
}

/* Clear the depthbuffer */
sflag SOY_drawClearDepth (soyAppHandle app) {
  soy_draw_clear_depth_buffer(&app->draw.drawParams, &app->draw.draw3DParams.drawRect, app->depthBuffer.mem);
  return TRUE;
}

/* Draw a single pixel */
sflag SOY_drawPixel (soyAppHandle app, int32 x, int32 y) {
  return soy_draw_pixel(&app->draw.drawParams, x, y);
}

/* Draw a line using Bresenham's algorithm */
sflag SOY_drawLine (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2) {
  return soy_draw_line(&app->draw.drawParams, x1, y1, x2, y2);
}

/* Draw a rectangle with 1 pixel thick outline */
sflag SOY_drawRect (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2) {
  return soy_draw_rect(&app->draw.drawParams, x1, y1, x2, y2);
}

/* Draw a filled rectangle */
sflag SOY_drawRectF (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return SOY_drawRect(app, x1+1, y1+1, x2-1, y2-1);
  } else {
    return soy_draw_fill(&app->draw.drawParams, x1, y1, x2, y2);
  }
}

/* Draw a gradient. 'v' = vertical 'h' = horizontal 'r' = radial */
sflag SOY_drawGradient (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2,
    soyPixel32 colA, soyPixel32 colB, int8 type) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return SOY_drawRect(app, x1+1, y1+1, x2-1, y2-1);
  }
  switch(type) {
    case('v'): {
      soy_draw_gradient_v(&app->draw.drawParams, x1, y1, x2, y2, colA, colB);
      break;
    }
    case('h'): {
      soy_draw_gradient_h(&app->draw.drawParams, x1, y1, x2, y2, colA, colB);
      break;
    }
    case('r'): {
      soy_draw_gradient_c(&app->draw.drawParams, x1, y1, x2, y2, colA, colB);
      break;
    }
    default: { break; }
  }
  return TRUE;
}

/* Draw a rectangle masked by a pattern */
sflag SOY_drawPattern (soyAppHandle app, cString pattern,
    int32 shift, int32 x1, int32 y1, int32 x2, int32 y2) {
  return soy_draw_fill_pattern(&app->draw.drawParams, pattern, shift, x1, y1, x2, y2);
}

/* Draw an elipse with 1 pixel thick outline */
sflag SOY_drawEllipse (soyAppHandle app, int32 x, int32 y, int32 w, int32 h) {
  return soy_draw_ellipse(&app->draw.drawParams, x, y, w, h);
}

sflag SOY_drawCircleF(soyAppHandle app, int32 xm, int32 ym, int32 r) {
  return soy_draw_circle_fill(&app->draw.drawParams, xm, ym, r);
}

  /* Draw a filled triangle */
sflag SOY_drawTriangleF (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3) {
  return soy_draw_triangle_filled(&app->draw.drawParams, x1, y1, x2, y2, x3, y3);
}

/* Draw a mask in a solid colours */
sflag SOY_drawMask (soyAppHandle app, int32 x, int32 y, const int8* spr, uint32 sideLen) {
  soy_draw_mask(&app->draw.drawParams, x, y, spr, sideLen);
  return TRUE;
}

/* Draw a 24bit bitmap image */
sflag SOY_drawBitmap (soyAppHandle app, int32 x, int32 y, int32 width, int32 height, soyBitmapHandle bitmap) {
  return soy_draw_bitmap32(&app->draw.drawParams, x, y, bitmap->bytes,
      bitmap->width, bitmap->height,
      width, height
      );
}

/* draw a fixed size sprite */
sflag SOY_drawSprite (soyAppHandle app, int32 x, int32 y, soyBitmapHandle bitmap) {
  return soy_draw_sprite(&app->draw.drawParams, bitmap, x, y, bitmap->width, bitmap->height, 0, 0);
}

/* Draw a repeating background */
sflag SOY_drawBackground (soyAppHandle app, soyBitmapHandle bitmap) {
  return soy_draw_background(&app->draw.drawParams, bitmap);
}

/* Draw a single character */
sflag SOY_drawChar (soyAppHandle app, int32 x, int32 y, int8 glyph) {
  return soy_draw_char(&app->draw.drawParams, x, y, glyph, &app->draw.font);
}

/* Draw text in the default GUI style */
sflag SOY_drawGUIText (soyAppHandle app, int32 x, int32 y, cString text) {
  return soy_draw_text(&app->draw.drawParams, x, y, text, soy_cstring_len(text), &app->draw.font);
}

/* Draw text with specified font */
sflag SOY_drawText (soyAppHandle app, int32 x, int32 y, cString text, soyFontHandle font) {
  return soy_draw_text_optimized(&app->draw.drawParams, x, y, text, font);
}

/* Query how much horizontal space the drawn text would take up in pixels using the specified font */
uint32 SOY_drawTextLen (cString text, soyFontHandle font) {
  return soy_draw_text_len(text, soy_cstring_len(text), font);
}

/* Draw faux-bold text with specified font */
sflag SOY_drawTextB (soyAppHandle app, int32 x, int32 y, cString text, soyFontHandle font) {
  soy_draw_text_optimized_bold(&app->draw.drawParams, x, y, text, font);
  return TRUE;
}

/* Draw faux-italix text with specified font and desired shear amount */
sflag SOY_drawTextI (soyAppHandle app, int32 x, int32 y, cString text, soyFontHandle font, real32 amnt) {
  return soy_draw_text_italic(&app->draw.drawParams, x, y, text, soy_cstring_len(text), font, amnt);
}

/* Draw faux-italix faux-bold text with specified font and desired shear amount */
sflag SOY_drawTextIB (soyAppHandle app, int32 x, int32 y, cString text, soyFontHandle font, real32 amnt) {
  soy_draw_text_italic(&app->draw.drawParams, x, y, text, soy_cstring_len(text), font, amnt);
  soy_draw_text_italic(&app->draw.drawParams, x+1, y, text, soy_cstring_len(text), font, amnt);
  return TRUE;
}

/* Set 3D drawing extents */
sflag SOY_draw3DSetExtents (soyAppHandle app, int32 x, int32 y, uint32 width, uint32 height) {
  app->draw.draw3DParams.mainCamera = soy_3d_camera(width, height);
  app->draw.draw3DParams.drawRect.x = x;
  app->draw.draw3DParams.drawRect.y = y;
  app->draw.draw3DParams.drawRect.width = width;
  app->draw.draw3DParams.drawRect.height = height;
  return TRUE;
}

/* Draw a 3D model */
sflag SOY_draw3DModel (soyAppHandle app, soyModelHandle model, soyBitmapHandle texture,
    real32 x, real32 y, real32 z,
    real32 rx, real32 ry, real32 rz,
    real32 sx, real32 sy, real32 sz) {
  soyTransform t = soy_transform(x, y, z, rx, ry, rz, sx, sy, sz);
  return soy_3d_draw_model(
      &app->draw.drawParams,
      &app->draw.draw3DParams,
      (real32*)app->depthBuffer.mem,
      model, texture, t);
}

/* Change rendering mode. textured/solid/wireframe */
sflag SOY_draw3DRenderMode (soyAppHandle app, soyRenderMode mode) {
  return soy_3d_change_mode(&app->draw.draw3DParams, mode);
}

sflag first = FALSE;

sflag SOY_drawSpriteGUI(soyAppHandle app,
    int32 ix, int32 iy, int32 width, int32 height, uint32 index) {
  uint32 x, y;
  soyBitmap* guiAtlas = &app->draw.guiAtlas;
  uint32 pW = width+1;
  uint32 pH = height+1;
  uint32 bitmapWidth = guiAtlas->width;
  uint32 bitmapGrid = 16;
  uint32 seg = 4;
  uint32 indX = (index%4)*bitmapGrid;
  uint32 indY = (index/4)*bitmapGrid;
  uint32 rightPosBmp = bitmapGrid-seg;
  uint32 botPosBmp = bitmapGrid-seg;
  uint32 rightPosCanv = pW-seg;
  uint32 botPosCanv = pH-seg;
  soyPixel32* atlas = ((soyPixel32*)guiAtlas->bytes);
  soyPixel32 curColour;
  if(guiAtlas->bytes == NULL || guiAtlas->rawSize == 0) {
    return FALSE;
  }
  if(width < 0 || height < 0) { return FALSE; }
  for(x = 0; x < seg; x++) {
    for(y = 0; y < seg; y++) {
      curColour = ((atlas[(x+indX)+(y+indY)*bitmapWidth]));
      if(!(curColour.r == 255 && curColour.g == 0 && curColour.b == 255)) {
        SOY_drawSetPaletteColour(app, soy_draw_convert_colour_to_index(curColour));
        SOY_drawPixel(app, ix+x, iy+y);
      }
    }
  }
  
  for(x = 0; x < seg; x++) {
    for(y = 0; y < seg; y++) {
      curColour = ((atlas[(x+rightPosBmp+indX)+(y+indY)*bitmapWidth]));
      if(!(curColour.r == 255 && curColour.g == 0 && curColour.b == 255)) {
        SOY_drawSetPaletteColour(app, soy_draw_convert_colour_to_index(curColour));
        SOY_drawPixel(app, ix+rightPosCanv+x, iy+y);
      }
    }
  }
  
  for(x = 0; x < seg; x++) {
    for(y = 0; y < seg; y++) {
      curColour = ((atlas[(x+rightPosBmp+indX)+(y+botPosBmp+indY)*bitmapWidth]));
      if(!(curColour.r == 255 && curColour.g == 0 && curColour.b == 255)) {
        SOY_drawSetPaletteColour(app, soy_draw_convert_colour_to_index(curColour));
        SOY_drawPixel(app, ix+rightPosCanv+x, iy+botPosCanv+y);
      }
    }
  }
  
  for(x = 0; x < seg; x++) {
    for(y = 0; y < seg; y++) {
      curColour = ((atlas[(x+indX)+(y+botPosBmp+indY)*bitmapWidth]));
      if(!(curColour.r == 255 && curColour.g == 0 && curColour.b == 255)) {
        SOY_drawSetPaletteColour(app, soy_draw_convert_colour_to_index(curColour));
        SOY_drawPixel(app, ix+x, iy+botPosCanv+y);
      }
    }
  }
  
  for(x = 0; x < pW-seg*2; x++) {
    for(y = 0; y < seg; y++) {
      curColour = ((atlas[(seg+x%(seg*2)+indX)+(y+indY)*bitmapWidth]));
      if(!(curColour.r == 255 && curColour.g == 0 && curColour.b == 255)) {
        SOY_drawSetPaletteColour(app, soy_draw_convert_colour_to_index(curColour));
        SOY_drawPixel(app, ix+seg+x, iy+y);
      }
    }
  }
  
  for(x = 0; x < pW-seg*2; x++) {
    for(y = 0; y < seg; y++) {
      curColour = ((atlas[(seg+x%(seg*2)+indX)+(y+botPosBmp+indY)*bitmapWidth]));
      if(!(curColour.r == 255 && curColour.g == 0 && curColour.b == 255)) {
        SOY_drawSetPaletteColour(app, soy_draw_convert_colour_to_index(curColour));
        SOY_drawPixel(app, ix+seg+x, iy+botPosCanv+y);
      }
    }
  }
  
  for(y = 0; y < pH-seg*2; y++) {
    for(x = 0; x < seg; x++) {
      curColour = ((atlas[(x+indX)+(seg+y%(seg*2)+indY)*bitmapWidth]));
      if(!(curColour.r == 255 && curColour.g == 0 && curColour.b == 255)) {
        SOY_drawSetPaletteColour(app, soy_draw_convert_colour_to_index(curColour));
        SOY_drawPixel(app, ix+x, iy+seg+y);
      }
    }
  }
 
  for(y = 0; y < pH-seg*2; y++) {
    for(x = 0; x < seg; x++) {
      curColour = ((atlas[(x+rightPosBmp+indX)+(seg+y%(seg*2)+indY)*bitmapWidth]));
      if(!(curColour.r == 255 && curColour.g == 0 && curColour.b == 255)) {
        SOY_drawSetPaletteColour(app, soy_draw_convert_colour_to_index(curColour));
        SOY_drawPixel(app, ix+rightPosCanv+x, iy+seg+y);
      }
    }
  }

  if(index < 8) {
    for(y = 0; y < pH-seg*2; y++) {
      for(x = 0; x < pW-seg*2; x++) {
        curColour = ((atlas[(seg+(x%(seg*2))+indX)+(seg+(y%(seg*2))+indY)*bitmapWidth]));
        if(!(curColour.r == 255 && curColour.g == 0 && curColour.b == 255)) {
          SOY_drawSetPaletteColour(app, soy_draw_convert_colour_to_index(curColour));
          SOY_drawPixel(app, ix+seg+x, iy+seg+y);
        }  
      }
    }
  }
  return TRUE;
}

sflag SOY_drawSpriteGUIMicro(soyAppHandle app,
    int32 x, int32 y, int32 width, int32 height, uint32 index) {
  int32 grid = 16;
  int32 seg = 8;
  int32 dx, dy;
  soyBitmap* guiAtlas = &app->draw.guiAtlas;
  soyPixel32 curColour;
  int32 msi = 4*grid*guiAtlas->width;
  if(guiAtlas->bytes == NULL || guiAtlas->rawSize == 0) {
    return FALSE;
  }
  for(dy = 0; dy < height; dy++) {
    for(dx = 0; dx < width; dx++) {
      curColour = ((soyPixel32*)guiAtlas->bytes)
        [(msi)+(index%8)*seg+(dx%seg)+((dy%seg)+(index/8)*seg)*guiAtlas->width];
      if((curColour.r == 255 && curColour.b == 255 && curColour.g == 0)) {
      } else {
        if(curColour.r < 32) {
          SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
        } else if(curColour.r <= 128) {
          SOY_drawSetPaletteColour(app, SOY_GUI_DARK);
        } else if(curColour.r <= 192) {
          SOY_drawSetPaletteColour(app, SOY_GUI_NEUTRAL);
        } else if(curColour.r <= 228) {
          SOY_drawSetPaletteColour(app, SOY_GUI_LIGHT);
        } else {
          SOY_drawSetPaletteColour(app, SOY_GUI_LIGHTEST);
        }
        SOY_drawPixel(app, x+dx, y+dy);
      }
    }
  }
  return TRUE;
}

/* Draw a beveled rectangle */
sflag SOY_drawGUIPanel (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return soy_draw_rect(&app->draw.drawParams, x1, y1, x1+x2, y1+y2);
  } else if(app->draw.spriteGUI) {
    SOY_drawSpriteGUI(app, x1, y1, x2, y2, 0);
    return TRUE;
  } else {
    return soy_draw_gui_panel(&app->draw.drawParams, x1, y1, x2, y2, app->draw.guiPalette);
  }
}

/* Draw a beveled rectangle in the 'mouse hover' state */
sflag SOY_drawGUIPanelHover (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return soy_draw_rect(&app->draw.drawParams, x1, y1, x1+x2, y1+y2);
  } else if(app->draw.spriteGUI) {
    SOY_drawSpriteGUI(app, x1, y1, x2, y2, 2);
    return TRUE;
  } else {
    return soy_draw_gui_panel_hover(&app->draw.drawParams, x1, y1, x2, y2, app->draw.guiPalette);
  }
}

/* Draw a beveled rectangle in the 'mouse press' state */
sflag SOY_drawGUIPanelPress (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return soy_draw_rect(&app->draw.drawParams, x1, y1, x1+x2, y1+y2);
  } else if(app->draw.spriteGUI) {
    SOY_drawSpriteGUI(app, x1, y1, x2, y2, 1);
    return TRUE;
  } else {
    return soy_draw_gui_panel_press(&app->draw.drawParams, x1, y1, x2, y2, app->draw.guiPalette);
  }
}

/* Draw a beveled border with the light coming from north west */
sflag SOY_drawGUIBorderOuter (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return soy_draw_rect(&app->draw.drawParams, x1, y1, x2, y2);
  } else if(app->draw.spriteGUI) {
    SOY_drawSpriteGUI(app, x1, y1, x2-x1, y2-y1, 8);
    return TRUE;
  } else {
    return soy_draw_gui_border_outer(&app->draw.drawParams, x1, y1, x2, y2, app->draw.guiPalette);
  }
}

/* Draw a beveled border with the light coming from south east */
sflag SOY_drawGUIBorderInner (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return soy_draw_rect(&app->draw.drawParams, x1, y1, x2, y2);
  } else if(app->draw.spriteGUI) {
    SOY_drawSpriteGUI(app, x1, y1, x2-x1, y2-y1, 9);
    return TRUE;
  } else {
    return soy_draw_gui_border_inner(&app->draw.drawParams, x1, y1, x2, y2, app->draw.guiPalette);
  }
}

/* Draw a thin beveled border with the light coming from north west */
sflag SOY_drawGUIRectOuter (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return soy_draw_rect(&app->draw.drawParams, x1, y1, x2, y2);
  } else if(app->draw.spriteGUI) {
    SOY_drawSpriteGUI(app, x1, y1, x2-x1, y2-y1, 12);
    return TRUE;
  } else {
    return soy_draw_gui_rect_outer(&app->draw.drawParams, x1, y1, x2, y2, app->draw.guiPalette);
  }
}

/* Draw a thin beveled border with the light coming from south east */
sflag SOY_drawGUIRectInner (soyAppHandle app, int32 x1, int32 y1, int32 x2, int32 y2) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return soy_draw_rect(&app->draw.drawParams, x1, y1, x2, y2);
  } else if(app->draw.spriteGUI) {
    SOY_drawSpriteGUI(app, x1, y1, x2-x1, y2-y1, 13);
    return TRUE;
  } else {
    return soy_draw_gui_rect_inner(&app->draw.drawParams, x1, y1, x2, y2, app->draw.guiPalette);
  }
}

sflag SOY_drawDebug(soyAppHandle app, sflag enable) {
  app->draw.drawParams.debugDraw = enable;
  return TRUE;
}

/* Draw a text box object */
sflag SOY_drawTextBox (soyAppHandle app, soyTextBoxHandle box) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_GRADSTARTINACTIVE);
    SOY_drawGUIText(app, box->rect.x+2, box->rect.y+1, (int8*)box->mem);
    return soy_draw_rect(&app->draw.drawParams,
        box->rect.x,
        box->rect.y,
        box->rect.x+box->rect.width,
        box->rect.y+box->rect.height);
  } else {
    return soy_draw_textbox(&app->draw.drawParams, box, app->draw.guiPalette);
  }
}

sflag SOY_textBoxMove(soyTextBoxHandle box, int32 x, int32 y) {
  return soy_rect_resize(&box->rect, x, y, box->rect.width, box->rect.height);
}

sflag SOY_textBoxResize(soyTextBoxHandle box, uint32 width, uint32 height) {
  return soy_textbox_resize(box, box->rect.x, box->rect.y, width, height);
}

/* Draw a button object */
sflag SOY_drawButton (soyAppHandle app, soyButtonHandle button, int32 padX, int32 padY) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    if(button->type != SOY_BUTTON_MICRO) {
      SOY_drawGUIText(app, button->rect.x+2, button->rect.y+1, button->rect.label);
    }
    return soy_draw_rect(&app->draw.drawParams,
        button->rect.x,
        button->rect.y,
        button->rect.x+button->rect.width,
        button->rect.y+button->rect.height);
  } else if(app->draw.spriteGUI) {
    if(SOY_rectPressedL(&button->rect)) {
      SOY_drawSpriteGUI(app,
        button->rect.x, button->rect.y,
        button->rect.width, button->rect.height, 5);
      SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
      SOY_drawGUIText(app, button->rect.x+2+1, button->rect.y+2+1, button->rect.label);
    } else if(SOY_rectSelected(&button->rect)) {
      SOY_drawSpriteGUI(app,
        button->rect.x, button->rect.y,
        button->rect.width, button->rect.height, 5);
      SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
      SOY_drawGUIText(app, button->rect.x+2+1, button->rect.y+2+1, button->rect.label);
    } else if(SOY_rectHover(&button->rect)) {
      SOY_drawSpriteGUI(app,
        button->rect.x, button->rect.y,
        button->rect.width, button->rect.height, 6);
      SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
      SOY_drawGUIText(app, button->rect.x+2, button->rect.y+2, button->rect.label);
    } else {
      SOY_drawSpriteGUI(app,
        button->rect.x, button->rect.y,
        button->rect.width, button->rect.height, 4);
      SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
      SOY_drawGUIText(app, button->rect.x+2, button->rect.y+2, button->rect.label);
    }
    return TRUE;
  } else {
    return soy_button_draw(&app->draw.drawParams, app->draw.guiPalette, button, padX, padY);
  }
}

sflag SOY_drawButtonMicro (soyAppHandle app, soyButtonHandle button, uint32 index, cString mask) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return SOY_drawButton(app, button, 0, 0);
  } else if(app->draw.spriteGUI) {
    if(SOY_rectPressedL(&button->rect)) {
      SOY_drawSpriteGUIMicro(app,
        button->rect.x+1, button->rect.y+1,
        button->rect.width+1, button->rect.height+1, index);
    } else if(SOY_rectSelected(&button->rect)) {
      SOY_drawSpriteGUIMicro(app,
        button->rect.x+1, button->rect.y+1,
        button->rect.width+1, button->rect.height+1, index);
    } else if(SOY_rectHover(&button->rect)) {
      SOY_drawSpriteGUIMicro(app,
        button->rect.x, button->rect.y,
        button->rect.width+1, button->rect.height+1, index);
    } else {
      SOY_drawSpriteGUIMicro(app,
        button->rect.x, button->rect.y,
        button->rect.width+1, button->rect.height+1, index);
    }
    return TRUE;
  } else {
    return soy_button_micro_draw(&app->draw.drawParams, app->draw.guiPalette, mask, button);
  }
}
 
/* Draw a button object */
sflag SOY_drawGUI (soyAppHandle app,
    int32 x, int32 y, int32 width, int32 height, guiColour bgCol, sflag state, uint32 bmpIndex) {
  if(app->draw.drawParams.debugDraw) {
    SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
    return soy_draw_rect(&app->draw.drawParams,
        x,
        y,
        x+width,
        y+height);
  } else if(app->draw.spriteGUI) {
    SOY_drawSpriteGUI(app,
        x, y,
        width, height, bmpIndex);
    return TRUE;
  } else {
    SOY_drawSetPaletteColour(app, bgCol);
    soy_draw_fill(&app->draw.drawParams, x+1, y+1, x+width, y+height);
    if(state) {
      return soy_draw_gui_rect_inner(&app->draw.drawParams,
          x, y, x+width, y+height, app->draw.guiPalette);
    } else {
      return soy_draw_gui_rect_outer(&app->draw.drawParams,
          x, y, x+width, y+height, app->draw.guiPalette);
    }
  }
}

_docend

soyButton exitButton;
soyButton minButton;
soyButton maxButton;
uint8 btnChar1[2] = { 255, 0 };
uint8 btnChar2[2] = { 254, 0 };
uint8 btnChar3[2] = { 253, 0 };

sflag commandQuit (soyCommandHandle command, void* args) {
  soyAppHandle hApp = (soyAppHandle) args;
  if(args == NULL) { return FALSE; }
  SLOG1("Command %s fired\n", command->name);
  SOY_appStop(hApp);
  return TRUE;
}

sflag commandMini (soyCommandHandle command, void* args) {
  soyAppHandle hApp = (soyAppHandle) args;
  if(args == NULL) { return FALSE; }
  SLOG1("Command %s fired\n", command->name);
  SOY_windowMinimize(hApp);
  return TRUE;
}

sflag commandMax (soyCommandHandle command, void* args) {
  soyAppHandle hApp = (soyAppHandle) args;
  if(args == NULL) { return FALSE; }
  SLOG1("Command %s fired\n", command->name);
  if(command->timesPressed%2 == 0) {
    SOY_windowFullscreen(hApp);
  } else {
    SOY_windowRestore(hApp);
  }
  return TRUE;
}

_docstart

_docheader
/* Base GUI */

/* Initialize base GUI - border, titlebar, minimize, fullscreen, quit buttons... */
sflag SOY_initGUI (soyAppHandle app, int32 width, int32 height) {
  uint8 fullscreenKeys[2] = {SOY_ALT, SOY_F};
  soyCommandHandle curC = NULL;
  curC = 
  SOY_commandAddDuo(app, commandQuit, app, SOY_ALT, SOY_F4, TRUE, 0, 0, "quit");
  soy_command_restrict_input(curC, TRUE);
  curC = 
  SOY_commandAdd(app, commandMini, app, 0, TRUE, 0, 0, "mini");
  soy_command_restrict_input(curC, TRUE);
  curC = 
  SOY_commandAddMulti(app, commandMax, app, fullscreenKeys, 2, TRUE, 0, 0, "max");
  soy_command_restrict_input(curC, TRUE);

  exitButton = SOY_button(app, (int8*)btnChar1,
      width-17, 2, SOY_commandFind(app, "quit"));
  minButton = SOY_button(app, (int8*)btnChar2,
      width-17-14-1, 2, SOY_commandFind(app, "mini"));
  maxButton = SOY_button(app, (int8*)btnChar3,
      width-17-14-1-14-1, 2, SOY_commandFind(app, "max"));
  
  soy_button_disable_press_anim(&exitButton);
  soy_button_disable_hover_anim(&exitButton);
  
  soy_button_disable_press_anim(&minButton);
  soy_button_disable_hover_anim(&minButton);

  soy_button_disable_press_anim(&maxButton);
  soy_button_disable_hover_anim(&maxButton);


  SOY_buttonResize(&exitButton,
      exitButton.rect.x,
      exitButton.rect.y,
      exitButton.rect.width,
      exitButton.rect.height-2);
  SOY_buttonResize(&minButton,
      minButton.rect.x,
      minButton.rect.y,
      minButton.rect.width,
      minButton.rect.height-2);
  SOY_buttonResize(&maxButton,
      maxButton.rect.x,
      maxButton.rect.y,
      maxButton.rect.width,
      maxButton.rect.height-2);
  return height;
}

/* Draw base GUI */
sflag SOY_drawBaseGUI (soyAppHandle app, int32 width, int32 height) {
  int8 msStr[32];
  int8 usrStr[32];
  uint32 nameLen = 0;
  sprintf(msStr, "ms: %.3f", app->timing.msPerFrame);
  sprintf(usrStr, "usr: %.3f", app->userTiming.msPerFrame);
  SOY_drawSetPaletteColour(app, SOY_GUI_BACKGROUND); 
  if(SOY_windowIsFocused(app)) {
    SOY_drawGradient(app, 1, 2, width-1, 18,
      app->draw.guiPalette.colours[SOY_GUI_GRADSTARTACTIVE],
      app->draw.guiPalette.colours[SOY_GUI_GRADENDACTIVE],
      'h'
      );
  } else {
    SOY_drawGradient(app, 1, 2, width-1, 18,
      app->draw.guiPalette.colours[SOY_GUI_GRADSTARTINACTIVE],
      app->draw.guiPalette.colours[SOY_GUI_GRADENDINACTIVE],
      'h'
      );
  }

  SOY_drawGUIBorderOuter(app, 0, 0, width-1, 18);
  SOY_drawGUIBorderOuter(app, 0, 19, width-1, height-1);
  SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
  SOY_buttonMove(&exitButton, width-16, 2);
  SOY_buttonUpdate(app, &exitButton);
  SOY_buttonMove(&maxButton, width-16-14-1, 2);
  SOY_buttonUpdate(app, &maxButton);
  SOY_buttonMove(&minButton, width-16-14-1-14-1, 2);
  SOY_buttonUpdate(app, &minButton);

  SOY_drawSetPaletteColour(app, SOY_GUI_DARKEST);
  
  nameLen = SOY_drawTextLen(app->name, &app->draw.font);
  
  if(SOY_windowIsFocused(app)) {
    SOY_drawSetPaletteColour(app, SOY_GUI_TEXTINACTIVE);
  } else {
    SOY_drawSetPaletteColour(app, SOY_GUI_TEXTACTIVE);
  }

  SOY_drawTextB(app, 6, 3, app->name, &app->draw.font);
  if(SOY_width(app) > 6+nameLen+32+100) {
    SOY_drawTextI(app, 6+nameLen+32, 3, msStr, &app->draw.font, 0.14f);
  }
  if(SOY_width(app) > 6+nameLen+96+32+100) {
    SOY_drawTextI(app, 6+nameLen+96+32, 3, usrStr, &app->draw.font, 0.14f);
  }

  SOY_drawButton(app, &exitButton, 2, 2);
  SOY_drawButton(app, &minButton, 2, 2);
  SOY_drawButton(app, &maxButton, 2, 2);

  return TRUE;
}

_docheader
/* Fonts */

/* Parse a bitmap image containing the font */
sflag SOY_fontFromBitmap (soyFontHandle font, soyBitmapHandle bitmap, uint32 gridSize) {
  return soy_font_set_atlas(font, bitmap, gridSize);
}

/* Set a global kerning modifier to the font */
sflag SOY_fontSetKerning (soyFontHandle font, uint32 kerning) {
  return soy_font_set_kerning(font, kerning);
}

/* Set the font's scale */
sflag SOY_fontSetScale (soyFontHandle font, real32 size) {
  return soy_font_set_scale(font, size);
}

/* Set the font's vertical kerning modifier */
sflag SOY_fontSetVertical (soyFontHandle font, real32 vertical) {
  return soy_font_set_vertical(font, vertical);
}

typedef enum enum_soyAppDefaultFontType {
  SOY_FONT_DEFAULT,
  SOY_FONT_SMALL,
  SOY_FONT_BIG
} soyAppDefaultFontType;

sflag SOY_fontSelect(soyAppHandle app, soyAppDefaultFontType type) {
  switch(type) {
    case(SOY_FONT_DEFAULT): {
      app->draw.font = app->draw.normalFont;
      break;  
    }
    case(SOY_FONT_SMALL): {
      app->draw.font = app->draw.smallFont;
      break;
    }
    case(SOY_FONT_BIG): {
      app->draw.font = app->draw.bigFont;
      break;
    }
  default: { break; }
  }
  return TRUE;
}

_docheader
/* Sound */

/* Get the pitch in hz from a note */
real32 SOY_soundGetPitch (soyNote note) {
  return soy_sound_pitch(note);
}

/* Send sound sample to specified channel */
sflag SOY_soundToChannel (soyAppHandle app, soySound sample, uint32 channel) {
  return soy_audio_add_sound(&app->audio, sample, channel);
}

/* Set the pitch of specified sample */
sflag SOY_soundSetPitch (soySoundHandle sample, soyNote note) {
  soy_sound_set_pitch(sample, note);
  return TRUE;
}

/* Set the volume of specified sample */
sflag SOY_soundSetVolume (soySoundHandle sample, real32 level) {
  soy_sound_set_volume(sample, level);
  return TRUE;
}

/* Start fading the sample */
sflag SOY_soundFade (soySoundHandle sample, real64 amount) {
  return soy_sound_fade(sample, amount);
}

/* Play sound sample */
sflag SOY_soundPlay (soySoundHandle sample) {
  soy_sound_play(sample);
  return TRUE;
}

/* Pause sound sample */
sflag SOY_soundPause (soySoundHandle sample) {
  return soy_sound_pause(sample);
}

/* Stop sound sample */
sflag SOY_soundStop (soySoundHandle sample) {
  return soy_sound_stop(sample);
}

/* Initialize sound sample */
sflag SOY_soundInit (soySoundHandle sample) {
  return soy_sound_init(sample);
}

/* Get a handle to the sound in mixer channel #id */
soySoundHandle SOY_soundGet (soyAppHandle app, uint32 id) {
  return &app->audio.mixer.channels[id];
}

_docheader
/* Loaders */

/* Load an .obj mesh */
sflag SOY_loadMeshObj (soyAppHandle app, soyModelHandle model, cString file, cString name) {
  soy_3d_create_model(model, &app->memory.meshLoadMemory,
      &app->memory.meshMemory, file, name);
  return TRUE;
}

/* Load a 24bit bitmap image */
sflag SOY_loadImageBmp (soyAppHandle app, soyBitmapHandle bitmap,
    cString path, sflag flipV, sflag flipH) {
  soy_bitmap_create(
      &app->memory.imageLoadMemory,
      &app->memory.imageMemory,
      bitmap, path, "brga", flipV, flipH);
  return TRUE;
}

/* Load a 24bit bitmap image with custom rgba format */
sflag SOY_loadImageBmpF (soyAppHandle app, soyBitmapHandle bitmap,
    cString path, cString format, sflag flipV, sflag flipH) {
  soy_bitmap_create(
      &app->memory.imageLoadMemory,
      &app->memory.imageMemory,
      bitmap, path, format, flipV, flipH);
  return TRUE;
}

sflag SOY_freeImageBmp(soyBitmapHandle bitmap) {
  return soy_bitmap_free(bitmap);
}

/* Load a .raw sound file. Currently only 16bit mono at 48000hz is supported */
sflag SOY_loadSoundRaw (soyAppHandle app, cString name,
    soySoundHandle sample, cString path, uint32 channels, uint32 bitrate, uint32 sampFreq) {
  sflag ret = FALSE;
  if(sample->name == NULL) {
    ret = soy_audio_load_raw(&app->audio, name, sample, path, channels, bitrate, sampFreq);
    sample->name = name;
  } else {
    SLOG1("Sample already exists: %s\n", name);
  }
  return ret;
}

_docheader
/* Multi threading */

/* Create thread */
sflag SOY_threadCreate(soyThread* thr, pfSoyFunc func, soyHandle args) {
  return soy_thread_create(thr, func, args);
}

/* Free thread */
sflag SOY_threadFree(soyThread* thr) {
  return soy_thread_free(thr);
}

_docheader
/* Dynamic Libraries */

/* Load a dll */
sflag SOY_libraryLoad (soyLibrary* lib, cString file) {
  soy_library_load(lib, file);
  return TRUE;
}

/* Free a dll */
sflag SOY_libraryFree (soyLibrary* lib) {
  soy_library_free(lib);
  return TRUE;
}

/* Load a function in the dll */
soyProcess SOY_processLoad (soyLibrary* lib, cString label) {
  return soy_library_load_proc(lib, label);
}

/* Load a module */
sflag SOY_moduleLoad (soyModuleHandle mod, cString lib, cString name, soyHandle args) {
  return soy_module_create(mod, lib, name, args);
}

/* Unload a module */
sflag SOY_moduleUnload (soyModuleHandle mod, void* destructArgs) {
  return soy_module_free(mod, destructArgs);
}

sflag SOY_moduleFree (soyAppHandle app, soyModuleHandle mod) {
  return soy_module_free(mod, app);
}

/* Update a soy module */
sflag SOY_moduleUpdate (soyAppHandle app, soyModuleHandle mod) {
  return soy_module_update_func(app, mod);
}

/* Render a soy module */
sflag SOY_moduleRender (soyAppHandle app, soyModuleHandle mod) {
  return soy_module_render_func(app, mod);
}

/* Resize a soy module */
sflag SOY_moduleResize (soyAppHandle app, soyModuleHandle mod) {
  return soy_module_resize_func(app, mod);
}

/* Reload a soy module if it was changed */
sflag SOY_moduleReload (soyAppHandle app, soyModuleHandle mod) {
  return soy_module_reload_if_changed(mod, app);
}

_docheader
/* Math */

real64 SOY_perlin2D(real64 x, real64 y, real64 freq, int32 depth) {
  return soy_perlin_noise_2d(x, y, freq, depth);
}

soyPixel32 SOY_lerpPixel32(soyPixel32 s, soyPixel32 e, real32 t) {
  return soy_pixel32_lerp(s, e, t);
}

real32 SOY_bellCurve(real32 x, real32 shift, real32 falloff, uint32 power) {
  return soy_normal_distribution(x, shift, falloff, power);
}

real32 SOY_invExp(real32 x, real32 grad, real32 zero) {
  return 1.0f - (real32)pow((real64)x/zero, (real64)grad);
}

real32 SOY_smoothExp(real32 x, real32 c) {
  real32 xA = fabs(x*c);
  xA*=xA;
  return 1 - (xA/(1+xA));
}

/* Vector4 modulo */
vec4f SOY_vec4fMod (vec4f p, vec4f m) {
  return soy_vec4f(p.x-m.x*floor(p.x/m.x),
      p.y-m.y*floor(p.y/m.y), p.z-m.z*floor(p.z/m.z), 1.0f);
}

/* Vector4 absolute */
vec4f SOY_vec4fAbs (vec4f p) {
  vec4f ret = p;
  if(p.x < 0) { ret.x*=-1.0f; }
  if(p.y < 0) { ret.y*=-1.0f; }
  if(p.z < 0) { ret.z*=-1.0f; }
  if(p.w < 0) { ret.w*=-1.0f; }
  return ret;
}

/* Vector4 maximum */
vec4f SOY_vec4fMax (vec4f p, vec4f max) {
  return soy_vec4f(
      (p.x > max.x) ? p.x : max.x,
      (p.y > max.y) ? p.y : max.y,
      (p.z > max.z) ? p.z : max.z,
      (p.w > max.w) ? p.w : max.w
      );
}

/* Vector4 minimum */
vec4f SOY_vec4fMin (vec4f p, vec4f min) {
  return soy_vec4f(
      (p.x < min.x) ? p.x : min.x,
      (p.y < min.y) ? p.y : min.y,
      (p.z < min.z) ? p.z : min.z,
      (p.w < min.w) ? p.w : min.w
      );
}

/* Easing function */
real32 SOY_easeInOutSmooth (real32 x, real32 fallof) {
  return x < 0.5 ? 8 * x * x * x * x : 1 - pow(-2 * x + 2, fallof) / 2;
}
real32 SOY_easeInSine (real32 x) {
  return 1 - cos((x*SOY_PI)/2.0f);
}
real32 SOY_easeOutSine (real32 x) {
  return sin((x*SOY_PI)/2.0f);
}
real32 SOY_easeInOutSine (real32 x) {
  return -(cos(SOY_PI*x)-1.0f)/2.0f;
}
real32 SOY_easeInQuad (real32 x) {
  return x*x;
}
real32 SOY_easeOutQuad (real32 x) {
  return 1-(1-x)*(1-x);
}
real32 SOY_easeInOutQuad (real32 x) {
  return ((x < 0.5f) ? 2*x*x : 1 - soy_pow(-2*x+2, 2)/2);
}
real32 SOY_easeInCubic (real32 x) {
  return x*x*x;
}
real32 SOY_easeOutCubic (real32 x) {
  return 1-soy_pow(1-x, 3);
}
real32 SOY_easeInOutCubic (real32 x) {
  return (x < 0.5f) ? 4*x*x*x : 1-soy_pow(-2*x+2, 3)/2;
}
real32 SOY_easeInExp (real32 x) {
  return (x == 0) ? 0 : pow(2, 10*x-10);
}
real32 SOY_easeOutExp (real32 x) {
  return (x == 1) ? 1 : 1 - pow(2, -10*x);
}
real32 SOY_easeInCirc (real32 x) {
  return sqrt(1-soy_pow(x, 2));
}
real32 SOY_easeOutCirc (real32 x) {
  return sqrt(1-soy_pow(x-1, 2));
}
real32 SOY_easeInElastic (real32 x) {
  real32 c4 = (2*SOY_PI)/3;
  return (x == 0) ? 0 : (x == 1) ? 1 : -pow(2, 10*x-10)*sin((x*10-10.75)*c4);
}
real32 SOY_easeOutElastic (real32 x) {
  real32 c4 = (2*SOY_PI)/3;
  return (x == 0) ? 0 : (x == 1) ? 1 : pow(2, -10*x)*sin((x*10-0.75)*c4)+1;
}

/* Signed distance fields */
real32 SOY_sdfSphere (vec4f p, real32 s) {
  return soy_vec4f_length(p)-s;
}
real32 SOY_sdfTorus (vec4f p, vec4f t) {
  vec4f c;
  vec4f q;
  p.y = 0;
  c = soy_vec4f(0, p.y, 0, 0);
  q = soy_vec4f(soy_vec4f_length(p)-t.x, c.y, 0, 0);
  return soy_vec4f_length(q)-t.y;
}

_docheader
/* Debug Views */

/* Print out the memory state to the console */
sflag SOY_printMemoryState (soyAppHandle app) {
  return soy_memory_print_state(&app->memMan);
}

#endif /* SOY_LIB_IMPLEMENTATION */

#endif
