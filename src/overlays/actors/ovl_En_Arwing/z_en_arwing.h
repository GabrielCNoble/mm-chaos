#ifndef Z_EN_ARWING_H
#define Z_EN_ARWING_H

#include "ultra64.h"
#include "global.h"
#include "assets/objects/object_arwing/object_arwing.h"

struct EnArwing;

typedef enum {
    /* 0x00 */ ARWING_CUTSCENE_ARWING = 0,
    /* 0x01 */ ARWING_ARWING = 1,
               ARWING_FRIENDLY = 2,
    // /* 0x64 */ ARWING_LASER = 100
} ArwingType;

typedef enum {
    /* 0x00 */ ARWING_STATE_FLYING          = 0,
    /* 0x01 */ ARWING_STATE_TARGET_LOCKED   = 1,
    /* 0x02 */ ARWING_STATE_DEMO            = 2,
    /* 0x03 */ ARWING_STATE_CRASHING        = 3,
    /* 0x64 */ ARWING_STATE_LASER           = 100
} ArwingState;

typedef enum {
    /* 0x00 */ ARWING_CUTSCENE_MODE_NONE,
    /* 0x01 */ ARWING_CUTSCENE_MODE_SETUP,
    /* 0x02 */ ARWING_CUTSCENE_MODE_PLAY
} ArwingDemoMode;

typedef enum {
    /* 0x00 */ ARWING_DRAW_MODE_ARWING,
    /* 0x01 */ ARWING_DRAW_MODE_ALL,
    /* 0x02 */ ARWING_DRAW_MODE_EFFECT
} ArwingDrawMode;

typedef enum {
    /* 0x00 */ ARWING_EFFECT_AVAILABLE,
    /* 0x01 */ ARWING_EFFECT_DEBRIS,
    /* 0x02 */ ARWING_EFFECT_FIRE,
    /* 0x03 */ ARWING_EFFECT_SMOKE,
    /* 0x04 */ ARWING_EFFECT_FLASH
} ArwingEffectType;

typedef enum {
    /* 0x00 */ ARWING_TIMER_ARWING_UPDATE_STATE = 0,
    /* 0x00 */ ARWING_TIMER_LASER_DEATH = 0,
    /* 0x01 */ ARWING_TIMER_ARWING_ENTER_LOCKED_ON,
    // /* 0x02 */ ARWING_TIMER_ARWING_UPDATE_BG_INFO,
    /* 0x03 */ ARWING_TIMER_COUNT
} ArwingTimers;

// struct EnArwingLaser
// {
//     Vec3f               position;
//     Vec3s               rotation;
//     ColliderCylinder    collider;
// };

#define EN_ARWING_MAX_LASORS 16

typedef struct EnArwing {
    /* 0x0000 */ Actor              actor;
    /* 0x014C */ u8                 shouldExplode;
    /* 0x014D */ u8                 drawMode;
    /* 0x014E */ u8                 state;
    /* 0x0150 */ s16                timers[ARWING_TIMER_COUNT];
    /* 0x0158 */ Vec3f              targetPosition;
    /* 0x0164 */ Vec3f              targetDirection;
    /* 0x0170 */ Vec3f              acceleration;
    /* 0x017C */ u8                 frameCounter;
    /* 0x017D */ u8                 shouldShootLaser;
    /* 0x0180 */ f32                roll;
    /* 0x0184 */ s16                crashingTimer;
    /* 0x0186 */ s16                deathTimer;
    /* 0x0188 */ Vec3f              floorTangent;
    /* 0x0194 */ ColliderCylinder   collider;
                 Actor *            target_actor;
                //  u8                 laser_count;
    // struct EnArwingLaser            lasers[EN_ARWING_MAX_LASORS];
    // /* 0x01E0 */ u8 cutsceneMode;
    // /* 0x01E2 */ s16 subCamId;
    // /* 0x01E4 */ Vec3f subCamEye;
    // /* 0x01F0 */ Vec3f subCamAt;
    // /* 0x01FC */ s16 cutsceneTimer;
    // /* 0x01FE */ char unk_1FE[0x06];
} EnArwing; // size = 0x0204 

#define ARWING_EFFECT_COUNT 100

typedef struct EnArwingEffect {
    /* 0x0000 */ u8 type;
    /* 0x0001 */ u8 random;
    /* 0x0004 */ Vec3f position;
    /* 0x0010 */ Vec3f velocity;
    /* 0x001C */ Vec3f acceleration;
    /* 0x0028 */ Color_RGBAf primColor;
    /* 0x0038 */ Color_RGBAf envColor;
    /* 0x0048 */ s16 bounces;
    /* 0x004A */ s16 timer;
    /* 0x004C */ f32 scale;
    /* 0x0050 */ f32 maxScale;
    /* 0x0054 */ f32 rotationY;
    /* 0x0058 */ f32 rotationX;
    /* 0x005C */ f32 floorHeight;
    /* 0x0060 */ Vec3f floorTangent;
} EnArwingEffect; // size = 0x6C

#endif
