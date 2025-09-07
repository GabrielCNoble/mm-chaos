#ifndef Z64EFF_GORON_BLIZZARD
#define Z64EFF_GORON_BLIZZARD

#include "ultra64.h"
#include "z64math.h"
#include "unk.h"

struct GraphicsContext;

#define EFF_GORON_BLIZZARD_PARTICLES 32

typedef struct EffectGoronBlizzardParticle
{
    // /* 0x00 */ u8           isEnabled;
    // /* 0x01 */ u8           unk_01;
    // /* 0x02 */ u8           unk_02;
    // /* 0x01 */ u8           unk_01;
    /* 0x01 */ u8           start_life; 
    /* 0x02 */ u8           cur_life;
    /* 0x03 */ UNK_TYPE1    unk_03[0xD];
    /* 0x10 */ Vec3f        unk_10; // effect position
    /* 0x1C */ Vec3f        unk_1C; // effect velocity
    /* 0x28 */ Vec3f        unk_28; // effect acceleration
    /* 0x34 */ f32          unk_34; // effect scale
    // /* 0x38 */ f32          unk_38;
} EffectGoronBlizzardParticle;

typedef struct EffectGoronBlizzard {
    // /* 0x00 */ u8           isEnabled;
    // /* 0x01 */ u8           unk_01;
    // /* 0x02 */ u8           unk_02; // effect tex coord scroll
    // /* 0x03 */ UNK_TYPE1    unk_03[0xD];
    // /* 0x10 */ Vec3f        unk_10; // effect position
    // /* 0x1C */ Vec3f        unk_1C; // effect velocity
    // /* 0x28 */ Vec3f        unk_28; // effect acceleration
    // /* 0x34 */ f32          unk_34; // effect scale
    // /* 0x38 */ f32          unk_38;
    Vec3f                       position;
    Vec3f                       particle_start_position;
    Vec3f                       particle_start_velocity;
    Vec3f                       particle_acceleration;
    f32                         particle_scale_increment;
    f32                         particle_start_scale;
    u8                          particle_min_life; 
    EffectGoronBlizzardParticle particles[EFF_GORON_BLIZZARD_PARTICLES]; 
} EffectGoronBlizzard; // size = 0x3C


void EffectGoronBlizzard_Init(void* thisx, void* initParamsx);
void EffectGoronBlizzard_Destroy(void* thisx);
void EffectGoronBlizzard_SpawnParticles(void *this);
void EffectGoronBlizzard_UpdateParticles(void *thisx);
s32  EffectGoronBlizzard_Update(void* thisx);
void EffectGoronBlizzard_Draw(void* thisx, struct GraphicsContext* gfxCtx);



#endif
