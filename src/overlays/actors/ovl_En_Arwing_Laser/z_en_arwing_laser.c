#include "z_en_arwing_laser.h"
#include "libc/math.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/effects/ovl_Effect_Ss_Fire_Tail/z_eff_ss_fire_tail.h"
#include "overlays/effects/ovl_Effect_Ss_Dust/z_eff_ss_dust.h"
#include "chaos_fuckery.h"

// #define FLAGS (ACTOR_FLAG_0 | ACTOR_FLAG_2 | ACTOR_FLAG_4 | ACTOR_FLAG_5)
#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE | ACTOR_FLAG_IGNORE_QUAKE)

void EnArwingLaser_Init(Actor* thisx, PlayState* play);
void EnArwingLaser_Destroy(Actor* thisx, PlayState* play);
void EnArwingLaser_Update(Actor* thisx, PlayState* play2);
void EnArwingLaser_Draw(Actor* thisx, PlayState* play);

// void EnArwing_UpdateEffects(PlayState* play);
// void EnArwing_DrawEffects(PlayState* play);

// void EnArwing_CreateDebrisEffect(PlayState* play, Vec3f* position, Vec3f* velocity, Vec3f* acceleration, f32 scale,
//                                    f32 floorHeight);
// void EnArwing_CreateFireEffect(PlayState* play, Vec3f* pos, f32 scale);
// void EnArwing_CreateSmokeEffect(PlayState* play, Vec3f* position, f32 scale);
// void EnArwing_CreateFlashEffect(PlayState* play, Vec3f* position, f32 scale, f32 floorHeight, Vec3f* floorTangent);

// void EnArwing_CalculateFloorTangent(EnArwing* this);

ActorProfile En_Arwing_Laser_Profile = {
    /**/ ACTOR_EN_ARWING_LASER,
    /**/ ACTORCAT_BOSS,
    /**/ FLAGS,
    /**/ OBJECT_ARWING,
    /**/ sizeof(EnArwingLaser),
    /**/ EnArwingLaser_Init,
    /**/ EnArwingLaser_Destroy,
    /**/ EnArwingLaser_Update,
    /**/ EnArwingLaser_Draw,
};

// static u8 sIsEffectsInitialized = false;

static Vec3f sZeroVector = { 0.0f, 0.0f, 0.0f };

// static InitChainEntry sInitChain[] = {
//     // ICHAIN_VEC3F_DIV1000(scale, 5, ICHAIN_CONTINUE),
//     // ICHAIN_F32_DIV1000(gravity, -500, ICHAIN_CONTINUE),
//     // ICHAIN_U8(targetMode, TARGET_MODE_2, ICHAIN_CONTINUE),
//     // ICHAIN_F32(targetArrowOffset, 4000, ICHAIN_STOP),
// };

