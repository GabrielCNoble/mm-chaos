#include <stdbool.h>
#include "z64play.h"
#include "z64effect.h"
#include "z64eff_goron_blizzard.h"
#include "assets/objects/object_dai/object_dai.h"
#include "gfx.h"
#include "sys_matrix.h"
#include "macros.h"
#include "rand.h"
#include "gfx_setupdl.h"
#include "chaos_fuckery.h"

void EffectGoronBlizzard_Init(void* thisx, void* initParamsx)
{
    return;    
}

void EffectGoronBlizzard_Destroy(void* thisx)
{
    return;
}

void EffectGoronBlizzard_SpawnParticles(void *this)
{
    EffectGoronBlizzard *goron_blizzard = this;
    u32 particle_index;
    f32 particle_life_factor = (f32)goron_blizzard->particle_min_life / 3.0f;

    for (particle_index = 0; particle_index < ARRAY_COUNT(goron_blizzard->particles); particle_index++) 
    {
        EffectGoronBlizzardParticle *particle = goron_blizzard->particles + particle_index;
        if(particle->cur_life == 0)
        {
            // if(particle_index > EFF_GORON_BLIZZARD_PARTICLES)
            // {
            //     Chaos_ConsolePrintf("Well, shit..."); 
            // }
            // Chaos_ConsolePrintf("Spawn particle %d", particle - goron_blizzard->particles);
            
            // effect->isEnabled = true;
            particle->start_life = (Rand_ZeroOne() * (2.0f * particle_life_factor)) + particle_life_factor;
            particle->cur_life = particle->start_life;
            particle->unk_10 = goron_blizzard->particle_start_position;
            particle->unk_1C = goron_blizzard->particle_start_velocity;
            particle->unk_28 = goron_blizzard->particle_acceleration;
            particle->unk_34 = goron_blizzard->particle_start_scale;
            // particle->unk_38 = arg5;
            // return effect;
        }
    }
}

void EffectGoronBlizzard_UpdateParticles(void *thisx)
{
    EffectGoronBlizzard *goron_blizzard = thisx;
    s32 particle_index;
    s32 count;

    for(particle_index = 0; particle_index < ARRAY_COUNT(goron_blizzard->particles); particle_index++)
    {
        EffectGoronBlizzardParticle *particle = goron_blizzard->particles + particle_index;
        
        if(particle->cur_life > 0)
        {
            particle->cur_life--;
            particle->unk_10.x += particle->unk_28.x;
            particle->unk_10.y += particle->unk_28.y;
            particle->unk_10.z += particle->unk_28.z;
            particle->unk_28.x += particle->unk_1C.x;
            particle->unk_28.y += particle->unk_1C.y;
            particle->unk_28.z += particle->unk_1C.z;
            particle->unk_34 += goron_blizzard->particle_scale_increment;
        }
    }
}

s32 EffectGoronBlizzard_Update(void* thisx)
{
    return 0;
}

void EffectGoronBlizzard_Draw(void* thisx, struct GraphicsContext* gfxCtx)
{
    PlayState* play = Effect_GetPlayState();
    s32 pad;
    s32 isDisplayListSet = false;
    s32 particle_index;
    f32 alpha;

 
    EffectGoronBlizzard *goron_blizzard = thisx;

    OPEN_DISPS(gfxCtx);

    // if(goron_blizzard->particle_count > 0)
    {
        Gfx_SetupDL25_Xlu(gfxCtx);
        gDPPipeSync(POLY_XLU_DISP++);
        gSPDisplayList(POLY_XLU_DISP++, object_dai_DL_000230);
        
        for(particle_index = 0; particle_index < ARRAY_COUNT(goron_blizzard->particles); particle_index++)
        {
            EffectGoronBlizzardParticle *particle = goron_blizzard->particles + particle_index;

            if(particle->cur_life > 0)
            {
                // if (effect->isEnabled == true) {
                gDPPipeSync(POLY_XLU_DISP++);
    
                // if (!isDisplayListSet) {
                //     gSPDisplayList(POLY_XLU_DISP++, object_dai_DL_000230);
                //     isDisplayListSet = true;
                // }
    
                Matrix_Push();
    
                alpha = ((f32)particle->cur_life / (f32)particle->start_life);
                alpha *= 255.0f;
    
                gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 195, 225, 235, (u8)alpha);
    
                gSPSegment(POLY_XLU_DISP++, 0x08,
                            Gfx_TwoTexScroll(gfxCtx, 0, (particle->cur_life + (particle_index * 3)) * 3,
                                            (particle->cur_life + (particle_index * 3)) * 15, 0x20, 0x40, 1, 0, 0, 0x20, 0x20));
    
                Matrix_Translate(particle->unk_10.x, particle->unk_10.y, particle->unk_10.z, MTXMODE_NEW);
                Matrix_ReplaceRotation(&play->billboardMtxF);
                Matrix_Scale(particle->unk_34, particle->unk_34, 1.0f, MTXMODE_APPLY);
    
                MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gfxCtx);
                gSPDisplayList(POLY_XLU_DISP++, object_dai_DL_0002E8);
    
                Matrix_Pop();
            }
        }
    }


    CLOSE_DISPS(gfxCtx);
}
