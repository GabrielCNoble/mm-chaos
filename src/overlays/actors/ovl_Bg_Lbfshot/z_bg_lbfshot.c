/*
 * File: z_bg_lbfshot.c
 * Overlay: ovl_Bg_Lbfshot
 * Description: Rainbow Hookshot Pillar (those in STT climb)
 */

#include "z_bg_lbfshot.h"
#include "objects/object_lbfshot/object_lbfshot.h"
#include "chaos_fuckery.h"

#define FLAGS (0x00000000)

#define THIS ((BgLbfshot*)thisx)

void BgLbfshot_Init(Actor* thisx, PlayState* play);
void BgLbfshot_Destroy(Actor* thisx, PlayState* play);
void BgLbfshot_Draw(Actor* thisx, PlayState* play);

ActorInit Bg_Lbfshot_InitVars = {
    /**/ ACTOR_BG_LBFSHOT,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_LBFSHOT,
    /**/ sizeof(BgLbfshot),
    /**/ BgLbfshot_Init,
    /**/ BgLbfshot_Destroy,
    /**/ Actor_Noop,
    /**/ BgLbfshot_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_STOP),
};

void BgLbfshot_Init(Actor* thisx, PlayState* play) {
    BgLbfshot* this = THIS;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    this->dyna.actor.uncullZoneForward = 4000.0f;
    DynaPolyActor_Init(&this->dyna, DYNA_TRANSFORM_POS);
    DynaPolyActor_LoadMesh(play, &this->dyna, &object_lbfshot_Colheader_0014D8);
    if(!Chaos_GetConfigFlag(CHAOS_CONFIG_STONE_TOWER_CLIMB_ACTOR_CHASE))
    {
        thisx->flags |= ACTOR_FLAG_NO_CHASE;
    }
}
void BgLbfshot_Destroy(Actor* thisx, PlayState* play) {
    BgLbfshot* this = THIS;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}
void BgLbfshot_Draw(Actor* thisx, PlayState* play) {
    Gfx_DrawDListOpa(play, object_lbfshot_DL_000228);
}
