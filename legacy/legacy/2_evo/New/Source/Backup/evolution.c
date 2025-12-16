#define PAL_LIB_IMPLEMENTATION
#include "pal.h"

#define MAX_CELLS 8000
#define MAX_SENSORS 40
#define MAX_GENOME 64
#define GENE_STEP 4
#define MAX_COLLISION 16
#define MAX_WORLD_SIZE 1024
#define WORLD_TILE 8
#define MAX_NAME 12
#define FIT_SAMPLE_SIZE 16

#define  GENE_SRC     0x80000000
#define  GENE_OUT     0x00800000
#define  GENE_SRC_ID  0x7F000000
#define  GENE_OUT_ID  0x007F0000
#define  GENE_WEIGHT  0x0000FF00
#define  GENE_ANGLE   0x000000FF

#define NUM_VOWELS 10
#define NUM_CONSONANTS 32

cString str_nameVowels = "aeioueiouaie";

cString str_nameConsonants = "bcdfghjklmnprstvzxblskjghfrqmnbvpqwe";

cString str_seasonNames[] = {
  "Winter",
  "Spring",
  "Summer",
  "Autumn"
};

cString str_sensorNames[MAX_SENSORS] = {
  "D_NRG",
  "D_COLR",
  "D_MOVE",
  "D_PHER",
  "D_AGE",
  "D_FERT",
  "D_PROX",
  "I_CLK",
  "I_HUM",
  "I_TEMP",
  "I_FERT",
  "I_NRG",
  "I_AGE",
  "I_W_X",
  "I_W_Y",
  "I_VELO",
  "I_ANGL",
  "I_MEMO",
  "I_OBST",
  "O_CLK",
  "O_SENS",
  "O_PHER",
  "O_KILL",
  "O_ROT",
  "O_FRC1",
  "O_FRC2",
  "O_FRC3",
  "O_FRC4",
  "COM_1",
  "COM_2",
  "COM_3",
  "COM_4",
  "COM_5",
  "NULL",
  "NULL",
  "NULL",
  "NULL",
  "NULL",
  "NULL",
  "NULL"
};

typedef enum enum_sensorType {
  INPUT_DETECT_ENERGY,
  INPUT_DETECT_COLOUR,
  INPUT_DETECT_MOVEMENT,
  INPUT_DETECT_PHEROMONE,
  INPUT_DETECT_AGE,
  INPUT_DETECT_FERTILITY,
  INPUT_DETECT_PROXIMITY,
  INPUT_CLOCK,
  INPUT_HUMIDITY,
  INPUT_TEMPERATURE,
  INPUT_FERTILITY,
  INPUT_ENERGY,
  INPUT_AGE,
  INPUT_WORLDX,
  INPUT_WORLDY,
  INPUT_VELOCITY,
  INPUT_ANGLE,
  INPUT_MEMORY,
  INPUT_OBSTACLE,
  OUTPUT_CLOCK,
  OUTPUT_SENS,
  OUTPUT_PHEROMONE,
  OUTPUT_KILL,
  OUTPUT_ROTATION,
  OUTPUT_FORCE_1,
  OUTPUT_FORCE_2,
  OUTPUT_FORCE_3,
  OUTPUT_FORCE_4,
  COMPUTE_1,
  COMPUTE_2,
  COMPUTE_3,
  COMPUTE_4,
  COMPUTE_5,
  DUMMY_1,
  DUMMY_2,
  DUMMY_3,
  DUMMY_4,
  DUMMY_5,
  DUMMY_6,
  DUMMY_7
} sensorType;

#define ENERGY_SPENT_DETECT 0.0001f
#define ENERGY_SPENT_INPUT 0.0001f
#define ENERGY_SPENT_COMPUTE 0.001f
#define ENERGY_SPENT_OUTPUT 0.001f
#define ENERGY_SPENT_MOVE 0.001f
#define ENERGY_SPENT_KILL 0.5f;

typedef enum enum_linkType {
  LINK_INPUT,
  LINK_INTERNAL,
  LINK_OUTPUT
} linkType;

typedef struct tag_evoVector {
  real32 cx;
  real32 cy;
  real32 ex;
  real32 ey;
} evoVector;

typedef struct tag_evoLink {
  real32 weight;
  linkType inputType;
  linkType outputType;
  real32 internalValue;
  sensorType inputID;
  sensorType outputID;
} evoLink;


typedef struct tag_evoSensor {
  evoVector line;
  real32 angle;
  real32 relAngle;
  real32 inputVal;
  real32 currentVal;
  real32 outputVal;
  cString label;
  sflag active;
  sensorType type;
	real32 weight;
} evoSensor;

typedef struct tag_evoPhysics {
  real32 posX;
  real32 posY;
  real32 lookDir;
  real32 accelaration;
  real32 velocity;
  real32 rotation;
} evoPhysics;

typedef struct tag_evoGene {
  uint32 data;
} evoGene;

typedef struct tag_evoGenome {
  evoGene genes[MAX_GENOME];
  sflag mutant;
} evoGenome;

typedef struct tag_evoParams {
  real32 age;
  real32 size;
  real32 energy;
  real32 fertility;
  real32 clockRate;
  real32 sensitivity;
  evoLink links[MAX_GENOME];
  evoSensor sensors[MAX_SENSORS];
  uint32 activeLinkCount;
  real32 temperature;
  real32 humidity;
  real32 memory;
  real32 score;
  real32 pheromone;
  real32 obstacle;
} evoParams;

typedef struct tag_evoCell {
  evoGenome genome;
  evoParams params;
  evoPhysics physics;
  evoPhysics prevPhysics;
  int32 numGenes;
  int32 internalClock;
  real32 colR;
  real32 colB;
  real32 colG;
  real32 score;
  real32 avgConsumptionAccum;
  real32 avgConsumption;
  uint32 ID;
  uint32 index;
  uint32 collideIndex;
  uint32 children;
  sflag colliding;
  sflag selected;
  int8 name[MAX_NAME];
} evoCell;

typedef struct tag_evoTile {
  real32 foodValue;
  real32 pheromoneValue;
  real32 temperature;
  real32 humidity;
  real32 seasonTemp;
  real32 seasonHumid;
  sflag wall;
} evoTile;

/* temp - higher energy dissipation */

#define EVO_MENU_STATES 4
cString str_evoMenuState[] = {
  "MENU_WORLD",
  "MENU_CELL",
  "MENU_REWARD",
  "MENU_VIEW"
};

typedef struct tag_evoWorld {
  evoCell cells[MAX_CELLS];
  evoCell bestCells[MAX_CELLS];
  uint32 parentIndices[MAX_CELLS*2];
  evoTile worldTiles[MAX_WORLD_SIZE*MAX_WORLD_SIZE];
  evoTile worldTilesCopy[MAX_WORLD_SIZE*MAX_WORLD_SIZE];
  real32 friction;
  real32 timeScale;

  real32 mutation;
	real32 mutationUsr;

  int32 aliveCells;
  int32 maxCells;
  int32 startCells;
  int32 maxGenes;

  uint32 curSeed;

  int32 age;
  int32 maxAge;
  int32 generations;
  int32 simSpeed;

  int32 worldLeft;
  int32 worldRight;
  int32 worldUp;
  int32 worldDown;
  int32 worldWidth;
  int32 worldHeight;
  int32 prevWidth;
  int32 prevHeight;
  sflag simPlaying;

  int32 dayOfYear;
  int32 day;
  int32 month;
  int32 year;

  real32 avgTemp;
  real32 avgHumid;
  real32 ambientTemp;
  real32 ambientHumid;

  real32 rewardSex;
  real32 rewardEat;
  real32 rewardAge;
  real32 rewardProc;
  real32 rewardMove;
  real32 rewardSize;
  real32 rewardKill;

  real32 sizeLimit;
  real32 ageEffect;

  real32 foodNutrition;
  real32 foodGrowth;
  real32 foodDecay;
  real32 foodRange;

  real32 tempMod;
  real32 humidMod;
  real32 seasonMod;
  real32 seasonPhase;
  real32 featureSize;
  real32 worldDetail;

  int32 sliderAmt;
  int32 menuState;

  real32 foodOverlay;
  real32 tempOverlay;
  real32 humidOverlay;
} evoWorld;

typedef struct tag_evoSlider {
  int32 length;
  real32 min;
  real32 max;
  real32 curPos;
  real32 resetPos;
  sflag lockMouse;
  sflag selected;
  real32 pixelOffset;
  cString label;
  real32 outVal;
} evoSlider;


evoWorld global_evoWorld;
sflag global_keyLock = FALSE;
sflag global_randIndex = 0;
int32 global_holdDelay = 64;
sflag global_keyLockArrows = FALSE;

evoSlider evo_slider(cString label, real32 min, real32 max,
  real32 startPos) {
  evoSlider ret;
  ret.curPos = startPos;
  ret.resetPos = startPos;
  ret.outVal = startPos;
  ret.min = min;
  ret.max = max;
  ret.pixelOffset = 0.0f;
  ret.length = max - min;
  ret.lockMouse = FALSE;
  ret.selected = FALSE;
  ret.label = label;
  return ret;
}

