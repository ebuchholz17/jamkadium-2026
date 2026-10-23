#ifndef BUBBLE_GAME_H
#define BUBBLE_GAME_H

#include "entity_id.h"
#include "../gng_types.h"
#include "../gng_math.h"

#define GRID_NUM_ROWS 26
#define GRID_NUM_COLS 24

#define GRID_CELL_WIDTH 16.0f
#define GRID_CELL_HEIGHT 13.0f

#define BUBBLE_DIAMETER 16.0f
#define BUBBLE_RADIUS 8.0f
#define BUBBLE_RADIUS_SUCK 6.0f
#define BUBBLE_FLYING_RADIUS 4.5f

#define MAX_NUM_BUBBLES 800
#define BUBBLE_QUEUE_LENGTH 5

#define LAUNCHER_POS_X 192.0f 
#define LAUNCHER_POS_Y 314.0f 

#define BUBBLE_LAUNCH_SPEED 320.0f
#define BUBBLE_ZONE_WIDTH 384.0f 
#define BUBBLE_ZONE_HEIGHT 336.0f

#define LAUNCHED_BUBBLES_BEFORE_DROP 10
#define BUBBLES_DROP_TIMER 10.0f

#define MAX_NUM_ENEMIES 100
#define ENEMY_SPAWN_Y 200.0f
#define ENEMY_SPAWN_RANGE 80.0f

#define MAX_NUM_PROJECTILES 1000

#define POPPED_BUBBLE_GOLD 1
#define DROPPED_BUBBLE_GOLD 2

#define FLOATING_TEXT_RISE_DURATION 0.25f
#define FLOATING_TEXT_TOTAL_DURATION 1.0f

#define ENEMY_GROW_TIME 0.5f
#define ENEMY_ATTACK_TIME 0.75f
#define ENEMY_MAX_SIZE 30

#define MAX_LIVES 14
#define BASE_START_X 384.0f

typedef struct ScratchMemMarker {
    void *marker;
} ScratchMemMarker;

typedef enum {
    BUBBLE_COLOR_NONE,
    BUBBLE_COLOR_RED,
    BUBBLE_COLOR_YELLOW,
    BUBBLE_COLOR_GREEN,
    BUBBLE_COLOR_BLUE,
    BUBBLE_COLOR_PURPLE,
    BUBBLE_COLOR_COUNT,
} BubbleColor;

typedef enum {
    BUBBLE_TYPE_NORMAL,
    BUBBLE_TYPE_UNIT
} BubbleType;

typedef enum {
    UNIT_TYPE_ARCHER,
    UNIT_TYPE_KNIGHT,
    UNIT_TYPE_CANNON,
    UNIT_TYPE_FARMER
} UnitType;

typedef struct Bubble {
    EntityID id;
    b32 active;
    vec2 pos;
    BubbleColor color;

    BubbleType type;

    i32 row;
    i32 col;
    b32 flying;
    b32 falling;
    b32 marked;
    vec2 vel;

    UnitType unitType;
    f32 cooldownDuration;
    f32 cooldownTimer;
    f32 range;

    f32 projectileDamage;
    f32 projectileSpeed;

    b32 isDiamond;
} Bubble;

typedef struct BubbleQueueItem {
    BubbleType type;
    BubbleColor color;
    UnitType unitType;
} BubbleQueueItem;

typedef enum {
    ENTITY_TYPE_BUBBLE,
} EntityType;

typedef struct GridCoord {
    i32 row;
    i32 col;
} GridCoord;

typedef struct GridSlot {
    i32 row; 
    i32 col;
    EntityID id;
    b32 marked;
} GridSlot;

typedef enum {
    ENEMY_TYPE_GOBLIN,
    ENEMY_TYPE_GARGOYLE,
    ENEMY_TYPE_RAT,
    ENEMY_TYPE_BLACK_KNIGHT,
    ENEMY_TYPE_COUNT
} EnemyType;

typedef enum {
    ENEMY_STATE_WALKING,
    ENEMY_STATE_SUCKING_IN_BUBBLE,
    ENEMY_STATE_GROWING,
    ENEMY_STATE_ATTACKING_BASE,
    ENEMY_STATE_DYING
} EnemyState;

typedef enum {
    PROJECTILE_TYPE_NONE,
    PROJECTILE_TYPE_ARROW,
    PROJECTILE_TYPE_SLASH,
    PROJECTILE_TYPE_CANNONBALL
} ProjectileType;

