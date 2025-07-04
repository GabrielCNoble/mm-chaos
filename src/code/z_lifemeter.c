#include "z64lifemeter.h"

#include "global.h"
#include "su_mtx.h"

#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

#include "assets/interface/parameter_static/parameter_static.h"
#include "chaos_fuckery.h"

extern struct ChaosContext gChaosContext;

s16 sHeartsPrimColors[3][3] = { { 255, 70, 50 }, { 255, 190, 0 }, { 100, 100, 255 } };
s16 sHeartsEnvColors[3][3] = { { 50, 40, 60 }, { 255, 0, 0 }, { 0, 0, 255 } };
s16 sHeartsPrimFactors[3][3] = { { 0, 0, 0 }, { 0, 120, -50 }, { -155, 30, 205 } };
s16 sHeartsEnvFactors[3][3] = { { 0, 0, 0 }, { 205, -40, -60 }, { -50, -40, 195 } };
s16 sHeartsDDPrimColors[3][3] = { { 255, 255, 255 }, { 255, 190, 0 }, { 100, 100, 255 } };
s16 sHeartsDDEnvColors[3][3] = { { 200, 0, 0 }, { 255, 0, 0 }, { 0, 0, 255 } };
s16 sHeartsDDPrimFactors[3][3] = { { 0, 0, 0 }, { 0, -65, -255 }, { -155, -155, 0 } };
s16 sHeartsDDEnvFactors[3][3] = { { 0, 0, 0 }, { 55, 0, 0 }, { -200, 0, 255 } };

s16 sBeatingHeartsDDPrim[3];
s16 sBeatingHeartsDDEnv[3];
s16 sHeartsDDPrim[2][3];
s16 sHeartsDDEnv[2][3];

static TexturePtr sHeartTextures[] = {
    gHeartFullTex,         gHeartQuarterTex,      gHeartQuarterTex,      gHeartQuarterTex,
    gHeartQuarterTex,      gHeartQuarterTex,      gHeartHalfTex,         gHeartHalfTex,
    gHeartHalfTex,         gHeartHalfTex,         gHeartHalfTex,         gHeartThreeQuarterTex,
    gHeartThreeQuarterTex, gHeartThreeQuarterTex, gHeartThreeQuarterTex, gHeartThreeQuarterTex,
};

static TexturePtr sHeartDDTextures[] = {
    gDefenseHeartFullTex,         gDefenseHeartQuarterTex,      gDefenseHeartQuarterTex,
    gDefenseHeartQuarterTex,      gDefenseHeartQuarterTex,      gDefenseHeartQuarterTex,
    gDefenseHeartHalfTex,         gDefenseHeartHalfTex,         gDefenseHeartHalfTex,
    gDefenseHeartHalfTex,         gDefenseHeartHalfTex,         gDefenseHeartThreeQuarterTex,
    gDefenseHeartThreeQuarterTex, gDefenseHeartThreeQuarterTex, gDefenseHeartThreeQuarterTex,
    gDefenseHeartThreeQuarterTex,
};

void LifeMeter_Init(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    interfaceCtx->healthTimer = 320;

    interfaceCtx->health = gSaveContext.save.saveInfo.playerData.health;

    interfaceCtx->lifeColorChange = 0;
    interfaceCtx->lifeColorChangeDirection = 0;

    interfaceCtx->lifeSizeChange = interfaceCtx->lifeColorChange;
    interfaceCtx->lifeSizeChangeDirection = interfaceCtx->lifeColorChangeDirection;

    interfaceCtx->heartsPrimR[0] = 255;
    interfaceCtx->heartsPrimG[0] = 70;
    interfaceCtx->heartsPrimB[0] = 50;

    interfaceCtx->heartsEnvR[0] = 50;
    interfaceCtx->heartsEnvG[0] = 40;
    interfaceCtx->heartsEnvB[0] = 60;

    interfaceCtx->heartsPrimR[1] = 255;
    interfaceCtx->heartsPrimG[1] = 70;
    interfaceCtx->heartsPrimB[1] = 50;

    interfaceCtx->heartsEnvR[1] = 50;
    interfaceCtx->heartsEnvG[1] = 40;
    interfaceCtx->heartsEnvB[1] = 60;

    sHeartsDDPrim[0][0] = sHeartsDDPrim[1][0] = 255;
    sHeartsDDPrim[0][1] = sHeartsDDPrim[1][1] = 255;
    sHeartsDDPrim[0][2] = sHeartsDDPrim[1][2] = 255;

    sHeartsDDEnv[0][0] = sHeartsDDEnv[1][0] = 200;
    sHeartsDDEnv[0][1] = sHeartsDDEnv[1][1] = 0;
    sHeartsDDEnv[0][2] = sHeartsDDEnv[1][2] = 0;
}