sflag evo_slider_update(palAppHandle app, int32 x, int32 y, int32 length, evoSlider* slider) {
  int32 mx = PAL_mouseX(app);
  int32 my = PAL_mouseY(app);
  real32 handlePos = x + slider->curPos*(real32)length;
  real32 mouseDif = ((real32)PAL_mouseX(app) - handlePos) / (real32)length;
  if(PAL_mouseLeft(app) &&
     mx > (int32)handlePos-8 && mx < (int32)handlePos+8 && my > y && my < y+8) {
    if(!slider->lockMouse) {
      slider->selected = TRUE;
    }
  }
  if(slider->selected) {
    slider->curPos += mouseDif;
	}
  if(PAL_mouseLeft(app)) {
    slider->lockMouse = TRUE;
  } else {
    slider->lockMouse = FALSE;
    slider->selected = FALSE;
  }
  slider->curPos = pal_min(pal_max(slider->curPos, 0.0f), 1.0f);
	slider->pixelOffset = slider->curPos*(real32)length;
  slider->outVal = slider->curPos*(slider->max - slider->min);
  return TRUE;
}

real32 evo_sigmoid(real32 x, real32 s) {
  return 0.5f + ((x*s) / (0.5f + fabs(x*s)))*0.5f;
}

real32 evo_tanh(real32 x, real32 s) {
  return ((x*s) / (1.0f + fabs(x*s)))*0.5f;
}

sflag evo_print_byte_as_bits(int8* buf, int32 val) {
  uint32 i;
  for (i = 0; i < 32; i++) {
    sprintf(buf+i, "%c", (val & (1 << (i))) ? '1' : '0');
  }
  *(buf+32) = 0;
  return TRUE;
}

evoVector evo_vector_radial(real32 angle, real32 start, real32 end) {
  evoVector v;
  v.cx = sin(angle)*start;
  v.cy = cos(angle)*start;
  v.ex = sin(angle)*end;
  v.ey = cos(angle)*end;
  return v;
}

evoPhysics evo_physics(real32 x, real32 y, real32 lookDir) {
  evoPhysics p;
  p.accelaration = 0.0f;
  p.lookDir = lookDir;
  p.posX = x;
  p.posY = y;
  p.velocity = 0.0f;
  return p;
}

evoLink evo_link(evoGene gene) {
  evoLink s;
  s.inputType = (gene.data & GENE_SRC) ? LINK_INPUT : LINK_INTERNAL;
  s.inputID = ((gene.data & GENE_SRC_ID) >> 24)%MAX_SENSORS;
  s.outputType = (gene.data & GENE_OUT) ? LINK_OUTPUT : LINK_INTERNAL;
  s.outputID = ((gene.data & GENE_OUT_ID) >> 16)%MAX_SENSORS;
  s.weight = (((real32)(((gene.data & GENE_WEIGHT) >> 8)-127))/255.0f);
  s.internalValue = 0.0f;
  return s;
}

evoSensor evo_sensor(cString label, uint32 gene) {
  evoSensor s;
  real32 angle = ((real32)(gene & GENE_ANGLE) / 255.0f)*PAL_PI*2.0f;
  s.weight = (real32)((gene & GENE_WEIGHT) >> 8) / 255.0f;
	s.inputVal = 0.0f;
  s.outputVal = 0.0f;
  s.currentVal = 0.0f;
  s.label = label;
  s.line = evo_vector_radial(angle, 0.6f*10.0f, 1.6f*10.0f);
  s.angle = angle;
  s.relAngle = 0.0f;
  return s;
}

evoGene evo_gene_rand(uint32 seed) {
  evoGene g;
  uint32 rI = seed;
  rI |= rand()<<17;
  rI |= rand();
  rI ^= rand()<<9;
  rI ^= 0xAAAAAAAA;
  g.data = (uint32)rI;
  return g;
}

evoGenome evo_genome_rand(uint32 seed) {
  uint32 i;
  evoGenome g;
  g.mutant = FALSE;
  for(i = 0; i < MAX_GENOME; i++) {
    g.genes[i] = evo_gene_rand(rand()+seed);
  }
  return g;
}

evoGenome evo_genome_inherit(evoGenome* a, evoGenome* b, int32 numGenes, real32 mutation) {
  int32 i;
  evoGenome g;
  uint32 parentChance = 0;
  g.mutant = FALSE;
  for(i = 0; i < numGenes; i++) {
    parentChance = rand()%100;
    if(parentChance > 50) {
      g.genes[i] = a->genes[i];
    } else {
      g.genes[i] = b->genes[i];
    }
    if(mutation > ((real32)rand()/0xFFFF)) {
      g.mutant = TRUE;
      g.genes[i] = evo_gene_rand(rand());
    }
  }
  return g;
}

evoParams evo_params(evoGenome* genome, int32 maxGenes) {
  evoParams p;
  int32 i;
  p.activeLinkCount = 0;
  p.size = 1.0f;
  p.age = 0.0f;
  p.energy = 1.0f;
  p.fertility = 0.0f;
  p.clockRate = 0.01f;
  p.sensitivity = 0.5f;
  p.memory = 0.0f;
  p.pheromone = 0.0f;
  p.temperature = 0.0f;
  p.humidity = 0.0f;
  p.obstacle = 0.0f;
  for(i = 0; i < pal_min(MAX_GENOME, maxGenes); i++) {
    p.links[i] = evo_link(genome->genes[i]);
  }
  for(i = 0; i < MAX_SENSORS; i++) {
    p.sensors[i] = evo_sensor(str_sensorNames[i], genome->genes[i%maxGenes].data);
    p.sensors[i].type = i;
    p.sensors[i].active = TRUE;
    p.activeLinkCount++;
  }
  return p;
}

sflag evo_cell_gen_name(evoCell* cell) {
  int32 i, j;
  uint32 randIndexA = 0;
  uint32 randIndexB = 0;
  uint32 randIndexC = 0;
  j = cell->numGenes-1;
  for(i = 0; i < cell->numGenes; i++) {
    randIndexA+=cell->genome.genes[i].data;
    randIndexB+=cell->genome.genes[i].data^cell->genome.genes[j].data;
    randIndexC+=cell->genome.genes[i].data&cell->genome.genes[j].data;
    j--;
  }
  i = 0;
  while(i < pal_max((randIndexA%100 < 10) ? ((randIndexB%100 < 10) ? 2 : 3) : 4,
    (uint32)(cell->genome.genes
    [
      (uint32)(randIndexA+randIndexB+randIndexC)%(cell->numGenes)
    ]
    .data)%MAX_NAME)) {
      cell->name[i] =
      (((i+((randIndexA+i)%0xAA == 0))%2) == (randIndexB%2 == 0)) ?
       str_nameVowels[cell->genome.genes[(randIndexB+i)%cell->numGenes].data%NUM_VOWELS] :
       str_nameConsonants[cell->genome.genes[(randIndexC+i)%cell->numGenes].data%NUM_CONSONANTS];
      i++;
  }
  cell->name[i] = 0;
  return TRUE;
}

evoCell evo_cell(real32 x, real32 y,
    real32 dir,
    evoCell* parentA,
    evoCell* parentB,
    uint32 seed,
    uint32 maxGenes,
    real32 mutation,
    uint32 index,
    uint32 arrayIndex) {
  evoCell c;
  c.internalClock = 0;
  c.numGenes = maxGenes;
  c.ID = index;
  c.colliding = FALSE;
  c.index = arrayIndex;
  c.score = 0.0f;
  c.avgConsumption = 0.0f;
  c.avgConsumptionAccum = 0.0f;
  c.children = 0;
  if(parentA == NULL || parentB == NULL) {
    c.genome = evo_genome_rand(seed);
    c.colR = (rand()%255)/255.0f;
    c.colG = (rand()%255)/255.0f;
    c.colB = (rand()%255)/255.0f;
    evo_cell_gen_name(&c);
  } else {
    c.genome = evo_genome_inherit(&parentA->genome, &parentB->genome, maxGenes, mutation);
    c.colR = (parentA->colR+parentB->colR)*0.5f;
    c.colG = (parentA->colG+parentB->colG)*0.5f;
    c.colB = (parentA->colB+parentB->colB)*0.5f;
    evo_cell_gen_name(&c);
  }
	c.selected = FALSE;
  c.params = evo_params(&c.genome, maxGenes);
  c.physics = evo_physics(x, y, dir);
  return c;
}

sflag evo_world_resize(palAppHandle app, evoWorld* w) {
  w->prevWidth = w->worldWidth;
  w->prevHeight = w->worldHeight;
  w->worldLeft = 32;
  w->worldRight = PAL_width(app)-480;
  w->worldUp = 64;
  w->worldDown = PAL_height(app)-32;
  w->worldWidth = pal_min((w->worldRight - w->worldLeft), MAX_WORLD_SIZE*WORLD_TILE);
  w->worldHeight = pal_min((w->worldDown - w->worldUp), MAX_WORLD_SIZE*WORLD_TILE);
  return TRUE;
}

static uint32 global_idCounter = 0;

