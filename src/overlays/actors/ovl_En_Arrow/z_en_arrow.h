#ifndef Z_EN_ARROW_H
#define Z_EN_ARROW_H

#include "global.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

struct EnArrow;

typedef void (*EnArrowActionFunc)(struct EnArrow*, PlayState*);

typedef enum ArrowType {
    /* 0 */ ARROW_TYPE_NORMAL_LIT, // Normal arrow lit on fire
    /* 1 */ ARROW_TYPE_NORMAL_HORSE, // Normal arrow shot while riding a horse
    /* 2 */ ARROW_TYPE_NORMAL,
    /* 3 */ ARROW_TYPE_FIRE,
    /* 4 */ ARROW_TYPE_ICE,
    /* 5 */ ARROW_TYPE_LIGHT,
    /* 6 */ ARROW_TYPE_SLINGSHOT,
    /* 7 */ ARROW_TYPE_DEKU_BUBBLE,
    /* 8 */ ARROW_TYPE_DEKU_NUT,
            ARROW_TYPE_BUCKSHOT = 1 << 8,
            ARROW_TYPE_NONE,
} ArrowType;

#define ARROW_IS_MAGICAL(arrowType) (((arrowType) >= ARROW_TYPE_FIRE) && ((arrowType) <= ARROW_TYPE_LIGHT))
#define ARROW_GET_MAGIC_FROM_TYPE(arrowType) (s32)((arrowType) - ARROW_TYPE_FIRE)
#define ARROW_IS_ARROW(arrowType) ((arrowType) < ARROW_TYPE_SLINGSHOT)

#define ENARROW_ARROW_MAX_FLIGHT_TIME       16
#define ENARROW_NORMAL_ARROW_LODGED_TIME    20
#define ENARROW_LIT_ARROW_LODGED_TIME       60
#define ENARROW_ARROW_NEAR_DEATH_GRAVITY -0.4f

typedef enum ArrowMagic {
    /* -1 */ ARROW_MAGIC_INVALID = -1,
    /*  0 */ ARROW_MAGIC_FIRE = ARROW_GET_MAGIC_FROM_TYPE(ARROW_TYPE_FIRE),
    /*  1 */ ARROW_MAGIC_ICE = ARROW_GET_MAGIC_FROM_TYPE(ARROW_TYPE_ICE),
    /*  2 */ ARROW_MAGIC_LIGHT = ARROW_GET_MAGIC_FROM_TYPE(ARROW_TYPE_LIGHT),
    /*  3 */ ARROW_MAGIC_DEKU_BUBBLE // Only used in Player. Does not map to ARROW_TYPE_SLINGSHOT
} ArrowMagic;


typedef enum ArrowHitFlags {
    ARROW_HIT_FLAG_1    = 1,
    ARROW_HIT_FLAG_2    = 1 << 1,
    ARROW_HIT_FLAG_4    = 1 << 2
} ArrowHitFlags;

typedef struct {
    /* 0x144 */ SkelAnime skelAnime;
    /* 0x188 */ Vec3s jointTable[ARROW_LIMB_MAX];
} EnArrowArrow; // size = 0x1A8

typedef struct {
    /* 0x144 */ f32 unk_144;
    /* 0x148 */ u8 unk_148;
    /* 0x149 */ s8 unk_149;
    /* 0x14A */ s16 unk_14A;
    /* 0x14C */ s16 unk_14C;
} EnArrowBubble; // size = 0x150

typedef struct EnArrow {
    /* 0x000 */ Actor actor;
    union {
        EnArrowArrow arrow;
        EnArrowBubble bubble;
    };
    /* 0x1A8 */ ColliderQuad collider;
    /* 0x228 */ Vec3f unk_228;
    /* 0x234 */ Vec3f unk_234;
    /* 0x240 */ s32 unk_240;
    /* 0x244 */ WeaponInfo unk_244;
    /* 0x260 */ u8 unk_260; // timer in OoT (alive timer)
    /* 0x261 */ u8 unk_261; // hitFlags in OoT
    /* 0x262 */ u8 unk_262; // hit background? (now chaos effect)
    /* 0x263 */ u8 unk_263;
    /* 0x264 */ Actor* unk_264; // hit actor
    /* 0x268 */ Vec3f unk_268;
    /* 0x274 */ EnArrowActionFunc actionFunc;
} EnArrow; // size = 0x278

#endif // Z_EN_ARROW_H
