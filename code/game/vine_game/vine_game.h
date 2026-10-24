#ifndef VINE_GAME_H
#define VINE_GAME_H

#include "../gng_math.h"

#define CELL_DIM 12

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

    i32 targetRow;
    i32 targetCol;

    b32 isRoot;
    b32 isHead;

    vine_node *prev;
    vine_node *next0;
    vine_node *next1;
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

    mine_cell *cells;
    i32 fieldWidth;
    i32 fieldHeight;
    i32 numBombs;

    vine_game_state state;

    b32 firstCellWasOpened;
    i32 score;

    f32 vineSeekDelay;

    vine_node_list vineNodeList;
} vine_game;

#endif