sflag evo_world_tiles_init(evoWorld* w, sflag reset, real32 seed) {
  uint32 x, y;
  uint32 worldWidth = w->worldWidth/WORLD_TILE;
  uint32 worldHeight = w->worldHeight/WORLD_TILE;
  real64 randXH = (real64)(seed*0.01f);
  real64 randYH = (real64)(seed*0.001f);
  real64 randXT = (real64)(seed*0.02f);
  real64 randYT = (real64)(seed*0.002f);
  for(y = 0; y < worldHeight; y++) {
    for(x = 0; x < worldWidth; x++) {
      if(reset) {
        w->worldTiles[x+y*worldWidth].wall = FALSE;
        w->worldTiles[x+y*worldWidth].foodValue = 0.0f;
        w->worldTiles[x+y*worldWidth].pheromoneValue = 0.0f;
      }
      w->worldTiles[x+y*worldWidth].humidity =
        ((real32)PAL_perlin2D(randXH+(real64)x,
                      randYH+(real64)y,
                      (real64)w->featureSize,
                      (uint32)w->worldDetail))*w->ambientHumid*
        PAL_bellCurve(((real32)(y*10)/worldHeight), -5.0f, 0.02f, 2);
      w->worldTiles[x+y*worldWidth].temperature =
        ((real32)PAL_perlin2D((real64)randXT+(real64)x,
                      randYT+(real64)y,
                      (real64)(w->featureSize*1.4f),
                      (uint32)w->worldDetail))*w->ambientTemp*
        PAL_bellCurve(((real32)(y*10)/worldHeight), -5.0f, 0.02f, 2);
      w->worldTiles[x+y*worldWidth].seasonTemp = w->worldTiles[x+y*worldWidth].temperature;
      w->worldTiles[x+y*worldWidth].seasonHumid = w->worldTiles[x+y*worldWidth].humidity;
    }
  }
  return TRUE;
}

sflag evo_world_populate(evoWorld* w, sflag reset) {
  real32 randX;
  real32 randY;
  real32 randDir;
  int32 i;
  int32 worldWidth = (w->worldRight - w->worldLeft);
  int32 worldHeight = (w->worldDown - w->worldUp);
  w->aliveCells = 0;
  if(!reset) {
    w->curSeed = w->curSeed;
  } else {
    w->curSeed = rand();
  }
  evo_world_tiles_init(w, reset, w->curSeed);
  if(reset) {
    for(i = 0; i < pal_min(w->maxCells, w->startCells); i++) {
      randX = w->worldLeft+(real32)(rand()%worldWidth);
      randY = w->worldUp+(real32)(rand()%worldHeight);
      randDir = ((real32)(rand()%360)/360.0f)*PAL_PI*2.0f;
      w->cells[i] = evo_cell(randX, randY,
        randDir, NULL, NULL, rand(), w->maxGenes,
				w->mutation*w->mutationUsr, global_idCounter++, i);
      w->bestCells[i%FIT_SAMPLE_SIZE] = w->cells[i];
      w->aliveCells++;
    }
  } else {
    for(i = 0; i < pal_min(w->maxCells, w->startCells); i++) {
      randX = w->worldLeft+(real32)(rand()%worldWidth);
      randY = w->worldUp+(real32)(rand()%worldHeight);
      randDir = ((real32)(rand()%360)/360.0f)*PAL_PI*2.0f;
      w->cells[i] = evo_cell(randX, randY,
        randDir,
        &w->bestCells[rand()%(pal_min(i+1, FIT_SAMPLE_SIZE))],
        &w->bestCells[rand()%(pal_min(i+1, FIT_SAMPLE_SIZE))],
        rand(), w->maxGenes, w->mutation*w->mutationUsr,
				global_idCounter++, i);
      w->aliveCells++;
    }
  }
  w->generations++;
  return TRUE;
}

sflag evo_world_reset(evoWorld* w) {
	w->generations = 0;
  global_idCounter = 0;
  return TRUE;
}

evoSlider slider[32];
palButton buttons[8];
real32 neuronTextCols[MAX_SENSORS];

sflag evo_world_init(palAppHandle app,
    evoWorld* w, real32 mutation, int32 maxGenes, int32 maxCells, int32 startCells, sflag reset) {
  int32 i;
  w->friction = 0.001f;
  w->worldLeft = 32;
  w->worldRight = PAL_width(app)-480;
  w->worldUp = 64;
  w->worldDown = PAL_height(app)-32;
  w->timeScale = 1.0f;
  w->maxGenes = maxGenes;
  w->maxCells = maxCells;
  w->startCells = startCells;
  w->age = 0;
  w->maxAge = 0;
  w->simSpeed = 1;
  w->simPlaying = TRUE;
  w->mutation = mutation;
  w->mutationUsr = 0.5f;
	w->worldWidth = (w->worldRight - w->worldLeft);
  w->worldHeight = (w->worldDown - w->worldUp);
  w->day = 0;
  w->month = 0;
  w->year = 0;
  w->dayOfYear = 0;
  w->avgTemp = 0;
  w->avgHumid = 0;
  w->ambientTemp = 1.5f;
  w->ambientHumid = 0.5f;
  w->foodNutrition = 0.5f;
  w->foodGrowth = 0.5f;
  w->foodDecay = 0.5f;
  w->tempMod = 1.0f;
  w->humidMod = 1.0f;
  w->seasonMod = 1.0f;
  w->rewardEat = 0.5f;
  w->rewardAge = 0.5f;
  w->rewardSex = 0.5f;
  w->rewardKill = 0.5f;
  w->rewardMove = 0.5f;
  w->rewardProc = 0.5f;
  w->sizeLimit = 20.0f;
  w->curSeed = 0;
  w->foodRange = 0.0f;
  w->foodOverlay = 0.0f;
  w->foodOverlay = 0.0f;
  w->foodOverlay = 0.0f;
  w->prevWidth = w->worldWidth;
  w->prevHeight = w->worldHeight;

  if(reset) {
    evo_world_reset(w);
  }
  evo_world_populate(w, reset);
  for(i = 0; i < MAX_SENSORS; i++) {
    neuronTextCols[i] = 0.0f;
  }
  return TRUE;
}

sflag evo_world_repopulate(palAppHandle app,
    evoWorld* w, real32 mutation, int32 maxGenes, int32 maxCells, int32 startCells, sflag reset) {
  int32 i;
  w->friction = 0.001f;
  w->worldLeft = 32;
  w->worldRight = PAL_width(app)-480;
  w->worldUp = 64;
  w->worldDown = PAL_height(app)-32;
  w->timeScale = 1.0f;
  w->maxGenes = maxGenes;
  w->maxCells = maxCells;
  w->startCells = startCells;
  w->simPlaying = TRUE;
	w->worldWidth = (w->worldRight - w->worldLeft);
  w->worldHeight = (w->worldDown - w->worldUp);
  w->prevWidth = w->worldWidth;
  w->prevHeight = w->worldHeight;
  if(reset) {
    evo_world_reset(w);
  }
  evo_world_populate(w, reset);
  for(i = 0; i < MAX_SENSORS; i++) {
    neuronTextCols[i] = 0.0f;
  }
  return TRUE;
}

sflag evo_cell_physics_update(evoWorld* world, evoPhysics* physics) {
  physics->velocity = 0.0f;
  physics->accelaration*=(1.0f-world->friction)*0.8f;
  physics->rotation*=(1.0f-world->friction)*0.8f;
  physics->velocity += physics->accelaration;
  physics->posX += sin(physics->lookDir)*physics->velocity;
  physics->posY += cos(physics->lookDir)*physics->velocity;
  physics->lookDir+=physics->rotation;
  if(physics->posX < (real32)world->worldLeft+4.0f) { physics->posX = (real32)world->worldLeft+4.0f; }
  if(physics->posX > (real32)world->worldRight-4.0f) { physics->posX = (real32)world->worldRight-4.0f; }
  if(physics->posY < (real32)world->worldUp+4.0f) { physics->posY = (real32)world->worldUp+4.0f; }
  if(physics->posY > (real32)world->worldDown-4.0f) { physics->posY = (real32)world->worldDown-4.0f; }
  return TRUE;
}

sflag evo_cell_exert_rotation(evoCell* cell, real32 amount) {
  real32 rA = 0.0f;
  if(amount > -(1.0f-cell->params.energy) && amount < (1.0f-cell->params.energy)) { return TRUE; }
  rA = evo_tanh(amount, 0.1f)*cell->params.energy*
    PAL_smoothExp(cell->params.size, 0.25f);
  cell->physics.rotation+=rA;
  cell->params.energy-=evo_sigmoid(rA, cell->params.energy)*0.0001f;
  return TRUE;
}

sflag evo_cell_exert_force(evoCell* cell, real32 amount, real32 angle) {
  real32 rA = 0.0f;
  if(amount > -(1.0f-cell->params.energy) && amount < (1.0f-cell->params.energy)) { return TRUE; }
  rA = evo_tanh(amount, 0.1f)*cell->params.energy*
    PAL_smoothExp(cell->params.size, 0.25f);
  cell->physics.accelaration+=rA*5.0f;
  cell->physics.rotation+=evo_tanh(cell->physics.lookDir-angle, 1.0f)*rA*0.0001f;
  cell->params.energy-=evo_sigmoid(rA, cell->params.energy)*0.0001f;
  return TRUE;
}

sflag evo_cell_check_collision(evoCell* a, evoCell* b) {
  return (pal_distance_between_points(
        a->physics.posX,
        a->physics.posY,
        b->physics.posX,
        b->physics.posY
        ) < a->params.size*2);
}

#define EVO_COL(a, b) \
	(sqrt( \
				(b.physics.posX-a.physics.posX)*(b.physics.posX-a.physics.posX)+\
				(b.physics.posY-a.physics.posY)*(b.physics.posY-a.physics.posY)))

