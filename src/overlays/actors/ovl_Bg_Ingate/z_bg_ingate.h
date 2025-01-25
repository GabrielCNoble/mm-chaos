#ifndef Z_BG_INGATE_H
#define Z_BG_INGATE_H

#include "global.h"

struct BgIngate;

typedef void (*BgIngateActionFunc)(struct BgIngate*, PlayState*);

#define BGINGATE_GET_PATH_INDEX(thisx) ((thisx)->params & 0xFF)

#define BGINGATE_REACHED_DEKU_CASTLE (1 << 1)
#define BGINGATE_PLAYER_DISEMBARKED  (1 << 2)
#define BGINGATE_RIDE_JUST_STARTED   (1 << 4)
#define BGINGATE_PLAYER_LANDED       (1 << 14)
#define BGINGATE_PLAYER_ON_TOP       (1 << 15)

typedef struct BgIngate {
    /* 0x000 */ DynaPolyActor dyna;
    /* 0x15C */ BgIngateActionFunc actionFunc;
    /* 0x160 */ u16 unk160;
    /* 0x164 */ Path* timePath;
    /* 0x168 */ s16 timePathTimeSpeed;
    /* 0x16A */ s16 unk16A;
    /* 0x16C */ s16 unk16C;
    /* 0x16E */ s16 csId;
    /* 0x170 */ Vec3f timePathTargetPos;
    /* 0x17C */ f32 timePathProgress;
    /* 0x180 */ s32 timePathTotalTime;
    /* 0x184 */ s32 timePathWaypointTime;
    /* 0x188 */ s32 timePathWaypoint;
    /* 0x18C */ s32 timePathElapsedTime;
} BgIngate; // size = 0x190

#endif // Z_BG_INGATE_H