static ColliderCylinderInit sLaserCylinderInit = {
    {
        // COLTYPE_METAL,
        COL_MATERIAL_METAL,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        // ELEMTYPE_UNK0,
        ELEM_MATERIAL_UNK0,
        { 0xFFCFFFFF, 0x00, 0x04 },
        { 0xFFDFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 15, 30, 10, { 0, 0, 0 } },
};

void EnArwingLaser_Destroy(Actor* thisx, PlayState* play) {
    EnArwingLaser* this = (EnArwingLaser*)thisx;
    Collider_DestroyCylinder(play, &this->collider);
}

void EnArwingLaser_Init(Actor* thisx, PlayState* play) {
    Player *player = GET_PLAYER(play);
    EnArwingLaser* this = (EnArwingLaser*)thisx;

    // Actor_ProcessInitChain(&this->actor, sInitChain);

    // Collider_InitCylinder(play, &this->collider);

    // Initialize the Arwing laser.
    // if (this->actor.params == ARWING_LASER) {
    //     this->state = ARWING_STATE_LASER;
    //     this->timers[ARWING_TIMER_LASER_DEATH] = 70;
    this->life = 70;
    this->actor.speed = 35.0f;
    Actor_UpdateVelocityWithoutGravity(&this->actor);
    Actor_UpdatePos(&this->actor);
    this->actor.scale.x = 0.4f;
    this->actor.scale.y = 0.4f;
    this->actor.scale.z = 2.0f;
    this->actor.speed = 70.0f;
    this->actor.shape.rot.x = -this->actor.shape.rot.x;

    Actor_UpdateVelocityWithoutGravity(&this->actor);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sLaserCylinderInit);
    Actor_PlaySfx(&this->actor, NA_SE_IT_SWORD_REFLECT_MG);
    this->actor.flags |= ACTOR_FLAG_CHAOS;
    // } 
    // else 
    // { // Initialize the Arwing.
        // this->actor.flags |= ACTOR_FLAG_0;
    // this->actor.targetMode = TARGET_MODE_3;
    // Collider_SetCylinder(play, &this->collider, &this->actor, &sLaserCylinderInit);
    // this->actor.colChkInfo.health = 0;
        // this->actor.scale.x = 1.0f;
        // this->actor.scale.y = 1.0f;
        // this->actor.scale.z = 1.0f;

        // Update the Arwing to play the intro cutscene.
        // if (this->actor.params == ARWING_CUTSCENE_ARWING)
        // {
    // this->timers[ARWING_TIMER_ARWING_UPDATE_STATE] = 70;
    // this->timers[ARWING_TIMER_ARWING_ENTER_LOCKED_ON] = 250;
    // this->state = ARWING_STATE_DEMO;
    // this->actor.world.rot.x = 0x4000;
    // this->cutsceneMode = ARWING_CUTSCENE_MODE_SETUP;
    // this->cutsceneTimer = defaultCutsceneTimer;
    // this->timers[ARWING_TIMER_ARWING_UPDATE_BG_INFO] = 20;
    // this->targetPosition.x = player->actor.world.pos.x + random_direction.x;
    // this->targetPosition.y = player->actor.world.pos.y + 300.0f;
    // this->targetPosition.z = player->actor.world.pos.z + random_direction.z;
    // this->laser_count = 0;
    // Actor_PlaySfx(&this->actor, NA_SE_IT_SWORD_REFLECT_MG);
        // }

        // Initialize all effects to available if effects have not been initialized.
        // if (!sIsEffectsInitialized) {
        //     sIsEffectsInitialized = true;
        //     play->specialEffects = sEffects;
        //     for (i = 0; i < ARWING_EFFECT_COUNT; i++) {
        //         sEffects[i].type = ARWING_EFFECT_AVAILABLE;
        //     }
        //     this->drawMode = ARWING_DRAW_MODE_ALL;
        // }
    // }
}

void EnArwingLaser_Update(Actor* thisx, PlayState* play2) {
    u8 hasAtHit = false;
    s16 i;
    s16 xRotationTarget;
    s16 rotationScale;
    PlayState* play = play2;
    EnArwingLaser* this = (EnArwingLaser*)thisx;
    Player* player = GET_PLAYER(play);

    Actor_UpdatePos(&this->actor);

    // Check if the laser has hit a target.
    if (this->collider.base.atFlags & AT_HIT) {
        hasAtHit = true;
    }

    // Set laser collider properties.
    this->collider.dim.radius = 23;
    this->collider.dim.height = 25;
    this->collider.dim.yShift = -10;
    // this->collider.info.toucher.dmgFlags |= DMG_SWORD;
    
    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
    Actor_UpdateBgCheckInfo(play, &this->actor, 50.0f, 80.0f, 100.0f,
                            UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_2 | UPDBGCHECKINFO_FLAG_4);

    // Check if the laser has hit a target, timed out, or hit the ground or a wall.
    if ((this->actor.bgCheckFlags & (BGCHECKFLAG_GROUND | BGCHECKFLAG_WALL)) || hasAtHit ||
        this->life == 0) {
        // Kill the laser.
        Actor_Kill(&this->actor);
        // Player laser sound effect if the laser did not time out.
        if (this->life != 0) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_FANTOM_VOICE - SFX_FLAG);
        }
    }
}

void EnArwingLaser_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    EnArwingLaser* this = (EnArwingLaser*)thisx;

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    // Draw Arwing lasers.
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 0, 255, 0, 255);

    Matrix_Translate(25.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    // gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx),
    //             G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, gArwingLaserDL);

    Matrix_Translate(-50.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    // gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx),
    //             G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, gArwingLaserDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