void LifeMeter_UpdateColors(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    f32 factorBeating = interfaceCtx->lifeColorChange * 0.1f;
    f32 ddFactor;
    s32 type = 0;
    s32 ddType;
    s16 rFactor;
    s16 gFactor;
    s16 bFactor;

    if (interfaceCtx) {}

    if (interfaceCtx->lifeColorChangeDirection != 0) {
        interfaceCtx->lifeColorChange--;
        if (interfaceCtx->lifeColorChange <= 0) {
            interfaceCtx->lifeColorChange = 0;
            interfaceCtx->lifeColorChangeDirection = 0;
        }
    } else {
        interfaceCtx->lifeColorChange++;
        if (interfaceCtx->lifeColorChange >= 10) {
            interfaceCtx->lifeColorChange = 10;
            interfaceCtx->lifeColorChangeDirection = 1;
        }
    }

    ddFactor = factorBeating;

    interfaceCtx->heartsPrimR[0] = 255;
    interfaceCtx->heartsPrimG[0] = 70;
    interfaceCtx->heartsPrimB[0] = 50;

    interfaceCtx->heartsEnvR[0] = 50;
    interfaceCtx->heartsEnvG[0] = 40;
    interfaceCtx->heartsEnvB[0] = 60;

    interfaceCtx->heartsPrimR[1] = sHeartsPrimColors[type][0];
    interfaceCtx->heartsPrimG[1] = sHeartsPrimColors[type][1];
    interfaceCtx->heartsPrimB[1] = sHeartsPrimColors[type][2];

    interfaceCtx->heartsEnvR[1] = sHeartsEnvColors[type][0];
    interfaceCtx->heartsEnvG[1] = sHeartsEnvColors[type][1];
    interfaceCtx->heartsEnvB[1] = sHeartsEnvColors[type][2];

    rFactor = sHeartsPrimFactors[0][0] * factorBeating;
    gFactor = sHeartsPrimFactors[0][1] * factorBeating;
    bFactor = sHeartsPrimFactors[0][2] * factorBeating;

    interfaceCtx->beatingHeartPrim[0] = (u8)(rFactor + 255) & 0xFF;
    interfaceCtx->beatingHeartPrim[1] = (u8)(gFactor + 70) & 0xFF;
    interfaceCtx->beatingHeartPrim[2] = (u8)(bFactor + 50) & 0xFF;

    rFactor = sHeartsEnvFactors[0][0] * factorBeating;
    gFactor = sHeartsEnvFactors[0][1] * factorBeating;
    bFactor = sHeartsEnvFactors[0][2] * factorBeating;

    if (1) {}
    ddType = type;

    interfaceCtx->beatingHeartEnv[0] = (u8)(rFactor + 50) & 0xFF;
    interfaceCtx->beatingHeartEnv[1] = (u8)(gFactor + 40) & 0xFF;
    interfaceCtx->beatingHeartEnv[2] = (u8)(bFactor + 60) & 0xFF;

    sHeartsDDPrim[0][0] = 255;
    sHeartsDDPrim[0][1] = 255;
    sHeartsDDPrim[0][2] = 255;

    sHeartsDDEnv[0][0] = 200;
    sHeartsDDEnv[0][1] = 0;
    sHeartsDDEnv[0][2] = 0;

    sHeartsDDPrim[1][0] = sHeartsDDPrimColors[ddType][0];
    sHeartsDDPrim[1][1] = sHeartsDDPrimColors[ddType][1];
    sHeartsDDPrim[1][2] = sHeartsDDPrimColors[ddType][2];

    sHeartsDDEnv[1][0] = sHeartsDDEnvColors[ddType][0];
    sHeartsDDEnv[1][1] = sHeartsDDEnvColors[ddType][1];
    sHeartsDDEnv[1][2] = sHeartsDDEnvColors[ddType][2];

    rFactor = sHeartsDDPrimFactors[ddType][0] * ddFactor;
    gFactor = sHeartsDDPrimFactors[ddType][1] * ddFactor;
    bFactor = sHeartsDDPrimFactors[ddType][2] * ddFactor;

    sBeatingHeartsDDPrim[0] = (u8)(rFactor + 255) & 0xFF;
    sBeatingHeartsDDPrim[1] = (u8)(gFactor + 255) & 0xFF;
    sBeatingHeartsDDPrim[2] = (u8)(bFactor + 255) & 0xFF;

    rFactor = sHeartsDDEnvFactors[ddType][0] * ddFactor;
    gFactor = sHeartsDDEnvFactors[ddType][1] * ddFactor;
    bFactor = sHeartsDDEnvFactors[ddType][2] * ddFactor;

    sBeatingHeartsDDEnv[0] = (u8)(rFactor + 200) & 0xFF;
    sBeatingHeartsDDEnv[1] = (u8)(gFactor + 0) & 0xFF;
    sBeatingHeartsDDEnv[2] = (u8)(bFactor + 0) & 0xFF;
}