real32 evo_sensor_update(evoWorld* world, evoCell* cell, evoSensor* sensor, evoTile* curTile) {
  real32 energySpent = 0.0f;
  sensor->inputVal = evo_tanh(sensor->inputVal, 1.0f);
  sensor->currentVal = 0.0f;
  sensor->relAngle = sensor->angle+cell->physics.lookDir;
  sensor->line = evo_vector_radial(sensor->relAngle,
      0.6f*cell->params.size,
      1.6f*cell->params.size);

  switch(sensor->type) {
    case(INPUT_CLOCK): {
      sensor->currentVal = sin((real32)cell->internalClock*0.001f*cell->params.clockRate);
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_FERTILITY): {
      sensor->currentVal = evo_sigmoid(cell->params.fertility, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_ANGLE): {
      sensor->currentVal = sin(cell->physics.lookDir)*cell->params.sensitivity;
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_WORLDX): {
      sensor->currentVal = evo_tanh(cell->physics.posX, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_WORLDY): {
      sensor->currentVal = evo_tanh(cell->physics.posY, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_VELOCITY): {
      sensor->currentVal = evo_sigmoid(cell->physics.velocity, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_HUMIDITY): {
      sensor->currentVal = evo_tanh(cell->params.humidity, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_TEMPERATURE): {
      sensor->currentVal = evo_tanh(cell->params.temperature, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_MEMORY): {
      sensor->currentVal = evo_tanh(cell->params.memory, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_AGE): {
      sensor->currentVal = evo_sigmoid(cell->params.age, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_ENERGY): {
      sensor->currentVal = evo_sigmoid(1.0f-cell->params.energy, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_INPUT;
      break;
    }

    case(INPUT_DETECT_ENERGY): {
      if(!cell->colliding) { break; }
      sensor->currentVal = evo_sigmoid(world->cells[cell->collideIndex].params.energy, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_DETECT;
      break;
    }

    case(INPUT_DETECT_AGE): {
      if(!cell->colliding) { break; }
      sensor->currentVal = evo_sigmoid(world->cells[cell->collideIndex].params.age, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_DETECT;
      break;
    }

    case(INPUT_DETECT_MOVEMENT): {
      if(!cell->colliding) { break; }
      sensor->currentVal = evo_sigmoid(world->cells[cell->collideIndex].physics.velocity, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_DETECT;
      break;
    }

    case(INPUT_DETECT_PROXIMITY): {
      if(cell->colliding) {
        sensor->currentVal = pal_max(0.0f, pal_min(1.0f,
            1.0f-pal_distance_between_points(
              world->cells[cell->collideIndex].physics.posX,
              world->cells[cell->collideIndex].physics.posY,
              cell->physics.posX,
              cell->physics.posY)));

        energySpent += ENERGY_SPENT_DETECT;
      }
      break;
    }

    case(INPUT_DETECT_COLOUR): {
      if(!cell->colliding) { break; }
      sensor->currentVal += fabs(world->cells[cell->collideIndex].colR-cell->colR);
      sensor->currentVal += fabs(world->cells[cell->collideIndex].colG-cell->colG);
      sensor->currentVal += fabs(world->cells[cell->collideIndex].colB-cell->colB);
      sensor->currentVal = evo_sigmoid(sensor->currentVal/3.0f, cell->params.sensitivity);
      energySpent += ENERGY_SPENT_DETECT;
      break;
    }

    case(INPUT_DETECT_PHEROMONE): {
      sensor->currentVal = pal_min(pal_max(0.0f, curTile->pheromoneValue), 1.0f);
      energySpent += ENERGY_SPENT_DETECT;
      break;
    }

    case(INPUT_DETECT_FERTILITY): {
      if(!cell->colliding) { break; }
      break;
    }

    case(OUTPUT_KILL): {
      if(!cell->colliding) { break; }
      world->cells[cell->collideIndex].params.size*=
        1.0f-(pal_max(pal_min(1.0f, sensor->inputVal*0.1f), 0.0f));
      world->cells[cell->collideIndex].params.energy*=
        1.0f-(pal_max(pal_min(1.0f, sensor->inputVal*0.1f), 0.0f));
      energySpent += ENERGY_SPENT_DETECT;
      break;
    }

    case(OUTPUT_PHEROMONE): {
      curTile->pheromoneValue += sensor->inputVal*cell->params.fertility;
      curTile->pheromoneValue = pal_min(pal_max(0.0f, curTile->pheromoneValue), 1.0f);
      energySpent += ENERGY_SPENT_OUTPUT;
      break;
    }

    case(OUTPUT_CLOCK): {
      cell->params.clockRate = evo_sigmoid(sensor->inputVal, 1.0f);
      energySpent += ENERGY_SPENT_OUTPUT;
      break;
    }

    case(OUTPUT_SENS): {
      cell->params.sensitivity = evo_sigmoid(sensor->inputVal, 1.0f);
      energySpent += ENERGY_SPENT_OUTPUT;
      break;
    }

    case(OUTPUT_FORCE_1): {
      evo_cell_exert_force(cell, sensor->inputVal, sensor->angle);
      energySpent += ENERGY_SPENT_MOVE*fabs(sensor->inputVal);
      cell->score += world->rewardMove*(fabs(sensor->inputVal));
      break;
    }

    case(OUTPUT_FORCE_2): {
      evo_cell_exert_force(cell, sensor->inputVal, sensor->angle);
      energySpent += ENERGY_SPENT_MOVE*fabs(sensor->inputVal);
      cell->score += world->rewardMove*(fabs(sensor->inputVal));
      break;
    }

    case(OUTPUT_FORCE_3): {
      evo_cell_exert_force(cell, -sensor->inputVal, sensor->angle);
      energySpent += ENERGY_SPENT_MOVE*fabs(sensor->inputVal);
      cell->score += world->rewardMove*(fabs(sensor->inputVal));
      break;
    }

    case(OUTPUT_FORCE_4): {
      evo_cell_exert_force(cell, -sensor->inputVal, sensor->angle);
      energySpent += ENERGY_SPENT_MOVE*fabs(sensor->inputVal);
      cell->score += world->rewardMove*(fabs(sensor->inputVal));
      break;
    }

    case(OUTPUT_ROTATION): {
      evo_cell_exert_rotation(cell, sensor->inputVal);
      energySpent += ENERGY_SPENT_MOVE*fabs(sensor->inputVal);
      cell->score += world->rewardMove*(fabs(sensor->inputVal));
      break;
    }

    case(COMPUTE_1): {
      sensor->currentVal = (sensor->inputVal);
      energySpent += ENERGY_SPENT_COMPUTE;
      break;
    }

    case(COMPUTE_2): {
      sensor->currentVal = (sensor->inputVal*0.5f);
      energySpent += ENERGY_SPENT_COMPUTE;
      break;
    }

    case(COMPUTE_3): {
      sensor->currentVal = ((sensor->inputVal)*cell->params.sensitivity);
      energySpent += ENERGY_SPENT_COMPUTE;
      break;
    }

    case(COMPUTE_4): {
      sensor->currentVal = (sensor->inputVal);
      cell->params.memory = sensor->currentVal;
      energySpent += ENERGY_SPENT_COMPUTE;
      break;
    }

    case(COMPUTE_5): {
      sensor->currentVal = (sensor->inputVal);
      cell->params.memory = sensor->currentVal;
      energySpent += ENERGY_SPENT_COMPUTE;
      break;
    }
    default: { sensor->currentVal =
			evo_sigmoid(sensor->inputVal, sensor->weight); break; }
  }
  sensor->outputVal = sensor->currentVal;
  return energySpent / (real32)MAX_SENSORS;
}

sflag global_mouseLock = FALSE;

sflag evo_link_update(evoCell* cell, evoLink* link) {
  cell->params.sensors[link->outputID%MAX_SENSORS].inputVal +=
    cell->params.sensors[link->inputID%MAX_SENSORS].outputVal*link->weight;
  link->internalValue = cell->params.sensors[link->inputID%MAX_SENSORS].outputVal*link->weight;
  return TRUE;
}

evoCell* evo_cell_update(palAppHandle app, evoWorld* world, evoCell* cell, uint32 index) {
  int32 i;
	real32 work;
  real32 colX;
  real32 prevSize;
	real32 humidSens;
	real32 engDelta;
	real32 tempSens;
	evoCell cc;
  real32 colY;
  uint32 colIndexX;
  uint32 colIndexY;
	int32 tileIndexX;
	int32 tileIndexY;
	int32 tileIndex;
	evoCell* childParent;
	evoTile* curTile;
	real32 eatAmount;
  real32 energySpent = 0.0f;
	int32 tileWidth = world->worldWidth/WORLD_TILE;
	int32 tileHeight = world->worldHeight/WORLD_TILE;
	if(!cell) { return NULL; }
	eatAmount = (cell->params.size*cell->params.size)*0.1f;
  tileIndexX = (int32)((cell->physics.posX-world->worldLeft)/(real32)WORLD_TILE);
  tileIndexY = (int32)((cell->physics.posY-world->worldUp)/(real32)WORLD_TILE);
  childParent = NULL;
  curTile = NULL;
	cell->colliding = FALSE;
  cell->index = index;
  tileIndexX = pal_min(tileIndexX, world->worldWidth/WORLD_TILE);
  tileIndexY = pal_min(tileIndexY, world->worldHeight/WORLD_TILE);
  tileIndex = tileIndexX + tileIndexY*(world->worldWidth/WORLD_TILE);
	tileIndex = pal_min(pal_max(tileIndex, tileWidth*tileHeight), 0.0f);

	curTile = &world->worldTiles[tileIndex];
  cell->prevPhysics = cell->physics;

  evo_cell_physics_update(world, &cell->physics);
  colX = cell->physics.posX+cell->physics.velocity*sin(cell->physics.lookDir);
  colY = cell->physics.posY+cell->physics.velocity*cos(cell->physics.lookDir);
  colIndexX = (uint32)((colX-world->worldLeft)/WORLD_TILE);
  colIndexY = (uint32)((colY-world->worldUp)/WORLD_TILE);
  if(world->worldTiles[colIndexX+colIndexY*(world->worldWidth/WORLD_TILE)].wall) {
    cell->physics.posX = cell->prevPhysics.posX;
    cell->physics.posY = cell->prevPhysics.posY;
  }
  for(i = 0; i < world->aliveCells; i++) {
    if(world->cells[i].ID != cell->ID) {
      cc = *cell;
			if(EVO_COL(cc, world->cells[i])) {
        cell->collideIndex = i;
        cell->colliding = TRUE;
        if(cell->params.fertility > 0.5f &&
						world->cells[i].params.fertility > 0.5f &&
           cell->params.energy > 0.5f &&
					 world->cells[i].params.energy > 0.5f &&
					 world->aliveCells < MAX_CELLS) {
          childParent = &world->cells[i];
          cell->params.fertility = 0.0f;
          cell->params.size *= 0.5f;
          cell->params.energy *= 0.5f;
          cell->children++;
          cell->score += world->rewardSex*((real32)(cell->children*100.0f));
          world->cells[i].params.fertility = 0.0f;
          world->cells[i].params.size *= 0.5f;
          world->cells[i].params.energy *= 0.5f;
          world->cells[i].children++;
          world->cells[i].score += world->rewardSex*((real32)(cell->children*100.0f));
        }
        break;
      }
    }
  }
  for(i = 0; i < MAX_SENSORS; i++) {
    cell->params.sensors[i].inputVal = 0.0f;
  }

  for(i = 0; i < pal_min(cell->numGenes, MAX_GENOME); i++) {
    evo_link_update(cell, &cell->params.links[i]);
  }

  for(i = 0; i < MAX_SENSORS; i++) {
    work = evo_sensor_update(world, cell, &cell->params.sensors[i], curTile);
  	work *= 0.1f;
	}

	humidSens = cell->params.sensors[i - 2].weight;
	tempSens = cell->params.sensors[i - 1].weight;

	energySpent = ((work + tempSens*0.1f) * cell->params.size) +
		(cell->params.age*cell->params.age)*(world->ageEffect*0.001f);

  cell->score += world->rewardEat*eatAmount*curTile->foodValue;
  cell->score += world->rewardAge*(1.0f/365.0f);
  cell->score += world->rewardProc*work;

	prevSize = cell->params.size;
	engDelta = cell->params.energy;

  cell->params.energy +=
		eatAmount*curTile->foodValue*world->foodNutrition +
		(curTile->humidity*humidSens*0.001f);

	engDelta = cell->params.energy - engDelta;

	cell->params.fertility += eatAmount / (1.0f + cell->params.age*0.001f);
  curTile->foodValue -= eatAmount;
	if(curTile->foodValue < 0.0f) { curTile->foodValue = 0.0f; }

	cell->params.temperature += curTile->seasonTemp - cell->params.temperature;
  cell->params.humidity += curTile->seasonHumid - cell->params.humidity;

	cell->avgConsumptionAccum += energySpent;

	if(cell->internalClock%60 == 0) {
    cell->avgConsumption = 100.0f*(cell->avgConsumptionAccum/60.0f);
    cell->avgConsumptionAccum = 0.0f;
  }

  cell->params.energy -= energySpent*0.001f;
	cell->params.fertility = pal_min(pal_max(0.0f, cell->params.fertility), 1.0f);
  cell->params.age += PAL_appTimestep(app);
	cell->params.size += 0.01f*pal_max(1.0f / (cell->params.age + 1.0f), 1.0f);
	cell->params.size = pal_max(pal_min(cell->params.size, world->sizeLimit), 1.0f);

	if(prevSize < cell->params.size) {
		cell->score += world->rewardSize;
	}
	cell->internalClock++;
  return childParent;
}

sflag evo_world_tiles_update(evoWorld* w) {
  int32 x;
  int32 y;
  evoTile* curTile;
  int32 tileWidth = w->worldWidth/WORLD_TILE;
  int32 tileHeight = w->worldHeight/WORLD_TILE;
  real32 avgTemp = 0.0f;
  real32 avgHumid = 0.0f;
  real32 sinTemp = 1.0f-
    ((sin((((real32)w->age/24)/360.0f-0.23f)*
											 (PAL_PI*2)))*(w->seasonMod*0.5f));
  real32 sinHumid = 1.0f-
    ((sin((real32)(w->age*0.000124))*
           cos((real32)(w->age*0.000001f))*
           sin((((real32)w->age/360)/666.0f)*(PAL_PI*2)))
		 *1.0f)*(w->seasonMod*0.5f);

  for(y = 0; y < tileHeight; y++) {
    for(x = 0; x < tileWidth; x++) {
      curTile = &w->worldTiles[x+y*tileWidth];
      curTile->seasonTemp = curTile->temperature*sinTemp*(w->tempMod);
      curTile->seasonHumid = curTile->humidity*sinHumid*(w->humidMod);
      curTile->pheromoneValue*=0.99f;
      avgTemp += curTile->seasonTemp;
      avgHumid += curTile->seasonHumid;
      curTile->foodValue*=1.0f-(w->foodDecay)*0.01f;
      if(curTile->foodValue < 1.0f) {
        curTile->foodValue +=
        (w->foodGrowth)*(PAL_bellCurve(curTile->seasonTemp, -0.6f, w->foodRange*w->foodRange, 2)*
          pal_max(curTile->seasonHumid*curTile->seasonHumid, 0.0f));
      }
      w->worldTilesCopy[x+y*tileWidth] = *curTile;
    }
  }
  avgTemp /= (real32)tileWidth*tileHeight;
  avgHumid /= (real32)tileWidth*tileHeight;
  w->avgTemp = avgTemp*100.0f-50.0f;
  w->avgHumid = avgHumid*100.0f;
  return TRUE;
}

int32 global_mousePrevX = 0;
int32 global_mousePrevY = 0;


sflag evo_world_update(palAppHandle app, evoWorld* w) {
  int32 i, j;
  evoCell* curCell;
  evoCell* parentCell;
  evoCell tempCell = w->cells[0];
  sflag ranked = FALSE;
  int32 numNew = 0;
  int32 bestIndex = 0;
  real32 randDir = 0.0f;
  int32 deadAmount = 0;
  int32 mTX = (PAL_mouseX(app)-w->worldLeft)/WORLD_TILE;
  int32 mTY = (PAL_mouseY(app)-w->worldUp)/WORLD_TILE;
  int32 aliveCells = w->aliveCells;
  real32 difX = (real32)pal_abs(global_mousePrevX - mTX);
  real32 difY = (real32)pal_abs(global_mousePrevY - mTY);
  real32 dist = fabs(sqrt(difX*difX + difY*difY));
  real32 deltaX = ((real32)global_mousePrevX - (real32)mTX)/dist;
  real32 deltaY = ((real32)global_mousePrevY - (real32)mTY)/dist;
  real32 rX = 0.0f;
  real32 rY = 0.0f;

  if((PAL_mouseLeft(app) || PAL_mouseRight(app)) &&
      mTX < (w->worldWidth/WORLD_TILE) &&
      mTY < (w->worldHeight/WORLD_TILE) &&
      PAL_keyPressed(app, PAL_SHIFT)) {
    if(PAL_mouseLeft(app)) {
      w->worldTiles[mTX+mTY*(w->worldWidth/WORLD_TILE)].wall = TRUE;
    } else{
      w->worldTiles[mTX+mTY*(w->worldWidth/WORLD_TILE)].wall = FALSE;
    }
  }
  if((PAL_mouseLeft(app) || PAL_mouseRight(app)) &&
      mTX+(int32)(dist*deltaX) < (w->worldWidth/WORLD_TILE) &&
      mTY+(int32)(dist*deltaY) < (w->worldHeight/WORLD_TILE) &&
      mTX+(int32)(dist*deltaX) > 0 &&
      mTY+(int32)(dist*deltaY) > 0 &&
      PAL_keyPressed(app, PAL_SHIFT)) {
    while(dist > 0.0f) {
      if(PAL_mouseLeft(app)) {
        w->worldTiles[(mTX+(int32)rX)+(mTY+(int32)rY)*(w->worldWidth/WORLD_TILE)].wall = TRUE;
      } else {
        w->worldTiles[(mTX+(int32)rX)+(mTY+(int32)rY)*(w->worldWidth/WORLD_TILE)].wall = FALSE;
      }
      rX += deltaX;
      rY += deltaY;
      dist-= 1.0f;
    }
  }

  global_mousePrevX = mTX;
  global_mousePrevY = mTY;
  for(i = 0; i <  pal_min(w->maxCells, aliveCells); i++) {
    curCell = &w->cells[i];
		if(!curCell) { break; }
    if(curCell->params.energy < 0.0f || curCell->params.size < 1.0f) {
      deadAmount++;
      tempCell = w->cells[i];
      w->cells[i] = w->cells[w->aliveCells-deadAmount];
      w->cells[w->aliveCells-deadAmount] = tempCell;
    }
  }

  aliveCells -= deadAmount;
  w->aliveCells = pal_max(0, pal_min((int32)aliveCells, (int32)MAX_CELLS - 1));
	
	for(i = 0; i <  pal_min(w->maxCells, w->aliveCells); i++) {
    ranked = FALSE;
    curCell = &w->cells[i];
#if 0

#endif
		if(curCell) {
			parentCell = evo_cell_update(app, w, curCell, i);
		}
#if 0

#endif
#if 1
		if(parentCell) {
			if(numNew*2 < MAX_CELLS-2) {
      	w->parentIndices[numNew*2] = i;
      	w->parentIndices[numNew*2+1] = parentCell->index;
      	numNew++;
    	}
		}
#endif
#if 0

#endif
#if 1
		for(j = 0; j < FIT_SAMPLE_SIZE; j++) {
      if(curCell->ID == w->bestCells[j].ID && curCell->score ) {
        w->bestCells[j] = *curCell;
        ranked = TRUE;
      }
      if(curCell->score < w->bestCells[j].score) {
        bestIndex++;
      }
    }
#endif
#if 0

#endif
#if 1
    if(bestIndex < FIT_SAMPLE_SIZE && !ranked) {
			if(!curCell) { printf("error: NULL cell: %d\n", i); }
      w->bestCells[bestIndex] = *curCell;
    }
#endif
  }

  /* sort best genes */
  if(w->aliveCells) {
    for(j = 0; j < FIT_SAMPLE_SIZE; j++) {
      for(i = 0; i < FIT_SAMPLE_SIZE-j; i++) {
        if(w->bestCells[i].score < w->bestCells[i+1].score) {
          tempCell = w->bestCells[i];
          w->bestCells[i] = w->bestCells[i+1];
          w->bestCells[i+1] = tempCell;
        }
      }
			if(w->bestCells[j].score > 0.0f) {
					w->bestCells[j].score -= 0.01f;
			} else {
				w->bestCells[j].score = 0.0f;
			}
    }
  }


  for(i = 0; i < numNew; i++) {
    if(w->aliveCells < w->maxCells) {
      randDir = ((rand()%360)/360.0f)*PAL_PI*2.0f;
      curCell = &w->cells[w->parentIndices[i*2]];
      parentCell = &w->cells[w->parentIndices[i*2+1]];
      if(!curCell || !parentCell) { break; }
      w->cells[w->aliveCells] =
      evo_cell(
        curCell->physics.posX+sin(randDir)*curCell->params.size*4.0f,
        curCell->physics.posY+cos(randDir)*curCell->params.size*4.0f,
        randDir,
        curCell,
        parentCell,
        rand(),
        w->maxGenes, w->mutation, global_idCounter++, w->aliveCells);
      w->aliveCells++;
    }
  }


  if(w->age > w->maxAge) {
    w->maxAge = w->age;

  }

  if(w->age%24 == 0) {
    evo_world_tiles_update(w);
    w->dayOfYear++;
    w->day++;

    if(w->dayOfYear%30 == 0) {
      w->day = 0;
      w->month++;
    }

    if(w->month > 11) {
      w->month = 0;
      w->dayOfYear = 0;
      w->day = 0;
      w->year++;
    }
  }

  w->age++;
  return TRUE;
}

sflag toggleHelp = TRUE;
sflag toggleNames = TRUE;
sflag toggleKeyLock = FALSE;
sflag global_sliderWidth = 256;

sflag evo_slider_draw(palAppHandle app, int32 x, int32 y, int32 length, evoSlider* slider) {
  int8 strBufA[32];
  int8 strBufB[32];
  PAL_drawSetColour(app, 255, 255, 255, 255);
  sprintf(strBufA, " %s:", slider->label);
  PAL_drawText(app, x+length+4, y, strBufA, &app->draw.font);
  sprintf(strBufB, "%.2f", slider->outVal);

  PAL_drawTextB(app, x+length+4+PAL_drawTextLen(strBufA, &app->draw.font), y, strBufB, &app->draw.font);
  PAL_drawLine(app, x, y+3, x+length, y+3);
  PAL_drawSetColour(app, 0, 0, 0, 255);
  PAL_drawLine(app, x, y+4, x+length, y+4);
  if(slider->selected) {
    PAL_drawSetColour(app, 255, 255, 255, 255);
  }
  PAL_drawRectF(app, x+(int32)slider->pixelOffset, y, x+(int32)slider->pixelOffset+8, y+8);
  return TRUE;
}

sflag evo_world_draw(palAppHandle app, evoWorld* w) {
  int32 i, j, x, y;
  int32 inputSensorID;
  int32 outputSensorID;
  uint8 foodCol;
  uint8 tempCol;
  uint8 rainCol;
  uint8 pherCol;
  evoCell* curCell;
  evoTile* curTile;
  evoCell* selectCell = NULL;
  evoSensor* curSen = NULL;
  evoLink* curLink = NULL;
  int8 strBuf[512];
  palPixel32 col;
  real32 neuronTextCol;
  real32 linkCol;
  real32 sensCol;
  int32 tileWidth = (w->worldWidth)/WORLD_TILE;
  int32 tileHeight = (w->worldHeight)/WORLD_TILE;
  int32 circleWidth = 128;
  int32 circlePosX = w->worldRight+circleWidth+24;
  int32 circlePosY = w->worldDown-circleWidth-24;
  int32 inputPosX;
  int32 inputPosY;
  int32 outputPosX;
  int32 outputPosY;

  if(PAL_keyPressed(app, PAL_F1)) {
    toggleHelp = TRUE;
  } else {
    toggleHelp = FALSE;
	}

  if(PAL_keyPressed(app, PAL_F2)) {
    if(!toggleKeyLock) {
      toggleNames = !toggleNames;
    }
  }

  if(PAL_keyPressedAny(app)) {
    toggleKeyLock = TRUE;
  } else {
    toggleKeyLock = FALSE;
  }

  PAL_drawSetColour(app, 255, 255, 255, 255);
  PAL_drawRect(app, w->worldLeft-8, w->worldUp-8, w->worldRight+8, w->worldDown+8);
  PAL_drawRect(app, w->worldRight+8+2, w->worldUp-8, PAL_width(app)-24, w->worldDown+8);
  sprintf(strBuf, "%02d/%02d/%04d Season: %6s Avg. Temp.: %.01f Avg. Humidity: %.01f",
      w->day, w->month, w->year, str_seasonNames[(w->month)/3], w->avgTemp, w->avgHumid);
  PAL_drawText(app, w->worldLeft, w->worldUp-24, strBuf, &app->draw.font);
  sprintf(strBuf, "Alive: %d Speed: %d Generation: %d Seed: %d",
      w->aliveCells, w->simSpeed, w->generations, w->curSeed
      );
  PAL_drawText(app, w->worldLeft, w->worldUp-24-16, strBuf, &app->draw.font);

  for(y = 0; y < tileHeight; y++) {
    for(x = 0; x < tileWidth; x++) {
      curTile = &w->worldTiles[x+y*tileWidth];
      foodCol = (uint8)(pal_min(pal_max(curTile->foodValue*w->foodOverlay, 0.0f), 1.0f)*64.0f);
      tempCol = (uint8)(pal_min(pal_max(curTile->seasonTemp*w->tempOverlay, 0.0f), 1.0f)*64.0f);
      rainCol = (uint8)(pal_min(pal_max(curTile->seasonHumid*w->humidOverlay, 0.0f), 1.0f)*64.0f);
      pherCol = (uint8)(pal_min(pal_max(curTile->pheromoneValue, 0.0f), 1.0f)*64.0f);
      if(!curTile->wall) {
        PAL_drawSetColour(app,
            tempCol+pherCol,
            foodCol,
            rainCol+pherCol, 255);
      } else {
        PAL_drawSetColour(app, 192, 192, 192, 255);
      }
      PAL_drawRectF(app,
          w->worldLeft+x*WORLD_TILE, w->worldUp+y*WORLD_TILE,
          w->worldLeft+x*WORLD_TILE+WORLD_TILE, w->worldUp+y*WORLD_TILE+WORLD_TILE);
    }
  }
  PAL_drawSetExtents(app,
      w->worldLeft,
      w->worldUp,
      w->worldLeft+w->worldWidth,
      w->worldUp+w->worldHeight);
  for(i = 0; i < pal_min(w->maxCells, w->aliveCells); i++) {
    curCell = &w->cells[i];
		if(!curCell) { return FALSE; }
    if(curCell->physics.posX - curCell->params.size*0.5f < (real32)w->worldLeft ||
				curCell->physics.posY - curCell->params.size*0.5f < (real32)w->worldUp ||
				curCell->physics.posX - curCell->params.size*0.5f > (real32)w->worldRight ||
				curCell->physics.posY - curCell->params.size*0.5f > (real32)w->worldDown) {
			break;
		}
		if(PAL_mouseLeft(app)) {
      if(pal_distance_between_points(
          (real32)PAL_mouseX(app),
          (real32)PAL_mouseY(app),
          curCell->physics.posX,
          curCell->physics.posY) < pal_max(2.0f, curCell->params.size)) {
        if(!global_mouseLock) {
          curCell->selected = TRUE;
          global_mouseLock = TRUE;
        }
      } else {
        if(!global_mouseLock) {
          curCell->selected = FALSE;
        }
      }
    } else {
      global_mouseLock = FALSE;
    }
    col = pal_pixel32(
      (uint8)(curCell->colR*255.0f*curCell->params.energy),
      (uint8)(curCell->colG*255.0f*curCell->params.energy),
      (uint8)(curCell->colB*255.0f*curCell->params.energy),
      255);
    if(curCell->selected) { selectCell = curCell; }
    for(j = 0; j < MAX_SENSORS; j++) {
      curSen = &curCell->params.sensors[j];
      sensCol = evo_sigmoid(curSen->outputVal, 1.0f);
      PAL_drawSetColour(app,
        (uint8)((real32)col.r*sensCol),
        (uint8)((real32)col.g*sensCol),
        (uint8)((real32)col.b*sensCol),
        255);
      PAL_drawLine(app,
        curCell->physics.posX+curSen->line.cx,
        curCell->physics.posY+curSen->line.cy,
        curCell->physics.posX+curSen->line.ex,
        curCell->physics.posY+curSen->line.ey
        );
    }
    PAL_drawSetPixel(app, col);
    PAL_drawCircleF(app, (int32)(curCell->physics.posX), (int32)(curCell->physics.posY),
                          (int32)(curCell->params.size));
    PAL_drawSetPixel(app, col);
    if(toggleNames) {
      sprintf(strBuf, "%s", curCell->name);
      PAL_drawSetColour(app, 128, 128, 128, 255);
      if(curCell->genome.mutant) {
        PAL_drawSetColour(app, 128, 192, 128, 255);
      }
      PAL_drawText(app,
        (int32)curCell->physics.posX-(PAL_drawTextLen(strBuf, &app->draw.font)/2),
        (int32)curCell->physics.posY,
        strBuf, &app->draw.font);
    }
    PAL_drawSetColour(app, 255, 255, 255, 255);
    PAL_drawLine(app,
        curCell->physics.posX+sin(curCell->physics.lookDir)*0.5f*curCell->params.size,
        curCell->physics.posY+cos(curCell->physics.lookDir)*0.5f*curCell->params.size,
        curCell->physics.posX+sin(curCell->physics.lookDir)*2.0f*curCell->params.size,
        curCell->physics.posY+cos(curCell->physics.lookDir)*2.0f*curCell->params.size
      );
    PAL_drawSetColour(app, 255, 0, 0, 255);
    if(curCell->colliding) {
      PAL_drawEllipse(app,
        (int32)curCell->physics.posX,
        (int32)curCell->physics.posY,
        pal_max(1, (int32)curCell->params.size),
        pal_max(1, (int32)curCell->params.size)
        );
    }
  }
  PAL_drawResetExtents(app);
  if(selectCell) {
    sprintf(strBuf,
        "name:%s\nage:%.04f\nenergy:%.04f\nfertility:%.04f\n"
        "size:%.04f\nscore:%.04f\navg. proc:%.04f\ntemp:%.04f",
        selectCell->name,
        selectCell->params.age,
        selectCell->params.energy,
        selectCell->params.fertility,
        selectCell->params.size,
        selectCell->score,
        selectCell->avgConsumption,
        selectCell->params.temperature
        );
    PAL_drawSetColour(app, 255, 255, 255, 255);
    PAL_drawText(app, w->worldRight+16, w->worldUp, strBuf, &app->draw.font);

    col = pal_pixel32(
      (uint8)(selectCell->colR*255.0f*selectCell->params.energy),
      (uint8)(selectCell->colG*255.0f*selectCell->params.energy),
      (uint8)(selectCell->colB*255.0f*selectCell->params.energy),
      255);

    PAL_drawSetPixel(app, col);
    PAL_drawEllipse(app, PAL_width(app)-32-64, w->worldUp+32, 10, 10);

    for(i = 0; i < MAX_SENSORS; i++) {
      inputPosX = (int32)(sin(((real32)i/MAX_SENSORS)*PAL_PI*2)*circleWidth);
      inputPosY = (int32)(cos(((real32)i/MAX_SENSORS)*PAL_PI*2)*circleWidth);
      PAL_drawSetColour(app,
          (uint8)(selectCell->params.sensors[i].outputVal*255),
          (uint8)(selectCell->params.sensors[i].outputVal*255),
          (uint8)(selectCell->params.sensors[i].outputVal*255),
          255);
			PAL_drawCircleF(app,
          circlePosX+inputPosX,
          circlePosY+inputPosY,
          4);

      neuronTextCols[i] *= 0.99f-(1.0f-neuronTextCols[i])*0.2f;
      neuronTextCol = ((uint32)(neuronTextCols[i]*192.0f));
      PAL_drawSetColour(app, 64+neuronTextCol, 64+neuronTextCol, 64+neuronTextCol, 255);
      if(pal_distance_between_points(
            (real32)PAL_mouseX(app),
            (real32)PAL_mouseY(app),
            (real32)circlePosX+inputPosX,
            (real32)circlePosY+inputPosY
            ) < 8.0f) {
        neuronTextCols[i] = 1.0f;
      }
      if(neuronTextCols[i] > 0.1f) {
        sprintf(strBuf, "%s",
          str_sensorNames[selectCell->params.sensors[i].type]
          );
        PAL_drawText(app,
          circlePosX-8+inputPosX,
          circlePosY-16+inputPosY,
          strBuf, &app->draw.font);
      }
    }

    for(i = 0; i < MAX_GENOME; i++) {
      curLink = &selectCell->params.links[i];
      inputSensorID = curLink->inputID%MAX_SENSORS;
      outputSensorID = curLink->outputID%MAX_SENSORS;
      inputPosX = (int32)(sin(((real32)inputSensorID/MAX_SENSORS)*PAL_PI*2)*circleWidth);
      outputPosX = (int32)(sin(((real32)outputSensorID/MAX_SENSORS)*PAL_PI*2)*circleWidth);
      inputPosY = (int32)(cos(((real32)inputSensorID/MAX_SENSORS)*PAL_PI*2)*circleWidth);
      outputPosY = (int32)(cos(((real32)outputSensorID/MAX_SENSORS)*PAL_PI*2)*circleWidth);
      PAL_drawSetPixel(app, col);
      curSen = &selectCell->params.sensors[inputSensorID];
      PAL_drawLine(app,
        PAL_width(app)-32-64+sin(curSen->angle)*5.0f,
        w->worldUp+32+cos(curSen->angle)*5.0f,
        PAL_width(app)-32-64+sin(curSen->angle)*15.0f,
        w->worldUp+32+cos(curSen->angle)*15.0f
        );
      curSen = &selectCell->params.sensors[outputSensorID];
      PAL_drawLine(app,
        PAL_width(app)-32-64+sin(curSen->angle)*5.0f,
        w->worldUp+32+cos(curSen->angle)*5.0f,
        PAL_width(app)-32-64+sin(curSen->angle)*15.0f,
        w->worldUp+32+cos(curSen->angle)*15.0f
        );
      PAL_drawSetColour(app, 128, 128, 128, 255);
      sprintf(strBuf, "%08X",
          selectCell->genome.genes[i].data
          );
      PAL_drawText(app,
          w->worldRight+16+(i/(MAX_GENOME/4))*(8*7+2),
          w->worldUp+128+(i%(MAX_GENOME/4))*14, strBuf, &app->draw.font);
      linkCol = evo_tanh(curLink->internalValue+1.0f, 1.0f)*128.0f;
      PAL_drawSetColour(app, 0, (uint8)linkCol*0.5f, (uint8)linkCol, 255);
      PAL_drawLine(app,
          circlePosX+inputPosX,
          circlePosY+inputPosY,
          circlePosX+outputPosX,
          circlePosY+outputPosY);
    }
  } else {
    PAL_drawSetColour(app, 255, 255, 255, 255);
    sprintf(strBuf, "Ranking:");
    PAL_drawTextB(app, w->worldRight+16, w->worldUp, strBuf, &app->draw.font);
    for(i = 0; i < FIT_SAMPLE_SIZE; i++) {
      sprintf(strBuf, "%s (%d) - %.04f", w->bestCells[i].name, w->bestCells[i].ID, w->bestCells[i].score);
      PAL_drawText(app, w->worldRight+16, w->worldUp+i*14+16, strBuf, &app->draw.font);
    }

    for(i = 0; i < w->sliderAmt; i++) {
      evo_slider_draw(app, w->worldRight+16, w->worldUp+FIT_SAMPLE_SIZE*20+i*14,
          global_sliderWidth, &slider[i]);
    }
    for(i = 0; i < 2; i++) {
      PAL_drawButton(app, &buttons[i], 3, 2);
    }
    PAL_drawSetColour(app, 255, 255, 255, 255);
    sprintf(strBuf, "%s", str_evoMenuState[w->menuState]);
    PAL_drawText(app, w->worldRight+16+buttons[0].rect.width*2+16,
        w->worldUp+FIT_SAMPLE_SIZE*17+4, strBuf, &app->draw.font);
  }
  if(toggleHelp) {
    sprintf(strBuf,
        "HELP: (Press F1 to hide)\n"
        "    F1 - Help\n"
        "    F2 - Toggle cell names visible\n"
        "    F5 - Generate new random world\n"
        "    Shift + L. Mouse - Draw wall on map\n"
        "    Shift + R. Mouse - Erase wall from map\n"
        );
    PAL_drawSetColour(app, 255, 255, 255, 255);
    PAL_drawText(app, w->worldLeft+16, w->worldUp+16, strBuf, &app->draw.font);
  }
  return TRUE;
}

sflag commandMenuUp(palCommandHandle command, void* args) {
  evoWorld* e = (evoWorld*)args;
  if(e->menuState < EVO_MENU_STATES-1) {
    e->menuState++;
  }
  return (args && command);
}

sflag commandMenuDown(palCommandHandle command, void* args) {
  evoWorld* e = (evoWorld*)args;
  if(e->menuState > 0) {
    e->menuState--;
  }
  return (args && command);
}

sflag pal_module_construct(palApp* app) {
  uint32 sIndex = 0;
  if(!app) { return FALSE; }
  srand(0);
  global_randIndex = rand();
  global_evoWorld.worldDetail = 12.0f;
  global_evoWorld.featureSize = 0.02f;
  evo_world_init(app, &global_evoWorld, 0.001f, 32, MAX_CELLS, 128, TRUE);

  buttons[0] = PAL_button(app, "<",
      global_evoWorld.worldRight+16,
      global_evoWorld.worldUp+FIT_SAMPLE_SIZE*14+16,
      PAL_commandAdd(app, commandMenuUp, &global_evoWorld, PAL_RIGHT, TRUE, 0, 0, "cmUp"));
  buttons[1] = PAL_button(app, ">",
      global_evoWorld.worldRight+16,
      global_evoWorld.worldUp+FIT_SAMPLE_SIZE*14+16,
      PAL_commandAdd(app, commandMenuDown, &global_evoWorld, PAL_LEFT, TRUE, 0, 0, "cmDown")
      );

  slider[sIndex++] = evo_slider("Sim. Speed", 0.0f, 100.0f, 0.01f);
  slider[sIndex++] = evo_slider("Start Amt.", 0.0f, (real32)MAX_CELLS, 0.5f);
  slider[sIndex++] = evo_slider("Max Amt.  ", 0.0f, (real32)MAX_CELLS, 0.5f);
  slider[sIndex++] = evo_slider("Max Genes ", 0.0f, (real32)MAX_GENOME, 0.5f);

  slider[sIndex++] = evo_slider("Size Limit", 0.0f, 32.0f, 16.0f);
  slider[sIndex++] = evo_slider("Age Effect", 0.0f, 1.0f, 0.1f);

  slider[sIndex++] = evo_slider("Food Value", 0.0f, 1.0f, 0.5f);
  slider[sIndex++] = evo_slider("Food Grow ", 0.0f, 1.0f, 0.5f);
  slider[sIndex++] = evo_slider("Food Decay", 0.0f, 1.0f, 0.5f);
  slider[sIndex++] = evo_slider("Food Sens.", 0.0f, 1.0f, 0.5f);

  slider[sIndex++] = evo_slider("Fr. Scale ", 0.0f, 1.0f, 0.75f);
  slider[sIndex++] = evo_slider("Fr. Iter. ", 0.0f, 20.0f, 0.75f);

  slider[sIndex++] = evo_slider("Avg. Temp.", 0.0f, 2.0f, 0.75f);
  slider[sIndex++] = evo_slider("Avg. Rain ", 0.0f, 2.0f, 0.8f);
  slider[sIndex++] = evo_slider("Season Mod", 0.0f, 2.0f, 0.5f);

  slider[sIndex++] = evo_slider("Score Eat ", 0.0f, 2.0f, 0.5f);
  slider[sIndex++] = evo_slider("Score Age ", 0.0f, 2.0f, 0.5f);
  slider[sIndex++] = evo_slider("Score Sex ", 0.0f, 2.0f, 0.5f);
  slider[sIndex++] = evo_slider("Score Move", 0.0f, 2.0f, 0.5f);
  slider[sIndex++] = evo_slider("Score Proc", 0.0f, 2.0f, 0.5f);
  slider[sIndex++] = evo_slider("Score Size", 0.0f, 2.0f, 0.5f);
  slider[sIndex++] = evo_slider("Score Kill", 0.0f, 2.0f, 0.5f);

  slider[sIndex++] = evo_slider("Food Col. ", 0.0f, 1.0f, 0.5f);
  slider[sIndex++] = evo_slider("Temp Col. ", 0.0f, 1.0f, 0.5f);
  slider[sIndex++] = evo_slider("Rain Col. ", 0.0f, 1.0f, 0.5f);
  slider[sIndex++] = evo_slider("Mutation"	, 0.0f, 10.0f, 0.1f);
  global_evoWorld.sliderAmt = sIndex;
  return TRUE;
}

sflag pal_module_update(palApp* app) {
  evoWorld* w = &global_evoWorld;
  int32 i;
  int32 updateSteps = 0;
  int32 sIndex = 0;
  global_randIndex += 479001599;
  srand((uint32)((global_randIndex+w->generations*0xFFFF) ^ 0XAAAAAAAA));
  if(!app) { return FALSE; }
  updateSteps = w->simSpeed;

  w->simSpeed = (int32)slider[sIndex++].outVal;
  w->startCells = (int32)slider[sIndex++].outVal;
  w->maxCells = (int32)slider[sIndex++].outVal;
  w->maxGenes = (int32)slider[sIndex++].outVal;

  w->sizeLimit = slider[sIndex++].outVal;
  w->ageEffect = slider[sIndex++].outVal;

  w->foodNutrition = slider[sIndex++].outVal;
  w->foodGrowth = (slider[sIndex++].outVal);
  w->foodDecay = (slider[sIndex++].outVal);
  w->foodRange = (slider[sIndex++].outVal);

  w->featureSize = slider[sIndex++].outVal;
  w->worldDetail = slider[sIndex++].outVal;

  w->tempMod = slider[sIndex++].outVal - 1.0f;
  w->humidMod = slider[sIndex++].outVal - 1.0f;
  w->seasonMod = slider[sIndex++].outVal - 1.0f;

  w->rewardEat = slider[sIndex++].outVal - 1.0f;
  w->rewardAge = slider[sIndex++].outVal - 1.0f;
  w->rewardSex = slider[sIndex++].outVal - 1.0f;
  w->rewardMove = slider[sIndex++].outVal - 1.0f;
  w->rewardProc = slider[sIndex++].outVal - 1.0f;
  w->rewardSize = slider[sIndex++].outVal - 1.0f;
  w->rewardKill = slider[sIndex++].outVal - 1.0f;

  w->foodOverlay = slider[sIndex++].outVal;
  w->tempOverlay = slider[sIndex++].outVal;
  w->humidOverlay = slider[sIndex++].outVal;

	w->mutationUsr = slider[sIndex++].outVal;

	for(i = 0; i < 2; i++) {
    PAL_buttonMove(&buttons[i], w->worldRight+16+i*(buttons[i].rect.width+2),
        w->worldUp+16+FIT_SAMPLE_SIZE*15);
    PAL_buttonUpdate(app, &buttons[i]);
  }

  for(i = 0; i < w->sliderAmt; i++) {
      evo_slider_update(app, w->worldRight+16, w->worldUp+FIT_SAMPLE_SIZE*20+i*14,
      global_sliderWidth, &slider[i]);
  }


  while(updateSteps > 0 && w->simPlaying) {
    if(w->aliveCells < 1) {

      evo_world_repopulate(app, w, 0.001f, 32,
          w->maxCells, w->startCells, FALSE);

      evo_world_update(app, w);
      updateSteps = 0;
      break;
    } else {
    	evo_world_update(app, w);
		}

    updateSteps--;
  }

	if(PAL_keyPressed(app, PAL_UP)) {
    if((!global_keyLock) && w->simSpeed < 500) {
      w->simSpeed++;
    }
  }

  if(PAL_keyPressed(app, PAL_DOWN)) {
    if(!global_keyLock && w->simSpeed > 1) {
      w->simSpeed--;
    }
  }

  if(PAL_keyPressed(app, PAL_SPACE)) {
    if(!global_keyLock) {
      w->simPlaying = !w->simPlaying;
    }
  }
  if(PAL_keyPressed(app, PAL_F5)) {
    if(!global_keyLock) {
      evo_world_init(app, w, 0.001f, 32, w->maxCells,
          w->startCells, TRUE);
      evo_world_update(app, w);
    }
  }

  if(PAL_keyPressedAny(app)) {
    global_holdDelay--;
    if(global_holdDelay > 0) {
      global_keyLock = TRUE;
    } else {
      global_keyLock = FALSE;
    }
  } else {
    global_holdDelay = (uint32)(PAL_appTimestep(app)*1000);
    global_keyLock = FALSE;
  }

  return TRUE;
}

sflag pal_module_render(palApp* app) {
  if(!app) { return FALSE; }
  PAL_drawSetColour(app, 64, 64, 64, 255);
  PAL_drawClear(app);
  evo_world_draw(app, &global_evoWorld);

	return TRUE;
}

sflag pal_module_resize(palApp* app) {
  evoWorld* w = &global_evoWorld;
  if(!app) { return FALSE; }
  evo_world_resize(app, w);
  evo_world_tiles_init(w, FALSE, w->curSeed);

  return TRUE;
}

sflag pal_module_destroy(palApp* app) {

  if(!app) { return FALSE; }
  return TRUE;
}
