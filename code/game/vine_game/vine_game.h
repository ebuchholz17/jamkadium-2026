#ifndef VINE_GAME_H
#define VINE_GAME_H

#include "../gng_math.h"

#define CELL_DIM 24
#define CAMERA_SPEED 300.0f

typedef struct scratch_mem_marker {
    void *marker;
} scratch_mem_marker;

typedef struct entity_id {
    i32 val;
    i32 gen;
} entity_id;

typedef struct cell_coord {
    char *key;
    i32 row;
    i32 col;
} cell_coord;

#define HASH_MAP_TYPE cell_coord
#include "../hash_map.h"

typedef struct pathfinding_node pathfinding_node;
struct pathfinding_node {
    cell_coord coords;
    f32 value;
    pathfinding_node *prev;
};

#define LIST_TYPE pathfinding_node
#include "../list.h"

typedef struct mine_cell {
    i32 row;
    i32 col;
    b32 open;
    b32 hasBomb;
    b32 flagged;
    i32 numAdjBombs;

    b32 seen;
    b32 hasTrimmers;

    f32 sortValue;

    entity_id vineID;
} mine_cell;

typedef mine_cell *mine_cell_ptr;
#define LIST_TYPE mine_cell_ptr
#include "../list.h"

typedef enum {
    VINE_TARGET_BOMB_STRATEGY_RANDOM_DIR,
    VINE_TARGET_BOMB_STRATEGY_ORTHOGONAL_DIR,
    VINE_TARGET_BOMB_STRATEGY_NEAREST,
    VINE_TARGET_BOMB_STRATEGY_RANDOM
} vine_target_bomb_strategy;

typedef struct vine_node vine_node;
struct vine_node {
    entity_id id;
    b32 active;

    vec2 bombSeekDir;
    vine_target_bomb_strategy bombStrat;

    f32 seekTimer;

    i32 row;
    i32 col;

    i32 lifeTime;

    i32 targetRow;
    i32 targetCol;

    b32 isRoot;
    b32 isHead;

    entity_id prev;
    entity_id next0;
    entity_id next1;

    i32 headFrame;
    f32 headFrameTimer;

    b32 hasFlower;
    f32 flowerAppearTimer;
    f32 flowerSpin;
};

#define LIST_TYPE vine_node
#include "../list.h"

typedef enum {
    VINE_GAME_STATE_TITLE_SCREEN,
    VINE_GAME_STATE_GAMEPLAY,
    VINE_GAME_STATE_GAME_OVER_SCREEN,
    VINE_GAME_STATE_WIN_SCREEN
} vine_game_state;

typedef enum {
    VINE_GAME_DIFFICULTY_EASY,
    VINE_GAME_DIFFICULTY_MEDIUM,
    VINE_GAME_DIFFICULTY_HARD
} vine_game_difficulty;

typedef struct vine_game {
    b32 isInitialized;
    b32 shouldReset;
    f32 gameTime;

    vec2 pointerDownPos;
    vec2 gameSpacePointerDownPos;
    b32 dragging;
    vec2 dragStartCameraPos;
    b32 panning;

    f32 cameraX;
    f32 cameraY;
    f32 cameraZoom;

    mine_cell *cells;
    i32 fieldWidth;
    i32 fieldHeight;
    i32 numTrimmersInField;
    i32 numBombs;

    vine_game_state state;

    b32 firstCellWasOpened;
    i32 score;

    f32 vineSeekDelay;
    i32 vineSplitLifeTime;

    vine_node_list vineNodeList;
    i32 numTrimmers;

    b32 openedCellCausedGameOver;
} vine_game;

#endif