// Unused
s32 LifeMeter_SaveInterfaceHealth(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    gSaveContext.save.saveInfo.playerData.health = interfaceCtx->health;

    return 1;
}

// Unused
s32 LifeMeter_IncreaseInterfaceHealth(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    interfaceCtx->healthTimer = 320;
    interfaceCtx->health += 0x10;
    if (play->interfaceCtx.health >= gSaveContext.save.saveInfo.playerData.health) {
        play->interfaceCtx.health = gSaveContext.save.saveInfo.playerData.health;
        return true;
    }
    return false;
}

// Unused
s32 LifeMeter_DecreaseInterfaceHealth(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    if (interfaceCtx->healthTimer != 0) {
        interfaceCtx->healthTimer--;
    } else {
        interfaceCtx->healthTimer = 320;
        interfaceCtx->health -= 0x10;
        if (interfaceCtx->health <= 0) {
            interfaceCtx->health = 0;
            play->damagePlayer(play, -(((void)0, gSaveContext.save.saveInfo.playerData.health) + 1));
            return true;
        }
    }
    return false;
}

void LifeMeter_Draw(PlayState* play) {
    s32 pad[5];
    TexturePtr heartTex;
    s32 curColorSet;
    f32 offsetX;
    f32 offsetY;
    s32 i;
    f32 posY;
    f32 posX;
    f32 chaos_x = 0;
    f32 chaos_y = 0;
    f32 halfTexSize;
    f32 temp_f4;
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    Vtx* beatingHeartVtx = interfaceCtx->beatingHeartVtx;
    s32 fractionHeartCount = gSaveContext.save.saveInfo.playerData.health % LIFEMETER_FULL_HEART_HEALTH;
    s16 healthCapacity = gSaveContext.save.saveInfo.playerData.healthCapacity / LIFEMETER_FULL_HEART_HEALTH;
    s16 fullHeartCount = gSaveContext.save.saveInfo.playerData.health / LIFEMETER_FULL_HEART_HEALTH;
    s16 health_alpha = interfaceCtx->healthAlpha;
    s32 pad2;
    f32 lifesize = interfaceCtx->lifeSizeChange * 0.1f;
    u32 curCombineModeSet = 0;
    TexturePtr temp = NULL;
    s32 defense_hearts_count = gSaveContext.save.saveInfo.inventory.defenseHearts - 1;
    // u32 heart_snake = Chaos_IsCodeActive(CHAOS_CODE_HEART_SNAKE);
    u32 heart_snake = gChaosContext.ui.snake_state != CHAOS_SNAKE_GAME_STATE_NONE;

    if(heart_snake)
    {
        if(gChaosContext.ui.flags & CHAOS_SNAKE_GAME_FLAG_BLINK)
        {
            return;
        }

        health_alpha = 255;
    }

    OPEN_DISPS(gfxCtx);

    if ((gSaveContext.save.saveInfo.playerData.health % 0x10) == 0) {
        fullHeartCount--;
    }

    offsetY = 0.0f;
    offsetX = 0.0f;
    curColorSet = -1;

    for (i = 0; i < healthCapacity; i++) {

        // if(Chaos_IsCodeActive(CHAOS_CODE_WEIRD_UI))
        // {
        //     chaos_x = (Rand_ZeroOne() * 2.0f - 1.0f) * 8.0f;
        //     chaos_y = (Rand_ZeroOne() * 2.0f - 1.0f) * 8.0f;
        // }

        chaos_x = gChaosContext.ui.heart_containers[i].pos_x;
        chaos_y = gChaosContext.ui.heart_containers[i].pos_y;

        if ((defense_hearts_count < 0) || (defense_hearts_count < i)) 
        {
            if (i < fullHeartCount) {
                if (curColorSet != 0) {
                    curColorSet = 0;
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, interfaceCtx->heartsPrimR[0], interfaceCtx->heartsPrimG[0],
                                    interfaceCtx->heartsPrimB[0], health_alpha);
                    gDPSetEnvColor(OVERLAY_DISP++, interfaceCtx->heartsEnvR[0], interfaceCtx->heartsEnvG[0],
                                   interfaceCtx->heartsEnvB[0], 255);
                }
            } else if (i == fullHeartCount) {
                if (curColorSet != 1) {
                    curColorSet = 1;
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, interfaceCtx->beatingHeartPrim[0],
                                    interfaceCtx->beatingHeartPrim[1], interfaceCtx->beatingHeartPrim[2], health_alpha);
                    gDPSetEnvColor(OVERLAY_DISP++, interfaceCtx->beatingHeartEnv[0], interfaceCtx->beatingHeartEnv[1],
                                   interfaceCtx->beatingHeartEnv[2], 255);
                }
            } else if (fullHeartCount < i) {
                if (curColorSet != 2) {
                    curColorSet = 2;
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, interfaceCtx->heartsPrimR[0], interfaceCtx->heartsPrimG[0],
                                    interfaceCtx->heartsPrimB[0], health_alpha);
                    gDPSetEnvColor(OVERLAY_DISP++, interfaceCtx->heartsEnvR[0], interfaceCtx->heartsEnvG[0],
                                   interfaceCtx->heartsEnvB[0], 255);
                }
            } else {
                if (curColorSet != 3) {
                    curColorSet = 3;
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, interfaceCtx->heartsPrimR[1], interfaceCtx->heartsPrimG[1],
                                    interfaceCtx->heartsPrimB[1], health_alpha);
                    gDPSetEnvColor(OVERLAY_DISP++, interfaceCtx->heartsEnvR[1], interfaceCtx->heartsEnvG[1],
                                   interfaceCtx->heartsEnvB[1], 255);
                }
            }

            if (i < fullHeartCount) {
                heartTex = gHeartFullTex;
            } else if (i == fullHeartCount) {
                heartTex = sHeartTextures[fractionHeartCount];
            } else {
                heartTex = gHeartEmptyTex;
            }
        } 
        else 
        {
            if (i < fullHeartCount) {
                if (curColorSet != 4) {
                    curColorSet = 4;
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sHeartsDDPrim[0][0], sHeartsDDPrim[0][1], sHeartsDDPrim[0][2],
                                    health_alpha);
                    gDPSetEnvColor(OVERLAY_DISP++, sHeartsDDEnv[0][0], sHeartsDDEnv[0][1], sHeartsDDEnv[0][2], 255);
                }
            } else if (i == fullHeartCount) {
                if (curColorSet != 5) {
                    curColorSet = 5;
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sBeatingHeartsDDPrim[0], sBeatingHeartsDDPrim[1],
                                    sBeatingHeartsDDPrim[2], health_alpha);
                    gDPSetEnvColor(OVERLAY_DISP++, sBeatingHeartsDDEnv[0], sBeatingHeartsDDEnv[1],
                                   sBeatingHeartsDDEnv[2], 255);
                }
            } else if (i > fullHeartCount) {
                if (curColorSet != 6) {
                    curColorSet = 6;
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sHeartsDDPrim[0][0], sHeartsDDPrim[0][1], sHeartsDDPrim[0][2],
                                    health_alpha);
                    gDPSetEnvColor(OVERLAY_DISP++, sHeartsDDEnv[0][0], sHeartsDDEnv[0][1], sHeartsDDEnv[0][2], 255);
                }
            } else if (curColorSet != 7) {
                curColorSet = 7;
                gDPPipeSync(OVERLAY_DISP++);
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sHeartsDDPrim[1][0], sHeartsDDPrim[1][1], sHeartsDDPrim[1][2],
                                health_alpha);
                gDPSetEnvColor(OVERLAY_DISP++, sHeartsDDEnv[1][0], sHeartsDDEnv[1][1], sHeartsDDEnv[1][2], 255);
            }
            if (i < fullHeartCount) {
                heartTex = gDefenseHeartFullTex;
            } else if (i == fullHeartCount) {
                heartTex = sHeartDDTextures[fractionHeartCount];
            } else {
                heartTex = gDefenseHeartEmptyTex;
            }
        }

        if (temp != heartTex) {
            temp = heartTex;
            gDPLoadTextureBlock(OVERLAY_DISP++, heartTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
        }

        if (i != fullHeartCount || heart_snake) 
        {
            if ((defense_hearts_count < 0) || (i > defense_hearts_count)) {
                if (curCombineModeSet != 1) {
                    curCombineModeSet = 1;
                    Gfx_SetupDL39_Overlay(gfxCtx);
                    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE,
                                      0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
                }
            } else if (curCombineModeSet != 3) {
                curCombineModeSet = 3;
                Gfx_SetupDL39_Overlay(gfxCtx);
                gDPSetCombineLERP(OVERLAY_DISP++, ENVIRONMENT, PRIMITIVE, TEXEL0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0,
                                  ENVIRONMENT, PRIMITIVE, TEXEL0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
            }

            posX = chaos_x;
            posY = chaos_y;
            
            if(!heart_snake)
            {
                posX += 30.0f + offsetX;    
                posY += 26.0f + offsetY;
            }

            temp_f4 = 1.0f;
            temp_f4 /= 0.68f;
            temp_f4 *= 1 << 10;
            halfTexSize = 8.0f;
            halfTexSize *= 0.68f;
            gSPTextureRectangle(OVERLAY_DISP++, (s32)((posX - halfTexSize) * 4), (s32)((posY - halfTexSize) * 4),
                                (s32)((posX + halfTexSize) * 4), (s32)((posY + halfTexSize) * 4), G_TX_RENDERTILE, 0, 0,
                                (s32)temp_f4, (s32)temp_f4);
        } 
        else 
        {
            Mtx* mtx;
            f32 scale = 1.0f - (0.32f * lifesize);

            posX = chaos_x;
            posY = chaos_y;

            if(!heart_snake)
            {
                posX += -130.0f + offsetX;    
                posY += 94.5f + offsetY;
            }

            if ((defense_hearts_count < 0) || (defense_hearts_count < i)) {
                if (curCombineModeSet != 2) {
                    curCombineModeSet = 2;
                    Gfx_SetupDL42_Overlay(gfxCtx);
                    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE,
                                      0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
                    gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                }
            } else {
                if (curCombineModeSet != 4) {
                    curCombineModeSet = 4;
                    Gfx_SetupDL42_Overlay(gfxCtx);
                    gDPSetCombineLERP(OVERLAY_DISP++, ENVIRONMENT, PRIMITIVE, TEXEL0, PRIMITIVE, TEXEL0, 0, PRIMITIVE,
                                      0, ENVIRONMENT, PRIMITIVE, TEXEL0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
                    gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                }
            }
            mtx = GRAPH_ALLOC(gfxCtx, sizeof(Mtx));
            Mtx_SetTranslateScaleMtx(mtx, scale, scale, scale, posX, posY, 0.0f);
            gSPMatrix(OVERLAY_DISP++, mtx, G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPVertex(OVERLAY_DISP++, beatingHeartVtx, 4, 0);
            gSP1Quadrangle(OVERLAY_DISP++, 0, 2, 3, 1, 0);
        }

        offsetX += LIFEMETER_HEART_CONTAINER_SIZE;
        if (i == 9) {
            offsetY += LIFEMETER_HEART_CONTAINER_SIZE;
            offsetX = 0.0f;
        }
    }
    CLOSE_DISPS(gfxCtx);
}

void LifeMeter_UpdateSizeAndBeep(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    if (interfaceCtx->lifeSizeChangeDirection != 0) {
        interfaceCtx->lifeSizeChange--;
        if (interfaceCtx->lifeSizeChange <= 0) {
            interfaceCtx->lifeSizeChange = 0;
            interfaceCtx->lifeSizeChangeDirection = 0;
            if (!Player_InCsMode(play) && !IS_PAUSED(&play->pauseCtx) && LifeMeter_IsCritical() &&
                !Play_InCsMode(play)) {
                Audio_PlaySfx(NA_SE_SY_HITPOINT_ALARM);
            }
        }
    } else {
        interfaceCtx->lifeSizeChange++;
        if ((s32)interfaceCtx->lifeSizeChange >= 10) {
            interfaceCtx->lifeSizeChange = 10;
            interfaceCtx->lifeSizeChangeDirection = 1;
        }
    }
}

u32 LifeMeter_IsCritical(void) {
    s16 criticalThreshold;
    if (gSaveContext.save.saveInfo.playerData.healthCapacity <= 0x20) {
        criticalThreshold = 0x8;
    }else if (gSaveContext.save.saveInfo.playerData.healthCapacity <= 0x50) {
        criticalThreshold = 0x10;
    } else if (gSaveContext.save.saveInfo.playerData.healthCapacity <= 0xA0) {
        criticalThreshold = 0x18;
    } else if (gSaveContext.save.saveInfo.playerData.healthCapacity <= 0xF0) {
        criticalThreshold = 0x20;
    } else {
        criticalThreshold = 0x2C;
    }

    if ((criticalThreshold >= gSaveContext.save.saveInfo.playerData.health) &&
        (gSaveContext.save.saveInfo.playerData.health > 0)) {
        return true;
    }
    return false;
}