typedef enum {
    UPGRADE_TYPE_NONE,
    UPGRADE_TYPE_ARROW_ATK,
    UPGRADE_TYPE_SLASH_ATK,
    UPGRADE_TYPE_CANNONBALL_ATK,
    UPGRADE_TYPE_FARMING,
    UPGRADE_TYPE_COUNT
} UpgradeType;

typedef struct Enemy {
    EntityID id;
    EnemyType type;
    vec2 pos;
    b32 active;

    EnemyState state;
    i32 hitPoints;
    i32 maxHitPoints;
    i32 baseHitPoints;
    EntityID suckingBubbleID;
    f32 t;
    f32 baseRadius;
    f32 radius;
    i32 damage;

    i32 size;
    f32 scale;
    vec2 startPos;
    f32 speed;

    UpgradeType weakness;
    UpgradeType resists;

    f32 walkTimer;
    i32 walkFrame;
} Enemy;

typedef struct Button {
    char *upFrame;
    char *overFrame;
    char *downFrame;
    char *icon;
    char *label;
    char *labelIcon;
    b32 isOver;
    b32 isDown;
    vec2 pos;
    vec2 dims;
    vec2 labelOffset;
} Button;

typedef struct FloatingText {
    EntityID id;
    b32 active;
    i32 amount;
    char *icon;
    vec2 startPos;
    vec2 pos;
    f32 t;
} FloatingText;


typedef struct Projectile {
    EntityID id;
    EntityID target;
    vec2 pos;

    UpgradeType upgradeType;

    b32 active;
    f32 speed;
    i32 damage;
    f32 angle;
    char *frame;
} Projectile;

typedef struct EnemyWave {
    i32 numEnemies;
    f32 enemyProbs[ENEMY_TYPE_COUNT];
    EnemyType bossType;
    i32 bossSize;
    b32 spawnedBoss;

    f32 spawnTime;
} EnemyWave;

#define PARTICLE_BURST_TIME 0.2f

typedef struct BubbleParticles {
    EntityID id;
    vec2 pos;
    b32 active;

    BubbleColor color;
    f32 t;
    f32 angles[5];
} BubbleParticles;

#define LIST_TYPE Enemy
#include "../list.h"

typedef Enemy *EnemyPtr;
#define LIST_TYPE EnemyPtr
#include "../list.h"

#define LIST_TYPE Bubble
#include "../list.h"

typedef Bubble *BubblePtr;
#define LIST_TYPE BubblePtr
#include "../list.h"

#define LIST_TYPE BubbleColor
#include "../list.h"

#define LIST_TYPE BubbleQueueItem
#include "../list.h"

#define LIST_TYPE Projectile
#include "../list.h"

#define LIST_TYPE Button
#include "../list.h"

#define LIST_TYPE FloatingText
#include "../list.h"

typedef GridSlot *GridSlotPtr;
#define LIST_TYPE GridSlotPtr
#include "../list.h"

#define LIST_TYPE EnemyWave
#include "../list.h"

#define LIST_TYPE BubbleParticles
#include "../list.h"

typedef enum {
    GAME_STATE_TITLE,
    GAME_STATE_NORMAL
} GameState;

typedef struct bubble_game {
    GridSlot grid[GRID_NUM_ROWS * GRID_NUM_COLS];
    Bubble_list bubbles;
    BubbleQueueItem_list bubbleQueue;
    vec2 launcherTarget;
    i32 offsetIndex;
    i32 bubblesTilDrop;
    f32 dropTimer;

    Enemy_list enemies;
    f32 timeToSpawnNextEnemy;
    f32 enemySpawnTimer;

    Projectile_list projectiles;

    Button_list buttons;
    i32 gold;
    i32 diamonds;

    FloatingText_list floatingTexts;
    i32 lives;
    UpgradeType upgrades[UPGRADE_TYPE_COUNT];
    EnemyWave_list waves;
    i32 waveIndex;

    b32 win;
    b32 gameOver;

    BubbleParticles_list particles;
    GameState state;
    f32 gameEndTimer;
    b32 reset;

    b32 tooLowDanger;
    f32 tooLowDangerTimer;
    b32 lineRed;
    f32 gameTime;
    b32 bubblesBurst;
    b32 isInitialized;
} bubble_game;

#endif
