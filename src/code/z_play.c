#include "PR/ultratypes.h"

// Variables are put before most headers as a hacky way to bypass bss reordering
s16 sTransitionFillTimer;
struct Input D_801F6C18;
struct TransitionTile sTransitionTile;
s32 gTransitionTileState;
struct VisMono sPlayVisMono;
union Color_RGBA8_u32 gPlayVisMonoColor;
struct VisFbuf sPlayVisFbuf;
struct VisFbuf* sPlayVisFbufInstance;
struct BombersNotebook sBombersNotebook;
u8 sBombersNotebookOpen;
u8 sMotionBlurStatus;

#include "z64play.h"

#include "buffers.h"
#include "gfxalloc.h"
#include "idle.h"
#include "regs.h"
#include "sys_cfb.h"
#include "attributes.h"
#include "fault.h"

#include "z64bombers_notebook.h"
#include "z64debug_display.h"
#include "zelda_arena.h"
#include "z64quake.h"
#include "z64rumble.h"
#include "z64shrink_window.h"
#include "z64view.h"
#include "z64vis.h"
#include "z64visfbuf.h"
#include "z64lifemeter.h"

#include "overlays/gamestates/ovl_daytelop/z_daytelop.h"
#include "overlays/gamestates/ovl_opening/z_opening.h"
#include "overlays/gamestates/ovl_file_choose/z_file_select.h"
#include "libu64/debug.h"
#include "chaos_fuckery.h"
#include "libc/math.h"

#include "overlays/actors/ovl_En_Niw/z_en_niw.h"
#include "overlays/actors/ovl_En_Attack_Niw/z_en_attack_niw.h"
#include "overlays/actors/ovl_En_Rr/z_en_rr.h"
#include "overlays/actors/ovl_En_Wallmas/z_en_wallmas.h"
#include "overlays/actors/ovl_En_Rd/z_en_rd.h"
#include "overlays/actors/ovl_En_Skb/z_en_skb.h"
#include "overlays/actors/ovl_En_Weather_Tag/z_en_weather_tag.h"

s32 gDbgCamEnabled = false;
u8 D_801D0D54 = false;

extern struct ChaosContext gChaosContext;
extern u32 gCurrentBgmId;

typedef enum {
    /* 0 */ MOTION_BLUR_OFF,
    /* 1 */ MOTION_BLUR_SETUP,
    /* 2 */ MOTION_BLUR_PROCESS
} MotionBlurStatus;

void Play_DrawMotionBlur(PlayState* this) {
    GraphicsContext* gfxCtx = this->state.gfxCtx;
    s32 alpha;
    Gfx* gfx;
    Gfx* gfxHead;

    if (R_MOTION_BLUR_PRIORITY_ENABLED) {
        alpha = R_MOTION_BLUR_PRIORITY_ALPHA;

        if (sMotionBlurStatus == MOTION_BLUR_OFF) {
            sMotionBlurStatus = MOTION_BLUR_SETUP;
        }
    } else if (R_MOTION_BLUR_ENABLED) {
        alpha = R_MOTION_BLUR_ALPHA;

        if (sMotionBlurStatus == MOTION_BLUR_OFF) {
            sMotionBlurStatus = MOTION_BLUR_SETUP;
        }
    } else {
        alpha = 0;
        sMotionBlurStatus = MOTION_BLUR_OFF;
    }

    if (sMotionBlurStatus != MOTION_BLUR_OFF) {
        OPEN_DISPS(gfxCtx);

        gfxHead = POLY_OPA_DISP;
        gfx = Gfx_Open(gfxHead);

        gSPDisplayList(OVERLAY_DISP++, gfx);

        this->pauseBgPreRender.fbuf = gfxCtx->curFrameBuffer;
        this->pauseBgPreRender.fbufSave = this->unk_18E64;

        if (sMotionBlurStatus == MOTION_BLUR_PROCESS) {
            func_80170AE0(&this->pauseBgPreRender, &gfx, alpha);
        } else {
            sMotionBlurStatus = MOTION_BLUR_PROCESS;
        }

        PreRender_SaveFramebuffer(&this->pauseBgPreRender, &gfx);

        gSPEndDisplayList(gfx++);

        Gfx_Close(gfxHead, gfx);

        POLY_OPA_DISP = gfx;

        CLOSE_DISPS(gfxCtx);
    }
}

void Play_InitMotionBlur(void) {
    R_MOTION_BLUR_ENABLED = false;
    R_MOTION_BLUR_PRIORITY_ENABLED = false;
    sMotionBlurStatus = MOTION_BLUR_OFF;
}

void Play_DestroyMotionBlur(void) {
    R_MOTION_BLUR_ENABLED = false;
    R_MOTION_BLUR_PRIORITY_ENABLED = false;
    sMotionBlurStatus = MOTION_BLUR_OFF;
}

void Play_SetMotionBlurAlpha(u32 alpha) {
    R_MOTION_BLUR_ALPHA = alpha;
}

void Play_EnableMotionBlur(u32 alpha) {
    R_MOTION_BLUR_ALPHA = alpha;
    R_MOTION_BLUR_ENABLED = true;
}

void Play_DisableMotionBlur(void) {
    R_MOTION_BLUR_ENABLED = false;
}

// How much each color component contributes to the intensity image.
// These coefficients are close to what the YUV color space defines Y (luminance) as:
// https://en.wikipedia.org/wiki/YUV#Conversion_to/from_RGB
#define PLAY_INTENSITY_RED 2
#define PLAY_INTENSITY_GREEN 4
#define PLAY_INTENSITY_BLUE 1
#define PLAY_INTENSITY_NORM (0x1F * PLAY_INTENSITY_RED + 0x1F * PLAY_INTENSITY_GREEN + 0x1F * PLAY_INTENSITY_BLUE)
#define PLAY_INTENSITY_MIX(r, g, b, m) \
    ((((r)*PLAY_INTENSITY_RED + (g)*PLAY_INTENSITY_GREEN + (b)*PLAY_INTENSITY_BLUE) * (m)) / PLAY_INTENSITY_NORM)

/**
 * Converts an RGBA16 buffer to an Intensity Image
 *
 * @param[out] destI destination buffer
 * @param[in] srcRgba16 source buffer
 * @param[in] rgba16Width width of a full row for the RGBA16
 * @param[in] pixelLeft X coordinate of the top-left RGBA16 pixel to start with
 * @param[in] pixelTop Y coordinate of the top-left RGBA16 pixel to start with
 * @param[in] pixelRight X coordinate of the bottom-right RGBA16 pixel to end with
 * @param[in] pixelBottom Y coordinate of the bottom-right RGBA16 pixel to end with
 * @param[in] bitDepth bit depth for the intensity image
 */
void Play_ConvertRgba16ToIntensityImage(void* destI, u16* srcRgba16, s32 rgba16Width, s32 pixelLeft, s32 pixelTop,
                                        s32 pixelRight, s32 pixelBottom, s32 bitDepth) {
    s32 i;
    s32 j;
    u32 pixel;
    u32 r;
    u32 g;
    u32 b;

    switch (bitDepth) {
        case 4: {
            u8* destI4 = destI;
            u32 upper;
            u32 lower;

            for (i = pixelTop; i <= pixelBottom; i++) {
                for (j = pixelLeft; j <= pixelRight; j += 2) {
                    pixel = srcRgba16[i * rgba16Width + j];
                    r = RGBA16_GET_R(pixel);
                    g = RGBA16_GET_G(pixel);
                    b = RGBA16_GET_B(pixel);
                    upper = PLAY_INTENSITY_MIX(r, g, b, 15);

                    pixel = srcRgba16[i * rgba16Width + j + 1];
                    r = RGBA16_GET_R(pixel);
                    g = RGBA16_GET_G(pixel);
                    b = RGBA16_GET_B(pixel);
                    lower = PLAY_INTENSITY_MIX(r, g, b, 15);

                    *(destI4++) = (upper << 4) | lower;
                }
            }
            break;
        }

        case 5: {
            u8* destI5 = destI;

            for (i = pixelTop; i <= pixelBottom; i++) {
                for (j = pixelLeft; j <= pixelRight; j++) {
                    pixel = srcRgba16[i * rgba16Width + j];
                    r = RGBA16_GET_R(pixel);
                    g = RGBA16_GET_G(pixel);
                    b = RGBA16_GET_B(pixel);

                    pixel = 0;

                    *(destI5++) = PLAY_INTENSITY_MIX(r, g, b, 255) & 0xF8;
                }
            }
            break;
        }

        case 8: {
            u8* destI8 = destI;

            for (i = pixelTop; i <= pixelBottom; i++) {
                for (j = pixelLeft; j <= pixelRight; j++) {
                    pixel = srcRgba16[i * rgba16Width + j];

                    r = RGBA16_GET_R(pixel);
                    g = RGBA16_GET_G(pixel);
                    b = RGBA16_GET_B(pixel);

                    *(destI8++) = PLAY_INTENSITY_MIX(r, g, b, 255);
                }
            }
            break;
        }

        case 16: {
            u16* destI16 = destI;

            for (i = pixelTop; i <= pixelBottom; i++) {
                for (j = pixelLeft; j <= pixelRight; j++) {
                    *(destI16++) = srcRgba16[i * rgba16Width + j];
                }
            }
            break;
        }
    }
}

void Play_SetMotionBlurPriorityAlpha(u32 alpha) {
    R_MOTION_BLUR_PRIORITY_ALPHA = alpha;
}

void Play_EnableMotionBlurPriority(u32 alpha) {
    R_MOTION_BLUR_PRIORITY_ALPHA = alpha;
    R_MOTION_BLUR_PRIORITY_ENABLED = true;
}

void Play_DisableMotionBlurPriority(void) {
    R_MOTION_BLUR_PRIORITY_ENABLED = false;
}

// Will take the photograph, but doesn't compress and save it
void Play_TriggerPictoPhoto(void) {
    R_PICTO_PHOTO_STATE = PICTO_PHOTO_STATE_SETUP;
}

void Play_TakePictoPhoto(PreRender* prerender) {
    PreRender_ApplyFilters(prerender);
    Play_ConvertRgba16ToIntensityImage(gHiBuffer.pictoPhotoI8, prerender->fbufSave, SCREEN_WIDTH, PICTO_PHOTO_TOPLEFT_X,
                                       PICTO_PHOTO_TOPLEFT_Y, (PICTO_PHOTO_TOPLEFT_X + PICTO_PHOTO_WIDTH) - 1,
                                       (PICTO_PHOTO_TOPLEFT_Y + PICTO_PHOTO_HEIGHT) - 1, 8);
}

s32 Play_ChooseDynamicTransition(PlayState* this, s32 transitionType) {
    s32 nextTransitionType = transitionType;

    if (transitionType == TRANS_TYPE_FADE_DYNAMIC) {
        if (!gSaveContext.save.isNight) {
            nextTransitionType = TRANS_TYPE_FADE_WHITE;
        } else {
            nextTransitionType = TRANS_TYPE_FADE_BLACK;
        }
    }
    if (nextTransitionType != transitionType) {
        this->transitionType = nextTransitionType;
    }
    return nextTransitionType;
}

void Play_SetupTransition(PlayState* this, s32 transitionType) {
    TransitionContext* transitionCtx = &this->transitionCtx;
    s32 fbdemoType;

    bzero(transitionCtx, sizeof(TransitionContext));

    fbdemoType = -1;
    if (transitionType & TRANS_TYPE_WIPE3) {
        fbdemoType = FBDEMO_WIPE3;
    } else if ((transitionType & 0x78) == TRANS_TYPE_WIPE4) { // Checks not only type, but also a max value of 39
        fbdemoType = FBDEMO_WIPE4;
    } else if (!(transitionType & (TRANS_TYPE_WIPE3 | TRANS_TYPE_WIPE4))) {
        switch (transitionType) {
            case TRANS_TYPE_TRIFORCE:
                fbdemoType = FBDEMO_TRIFORCE;
                break;

            case TRANS_TYPE_WIPE:
            case TRANS_TYPE_WIPE_FAST:
                fbdemoType = FBDEMO_WIPE1;
                break;

            case TRANS_TYPE_FADE_BLACK:
            case TRANS_TYPE_FADE_WHITE:
            case TRANS_TYPE_FADE_BLACK_FAST:
            case TRANS_TYPE_FADE_WHITE_FAST:
            case TRANS_TYPE_FADE_BLACK_SLOW:
            case TRANS_TYPE_FADE_WHITE_SLOW:
            case TRANS_TYPE_FADE_WHITE_CS_DELAYED:
            case TRANS_TYPE_FADE_WHITE_INSTANT:
            case TRANS_TYPE_FADE_GREEN:
            case TRANS_TYPE_FADE_BLUE:
                fbdemoType = FBDEMO_FADE;
                break;

            case TRANS_TYPE_FILL_WHITE_FAST:
            case TRANS_TYPE_FILL_WHITE:
                this->transitionMode = TRANS_MODE_FILL_WHITE_INIT;
                break;

            case TRANS_TYPE_INSTANT:
                this->transitionMode = TRANS_MODE_INSTANT;
                break;

            case TRANS_TYPE_FILL_BROWN:
                this->transitionMode = TRANS_MODE_FILL_BROWN_INIT;
                break;

            case TRANS_TYPE_SANDSTORM_PERSIST:
                this->transitionMode = TRANS_MODE_SANDSTORM_INIT;
                break;

            case TRANS_TYPE_SANDSTORM_END:
                this->transitionMode = TRANS_MODE_SANDSTORM_END_INIT;
                break;

            case TRANS_TYPE_CS_BLACK_FILL:
                this->transitionMode = TRANS_MODE_CS_BLACK_FILL_INIT;
                break;

            case TRANS_TYPE_CIRCLE:
                fbdemoType = FBDEMO_CIRCLE;
                break;

            case TRANS_TYPE_WIPE5:
                fbdemoType = FBDEMO_WIPE5;
                break;

            default:
                fbdemoType = -1;
                _dbg_hungup("../z_play.c", 1420);
                break;
        }
    } else {
        fbdemoType = -1;
        _dbg_hungup("../z_play.c", 1423);
    }

    transitionCtx->transitionType = transitionType;
    transitionCtx->fbdemoType = fbdemoType;
    if (fbdemoType != -1) {
        Transition_Init(transitionCtx);
    }
}

void Play_ClearTransition(PlayState* this) {
    if (this->transitionCtx.fbdemoType != -1) {
        Transition_Destroy(&this->transitionCtx);
    }
    this->transitionCtx.transitionType = -1;
}

Gfx* Play_SetFog(PlayState* this, Gfx* gfx) {
    s32 fogFar = this->lightCtx.zFar * (5.0f / 64.0f);

    return Gfx_SetFogWithSync(gfx, this->lightCtx.fogColor[0], this->lightCtx.fogColor[1], this->lightCtx.fogColor[2],
                              0, this->lightCtx.fogNear, ((fogFar <= 1000) ? 1000 : fogFar));
}

void Play_Destroy(GameState* thisx) {
    PlayState* this = (PlayState*)thisx;
    GraphicsContext* gfxCtx = this->state.gfxCtx;

    if (sBombersNotebookOpen) {
        Sched_FlushTaskQueue();
        SysCfb_SetLoResMode();
        gfxCtx->curFrameBuffer = SysCfb_GetFramebuffer(gfxCtx->framebufferIndex % 2);
        gfxCtx->zbuffer = SysCfb_GetZBuffer();
        gfxCtx->viMode = gActiveViMode;
        gfxCtx->viConfigFeatures = gViConfigFeatures;
        gfxCtx->xScale = gViConfigXScale;
        gfxCtx->yScale = gViConfigYScale;
        gfxCtx->updateViMode = true;
        sBombersNotebookOpen = false;
    }

    BombersNotebook_Destroy(&sBombersNotebook);
    this->state.gfxCtx->callback = NULL;
    this->state.gfxCtx->callbackArg = NULL;
    Play_DestroyMotionBlur();

    if (R_PAUSE_BG_PRERENDER_STATE != PAUSE_BG_PRERENDER_OFF) {
        PreRender_ApplyFiltersSlowlyDestroy(&this->pauseBgPreRender);
        R_PAUSE_BG_PRERENDER_STATE = PAUSE_BG_PRERENDER_OFF;
    }

    R_PICTO_PHOTO_STATE = PICTO_PHOTO_STATE_OFF;
    PreRender_Destroy(&this->pauseBgPreRender);
    this->unk_18E58 = NULL;
    this->pictoPhotoI8 = NULL;
    this->unk_18E60 = NULL;
    this->unk_18E64 = NULL;
    this->unk_18E68 = NULL;
    Effect_DestroyAll(this);
    EffectSS_Clear(this);
    CollisionCheck_DestroyContext(this, &this->colChkCtx);

    if (gTransitionTileState == TRANS_TILE_READY) {
        TransitionTile_Destroy(&sTransitionTile);
        gTransitionTileState = TRANS_TILE_OFF;
    }

    if ((this->transitionMode == TRANS_MODE_INSTANCE_RUNNING) || D_801D0D54) {
        this->transitionCtx.destroy(&this->transitionCtx.instanceData);
        Play_ClearTransition(this);
        this->transitionMode = TRANS_MODE_OFF;
    }

    ShrinkWindow_Destroy();
    TransitionFade_Destroy(&this->unk_18E48);
    VisMono_Destroy(&sPlayVisMono);
    VisFbuf_Destroy(sPlayVisFbufInstance);
    sPlayVisFbufInstance = NULL;

    // Object_CleanupObjectTableFaultClient();

    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_92_80)) {
        Actor_CleanupContext(&this->actorCtx, this);
    }
    CLEAR_WEEKEVENTREG(WEEKEVENTREG_92_80);

    Interface_Destroy(this);
    KaleidoScopeCall_Destroy(this);
    KaleidoManager_Destroy();
    ZeldaArena_Cleanup();
}

#define PLAY_COMPRESS_BITS 5
#define PLAY_DECOMPRESS_BITS 8

void Play_CompressI8ToI5(void* srcI8, void* destI5, size_t size) {
    u32 i;
    u8* src = srcI8;
    s8* dest = destI5;
    s32 bitsLeft = PLAY_DECOMPRESS_BITS; // Bits left in the current dest pixel left to compress into
    u32 destPixel = 0;
    s32 shift;
    u32 srcPixel;

    for (i = 0; i < size; i++) {
        srcPixel = *src++;
        srcPixel = (srcPixel * 0x1F + 0x80) / 0xFF;
        shift = bitsLeft - PLAY_COMPRESS_BITS;
        if (shift > 0) {
            destPixel |= srcPixel << shift;
        } else {
            destPixel |= srcPixel >> -shift;
            *dest++ = destPixel;
            shift += PLAY_DECOMPRESS_BITS;
            destPixel = srcPixel << shift;
        }
        bitsLeft = shift;
    }

    if (bitsLeft < PLAY_DECOMPRESS_BITS) {
        *dest = destPixel;
    }
}

void Play_DecompressI5ToI8(void* srcI5, void* destI8, size_t size) {
    u32 i;
    u8* src = srcI5;
    s8* dest = destI8;
    s32 bitsLeft = PLAY_DECOMPRESS_BITS; // Bits left in the current src pixel left to decompress
    u32 destPixel;
    s32 shift;
    u32 srcPixel = *src++;

    for (i = 0; i < size; i++) {
        shift = bitsLeft - PLAY_COMPRESS_BITS;
        if (shift > 0) {
            destPixel = 0;
            destPixel |= srcPixel >> shift;
        } else {
            destPixel = 0;
            destPixel |= srcPixel << -shift;
            srcPixel = *src++;
            shift += PLAY_DECOMPRESS_BITS;
            destPixel |= srcPixel >> shift;
        }
        destPixel = (destPixel & 0x1F) * 0xFF / 0x1F;
        *dest++ = destPixel;
        bitsLeft = shift;
    }
}

f32 Play_GetWaterSurface(PlayState* this, Vec3f* pos, s32* lightIndex) {
    Player* player = GET_PLAYER(this);
    f32 waterSurfaceY = player->actor.world.pos.y;
    WaterBox* waterBox;
    s32 bgId;

    if (!WaterBox_GetSurfaceImpl(this, &this->colCtx, pos->x, pos->z, &waterSurfaceY, &waterBox, &bgId)) {
        return BGCHECK_Y_MIN;
    }

    if (waterSurfaceY < pos->y) {
        return BGCHECK_Y_MIN;
    }

    *lightIndex = WaterBox_GetLightSettingIndex(&this->colCtx, waterBox);
    return waterSurfaceY;
}

void Play_UpdateWaterCamera(PlayState* this, Camera* camera) {
    static s16 sQuakeIndex = -1;
    static s16 sIsCameraUnderwater = false;
    s32 pad;
    s32 lightIndex;
    Player* player = GET_PLAYER(this);

    sIsCameraUnderwater = camera->stateFlags & CAM_STATE_UNDERWATER;
    if (Play_GetWaterSurface(this, &camera->eye, &lightIndex) != BGCHECK_Y_MIN) {
        if (!sIsCameraUnderwater) {
            Camera_SetStateFlag(camera, CAM_STATE_UNDERWATER);
            sQuakeIndex = -1;
            Distortion_Request(DISTORTION_TYPE_UNDERWATER_ENTRY);
            Distortion_SetDuration(80);
        }

        Audio_SetExtraFilter(0x20);
        Environment_EnableUnderwaterLights(this, lightIndex);

        if ((sQuakeIndex == -1) || (Quake_GetTimeLeft(sQuakeIndex) == 10)) {
            s16 quakeIndex = Quake_Request(camera, QUAKE_TYPE_5);

            sQuakeIndex = quakeIndex;
            if (quakeIndex != 0) {
                Quake_SetSpeed(sQuakeIndex, 550);
                Quake_SetPerturbations(sQuakeIndex, 1, 1, 180, 0);
                Quake_SetDuration(sQuakeIndex, 1000);
            }
        }
        if (player->stateFlags3 & PLAYER_STATE3_8000) {
            Distortion_Request(DISTORTION_TYPE_ZORA_SWIMMING);
            Distortion_RemoveRequest(DISTORTION_TYPE_NON_ZORA_SWIMMING);
        } else {
            Distortion_Request(DISTORTION_TYPE_NON_ZORA_SWIMMING);
            Distortion_RemoveRequest(DISTORTION_TYPE_ZORA_SWIMMING);
        }
    } else {
        if (sIsCameraUnderwater) {
            Camera_UnsetStateFlag(camera, CAM_STATE_UNDERWATER);
        }
        Distortion_RemoveRequest(DISTORTION_TYPE_NON_ZORA_SWIMMING);
        Distortion_RemoveRequest(DISTORTION_TYPE_UNDERWATER_ENTRY);
        Distortion_RemoveRequest(DISTORTION_TYPE_ZORA_SWIMMING);
        if (sQuakeIndex != 0) {
            Quake_RemoveRequest(sQuakeIndex);
        }
        Environment_DisableUnderwaterLights(this);
        Audio_SetExtraFilter(0);
    }
}

void Play_UpdateTransition(PlayState* this) {
    s32 pad;

    if (this->transitionMode == TRANS_MODE_OFF) {
        return;
    }

    switch (this->transitionMode) {
        case TRANS_MODE_SETUP:
            if (this->transitionTrigger != TRANS_TRIGGER_END) {
                s16 sceneLayer = 0;

                Interface_SetHudVisibility(HUD_VISIBILITY_NONE);

                if (gSaveContext.nextCutsceneIndex >= 0xFFF0) {
                    sceneLayer = (gSaveContext.nextCutsceneIndex & 0xF) + 1;
                }

                if ((!(Entrance_GetTransitionFlags(this->nextEntrance + sceneLayer) & ENTR_TRANSITION_FLAG_PRESERVE_SEQS_ON_TRANSITION) ||
                     (this->nextEntrance == ENTRANCE(PATH_TO_MOUNTAIN_VILLAGE, 1) && !CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE)) ||
                     (this->nextEntrance == ENTRANCE(ROAD_TO_SOUTHERN_SWAMP, 1)   && !CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE)) ||
                     (this->nextEntrance == ENTRANCE(TERMINA_FIELD, 2)            && !CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE)) ||
                     (this->nextEntrance == ENTRANCE(ROAD_TO_IKANA, 1)            && !CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE))) &&
                        (!Environment_IsFinalHours(this) || (Entrance_GetSceneId(this->nextEntrance + sceneLayer) < 0 && 
                            (Entrance_GetBgmFlags(this->nextEntrance + sceneLayer) & ENTR_BGM_FLAG_SUPRESS_FINAL_HOURS_BGM)) ||
                            !Audio_IsFinalHours()) || Environment_IsForcedSequenceDisabled())
                {
                    Audio_MuteAllSeqExceptSystemAndOcarina(20);
                    gSaveContext.seqId = (u8)NA_BGM_DISABLED;
                    gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
                }
  
                // if (Environment_IsForcedSequenceDisabled()) {
                //     Audio_MuteAllSeqExceptSystemAndOcarina(20);
                //     gSaveContext.seqId = (u8)NA_BGM_DISABLED;
                //     gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
                // }

                // if (Environment_IsFinalHours(this) && (Entrance_GetSceneId(this->nextEntrance + sceneLayer) >= 0) &&
                //     (AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) == NA_BGM_FINAL_HOURS)) {
                //     Audio_MuteSfxAndAmbienceSeqExceptSysAndOca(20);
                // }

                if (Environment_IsFinalHours(this) && Audio_IsFinalHours() /* && !Environment_IsDungeonEntrance(this) */) 
                {
                    Audio_MuteSfxAndAmbienceSeqExceptSysAndOca(20);
                }
            }

            if (!D_801D0D54) {
                Play_SetupTransition(this, Play_ChooseDynamicTransition(this, this->transitionType));
            }

            if (this->transitionMode >= TRANS_MODE_FILL_WHITE_INIT) {
                // non-instance modes break out of this switch
                break;
            }
            FALLTHROUGH;
        case TRANS_MODE_INSTANCE_INIT: {
            s32 transWipeSpeed;
            s32 transFadeDuration;
            u32 color;

            this->transitionCtx.init(&this->transitionCtx.instanceData);

            if (this->transitionCtx.transitionType & (TRANS_TYPE_WIPE3 | TRANS_TYPE_WIPE4)) {
                this->transitionCtx.setType(&this->transitionCtx.instanceData,
                                            this->transitionCtx.transitionType | TRANS_TYPE_SET_PARAMS);
            }

            if ((this->transitionCtx.transitionType == TRANS_TYPE_WIPE_FAST) ||
                (this->transitionCtx.transitionType == TRANS_TYPE_FILL_WHITE_FAST)) {
                //! @bug TRANS_TYPE_FILL_WHITE_FAST will never reach this code.
                //! It is a non-instance type transition which doesn't run this case.
                transWipeSpeed = 28;
            } else {
                transWipeSpeed = 14;
            }
            gSaveContext.transWipeSpeed = transWipeSpeed;

            switch (this->transitionCtx.transitionType) {
                case TRANS_TYPE_FADE_BLACK_FAST:
                case TRANS_TYPE_FADE_WHITE_FAST:
                    transFadeDuration = 20;
                    break;

                case TRANS_TYPE_FADE_BLACK_SLOW:
                case TRANS_TYPE_FADE_WHITE_SLOW:
                    transFadeDuration = 150;
                    break;

                case TRANS_TYPE_FADE_WHITE_INSTANT:
                    transFadeDuration = 2;
                    break;

                default:
                    transFadeDuration = 60;
                    break;
            }
            gSaveContext.transFadeDuration = transFadeDuration;

            switch (this->transitionCtx.transitionType) {
                case TRANS_TYPE_FADE_WHITE:
                case TRANS_TYPE_FADE_WHITE_FAST:
                case TRANS_TYPE_FADE_WHITE_SLOW:
                case TRANS_TYPE_FADE_WHITE_CS_DELAYED:
                case TRANS_TYPE_FADE_WHITE_INSTANT:
                    color = RGBA8(160, 160, 160, 255);
                    break;

                case TRANS_TYPE_FADE_GREEN:
                    color = RGBA8(140, 140, 100, 255);
                    break;

                case TRANS_TYPE_FADE_BLUE:
                    color = RGBA8(70, 100, 110, 255);
                    break;

                default:
                    color = RGBA8(0, 0, 0, 255);
                    break;
            }

            this->transitionCtx.setColor(&this->transitionCtx.instanceData, color);
            if (this->transitionCtx.setEnvColor != NULL) {
                this->transitionCtx.setEnvColor(&this->transitionCtx.instanceData, color);
            }

            this->transitionCtx.setType(&this->transitionCtx.instanceData,
                                        (this->transitionTrigger == TRANS_TRIGGER_END) ? TRANS_INSTANCE_TYPE_FILL_OUT
                                                                                       : TRANS_INSTANCE_TYPE_FILL_IN);
            this->transitionCtx.start(&this->transitionCtx.instanceData);

            if (this->transitionCtx.transitionType == TRANS_TYPE_FADE_WHITE_CS_DELAYED) {
                this->transitionMode = TRANS_MODE_INSTANCE_WAIT;
            } else {
                this->transitionMode = TRANS_MODE_INSTANCE_RUNNING;
            }
            break;
        }

        case TRANS_MODE_INSTANCE_RUNNING:
            if (this->transitionCtx.isDone(&this->transitionCtx.instanceData)) {
                if (this->transitionTrigger != TRANS_TRIGGER_END) {
                    if (this->transitionCtx.transitionType == TRANS_TYPE_CIRCLE) {
                        D_801D0D54 = false;
                    }

                    if (gSaveContext.gameMode == GAMEMODE_OWL_SAVE) {
                        STOP_GAMESTATE(&this->state);
                        SET_NEXT_GAMESTATE(&this->state, TitleSetup_Init, sizeof(TitleSetupState));
                    } else if (gSaveContext.gameMode != GAMEMODE_FILE_SELECT) {
                        STOP_GAMESTATE(&this->state);
                        SET_NEXT_GAMESTATE(&this->state, Play_Init, sizeof(PlayState));
                        gSaveContext.save.entrance = this->nextEntrance;

                        if (gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) {
                            gSaveContext.minigameStatus = MINIGAME_STATUS_END;
                        }
                    } else { // GAMEMODE_FILE_SELECT
                        STOP_GAMESTATE(&this->state);
                        SET_NEXT_GAMESTATE(&this->state, FileSelect_Init, sizeof(FileSelectState));
                    }
                } else {
                    if (this->transitionCtx.transitionType == TRANS_TYPE_CIRCLE) {
                        D_801D0D54 = true;
                    } else {
                        this->transitionCtx.destroy(&this->transitionCtx.instanceData);
                        Play_ClearTransition(this);
                    }
                    this->transitionMode = TRANS_MODE_OFF;
                    if (gTransitionTileState == TRANS_TILE_READY) {
                        TransitionTile_Destroy(&sTransitionTile);
                        gTransitionTileState = TRANS_TILE_OFF;
                        GameState_SetFramerateDivisor(&this->state, 3);
                    }
                }
                this->transitionTrigger = TRANS_TRIGGER_OFF;
            } else {
                this->transitionCtx.update(&this->transitionCtx.instanceData, this->state.framerateDivisor);
            }
            break;
    }

    // update non-instance transitions
    switch (this->transitionMode) {
        case TRANS_MODE_FILL_WHITE_INIT:
            sTransitionFillTimer = 0;
            this->envCtx.fillScreen = true;
            this->envCtx.screenFillColor[0] = 160;
            this->envCtx.screenFillColor[1] = 160;
            this->envCtx.screenFillColor[2] = 160;

            if (this->transitionTrigger != TRANS_TRIGGER_END) {
                this->envCtx.screenFillColor[3] = 0;
                this->transitionMode = TRANS_MODE_FILL_IN;
            } else {
                this->envCtx.screenFillColor[3] = 255;
                this->transitionMode = TRANS_MODE_FILL_OUT;
            }
            break;

        case TRANS_MODE_FILL_IN:
            this->envCtx.screenFillColor[3] = (sTransitionFillTimer / 20.0f) * 255.0f;

            if (sTransitionFillTimer >= 20) {
                STOP_GAMESTATE(&this->state);
                SET_NEXT_GAMESTATE(&this->state, Play_Init, sizeof(PlayState));
                gSaveContext.save.entrance = this->nextEntrance;
                this->transitionTrigger = TRANS_TRIGGER_OFF;
                this->transitionMode = TRANS_MODE_OFF;
            } else {
                sTransitionFillTimer++;
            }
            break;

        case TRANS_MODE_FILL_OUT:
            this->envCtx.screenFillColor[3] = (1.0f - (sTransitionFillTimer / 20.0f)) * 255.0f;

            if (sTransitionFillTimer >= 20) {
                gTransitionTileState = TRANS_TILE_OFF;
                GameState_SetFramerateDivisor(&this->state, 3);
                this->transitionTrigger = TRANS_TRIGGER_OFF;
                this->transitionMode = TRANS_MODE_OFF;
                this->envCtx.fillScreen = false;
            } else {
                sTransitionFillTimer++;
            }
            break;

        case TRANS_MODE_FILL_BROWN_INIT:
            sTransitionFillTimer = 0;
            this->envCtx.fillScreen = true;
            this->envCtx.screenFillColor[0] = 170;
            this->envCtx.screenFillColor[1] = 160;
            this->envCtx.screenFillColor[2] = 150;

            if (this->transitionTrigger != TRANS_TRIGGER_END) {
                this->envCtx.screenFillColor[3] = 0;
                this->transitionMode = TRANS_MODE_FILL_IN;
            } else {
                this->envCtx.screenFillColor[3] = 255;
                this->transitionMode = TRANS_MODE_FILL_OUT;
            }
            break;

        case TRANS_MODE_INSTANT:
            if (this->transitionTrigger != TRANS_TRIGGER_END) {
                STOP_GAMESTATE(&this->state);
                SET_NEXT_GAMESTATE(&this->state, Play_Init, sizeof(PlayState));
                gSaveContext.save.entrance = this->nextEntrance;
                this->transitionTrigger = TRANS_TRIGGER_OFF;
                this->transitionMode = TRANS_MODE_OFF;
            } else {
                gTransitionTileState = TRANS_TILE_OFF;
                GameState_SetFramerateDivisor(&this->state, 3);
                this->transitionTrigger = TRANS_TRIGGER_OFF;
                this->transitionMode = TRANS_MODE_OFF;
            }
            break;

        case TRANS_MODE_INSTANCE_WAIT:
            if (gSaveContext.cutsceneTransitionControl != 0) {
                this->transitionMode = TRANS_MODE_INSTANCE_RUNNING;
            }
            break;

        case TRANS_MODE_SANDSTORM_INIT:
            if (this->transitionTrigger != TRANS_TRIGGER_END) {
                this->envCtx.sandstormState = SANDSTORM_FILL;
                this->transitionMode = TRANS_MODE_SANDSTORM;
            } else {
                this->envCtx.sandstormState = SANDSTORM_UNFILL;
                this->envCtx.sandstormPrimA = 255;
                this->envCtx.sandstormEnvA = 255;
                this->transitionMode = TRANS_MODE_SANDSTORM;
            }
            break;

        case TRANS_MODE_SANDSTORM:
            Audio_PlaySfx_2(NA_SE_EV_SAND_STORM - SFX_FLAG);
            if (this->transitionTrigger == TRANS_TRIGGER_END) {
                if (this->envCtx.sandstormPrimA < 110) {
                    gTransitionTileState = TRANS_TILE_OFF;
                    GameState_SetFramerateDivisor(&this->state, 3);
                    this->transitionTrigger = TRANS_TRIGGER_OFF;
                    this->transitionMode = TRANS_MODE_OFF;
                }
            } else {
                if (this->envCtx.sandstormEnvA == 255) {
                    if(gChaosContext.screen_slayer)
                    {        
                        *gChaosContext.fake_crash_pointer = false;
                    }
                    STOP_GAMESTATE(&this->state);
                    SET_NEXT_GAMESTATE(&this->state, Play_Init, sizeof(PlayState));
                    gSaveContext.save.entrance = this->nextEntrance;
                    this->transitionTrigger = TRANS_TRIGGER_OFF;
                    this->transitionMode = TRANS_MODE_OFF;
                }
            }
            break;

        case TRANS_MODE_SANDSTORM_END_INIT:
            if (this->transitionTrigger == TRANS_TRIGGER_END) {
                this->envCtx.sandstormState = SANDSTORM_DISSIPATE;
                this->envCtx.sandstormPrimA = 255;
                this->envCtx.sandstormEnvA = 255;

                // "It's here!!!!!!!!!"
                (void)"来た!!!!!!!!!!!!!!!!!!!!!";

                this->transitionMode = TRANS_MODE_SANDSTORM_END;
            } else {
                this->transitionMode = TRANS_MODE_SANDSTORM_INIT;
            }
            break;

        case TRANS_MODE_SANDSTORM_END:
            Audio_PlaySfx_2(NA_SE_EV_SAND_STORM - SFX_FLAG);

            if (this->transitionTrigger == TRANS_TRIGGER_END) {
                if (this->envCtx.sandstormPrimA <= 0) {
                    gTransitionTileState = TRANS_TILE_OFF;
                    GameState_SetFramerateDivisor(&this->state, 3);
                    this->transitionTrigger = TRANS_TRIGGER_OFF;
                    this->transitionMode = TRANS_MODE_OFF;
                }
            }
            break;

        case TRANS_MODE_CS_BLACK_FILL_INIT:
            sTransitionFillTimer = 0;
            this->envCtx.fillScreen = true;
            this->envCtx.screenFillColor[0] = 0;
            this->envCtx.screenFillColor[1] = 0;
            this->envCtx.screenFillColor[2] = 0;
            this->envCtx.screenFillColor[3] = 255;
            this->transitionMode = TRANS_MODE_CS_BLACK_FILL;
            break;

        case TRANS_MODE_CS_BLACK_FILL:
            if (gSaveContext.cutsceneTransitionControl != 0) {
                this->envCtx.screenFillColor[3] = gSaveContext.cutsceneTransitionControl;

                if (gSaveContext.cutsceneTransitionControl <= 100) {
                    gTransitionTileState = TRANS_TILE_OFF;
                    GameState_SetFramerateDivisor(&this->state, 3);
                    this->transitionTrigger = TRANS_TRIGGER_OFF;
                    this->transitionMode = TRANS_MODE_OFF;
                }
            }
            break;
    }
}

const char D_801DFA34[][4] = {
    "all", "a",  "a",  "b",  "b",  "c",  "c",  "d",   "d",   "e",  "e",  "f",  "fa", "fa", "fb", "fb",
    "fc",  "fc", "fd", "fd", "fe", "fe", "fg", "fg",  "fh",  "fh", "fi", "fi", "fj", "fj", "fk", "fk",
    "f",   "g",  "g",  "h",  "h",  "i",  "i",  "all", "all", "a",  "b",  "c",  "d",  "e",  "f",  "g",
    "h",   "i",  "f",  "fa", "fb", "fc", "fd", "fe",  "ff",  "fg", "fh", "fi", "fj", "fk",
};

// static s16 vert_pos_rand_list[32];
// static s16 tex_coord_rand_list[32];
// static u8 color_rand_list[32];

/* 256 entry list, each */
extern s16             gVertPosRandList[];
extern s16             gTexCoordRandList[];
extern u8              gColorRandList[];
u8                     vert_pos_rand_index = 0;
u8                     tex_coord_rand_index = 0;
u8                     color_rand_index = 0;

 
void Play_UpdateMain(PlayState* this) {
    // s32 pad;
    u32 index;
    void *room_segments[2];
    Input* input = this->state.input;
    Player *player = GET_PLAYER(this);
    Camera *camera = Play_GetCamera(this, CAM_ID_MAIN);
    s32 sp5C = false;
    u8 freezeFlashTimer;
    struct ChaosCode *code;

    // if(CHECK_BTN_ANY(input->press.button, BTN_L))
    // {
    //     Chaos_ActivateCode(CHAOS_CODE_WALLMASTER, 1);
    // }

    if(Chaos_GetConfigFlag(CHAOS_CONFIG_DPAD_DOWN_TO_DIE))
    {
        if(CHECK_BTN_ANY(input->cur.button, BTN_DDOWN) && !Play_IsChangingArea(this))
        {
            gChaosContext.link.dpad_down_timer++;

            if((gChaosContext.link.dpad_down_timer % 20) == 0)
            {
                Audio_PlaySfx(NA_SE_SY_WARNING_COUNT_E);
                Chaos_ConsolePrintf("Voiding out in %d", 6 - (gChaosContext.link.dpad_down_timer / 20));
            }
            
            if(gChaosContext.link.dpad_down_timer >= 100)
            {
                func_80169FDC(this);
            }
        }
        else
        {
            gChaosContext.link.dpad_down_timer = 0;
        }
    }

    if(this->pauseCtx.state == PAUSE_STATE_OFF)
    {
        if(Chaos_IsCodeActive(CHAOS_CODE_LOVELESS_MARRIAGE))
        {
            u32 type_chance = Chaos_RandS16Offset(0, 24);
            u32 type = LIKE_LIKE_TYPE_CHAOS;

            switch(type_chance)
            {
                // case 0:
                //     type = LIKE_LIKE_TYPE_LONG_RANGE;
                // break;

                case 1:
                    type = LIKE_LIKE_TYPE_SMOL;
                break;

                case 2:
                    type = LIKE_LIKE_TYPE_TURBO;
                break;

                case 3:
                    type = LIKE_LIKE_TYPE_VORE;
                break;

                default:
                    type = LIKE_LIKE_TYPE_CHAOS;
                break;
            }

            Chaos_SpawnActor(&this->actorCtx, this, ACTOR_EN_RR, 
                player->actor.world.pos.x, player->actor.world.pos.y + 20.0f, player->actor.world.pos.z,
                0, 0, 0, type);
            Chaos_DeactivateCode(CHAOS_CODE_LOVELESS_MARRIAGE);
        }

        if(Chaos_IsCodeActive(CHAOS_CODE_STARFOX))
        {
            Chaos_SpawnActor(&this->actorCtx, this, ACTOR_EN_ARWING, 
                player->actor.world.pos.x, player->actor.world.pos.y + 20.0f, player->actor.world.pos.z, 0, 0, 0, 0);
            Chaos_DeactivateCode(CHAOS_CODE_STARFOX);
        }

        if(Chaos_IsCodeActive(CHAOS_CODE_WALLMASTER))
        {
            Chaos_SpawnActor(&this->actorCtx, this, ACTOR_EN_WALLMAS, 
                player->actor.world.pos.x, player->actor.world.pos.y + 20.0f, player->actor.world.pos.z, 
                0, 0, 0, WALLMASTER_PARAMS((Chaos_RandS16Offset(0, 8) != 0) ? WALLMASTER_TYPE_FAKE : 0, 0, false));
            Chaos_DeactivateCode(CHAOS_CODE_WALLMASTER);
        }

        if(Chaos_IsCodeActive(CHAOS_CODE_REDEADASS_GROOVE))
        {
            Vec3f player_pos = player->actor.world.pos;
            player_pos.y += 20.0f;
            Chaos_SpawnRedeadDanceParty( &this->actorCtx, this, &player_pos);
            Chaos_DeactivateCode(CHAOS_CODE_REDEADASS_GROOVE);
        }

        if(Chaos_IsCodeActive(CHAOS_CODE_CHICKEN_ARISE))
        {
            if(gChaosContext.chicken.cucco.attackNiwSpawnTimer > 0)
            {
                gChaosContext.chicken.cucco.attackNiwSpawnTimer--;
            }

            if(gChaosContext.chicken.cucco.unkAttackNiwTimer > 0)
            {
                gChaosContext.chicken.cucco.unkAttackNiwTimer--;
            }

            if ((gChaosContext.chicken.cucco.attackNiwSpawnTimer == 0) && (gChaosContext.chicken.cucco.attackNiwCount < 7)) 
            {
                f32 xView = this->view.at.x - this->view.eye.x;
                f32 yView = this->view.at.y - this->view.eye.y;
                f32 zView = this->view.at.z - this->view.eye.z;
                f32 cucco_x = this->view.eye.x + ((Chaos_ZeroOne() - 0.5f) * xView);
                f32 cucco_y = this->view.eye.y + 50.0f + (yView * 0.5f) + Rand_CenteredFloat(0.3f);
                f32 cucco_z = this->view.eye.z + ((Chaos_ZeroOne() - 0.5f) * zView);

                Actor *cucco = Chaos_SpawnAsChild(&this->actorCtx, &gChaosContext.chicken.cucco.actor, this, ACTOR_EN_ATTACK_NIW, cucco_x,
                                            cucco_y, cucco_z, 0, 0, 0, ATTACK_NIW_CHAOS);    

                if (cucco != NULL) {
                    gChaosContext.chicken.cucco.attackNiwCount++;
                    gChaosContext.chicken.cucco.attackNiwSpawnTimer = 10;
                    if(Chaos_RandS16Offset(0, 11) == 0)
                    {
                        Actor_SetScale(cucco, 0.006f);
                    }
                    else
                    {
                        Actor_SetScale(cucco, 0.018f);
                    }
                }
            }
        }

        if(Chaos_IsCodeActive(CHAOS_CODE_FISH))
        {
            Actor *fish = Actor_Spawn(&this->actorCtx, this, ACTOR_EN_FISH, 
                player->actor.world.pos.x, player->actor.world.pos.y, player->actor.world.pos.z, 0, 0, 0, 0);

            fish->scale.x *= 10.0f;
            fish->scale.y *= 10.0f;
            fish->scale.z *= 10.0f;
            Chaos_DeactivateCode(CHAOS_CODE_FISH);
        }
        
        code = Chaos_GetCode(CHAOS_CODE_SILENT_FIELD);
        if(code != NULL)
        {
            if(gChaosContext.env.fog_lerp < 1.0f)
            {
                gChaosContext.env.fog_lerp += 0.005f;

                if(gChaosContext.env.fog_lerp > 1.0f)
                {
                    gChaosContext.env.fog_lerp = 1.0f;
                }
            }
            
            if(gChaosContext.env.fog_lerp >= 0.5f)
            {
                if(gChaosContext.env.stalchild_spawn_timer > 0)
                {
                    gChaosContext.env.stalchild_spawn_timer--;
                }

                if(gChaosContext.env.stalchild_spawn_timer == 0 && gChaosContext.env.stalchild_count < CHAOS_MAX_STALCHILDS &&
                    code->timer < 15)
                {
                    Vec3f point_a = player->actor.world.pos;
                    Vec3f point_b = point_a;
                    Vec3f spawn_pos;
                    // s16 yaw = Rand_Next() % 0x10000;
                    s16 yaw = Chaos_RandNext() % 0x10000;
                    CollisionPoly *poly;

                    point_a.x += Math_SinS(yaw) * 1000;
                    point_a.y += 30.0f;
                    point_a.z += Math_CosS(yaw) * 1000;

                    point_b.y += 30.0f;

                    if(!BgCheck_AnyLineTest1(&this->colCtx, &point_a, &point_b, &spawn_pos, &poly, false))
                    {
                        spawn_pos = point_a;
                    }

                    Chaos_SpawnActor(&this->actorCtx, this, ACTOR_EN_SKB, spawn_pos.x, spawn_pos.y, spawn_pos.z, 0, 0, 0, EN_SKB_TYPE_CHAOS);

                    gChaosContext.env.stalchild_count++;
                    gChaosContext.env.stalchild_spawn_timer = 30;
                }
            }
        }
        else if(gChaosContext.env.fog_lerp > 0.0f)
        {
            gChaosContext.env.fog_lerp -= 0.01f;

            if(gChaosContext.env.fog_lerp < 0.0f)
            {
                gChaosContext.env.fog_lerp = 0.0f;
            }
        }

        Chaos_UpdateSimonSays(this, input);
    }

    // if(Chaos_IsCodeActive(CHAOS_CODE_TEXTBOX))
    // {
    //     s16 entry_index = 0x09E5;
    //     // MessageTableEntry *entry = D_801C6B98 + entry_index;
    //     Message_StartTextbox(this, MESSAGE_ID_CONFIRM_SONG_OF_TIME_NORMAL, &player->actor);
    //     // CutsceneManager_Queue(CS_ID_GLOBAL_TALK);
    //     Chaos_DeactivateCode(CHAOS_CODE_TEXTBOX);
    // }

    // if(this->msgCtx.currentTextId == MESSAGE_ID_CONFIRM_SONG_OF_TIME_NORMAL)
    // {
    //     if(CHECK_BTN_ANY(input->press.button, BTN_A))
    //     {
    //         Message_CloseTextbox(this);
    //     }
    // }

    if(Chaos_IsCodeActive(CHAOS_CODE_WEIRD_UI))
    {
        u32 heart_index;
        u32 heart_count = gSaveContext.save.saveInfo.playerData.healthCapacity / LIFEMETER_FULL_HEART_HEALTH;
        for(heart_index = 0; heart_index < heart_count; heart_index++)
        {
            gChaosContext.ui.heart_containers[heart_index].pos_x = (Rand_ZeroOne() * 2.0f - 1.0f) * 8.0f;
            gChaosContext.ui.heart_containers[heart_index].pos_y = (Rand_ZeroOne() * 2.0f - 1.0f) * 8.0f;
        }
    }

    Chaos_UpdateSnakeGame(this, input);

    if(Chaos_IsCodeActive(CHAOS_CODE_FAST_TIME))
    {
        if(gChaosContext.time.fast_time_state == CHAOS_FAST_TIME_STATE_NONE)
        {
            gChaosContext.time.fast_time_state = CHAOS_FAST_TIME_STATE_SPEEDING_UP;
            Audio_PlaySfx(NA_SE_SY_STOPWATCH_TIMER_3);
        }

        Chaos_DeactivateCode(CHAOS_CODE_FAST_TIME);
    }

    // if(Chaos_IsCodeActive(CHAOS_CODE_LUCKY))
    // {
    //     // Environment_AddLightningBolts(this, 3, true);
    //     Chaos_GenerateThunderbolt(this);
    //     Chaos_DeactivateCode(CHAOS_CODE_LUCKY);
    // }

    if(gChaosContext.time.fast_time_state == CHAOS_FAST_TIME_STATE_SPEEDING_UP)
    {
        gSaveContext.save.timeSpeedOffset++;
        if(gSaveContext.save.timeSpeedOffset >= CHAOS_FAST_TIME_OFFSET)
        {
            gSaveContext.save.timeSpeedOffset = CHAOS_FAST_TIME_OFFSET;
            gChaosContext.time.fast_time_state = CHAOS_FAST_TIME_STATE_NONE;
        }
    }

    code = Chaos_GetCode(CHAOS_CODE_TERRIBLE_MUSIC);

    if(code != NULL)
    {
        // if(code->timer > 1)
        if(code->timer > 1)
        {
            gChaosContext.bgm.change_timer--;

            if(gChaosContext.bgm.change_timer == 0)
            {
                u16 frequency_duration = Chaos_RandS16Offset(5, 15);
                u16 tempo_duration = Chaos_RandS16Offset(5, 15);
                f32 scale;

                if(frequency_duration > tempo_duration)
                {
                    gChaosContext.bgm.change_timer = frequency_duration;
                }
                else
                {
                    gChaosContext.bgm.change_timer = tempo_duration;
                }

                scale = 0.25f + Chaos_ZeroOne() * 2.5f;
                SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, frequency_duration, scale * 1000.0f);
                SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_SUB, frequency_duration, scale * 1000.0f);
                scale = 0.25f + Chaos_ZeroOne() * 2.5f;
                SEQCMD_SET_TEMPO(SEQ_PLAYER_BGM_MAIN, tempo_duration, scale * 100.0f);
                SEQCMD_SET_TEMPO(SEQ_PLAYER_BGM_SUB, tempo_duration, scale * 100.0f);
            }
        }
        else
        {
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 10, 1000.0f);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_SUB, 10, 1000.0f);
            SEQCMD_RESET_TEMPO(SEQ_PLAYER_BGM_MAIN, 10);
            SEQCMD_RESET_TEMPO(SEQ_PLAYER_BGM_SUB, 10);
        }
    }

    if(Chaos_IsCodeActive(CHAOS_CODE_WEIRD_SKYBOX))
    {
        this->skyboxCtx.rot.x += Chaos_ZeroOne() * 0.25f;
        this->skyboxCtx.rot.y += Chaos_ZeroOne() * 0.25f;
        this->skyboxCtx.rot.z += Chaos_ZeroOne() * 0.25f;
    }

    gSegments[0x04] = OS_K0_TO_PHYSICAL(this->objectCtx.slots[this->objectCtx.mainKeepSlot].segment);
    gSegments[0x05] = OS_K0_TO_PHYSICAL(this->objectCtx.slots[this->objectCtx.subKeepSlot].segment);
    gSegments[0x02] = OS_K0_TO_PHYSICAL(this->sceneSegment);

    if (R_PICTO_PHOTO_STATE == PICTO_PHOTO_STATE_PROCESS) {
        R_PICTO_PHOTO_STATE = PICTO_PHOTO_STATE_READY;
        Sched_FlushTaskQueue();
        Play_TakePictoPhoto(&this->pauseBgPreRender);
        R_PICTO_PHOTO_STATE = PICTO_PHOTO_STATE_OFF;
    }

    Actor_SetMovementScale(this->state.framerateDivisor);

    if (FrameAdvance_Update(&this->frameAdvCtx, &input[1])) {
        if ((this->transitionMode == TRANS_MODE_OFF) && (this->transitionTrigger != TRANS_TRIGGER_OFF)) {
            this->transitionMode = TRANS_MODE_SETUP;
        }

        if (gTransitionTileState != TRANS_TILE_OFF) {
            switch (gTransitionTileState) {
                case TRANS_TILE_PROCESS:
                    if (TransitionTile_Init(&sTransitionTile, 10, 7) == NULL) {
                        gTransitionTileState = TRANS_TILE_OFF;
                    } else {
                        sTransitionTile.zBuffer = gZBufferPtr;
                        gTransitionTileState = TRANS_TILE_READY;
                        GameState_SetFramerateDivisor(&this->state, 1);
                    }
                    break;

                case TRANS_TILE_READY:
                    TransitionTile_Update(&sTransitionTile);
                    break;

                default:
                    break;
            }
        }

        Play_UpdateTransition(this);

        if (gTransitionTileState != TRANS_TILE_READY) {
            if ((gSaveContext.gameMode == GAMEMODE_NORMAL) &&
                (((this->msgCtx.msgMode == MSGMODE_NONE)) ||
                 ((this->msgCtx.currentTextId == 0xFF) && (this->msgCtx.msgMode == MSGMODE_TEXT_DONE) &&
                  (this->msgCtx.textboxEndType == TEXTBOX_ENDTYPE_PAUSE_MENU)) ||
                 ((this->msgCtx.currentTextId >= 0x100) && (this->msgCtx.currentTextId <= 0x200))) &&
                (this->gameOverCtx.state == GAMEOVER_INACTIVE)) {
                KaleidoSetup_Update(this);
            }

            sp5C = IS_PAUSED(&this->pauseCtx);

            AnimTaskQueue_Reset(&this->animTaskQueue);
            Object_UpdateEntries(&this->objectCtx);

            if (!sp5C && (IREG(72) == 0)) {
                this->gameplayFrames++;
                Rumble_SetUpdateEnabled(true);

                if ((this->actorCtx.freezeFlashTimer != 0) && (this->actorCtx.freezeFlashTimer-- < 5)) {
                    freezeFlashTimer = this->actorCtx.freezeFlashTimer;
                    if ((freezeFlashTimer > 0) && ((freezeFlashTimer % 2) != 0)) {
                        this->envCtx.fillScreen = true;
                        this->envCtx.screenFillColor[0] = this->envCtx.screenFillColor[1] =
                            this->envCtx.screenFillColor[2] = 150;
                        this->envCtx.screenFillColor[3] = 80;
                    } else {
                        this->envCtx.fillScreen = false;
                    }
                } else {
                    Room_ProcessRoomRequest(this, &this->roomCtx);
                    CollisionCheck_AT(this, &this->colChkCtx);
                    CollisionCheck_OC(this, &this->colChkCtx);
                    CollisionCheck_Damage(this, &this->colChkCtx);
                    CollisionCheck_ClearContext(this, &this->colChkCtx);


                    if (!this->haltAllActors) 
                    {
                        Actor_UpdateAll(this, &this->actorCtx);
                    }
                    Cutscene_UpdateManual(this, &this->csCtx);
                    Cutscene_UpdateScripted(this, &this->csCtx);
                    Effect_UpdateAll(this);
                    EffectSS_UpdateAllParticles(this);
                    EffFootmark_Update(this);
                }
            } else {
                Rumble_SetUpdateEnabled(false);
            }

            Room_Noop(this, &this->roomCtx.curRoom, &input[1], 0);
            Room_Noop(this, &this->roomCtx.prevRoom, &input[1], 1);
            Skybox_Update(&this->skyboxCtx);

            if (IS_PAUSED(&this->pauseCtx)) {
                KaleidoScopeCall_Update(this);
            } else if (this->gameOverCtx.state != GAMEOVER_INACTIVE) {
                GameOver_Update(this);
            }

            Message_Update(this);
            Interface_Update(this);
            AnimTaskQueue_Update(this, &this->animTaskQueue);
            SoundSource_UpdateAll(this);
            ShrinkWindow_Update(this->state.framerateDivisor);
            TransitionFade_Update(&this->unk_18E48, this->state.framerateDivisor);
        }
    }

    if(Chaos_IsCodeActive(CHAOS_CODE_BAD_CONNECTION))
    {
        switch(gChaosContext.link.bad_connection_mode)
        {
            case CHAOS_BAD_CONNECTION_ROLLBACK:

                if(player->maskObjectLoadState != 0)
                {
                    /* only do snapshot stuff if there's no mask dma in progress */
                    break;
                }

                if(gChaosContext.link.snapshot_timer > 0)
                {
                    gChaosContext.link.snapshot_timer--;
                }
                
                if(gChaosContext.link.snapshot_timer == 0 || player->actor.room != gChaosContext.link.player_snapshot.actor.room ||
                    player->actor.child != NULL && gChaosContext.link.player_snapshot.actor.child == NULL ||
                    player->actor.parent != NULL && gChaosContext.link.player_snapshot.actor.parent == NULL ||
                    player->doorActor != NULL && gChaosContext.link.door_snapshot.instance == NULL)
                {
                    Lib_MemCpy(&gChaosContext.link.player_snapshot, player, sizeof(Player));
                    Lib_MemCpy(gChaosContext.link.ammo, gSaveContext.save.saveInfo.inventory.ammo, sizeof(gChaosContext.link.ammo));
                    gChaosContext.link.magic_state = gSaveContext.magicState;
                    gChaosContext.link.magic_available = gSaveContext.save.saveInfo.playerData.magic;
                    gChaosContext.link.health = gSaveContext.save.saveInfo.playerData.health;
                    gChaosContext.link.snapshot_timer = Chaos_RandS16Offset(15, 65);
                    gChaosContext.link.bad_connection_timer = Chaos_RandS16Offset(2, 45);
                    // gChaosContext.link.snapshot_timer = 200;
                    // gChaosContext.link.bad_connection_timer = 20;

                    if(player->actor.parent != NULL)
                    {
                        Chaos_SnapshotParent(this, player->actor.parent);
                    }
                    else if(gChaosContext.link.parent_snapshot.instance != NULL)
                    {
                        Chaos_UnsnapshotParent(this, player->actor.parent);
                    }

                    if(player->actor.child != NULL)
                    {
                        Chaos_SnapshotChild(this, player->actor.child);

                        if(player->actor.child->id == ACTOR_EN_ARROW && player->actor.child->child != NULL)
                        {
                            /* when we reach here the child actor update functin will already have been called. In
                            the case of a magic arrow, the magic effect actor will already be spawned, so just snapshot */
                            Chaos_SnapshotMagicArrow(this, player->actor.child->child);
                        }
                        else if(gChaosContext.link.arrow_snapshot.instance != NULL)
                        {
                            /* not a magic arrow and the previous snapshot contains a magic arrow effect actor, 
                            so drop it */
                            Chaos_UnsnapshotMagicArrow(this, gChaosContext.link.arrow_snapshot.instance);
                        }
                    }
                    else if(gChaosContext.link.child_snapshot.instance != NULL)
                    {
                        /* player has no child actor but previous snapshot contains a child actor, 
                        so drop it */
                        Chaos_UnsnapshotChild(this, gChaosContext.link.child_snapshot.instance);

                        if(gChaosContext.link.arrow_snapshot.instance != NULL)
                        {
                            /* previous snapshot contains a magic arrow effect actor, so drop it */
                            Chaos_UnsnapshotMagicArrow(this, gChaosContext.link.arrow_snapshot.instance);
                        }
                    }

                    if(player->doorActor != NULL)
                    {
                        Chaos_SnapshotDoor(this, player->doorActor);
                    }
                    else if(gChaosContext.link.door_snapshot.instance != NULL)
                    {
                        Chaos_UnsnapshotDoor(this, gChaosContext.link.door_snapshot.instance);
                    }
                }

                if(gChaosContext.link.bad_connection_timer > 0)
                {
                    gChaosContext.link.bad_connection_timer--;
                }

                if(gChaosContext.link.bad_connection_timer == 0)
                {
                    s8 current_loaded_mask = player->maskId;

                    if(player->actor.child != NULL && player->actor.child != gChaosContext.link.player_snapshot.actor.child)
                    {
                        /* current child actor is different from what's in the snapshot, so kill it */
                        Actor_Kill(player->actor.child);
                        player->actor.child = NULL;
                    }

                    if(player->actor.child == NULL && gChaosContext.link.player_snapshot.actor.child != NULL)
                    {
                        /* player currently doesn't have a child actor but the snapshot has */
                        Actor *child = gChaosContext.link.player_snapshot.actor.child;

                        if(gChaosContext.link.child_snapshot.instance == NULL)
                        {
                            /* child actor got killed since the snapshot, so spawn it */
                            child = Actor_SpawnAsChild(&this->actorCtx, &player->actor, this, 
                                gChaosContext.link.child_snapshot.actor.id, 0, 0, 0, 0, 0, 0, 
                                gChaosContext.link.child_snapshot.actor.params);

                            gChaosContext.link.player_snapshot.heldActor = child;
                            gChaosContext.link.player_snapshot.actor.child = child;
                            gChaosContext.link.child_snapshot.instance = child;

                            if(child->id == ACTOR_EN_ARROW && gChaosContext.link.child_snapshot.actor.child != NULL)
                            {
                                /* we're spawning a magic arrow, so run its update function once so it spawns the magic
                                effect actor */
                                child->update(child, this);
                                gChaosContext.link.child_snapshot.actor.child = child->child;
                                gChaosContext.link.arrow_snapshot.actor.parent = child;
                                // gChaosContext.link.arrow_snapshot.actor.next = child->child->next;
                                // gChaosContext.link.arrow_snapshot.actor.prev = child->child->prev;
                            }
                        }
                        // else if(child->id == ACTOR_EN_ARROW && gChaosContext.link.child_snapshot.actor.child != NULL &&
                        //     gChaosContext.link.arrow_snapshot.instance == NULL)
                        // {
                        //     gChaosContext.link.child_snapshot.actor.child = NULL;
                        // }

                        // gChaosContext.link.child_snapshot.actor.next = child->next;
                        // gChaosContext.link.child_snapshot.actor.prev = child->prev;
                    }

                    if(player->actor.parent == NULL && gChaosContext.link.player_snapshot.actor.parent != NULL &&
                        gChaosContext.link.parent_snapshot.instance == NULL)
                    {
                        /* parent actor got killed since the snapshot, so spawn it */
                        Actor *parent = Actor_Spawn(&this->actorCtx, this, gChaosContext.link.parent_snapshot.actor.id, 0, 0, 0, 0, 0, 0, 
                            gChaosContext.link.parent_snapshot.actor.params);
                        gChaosContext.link.player_snapshot.actor.parent = parent;
                        gChaosContext.link.parent_snapshot.actor.child = &player->actor;
                    }

                    Lib_MemCpy(player, &gChaosContext.link.player_snapshot, sizeof(Player));

                    if(player->currentMask != PLAYER_MASK_NONE && player->maskId != current_loaded_mask)
                    {
                        player->maskId = current_loaded_mask;
                    }

                    Lib_MemCpy(gSaveContext.save.saveInfo.inventory.ammo, gChaosContext.link.ammo, sizeof(gChaosContext.link.ammo));
                    // gSaveContext.magicState = gChaosContext.link.magic_state;
                    gSaveContext.save.saveInfo.playerData.magic = gChaosContext.link.magic_available;
                    gSaveContext.save.saveInfo.playerData.health = gChaosContext.link.health;
                    gChaosContext.link.bad_connection_timer = Chaos_RandS16Offset(2, 45);

                    if(player->actor.parent != NULL)
                    {
                        Actor *parent = player->actor.parent;
                        ActorProfile *init_info = Actor_GetActorInit(&this->actorCtx, parent->id);

                        /* make sure we're not restoring stale stuff */
                        gChaosContext.link.parent_snapshot.actor.next = parent->next;
                        gChaosContext.link.parent_snapshot.actor.prev = parent->prev;
                        gChaosContext.link.parent_snapshot.actor.room = parent->room;

                        Lib_MemCpy(parent, &gChaosContext.link.parent_snapshot.actor, init_info->instanceSize);
                    }

                    if(player->actor.child != NULL)
                    {
                        Actor *child = player->actor.child;
                        ActorProfile *init_info = Actor_GetActorInit(&this->actorCtx, child->id);

                        /* make sure we're not restoring stale stuff */
                        gChaosContext.link.child_snapshot.actor.next = child->next;
                        gChaosContext.link.child_snapshot.actor.prev = child->prev;
                        gChaosContext.link.child_snapshot.actor.room = child->room;

                        Lib_MemCpy(child, &gChaosContext.link.child_snapshot.actor, init_info->instanceSize);

                        if(child->id == ACTOR_EN_ARROW && gChaosContext.link.child_snapshot.actor.child != NULL)
                        {
                            /* we're restoring a magic arrow */

                            if(gChaosContext.link.arrow_snapshot.instance == NULL)
                            {
                                /* magic effect got killed since the snapshot but the arrow wasn't, so run the 
                                arrow update function to respawn it */
                                child->child = NULL;
                                child->update(child, this);
                                gChaosContext.link.child_snapshot.actor.child = child->child;
                                gChaosContext.link.arrow_snapshot.actor.parent = child;
                            }

                            /* make sure we're not restoring stale pointers */
                            gChaosContext.link.arrow_snapshot.actor.next = child->child->next;
                            gChaosContext.link.arrow_snapshot.actor.prev = child->child->prev;
                            gChaosContext.link.arrow_snapshot.actor.room = child->child->room;

                            init_info = Actor_GetActorInit(&this->actorCtx, gChaosContext.link.arrow_snapshot.actor.id);
                            Lib_MemCpy(child->child, &gChaosContext.link.arrow_snapshot.actor, init_info->instanceSize);
                        }
                    }

                    if(player->doorActor != NULL)
                    {
                        Actor *door = player->doorActor;
                        ActorProfile *init_info = Actor_GetActorInit(&this->actorCtx, door->id);
                        gChaosContext.link.door_snapshot.actor.next = door->next;
                        gChaosContext.link.door_snapshot.actor.prev = door->prev;
                        Lib_MemCpy(door, &gChaosContext.link.door_snapshot.actor, init_info->instanceSize);
                    }
                }
            break;

            case CHAOS_BAD_CONNECTION_BUFFER:
                if(gChaosContext.link.bad_connection_timer > 0)
                {
                    gChaosContext.link.bad_connection_timer--;
                }
                if(gChaosContext.link.bad_connection_timer == 0)
                {
                    gChaosContext.link.input_frames += Chaos_RandS16Offset(2, 20);
                    gChaosContext.link.bad_connection_timer = Chaos_RandS16Offset(10, 25);
                }

                if(gChaosContext.link.input_frames > 0)
                {
                    gChaosContext.link.input_frames--;
                    gChaosContext.link.magic_available = gSaveContext.save.saveInfo.playerData.magic;
                }
                if(gChaosContext.link.input_frames == 0)
                {
                    gSaveContext.save.saveInfo.playerData.magic = gChaosContext.link.magic_available;
                    player->actor.freezeTimer = 2;
                }
            break;
        }
    }

    Chaos_UpdateChaos(this);

    if (!sp5C || gDbgCamEnabled) {
        s32 i;

        this->nextCamera = this->activeCamId;
        for (i = 0; i < NUM_CAMS; i++) {
            if ((i != this->nextCamera) && (this->cameraPtrs[i] != NULL)) {
                Camera_Update(this->cameraPtrs[i]);
            }
        }
        Camera_Update(this->cameraPtrs[this->nextCamera]);
    }

    if(Chaos_IsCodeActive(CHAOS_CODE_BEER_GOGGLES))
    {  
        if(gChaosContext.link.beer_goggles_state == CHAOS_BEER_GOGGLES_STATE_NONE)
        {
            gChaosContext.link.beer_goggles_state = CHAOS_BEER_GOGGLES_STATE_ACTIVE;
            SEQCMD_SET_TEMPO(SEQ_PLAYER_AMBIENCE, 40, 80);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_AMBIENCE, 40, 700);
            SEQCMD_SET_TEMPO(SEQ_PLAYER_BGM_MAIN, 40, 80);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 40, 700);
            SEQCMD_SET_TEMPO(SEQ_PLAYER_BGM_SUB, 40, 80);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_SUB, 40, 700);
            SEQCMD_SET_TEMPO(SEQ_PLAYER_FANFARE, 40, 80);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_FANFARE, 40, 700);
            SEQCMD_SET_TEMPO(SEQ_PLAYER_SFX, 40, 80);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_SFX, 40, 700);
        }
        else if(gActiveSeqs[SEQ_PLAYER_BGM_MAIN].tempoTimer == 0)
        {
            SEQCMD_SET_TEMPO(SEQ_PLAYER_AMBIENCE, 0, 80);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_AMBIENCE, 0, 700);
            SEQCMD_SET_TEMPO(SEQ_PLAYER_BGM_MAIN, 0, 80);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 0, 700);
            SEQCMD_SET_TEMPO(SEQ_PLAYER_BGM_SUB, 0, 80);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_SUB, 0, 700);
            SEQCMD_SET_TEMPO(SEQ_PLAYER_FANFARE, 0, 80);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_FANFARE, 0, 700);
            SEQCMD_SET_TEMPO(SEQ_PLAYER_SFX, 0, 80);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_SFX, 0, 700);
        }

        if(gChaosContext.link.beer_alpha < CHAOS_MAX_BEER_ALPHA)
        {
            gChaosContext.link.beer_alpha += 5;
        }

        gChaosContext.view.beer_pitch = fmodf(gChaosContext.view.beer_pitch + 0.0915f, M_PI * 2.0f);
        gChaosContext.view.beer_yaw = fmodf(gChaosContext.view.beer_yaw + 0.1593f, M_PI * 2.0f);
        gChaosContext.view.beer_roll = fmodf(gChaosContext.view.beer_roll + 0.0293f, M_PI * 2.0f);
        gChaosContext.view.beer_x_offset = fmodf(gChaosContext.view.beer_x_offset + 0.061f, M_PI * 2.0f);
        gChaosContext.view.beer_y_offset = fmodf(gChaosContext.view.beer_y_offset + 0.0950f, M_PI * 2.0f);
        gChaosContext.view.beer_time += 0.1f;
    }
    else if(gChaosContext.link.beer_alpha > 0)
    {
        if(gChaosContext.link.beer_goggles_state == CHAOS_BEER_GOGGLES_STATE_ACTIVE)
        {
            gChaosContext.link.beer_goggles_state = CHAOS_BEER_GOGGLES_STATE_NONE;
            SEQCMD_RESET_TEMPO(SEQ_PLAYER_AMBIENCE, 20);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_AMBIENCE, 20, 1000.0f);
            SEQCMD_RESET_TEMPO(SEQ_PLAYER_BGM_MAIN, 20);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, 20, 1000.0f);
            SEQCMD_RESET_TEMPO(SEQ_PLAYER_BGM_SUB, 20);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_SUB, 20, 1000.0f);
            SEQCMD_RESET_TEMPO(SEQ_PLAYER_FANFARE, 20);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_FANFARE, 20, 1000.0f);
            SEQCMD_RESET_TEMPO(SEQ_PLAYER_SFX, 20);
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_SFX, 20, 1000.0f);
        }
        gChaosContext.link.beer_alpha -= 5;
        gChaosContext.view.beer_sway.x *= 0.9f;
        gChaosContext.view.beer_sway.y *= 0.9f;
        gChaosContext.view.beer_sway.z *= 0.9f;
    }

    if(gChaosContext.link.beer_alpha > 0)
    {
        Camera *camera = &this->mainCamera;
        Vec3f forward_vec;
        Vec3f right_vec;
        Vec3f sway_offset;
        Vec3f pitch_yaw;
        f32 offset_x;
        f32 offset_y;
        f32 alpha_scale = (f32)gChaosContext.link.beer_alpha / (float)CHAOS_MAX_BEER_ALPHA;
        f32 periodic_probability_scale = gChaosContext.periodic_probability_scale;
        periodic_probability_scale = periodic_probability_scale * periodic_probability_scale * periodic_probability_scale;
        periodic_probability_scale = CLAMP_MAX(periodic_probability_scale, 1.0);
        alpha_scale *= periodic_probability_scale;

        gSfxBeerGogglesFreq = 0.75f * alpha_scale + (1.0f - alpha_scale);

        if(Chaos_GetConfigFlag(CHAOS_CONFIG_BEER_GOGGLES_BLUR))
        {
            Play_EnableMotionBlurPriority(gChaosContext.link.beer_alpha * periodic_probability_scale);   
        }
        Math_Vec3f_DistXYZAndStoreNormDiff(&camera->eye, &camera->at, 1.0f, &forward_vec);

        right_vec.x = forward_vec.y * camera->up.z - forward_vec.z * camera->up.y;
        right_vec.y = forward_vec.x * camera->up.z - forward_vec.z * camera->up.x;
        right_vec.z = forward_vec.x * camera->up.y - forward_vec.y * camera->up.x;

        offset_y = Math_SinF(gChaosContext.view.beer_y_offset) * 15.0f * alpha_scale;
        offset_x = Math_SinF(gChaosContext.view.beer_x_offset) * 15.0f * alpha_scale;

        sway_offset.x = camera->up.x * offset_y + right_vec.x * offset_x;
        sway_offset.y = camera->up.y * offset_y + right_vec.y * offset_x;
        sway_offset.z = camera->up.z * offset_y + right_vec.z * offset_x;

        offset_y = Math_SinF(gChaosContext.view.beer_pitch) * 15.0f * alpha_scale;
        offset_x = Math_SinF(gChaosContext.view.beer_yaw) * 15.0f * alpha_scale;

        pitch_yaw.x = camera->up.x * offset_y + right_vec.x * offset_x;
        pitch_yaw.y = camera->up.y * offset_y + right_vec.y * offset_x;
        pitch_yaw.z = camera->up.z * offset_y + right_vec.z * offset_x;        

        if(camera->mode == CAM_MODE_FIRSTPERSON || (player->stateFlags1 & PLAYER_STATE1_100000))
        {
            player->actor.focus.rot.x += offset_y * 3;
            player->actor.focus.rot.y += offset_x * 3;
            gChaosContext.view.beer_sway.x = sway_offset.x * 0.025f;
            gChaosContext.view.beer_sway.y = sway_offset.y * 0.025f;
            gChaosContext.view.beer_sway.z = sway_offset.z * 0.025f;
        }
        else
        {
            gChaosContext.view.beer_sway = sway_offset;
            camera->at.x += pitch_yaw.x * 0.035f;
            camera->at.y += pitch_yaw.y * 0.035f;
            camera->at.z += pitch_yaw.z * 0.035f;
            camera->roll += Math_SinF(gChaosContext.view.beer_roll) * 350.0f * alpha_scale;
        }

        View_SetDistortionOrientation(&this->view, Math_SinF(gChaosContext.view.beer_time), Math_CosF(gChaosContext.view.beer_time * 1.57), 0.0f);
        View_SetDistortionScale(&this->view, 1.0f + alpha_scale * 0.5f, 
                                             1.0f + alpha_scale * 0.5f, 
                                             1.0f + (0.8f + Math_CosF(gChaosContext.view.beer_time * 0.25)) * alpha_scale * 0.1f);
        View_SetDistortionSpeed(&this->view, 0.05f);
    }
    else if(gChaosContext.link.beer_goggles_state != CHAOS_BEER_GOGGLES_STATE_NONE)
    {
        gChaosContext.link.beer_goggles_state = CHAOS_BEER_GOGGLES_STATE_NONE;
        if(Chaos_GetConfigFlag(CHAOS_CONFIG_BEER_GOGGLES_BLUR))
        {
            Play_DisableMotionBlurPriority();
        }
        View_ClearDistortion(&this->view);
        gChaosContext.view.beer_sway.x = 0;
        gChaosContext.view.beer_sway.y = 0;
        gChaosContext.view.beer_sway.z = 0;
        gChaosContext.view.beer_time = 0;
        gSfxBeerGogglesFreq = 1.0f;
    }

    if(Chaos_IsCodeActive(CHAOS_CODE_SCALE_RANDOM_LIMB))
    {
        u32 limb_index = Chaos_RandS16Offset(PLAYER_LIMB_ROOT + 1, PLAYER_LIMB_MAX - 1);

        if(Chaos_RandS16Offset(0, 16) < 11)
        {
            gChaosContext.link.limb_scales[limb_index] += 0.1f;
        }
        else
        {
            gChaosContext.link.limb_scales[limb_index] -= 0.1f;
        }

        if(gChaosContext.link.limb_scales[limb_index] > 4.0f)
        {
            gChaosContext.link.limb_scales[limb_index] = 4.0f;
        }
        else if(gChaosContext.link.limb_scales[limb_index] < 0.1f)
        {
            gChaosContext.link.limb_scales[limb_index] = 0.1f;
        }

        Chaos_DeactivateCode(CHAOS_CODE_SCALE_RANDOM_LIMB);
    }

    if(Chaos_IsCodeActive(CHAOS_CODE_SWAP_LIMBS))
    {
        u32 swap_limb_count = Chaos_RandS16Offset(1, 5);

        while(swap_limb_count > 0)
        {
            u32 src_limb_index = Chaos_RandS16Offset(PLAYER_LIMB_ROOT + 1, PLAYER_LIMB_MAX - 1);
            u32 dst_limb_index = src_limb_index;
            u32 temp;
            while(dst_limb_index == src_limb_index)
            {
                dst_limb_index = Chaos_RandS16Offset(PLAYER_LIMB_ROOT + 1, PLAYER_LIMB_MAX - 1);
            }

            temp = gChaosContext.link.limb_map[dst_limb_index];
            gChaosContext.link.limb_map[dst_limb_index] = gChaosContext.link.limb_map[src_limb_index];
            gChaosContext.link.limb_map[src_limb_index] = temp;
            swap_limb_count--;
        }

        Chaos_DeactivateCode(CHAOS_CODE_SWAP_LIMBS);
    }

    if(Chaos_IsCodeActive(CHAOS_CODE_UNSTABLE_ROOMS))
    {
        u8 snap_to_player_timer = 0;
        room_segments[this->roomCtx.activeBufPage] = this->roomCtx.curRoom.segment;
        room_segments[this->roomCtx.activeBufPage ^ 1] = this->roomCtx.prevRoom.segment;

        if(gChaosContext.room.weirdness_behavior & CHAOS_UNSTABLE_ROOMS_BEHAVIOR_ROTATE)
        {
            if(gChaosContext.room.room_rotation_timer > 0)
            {
                gChaosContext.room.room_rotation_timer--;
            }

            if(gChaosContext.room.room_rotation_timer == 0)
            {
                gChaosContext.room.room_rotation_timer = Chaos_RandS16Offset(0, 30);
                gChaosContext.room.room_rotation.x = Chaos_RandS16Offset(-1000, 2000);
                gChaosContext.room.room_rotation.y = Chaos_RandS16Offset(-400, 800);
                gChaosContext.room.room_rotation.z = Chaos_RandS16Offset(-1000, 2000);
            }
        }

        for(index = 0; index < 2; index++)
        {
            if(room_segments[index] != NULL && gChaosContext.room.vert_list_list[index] != NULL /* && player->csId != CS_ID_GLOBAL_DOOR */)
            {
                u32 vert_list_index;
                RoomVertListList *vert_list_list;
                gSegments[0x03] = OS_K0_TO_PHYSICAL(room_segments[index]);
                vert_list_list = SEGMENTED_TO_K0(gChaosContext.room.vert_list_list[index]);

                if((gChaosContext.room.weirdness_behavior & CHAOS_UNSTABLE_ROOMS_BEHAVIOR_SNAP_TO_PLAYER) && (this->gameplayFrames % 4) == 0)
                {
                    u32 snap_index;
                    u32 snap_count = Chaos_RandS16Offset(1, 7);
                    for(snap_index = 0; snap_index < snap_count; snap_index++)
                    {
                        u32 vert_list_index = Chaos_RandNext() % vert_list_list->count;
                        Vec3f random_offset;
                        RoomVertList *room_vert_list = SEGMENTED_TO_K0(vert_list_list->room_vert_lists + vert_list_index);
                        Vtx *vertices = SEGMENTED_TO_K0(room_vert_list->verts);
                        u32 index = Chaos_RandNext() % room_vert_list->count;
                        Vtx *vtx = vertices + index;
                        random_offset.x = Rand_Centered() * 20.0f;
                        random_offset.y = Rand_ZeroOne() * 30.0f;
                        random_offset.z = Rand_Centered() * 20.0f;
                        vtx->v.ob[0] = player->actor.world.pos.x + random_offset.x;
                        vtx->v.ob[1] = player->actor.world.pos.y + random_offset.y;
                        vtx->v.ob[2] = player->actor.world.pos.z + random_offset.z;
                    }
                }

                for(vert_list_index = 0; vert_list_index < vert_list_list->count; vert_list_index++)
                {
                    RoomVertList *room_vert_list = SEGMENTED_TO_K0(vert_list_list->room_vert_lists + vert_list_index);
                    Vtx *vertices = SEGMENTED_TO_K0(room_vert_list->verts);
                    u32 vert_count = room_vert_list->count;
                    u32 rand_count = 0;

                    if(vert_count > 0)
                    {
                        if(gChaosContext.room.weirdness_behavior & CHAOS_UNSTABLE_ROOMS_BEHAVIOR_WOBBLE)
                        {
                            while(!(vert_count & 0x2000))
                            {
                                rand_count++;
                                vert_count <<= 1;
                            }

                            rand_count = 13 - rand_count;

                            if(rand_count > 1)
                            {
                                rand_count >>= 1;
                            }

                            while(rand_count > 0)
                            {
                                u32 index = Chaos_RandNext() % room_vert_list->count;
                                Vtx *vtx = vertices + index;
                                vtx->v.ob[0] += gVertPosRandList[vert_pos_rand_index++];
                                vtx->v.ob[1] += gVertPosRandList[vert_pos_rand_index++];
                                vtx->v.ob[2] += gVertPosRandList[vert_pos_rand_index++];

                                vtx->v.tc[0] += gTexCoordRandList[tex_coord_rand_index++];
                                vtx->v.tc[1] += gTexCoordRandList[tex_coord_rand_index++];
                                tex_coord_rand_index++;

                                vtx->v.cn[0] += gColorRandList[color_rand_index++];
                                vtx->v.cn[1] += gColorRandList[color_rand_index++];
                                vtx->v.cn[2] += gColorRandList[color_rand_index++];
                                rand_count--;
                            }
                        }
                    }
                }
            }
        }
    }

    if(Chaos_IsCodeActive(CHAOS_CODE_AIR_SUPPORT))
    {
        
    }
 
    if(Chaos_IsCodeActive(CHAOS_CODE_FAKE_CRASH))
    {
        Chaos_DeactivateCode(CHAOS_CODE_FAKE_CRASH);
        gChaosContext.fake_crash = true;
        gChaosContext.fake_crash_pointer = (u8 *)(Chaos_RandNext() % 0x7fffffff);
        *gChaosContext.fake_crash_pointer = false;
    }

    if(Chaos_IsCodeActive(CHAOS_CODE_BLIZZARD) && gChaosContext.env.blizzard_state == CHAOS_BLIZZARD_STATE_IDLE)
    {
        gChaosContext.env.blizzard_state = CHAOS_BLIZZARD_STATE_BLIZZARDING;
        gChaosContext.env.blizzard_timer = 0;
    }

    if(gChaosContext.env.blizzard_state == CHAOS_BLIZZARD_STATE_BLIZZARDING)
    {
        if(gChaosContext.env.wind_actor != NULL && gChaosContext.env.wind_actor->update != NULL)
        {
            EffectGoronBlizzard_SpawnParticles(&gChaosContext.env.blizzard);
        }
        else if(gChaosContext.env.blizzard_timer > 0)
        {
            gChaosContext.env.wind_actor = NULL;
            gChaosContext.env.blizzard_timer--;
        }

        if(gChaosContext.env.blizzard_timer == 0)
        {
            if(Chaos_IsCodeActive(CHAOS_CODE_BLIZZARD))
            {
                u16 wind_angle = Rand_Next();
                Vec3f wind_direction;
                Vec3f wind_pos;
                Vec3f particle_start_pos_offset;

                wind_direction.x = Math_SinS(wind_angle);
                wind_direction.y = 0.0f;
                wind_direction.z = Math_CosS(wind_angle);

                this->envCtx.windDirection.x = RAD_TO_BINANG(wind_direction.x);
                this->envCtx.windDirection.y = 0;
                this->envCtx.windDirection.z = RAD_TO_BINANG(wind_direction.z);

                particle_start_pos_offset.x = 0.0f;
                particle_start_pos_offset.y = 50.0f;
                particle_start_pos_offset.z = 0.0f;

                gChaosContext.env.blizzard.particle_start_velocity.x = 0.0f;
                gChaosContext.env.blizzard.particle_start_velocity.y = 6.0f;
                gChaosContext.env.blizzard.particle_start_velocity.x = 0.0f;

                gChaosContext.env.blizzard.particle_acceleration.x = -wind_direction.x * 80.0f;
                gChaosContext.env.blizzard.particle_acceleration.z = -wind_direction.z * 80.0f;
                gChaosContext.env.blizzard.particle_acceleration.y = -40.0f;
                gChaosContext.env.blizzard.particle_start_scale = 0.03f;
                gChaosContext.env.blizzard.particle_scale_increment = 0.04f;
                gChaosContext.env.blizzard.particle_min_life = 0x10;

                Math_Vec3f_SumScaled(&player->actor.world.pos, &wind_direction, 100.0f, &wind_pos);
                Actor_PlaySfx(&player->actor, NA_SE_EV_SNOWSTORM_HARD);
                gChaosContext.env.wind_actor = Actor_Spawn(&this->actorCtx, this, ACTOR_EN_WEATHER_TAG, wind_pos.x, wind_pos.y, wind_pos.z, 
                    0x1388, 0x708, 0x3E8, 0);

                Lib_Vec3f_TranslateAndRotateY(&wind_pos, wind_angle, &particle_start_pos_offset, &gChaosContext.env.blizzard.particle_start_position);
                gChaosContext.env.blizzard_timer = Chaos_RandS16Offset(5, 100);
            }
            else
            {
                gChaosContext.env.blizzard_state = CHAOS_BLIZZARD_STATE_IDLE;
            }
        }
        
        EffectGoronBlizzard_UpdateParticles(&gChaosContext.env.blizzard);
    }

    // if(Chaos_IsCodeActive(CHAOS_CODE_SCREEN_SLAYER))
    // {
    //     Chaos_DeactivateCode(CHAOS_CODE_SCREEN_SLAYER);
    //     gChaosContext.screen_slayer = CHAOS_SCREEN_SLAYER_STATE_WAIT_FOR_TRANSITION;;
    //     gChaosContext.fake_crash_pointer = (u8 *)(Chaos_RandNext() % 0x7fffffff);
    //     // *gChaosContext.fake_crash_pointer = false;
    // }

    // if(gChaosContext.screen_slayer == CHAOS_SCREEN_SLAYER_STATE_READY)
    // {
    //     *gChaosContext.fake_crash_pointer = CHAOS_SCREEN_SLAYER_STATE_IDLE;
    // }

    if (!sp5C) {
        Play_UpdateWaterCamera(this, this->cameraPtrs[this->nextCamera]);
        Distortion_Update();
    }

    Environment_Update(this, &this->envCtx, &this->lightCtx, &this->pauseCtx, &this->msgCtx, &this->gameOverCtx,
                       this->state.gfxCtx);

    if (this->sramCtx.status != 0) {
        if (gSaveContext.save.isOwlSave) {
            Sram_UpdateWriteToFlashOwlSave(&this->sramCtx);
        } else {
            Sram_UpdateWriteToFlashDefault(&this->sramCtx);
        }
    }

    if(!(gChaosContext.effect_restrictions & (CHAOS_CODE_RESTRICTION_FLAG_AFFECT_CUTSCENE |
                                            CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TRANSITION)))
    {
        if(gChaosContext.moon.moon_crash_timer > 0)
        {
            s32 time_until_moon_crash = TIME_UNTIL_MOON_CRASH - gChaosContext.moon.moon_crash_time_offset;

            gChaosContext.moon.moon_crash_timer--;

            if(time_until_moon_crash <= 0)
            {
                Interface_StartMoonCrash(this);
                gChaosContext.moon.moon_crash_timer = 0;
                gChaosContext.moon.moon_crash_time_offset = 0;
            }
        }
        else if(gChaosContext.moon.moon_crash_time_offset != 0)
        {
            // gChaosContext.moon.moon_crash_time_offset = 0;
            Chaos_ClearMoonCrash();
        }
    }
}

void Play_Update(PlayState* this) {
    if (!sBombersNotebookOpen) {
        if (this->pauseCtx.bombersNotebookOpen) {
            sBombersNotebookOpen = true;
            sBombersNotebook.loadState = BOMBERS_NOTEBOOK_LOAD_STATE_NONE;
        }
    } else if (CHECK_BTN_ALL(CONTROLLER1(&this->state)->press.button, BTN_L) ||
               CHECK_BTN_ALL(CONTROLLER1(&this->state)->press.button, BTN_B) ||
               CHECK_BTN_ALL(CONTROLLER1(&this->state)->press.button, BTN_START) ||
               (gIrqMgrResetStatus != IRQ_RESET_STATUS_IDLE)) {
        sBombersNotebookOpen = false;
        this->pauseCtx.bombersNotebookOpen = false;
        sBombersNotebook.loadState = BOMBERS_NOTEBOOK_LOAD_STATE_NONE;
        this->msgCtx.msgLength = 0;
        this->msgCtx.msgMode = MSGMODE_NONE;
        this->msgCtx.currentTextId = 0;
        this->msgCtx.stateTimer = 0;
        Audio_PlaySfx(NA_SE_SY_CANCEL);
    }
    if (sBombersNotebookOpen) {
        BombersNotebook_Update(this, &sBombersNotebook, this->state.input);
        Message_Update(this);
    } else {
        Play_UpdateMain(this);
    }
}

void Play_PostWorldDraw(PlayState* this) {
    if (IS_PAUSED(&this->pauseCtx)) {
        KaleidoScopeCall_Draw(this);
    }

    if (gSaveContext.gameMode == GAMEMODE_NORMAL) {
        Interface_Draw(this);
    }

    if (!IS_PAUSED(&this->pauseCtx) || (this->msgCtx.currentTextId != 0xFF)) {
        Message_Draw(this);
    }

    if (this->gameOverCtx.state != GAMEOVER_INACTIVE) {
        GameOver_FadeLights(this);
    }

    // Shrink the whole screen display (at the end of First and Second Day by default)
    if (gSaveContext.screenScaleFlag) {
        Gfx* gfx;
        Gfx* gfxHead;
        GraphicsContext* gfxCtx = this->state.gfxCtx;

        sPlayVisFbufInstance->scale = gSaveContext.screenScale / 1000.0f;

        OPEN_DISPS(gfxCtx);

        gfxHead = POLY_OPA_DISP;
        gfx = Gfx_Open(gfxHead);
        gSPDisplayList(OVERLAY_DISP++, gfx);

        VisFbuf_Draw(sPlayVisFbufInstance, &gfx, this->unk_18E60);

        gSPEndDisplayList(gfx++);
        Gfx_Close(gfxHead, gfx);
        POLY_OPA_DISP = gfx;

        CLOSE_DISPS(gfxCtx);
    }
}

void Play_DrawMain(PlayState* this) {
    GraphicsContext* gfxCtx = this->state.gfxCtx;
    Input inputs[MAXCONTROLLERS];
    Camera *camera = Play_GetCamera(this, CAM_ID_MAIN);
    Player *player = GET_PLAYER(this);
    Lights* lights;
    Vec3f temp;
    u8 sp25B = false;
    f32 zFar;

    if (R_PAUSE_BG_PRERENDER_STATE >= PAUSE_BG_PRERENDER_UNK4) {
        PreRender_ApplyFiltersSlowlyDestroy(&this->pauseBgPreRender);
        R_PAUSE_BG_PRERENDER_STATE = PAUSE_BG_PRERENDER_OFF;
    }

    if ((R_PAUSE_BG_PRERENDER_STATE <= PAUSE_BG_PRERENDER_SETUP) && (gTransitionTileState <= TRANS_TILE_SETUP)) {
        if (this->skyboxCtx.shouldDraw || (this->roomCtx.curRoom.roomShape->base.type == ROOM_SHAPE_TYPE_IMAGE)) {
            func_8012CF0C(gfxCtx, false, true, 0, 0, 0);
        } else {
            func_8012CF0C(gfxCtx, true, true, this->lightCtx.fogColor[0], this->lightCtx.fogColor[1],
                          this->lightCtx.fogColor[2]);
        }
    } else {
        func_8012CF0C(gfxCtx, false, false, 0, 0, 0);
    }

    OPEN_DISPS(gfxCtx);

    gSegments[0x04] = OS_K0_TO_PHYSICAL(this->objectCtx.slots[this->objectCtx.mainKeepSlot].segment);
    gSegments[0x05] = OS_K0_TO_PHYSICAL(this->objectCtx.slots[this->objectCtx.subKeepSlot].segment);
    gSegments[0x02] = OS_K0_TO_PHYSICAL(this->sceneSegment);

    gSPSegment(POLY_OPA_DISP++, 0x04, this->objectCtx.slots[this->objectCtx.mainKeepSlot].segment);
    gSPSegment(POLY_XLU_DISP++, 0x04, this->objectCtx.slots[this->objectCtx.mainKeepSlot].segment);
    gSPSegment(OVERLAY_DISP++, 0x04, this->objectCtx.slots[this->objectCtx.mainKeepSlot].segment);

    gSPSegment(POLY_OPA_DISP++, 0x05, this->objectCtx.slots[this->objectCtx.subKeepSlot].segment);
    gSPSegment(POLY_XLU_DISP++, 0x05, this->objectCtx.slots[this->objectCtx.subKeepSlot].segment);
    gSPSegment(OVERLAY_DISP++, 0x05, this->objectCtx.slots[this->objectCtx.subKeepSlot].segment);

    gSPSegment(POLY_OPA_DISP++, 0x02, this->sceneSegment);
    gSPSegment(POLY_XLU_DISP++, 0x02, this->sceneSegment);
    gSPSegment(OVERLAY_DISP++, 0x02, this->sceneSegment);

    if (1) {
        f32 scale_minus_one;
        f32 scale_factor = 0.3f;
        f32 camera_speed;
        f32 c_speed = 10.0f;
        Vec3f scale_axis = gChaosContext.view.camera_velocity;
        MtxF scale_transform;
        Mtx *view_scaling;

        // Math_Vec3f_Diff(&camera->eye, &gChaosContext.view.prev_camera_pos, &scale_axis);
        // camera_speed = Math3D_Normalize(&scale_axis);

        // scale_factor = sqrtf(1.0f - (camera_speed * camera_speed) / (c_speed * c_speed));
        // scale_factor = 2.0f - CLAMP_MIN(scale_factor, 0.0f);
        // scale_minus_one = scale_factor - 1.0f;

        // scale_transform.mf[0][0] = 1.0f + scale_minus_one * scale_axis.x * scale_axis.x;
        // scale_transform.mf[1][0] = scale_minus_one * scale_axis.x * scale_axis.y;
        // scale_transform.mf[2][0] = scale_minus_one * scale_axis.x * scale_axis.z;
        // scale_transform.mf[3][0] = 0.0f;

        // scale_transform.mf[0][1] = scale_minus_one * scale_axis.x * scale_axis.y;
        // scale_transform.mf[1][1] = 1.0f + scale_minus_one * scale_axis.y * scale_axis.y;
        // scale_transform.mf[2][1] = scale_minus_one * scale_axis.y * scale_axis.z;
        // scale_transform.mf[3][1] = 0.0f;

        // scale_transform.mf[0][2] = scale_minus_one * scale_axis.x * scale_axis.z;
        // scale_transform.mf[1][2] = scale_minus_one * scale_axis.y * scale_axis.z;
        // scale_transform.mf[2][2] = 1.0f + scale_minus_one * scale_axis.z * scale_axis.z;
        // scale_transform.mf[3][2] = 0.0f;
    
        // scale_transform.mf[0][3] = 0;
        // scale_transform.mf[1][3] = 0;
        // scale_transform.mf[2][3] = 0;
        // scale_transform.mf[3][3] = 1;

        ShrinkWindow_Draw(gfxCtx);

        POLY_OPA_DISP = Play_SetFog(this, POLY_OPA_DISP);
        POLY_XLU_DISP = Play_SetFog(this, POLY_XLU_DISP);

        zFar = this->lightCtx.zFar;
        if (zFar > 12800.0f) {
            zFar = 12800.0f;
        }

        View_SetPerspective(&this->view, this->view.fovy, this->view.zNear, zFar);

        View_Apply(&this->view, 0xF);

        // The billboard matrix temporarily stores the viewing matrix
        Matrix_MtxToMtxF(&this->view.viewing, &this->billboardMtxF);
        Matrix_MtxToMtxF(&this->view.projection, &this->viewProjectionMtxF);

        this->projectionMtxFDiagonal.x = this->viewProjectionMtxF.xx;
        this->projectionMtxFDiagonal.y = this->viewProjectionMtxF.yy;
        this->projectionMtxFDiagonal.z = -this->viewProjectionMtxF.zz;

        SkinMatrix_MtxFMtxFMult(&this->viewProjectionMtxF, &this->billboardMtxF, &this->viewProjectionMtxF);

        this->billboardMtxF.mf[3][2] = this->billboardMtxF.mf[3][1] = this->billboardMtxF.mf[3][0] =
            this->billboardMtxF.mf[2][3] = this->billboardMtxF.mf[1][3] = this->billboardMtxF.mf[0][3] = 0.0f;

        Matrix_Transpose(&this->billboardMtxF);

        this->billboardMtx = GRAPH_ALLOC(this->state.gfxCtx, 2 * sizeof(Mtx));

        Matrix_MtxFToMtx(&this->billboardMtxF, this->billboardMtx);
        Matrix_RotateYF(BINANG_TO_RAD((s16)(Camera_GetCamDirYaw(GET_ACTIVE_CAM(this)) + 0x8000)), MTXMODE_NEW);
        Matrix_ToMtx(this->billboardMtx + 1);

        if(Chaos_IsCodeActive(CHAOS_CODE_LENGTH_CONTRACTION))
        {
            gChaosContext.env.length_contraction_scale -= 0.023f;

            if(gChaosContext.env.length_contraction_scale < 0.25f)
            {
                gChaosContext.env.length_contraction_scale = 0.25f;
            }
        }
        else if(gChaosContext.env.length_contraction_scale < 1.0f)
        {
            gChaosContext.env.length_contraction_scale += 0.08f;
            if(gChaosContext.env.length_contraction_scale > 1.0f)
            {
                gChaosContext.env.length_contraction_scale = 1.0f;
            }
        }
 
        gSPSegment(POLY_OPA_DISP++, 0x01, this->billboardMtx);
        gSPSegment(POLY_XLU_DISP++, 0x01, this->billboardMtx);
        gSPSegment(OVERLAY_DISP++, 0x01, this->billboardMtx);

        if (1) {
            Gfx* sp218;
            Gfx* sp214 = POLY_OPA_DISP;

            sp218 = Gfx_Open(sp214);
            gSPDisplayList(OVERLAY_DISP++, sp218);

            if (((this->transitionMode == TRANS_MODE_INSTANCE_RUNNING) ||
                 (this->transitionMode == TRANS_TYPE_INSTANT)) ||
                D_801D0D54) {
                View spA8;

                View_Init(&spA8, gfxCtx);
                spA8.flags = 0xA;

                SET_FULLSCREEN_VIEWPORT(&spA8);

                View_ApplyTo(&spA8, &sp218);
                this->transitionCtx.draw(&this->transitionCtx.instanceData, &sp218);
            }

            TransitionFade_Draw(&this->unk_18E48, &sp218);

            if (gPlayVisMonoColor.a != 0) {
                sPlayVisMono.vis.primColor.rgba = gPlayVisMonoColor.rgba;
                VisMono_Draw(&sPlayVisMono, &sp218);
            }

            gSPEndDisplayList(sp218++);
            Gfx_Close(sp214, sp218);
            POLY_OPA_DISP = sp218;
        }

        if (gTransitionTileState == TRANS_TILE_READY) {
            Gfx* sp90 = POLY_OPA_DISP;

            TransitionTile_Draw(&sTransitionTile, &sp90);
            POLY_OPA_DISP = sp90;
            sp25B = true;
            goto PostWorldDraw;
        }

        PreRender_SetValues(&this->pauseBgPreRender, gCfbWidth, gCfbHeight, gfxCtx->curFrameBuffer, gfxCtx->zbuffer);

        if (R_PAUSE_BG_PRERENDER_STATE == PAUSE_BG_PRERENDER_PROCESS) {
            Sched_FlushTaskQueue();
            if (!gSaveContext.screenScaleFlag) {
                PreRender_ApplyFiltersSlowlyInit(&this->pauseBgPreRender);
            }
            R_PAUSE_BG_PRERENDER_STATE = PAUSE_BG_PRERENDER_READY;
            SREG(33) |= 1;
        } else {
            if (R_PAUSE_BG_PRERENDER_STATE == PAUSE_BG_PRERENDER_READY) {
                Gfx* sp8C = POLY_OPA_DISP;

                if (this->pauseBgPreRender.filterState == PRERENDER_FILTER_STATE_DONE) {
                    PreRender_RestoreFramebuffer(&this->pauseBgPreRender, &sp8C);
                } else {
                    func_80170798(&this->pauseBgPreRender, &sp8C);
                }

                gSPDisplayList(sp8C++, D_0E000000.syncSegments);
                POLY_OPA_DISP = sp8C;
                sp25B = true;
                goto PostWorldDraw;
            }

            if (!this->soaringCsOrSoTCsPlaying) {
                if (1) {
                    if (((u32)this->skyboxId != SKYBOX_NONE) && !this->envCtx.skyboxDisabled) 
                    {
                        if ((this->skyboxId == SKYBOX_NORMAL_SKY) || (this->skyboxId == SKYBOX_3)) {
                            Environment_UpdateSkybox(this->skyboxId, &this->envCtx, &this->skyboxCtx);
                            Skybox_Draw(&this->skyboxCtx, gfxCtx, this->skyboxId, this->envCtx.skyboxBlend,
                                        this->view.eye.x, this->view.eye.y, this->view.eye.z);
                        } else if (!this->skyboxCtx.shouldDraw) {
                            Skybox_Draw(&this->skyboxCtx, gfxCtx, this->skyboxId, 0, this->view.eye.x, this->view.eye.y,
                                        this->view.eye.z);
                        }

                        Environment_Draw(this);
                    }
                }
            

                lights = LightContext_NewLights(&this->lightCtx, gfxCtx);

                if(gChaosContext.env.length_contraction_scale < 1.0f)
                {
                    MtxF scale_transform;
                    Vec3f player_pos = player->actor.world.pos;
                    Vec3f scale_axis = gChaosContext.env.length_contraction_axis;
                    f32 scale_minus_one = gChaosContext.env.length_contraction_scale - 1.0f;

                    scale_transform.mf[0][0] = 1.0f + scale_minus_one * scale_axis.x * scale_axis.x;
                    scale_transform.mf[1][0] = scale_minus_one * scale_axis.x * scale_axis.y;
                    scale_transform.mf[2][0] = scale_minus_one * scale_axis.x * scale_axis.z;
                    scale_transform.mf[3][0] = 0.0f;

                    scale_transform.mf[0][1] = scale_minus_one * scale_axis.x * scale_axis.y;
                    scale_transform.mf[1][1] = 1.0f + scale_minus_one * scale_axis.y * scale_axis.y;
                    scale_transform.mf[2][1] = scale_minus_one * scale_axis.y * scale_axis.z;
                    scale_transform.mf[3][1] = 0.0f;

                    scale_transform.mf[0][2] = scale_minus_one * scale_axis.x * scale_axis.z;
                    scale_transform.mf[1][2] = scale_minus_one * scale_axis.y * scale_axis.z;
                    scale_transform.mf[2][2] = 1.0f + scale_minus_one * scale_axis.z * scale_axis.z;
                    scale_transform.mf[3][2] = 0.0f;
                
                    scale_transform.mf[0][3] = 0;
                    scale_transform.mf[1][3] = 0;
                    scale_transform.mf[2][3] = 0;
                    scale_transform.mf[3][3] = 1;

                    Matrix_Push();
                    Matrix_Translate(player_pos.x, player_pos.y, player_pos.z, MTXMODE_NEW);
                    Matrix_Mult(&scale_transform, MTXMODE_APPLY);
                    Matrix_Translate(-player_pos.x, -player_pos.y, -player_pos.z, MTXMODE_APPLY);
                    Matrix_ToMtx(&gChaosContext.env.length_contraction_matrix);
                    Matrix_Pop();

                    gSPMatrix(POLY_OPA_DISP++, &gChaosContext.env.length_contraction_matrix, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);
                    gSPMatrix(POLY_XLU_DISP++, &gChaosContext.env.length_contraction_matrix, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);
                }

                lights = LightContext_NewLights(&this->lightCtx, gfxCtx);

                if (this->roomCtx.curRoom.enablePosLights || (MREG(93) != 0)) {
                    lights->enablePosLights = true;
                }

                Lights_BindAll(lights, this->lightCtx.listHead, NULL, this);
                Lights_Draw(lights, gfxCtx);

            // if (1) {
                //! FAKE:
                // u32 roomDrawFlags = ((1) ? 1 : 0) | (((void)0, 1) ? 2 : 0);

                Scene_Draw(this);

                if (this->roomCtx.unk78) {
                    u32 room_flags = ROOM_DRAW_OPA | ROOM_DRAW_XLU;
                    Room_Draw(this, &this->roomCtx.curRoom, room_flags);

                    if(Chaos_IsCodeActive(CHAOS_CODE_DIRECTILE_DYSFUNCTION))
                    {
                        room_flags |= ROOM_DRAW_ROTATED;
                    }

                    Room_Draw(this, &this->roomCtx.prevRoom, room_flags);
                    gSPSegment(POLY_OPA_DISP++, 0x01, this->billboardMtx);
                    gSPSegment(POLY_XLU_DISP++, 0x01, this->billboardMtx);
                }
                // }

                if (this->skyboxCtx.shouldDraw) {
                    Vec3f quakeOffset;

                    if (1) {
                        quakeOffset = Camera_GetQuakeOffset(GET_ACTIVE_CAM(this));
                        Skybox_Draw(&this->skyboxCtx, gfxCtx, this->skyboxId, 0, this->view.eye.x + quakeOffset.x,
                                    this->view.eye.y + quakeOffset.y, this->view.eye.z + quakeOffset.z);
                    }
                }

                if (this->envCtx.precipitation[PRECIP_RAIN_CUR] != 0) {
                    Environment_DrawRain(this, &this->view, gfxCtx);
                }
            }

            // if (1) {
                Environment_FillScreen(gfxCtx, 0, 0, 0, this->bgCoverAlpha, FILL_SCREEN_OPA);
            // }

            // if (1) {
                // if(Chaos_IsCodeActive(CHAOS_CODE_LENGTH_CONTRACTION))
                // {
                //     gSPMatrix(POLY_OPA_DISP++, this->view.projectionPtr, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
                //     gSPMatrix(POLY_OPA_DISP++, this->view.viewingPtr, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);

                //     gSPMatrix(POLY_XLU_DISP++, this->view.projectionPtr, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
                //     gSPMatrix(POLY_XLU_DISP++, this->view.viewingPtr, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);
                // }
                if(gChaosContext.env.blizzard_state == CHAOS_BLIZZARD_STATE_BLIZZARDING)
                {
                    s16 slot_index = Object_GetPersistentSlot(&this->objectCtx, OBJECT_DAI);
                    gSPSegment(POLY_OPA_DISP++, 0x06, this->objectCtx.slots[slot_index].segment);
                    gSPSegment(POLY_XLU_DISP++, 0x06, this->objectCtx.slots[slot_index].segment);
                    EffectGoronBlizzard_Draw(&gChaosContext.env.blizzard, this->state.gfxCtx);
                }

                Actor_DrawAll(this, &this->actorCtx);

                // if(gChaosContext.env.draw_thunderbolt)
                // {
                //     // Gfx_SetupDL25_Opa(this->state.gfxCtx);
                //     gSPClearGeometryMode(POLY_OPA_DISP++, G_SHADE |
                //         G_SHADING_SMOOTH | G_CULL_BOTH
                //         | G_FOG | G_LIGHTING | G_TEXTURE_GEN
                //         | G_TEXTURE_GEN_LINEAR | G_LOD);
                //     gSPSegment(POLY_OPA_DISP++, 0x06, gChaosContext.env.thunderbolt_display_list);
                //     // gSPMatrix(POLY_OPA_DISP++, this->view.projectionPtr, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
                //     // gSPMatrix(POLY_OPA_DISP++, this->view.viewingPtr, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);

                //     gSPDisplayList(POLY_OPA_DISP++, gChaosContext.env.thunderbolt_display_list);
                //     // gSPDisplayList(POLY_OPA_DISP++, 0x06000000);
                // }
            // }

            // if (1) {
                if (!this->envCtx.sunDisabled) {
                    temp.x = this->view.eye.x + this->envCtx.sunPos.x;
                    temp.y = this->view.eye.y + this->envCtx.sunPos.y;
                    temp.z = this->view.eye.z + this->envCtx.sunPos.z;
                    Environment_DrawSunLensFlare(this, &this->envCtx, &this->view, gfxCtx, temp);
                }

                Environment_DrawCustomLensFlare(this);
            // }

            // if (1) {
                if (R_PLAY_FILL_SCREEN_ON) {
                    Environment_FillScreen(gfxCtx, R_PLAY_FILL_SCREEN_R, R_PLAY_FILL_SCREEN_G, R_PLAY_FILL_SCREEN_B,
                                           R_PLAY_FILL_SCREEN_ALPHA, FILL_SCREEN_OPA | FILL_SCREEN_XLU);
                }

                switch (this->envCtx.fillScreen) {
                    case 1:
                        Environment_FillScreen(gfxCtx, this->envCtx.screenFillColor[0], this->envCtx.screenFillColor[1],
                                               this->envCtx.screenFillColor[2], this->envCtx.screenFillColor[3],
                                               FILL_SCREEN_OPA | FILL_SCREEN_XLU);
                        break;

                    default:
                        break;
                }
            // }

            // if (1) {
                if (this->envCtx.sandstormState != SANDSTORM_OFF) {
                    Environment_DrawSandstorm(this, this->envCtx.sandstormState);
                }
            // }

            if (this->worldCoverAlpha != 0) {
                Environment_FillScreen(gfxCtx, 0, 0, 0, this->worldCoverAlpha, FILL_SCREEN_OPA | FILL_SCREEN_XLU);
            }

            // if (1) {
                DebugDisplay_DrawObjects(this);
            // }

            Play_DrawMotionBlur(this);

            if ((R_PAUSE_BG_PRERENDER_STATE == PAUSE_BG_PRERENDER_SETUP) ||
                (gTransitionTileState == TRANS_TILE_SETUP) || (R_PICTO_PHOTO_STATE == PICTO_PHOTO_STATE_SETUP)) {
                Gfx* sp74;
                Gfx* sp70 = POLY_OPA_DISP;

                sp74 = Gfx_Open(sp70);
                gSPDisplayList(OVERLAY_DISP++, sp74);
                this->pauseBgPreRender.fbuf = gfxCtx->curFrameBuffer;

                if (R_PAUSE_BG_PRERENDER_STATE == PAUSE_BG_PRERENDER_SETUP) {
                    R_PAUSE_BG_PRERENDER_STATE = PAUSE_BG_PRERENDER_PROCESS;
                    this->pauseBgPreRender.fbufSave = gfxCtx->zbuffer;
                    this->pauseBgPreRender.cvgSave = this->unk_18E58;
                } else if (R_PICTO_PHOTO_STATE == PICTO_PHOTO_STATE_SETUP) {
                    R_PICTO_PHOTO_STATE = PICTO_PHOTO_STATE_PROCESS;
                    this->pauseBgPreRender.fbufSave = gfxCtx->zbuffer;
                    this->pauseBgPreRender.cvgSave = this->unk_18E58;
                } else {
                    gTransitionTileState = TRANS_TILE_PROCESS;
                    this->pauseBgPreRender.fbufSave = gfxCtx->zbuffer;
                    this->pauseBgPreRender.cvgSave = NULL;
                }

                PreRender_SaveFramebuffer(&this->pauseBgPreRender, &sp74);

                if (this->pauseBgPreRender.cvgSave != NULL) {
                    PreRender_DrawCoverage(&this->pauseBgPreRender, &sp74);
                }

                gSPEndDisplayList(sp74++);
                Gfx_Close(sp70, sp74);
                POLY_OPA_DISP = sp74;
                this->unk_18B49 = 2;
                SREG(33) |= 1;
                goto SkipPostWorldDraw;
            }

        PostWorldDraw:
            // if (1) {
                Play_PostWorldDraw(this);
            // }

            // if(gChaosContext.env.draw_thunderbolt)
            // {
            //     Gfx_SetupDL25_Opa(this->state.gfxCtx);
            //     // gSPClearGeometryMode(POLY_OPA_DISP++, G_SHADE |
            //     //     G_SHADING_SMOOTH | G_CULL_BOTH
            //     //     | G_FOG | G_LIGHTING | G_TEXTURE_GEN
            //     //     | G_TEXTURE_GEN_LINEAR | G_LOD);
            //     // gSPSegment(POLY_OPA_DISP++, 0x06, gChaosContext.env.thunderbolt_display_list);
            //     // gSPMatrix(POLY_OPA_DISP++, this->view.projectionPtr, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
            //     // gSPMatrix(POLY_OPA_DISP++, this->view.viewingPtr, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);

            //     gSPDisplayList(POLY_OPA_DISP++, gChaosContext.env.thunderbolt_display_list);
            //     // gSPDisplayList(POLY_OPA_DISP++, 0x06000000);
            // }
        }
    }

SkipPostWorldDraw:

    // if(gChaosContext.env.draw_thunderbolt)
    // {
    //     Chaos_GenerateThunderbolt(this);
    //     // Gfx_SetupDL25_Opa(this->state.gfxCtx);
    //     // gSPClearGeometryMode(POLY_OPA_DISP++, G_SHADE |
    //     //     G_SHADING_SMOOTH | G_CULL_BOTH
    //     //     | G_FOG | G_LIGHTING | G_TEXTURE_GEN
    //     //     | G_TEXTURE_GEN_LINEAR | G_LOD);
    //     // gSPSetGeometryMode(POLY_OPA_DISP++, G_SHADE | G_LIGHTING |
    //     //     G_SHADING_SMOOTH
    //     //     | G_ZBUFFER | G_CULL_BACK);
    //     // gDPSetRenderMode(POLY_OPA_DISP++,G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    //     // // gSPSegment(POLY_OPA_DISP++, 0x06, NULL);
    //     // // gSPMatrix(POLY_OPA_DISP++, this->view.projectionPtr, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
    //     // // gSPMatrix(POLY_OPA_DISP++, this->view.viewingPtr, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);

    //     // gSPDisplayList(POLY_OPA_DISP++, gChaosContext.env.thunderbolt_display_list);
    //     // gSPDisplayList(POLY_OPA_DISP++, 0x00000000); 
    // }

    if ((this->view.unk164 != 0) && !gDbgCamEnabled) {
        Camera_Update(GET_ACTIVE_CAM(this));
        View_UpdateViewingMatrix(&this->view);
        this->view.unk164 = 0;
        if ((this->skyboxId != SKYBOX_NONE) && !this->envCtx.skyboxDisabled) {
            Skybox_UpdateMatrix(&this->skyboxCtx, this->view.eye.x, this->view.eye.y, this->view.eye.z);
        }
    }

    if (!sp25B) {
        Environment_DrawSkyboxStars(this);
    }


    CLOSE_DISPS(gfxCtx);

    bzero(inputs, sizeof(inputs));
    PadMgr_GetInput(inputs, false);
    Chaos_PrintCodes(this, &inputs[0]);
    Chaos_PrintConsoleLines(this);

    if(this->pauseCtx.state == PAUSE_STATE_OFF)
    {
        Chaos_PrintSnakeGameStuff(this);
        Chaos_PrintSimonSaysStuff(this);
    }
    
    if(Chaos_CanUpdateChaos(this))
    {
        if(CHECK_BTN_ANY(inputs[0].press.button, BTN_DLEFT))
        {
            if(!(gChaosContext.hide_actors & 2))
            {
                gChaosContext.hide_actors |= 2;
                gChaosContext.hide_actors ^= 1;
            }
        }
        else
        {
            gChaosContext.hide_actors &= ~2;
        }
    }
}

void Play_Draw(PlayState* this) {
    GraphicsContext* gfxCtx = this->state.gfxCtx;

    {
        GraphicsContext* gfxCtx2 = this->state.gfxCtx;

        if (sBombersNotebookOpen) {
            if (gSysCfbHiResEnabled != 1) {
                Sched_FlushTaskQueue();
                SysCfb_SetHiResMode();
                gfxCtx2->curFrameBuffer = SysCfb_GetFramebuffer(gfxCtx2->framebufferIndex % 2);
                gfxCtx2->zbuffer = SysCfb_GetZBuffer();
                gfxCtx2->viMode = gActiveViMode;
                gfxCtx2->viConfigFeatures = gViConfigFeatures;
                gfxCtx2->xScale = gViConfigXScale;
                gfxCtx2->yScale = gViConfigYScale;
                gfxCtx2->updateViMode = true;
            }
        } else {
            if (gSysCfbHiResEnabled != 0) {
                Sched_FlushTaskQueue();
                SysCfb_SetLoResMode();
                gfxCtx2->curFrameBuffer = SysCfb_GetFramebuffer(gfxCtx2->framebufferIndex % 2);
                gfxCtx2->zbuffer = SysCfb_GetZBuffer();
                gfxCtx2->viMode = gActiveViMode;
                gfxCtx2->viConfigFeatures = gViConfigFeatures;
                gfxCtx2->xScale = gViConfigXScale;
                gfxCtx2->yScale = gViConfigYScale;
                gfxCtx2->updateViMode = true;
            }
        }
    }

    if (sBombersNotebookOpen && ((SREG(2) != 2) || (gZBufferPtr == NULL))) {
        BombersNotebook_Draw(&sBombersNotebook, gfxCtx);
        Message_Draw(this);
    } else {
        Play_DrawMain(this);
    }
}

void Play_Main(GameState* thisx) {
    static Input* prevInput = NULL;
    PlayState* this = (PlayState*)thisx;

    prevInput = CONTROLLER1(&this->state);
    DebugDisplay_Init();

    {
        GraphicsContext* gfxCtx = this->state.gfxCtx;

        if (1) {
            this->state.gfxCtx = NULL;
        }
        Play_Update(this);
        this->state.gfxCtx = gfxCtx;
    }

    {
        Input input = *CONTROLLER1(&this->state);

        if (1) {
            *CONTROLLER1(&this->state) = D_801F6C18;
        }
        Play_Draw(this);
        *CONTROLLER1(&this->state) = input;
    }

    CutsceneManager_Update();
    CutsceneManager_ClearWaiting();
}

bool Play_InCsMode(PlayState* this) {
    return (this->csCtx.state != CS_STATE_IDLE) || Player_InCsMode(this);
}

f32 Play_GetFloorSurfaceImpl(PlayState* this, MtxF* mtx, CollisionPoly** poly, s32* bgId, Vec3f* pos) {
    f32 floorHeight = BgCheck_EntityRaycastFloor3(&this->colCtx, poly, bgId, pos);

    if (floorHeight > BGCHECK_Y_MIN) {
        func_800C0094(*poly, pos->x, floorHeight, pos->z, mtx);
    } else {
        mtx->xy = 0.0f;
        mtx->zx = 0.0f;
        mtx->yx = 0.0f;
        mtx->xx = 0.0f;
        mtx->wz = 0.0f;
        mtx->xz = 0.0f;
        mtx->wy = 0.0f;
        mtx->wx = 0.0f;
        mtx->zz = 0.0f;
        mtx->yz = 0.0f;
        mtx->zy = 0.0f;
        mtx->yy = 1.0f;
        mtx->xw = pos->x;
        mtx->yw = pos->y;
        mtx->zw = pos->z;
        mtx->ww = 1.0f;
    }

    return floorHeight;
}

void Play_GetFloorSurface(PlayState* this, MtxF* mtx, Vec3f* pos) {
    CollisionPoly* poly;
    s32 bgId;

    Play_GetFloorSurfaceImpl(this, mtx, &poly, &bgId, pos);
}

void* Play_LoadFile(PlayState* this, RomFile* entry) {
    size_t size = entry->vromEnd - entry->vromStart;
    void* allocp = THA_AllocTailAlign16(&this->state.tha, size);

    DmaMgr_RequestSync(allocp, entry->vromStart, size);

    return allocp;
}

void Play_InitEnvironment(PlayState* this, s16 skyboxId) {
    Skybox_Init(&this->state, &this->skyboxCtx, skyboxId);
    Environment_Init(this, &this->envCtx, 0);
}

void Play_InitScene(PlayState* this, s32 spawn) {
    this->curSpawn = spawn;
    this->linkActorEntry = NULL;
    this->actorCsCamList = NULL;
    this->setupEntranceList = NULL;
    this->setupExitList = NULL;
    this->naviQuestHints = NULL;
    this->setupPathList = NULL;
    this->sceneMaterialAnims = NULL;
    this->roomCtx.unk74 = NULL;
    this->numSetupActors = 0;
    Object_InitContext(&this->state, &this->objectCtx);
    LightContext_Init(this, &this->lightCtx);
    Scene_ResetTransitionActorList(&this->state, &this->transitionActors);
    Room_Init(this, &this->roomCtx);
    gSaveContext.worldMapArea = 0;
    Scene_ExecuteCommands(this, this->sceneSegment);
    Play_InitEnvironment(this, this->skyboxId);
}

void Play_SpawnScene(PlayState* this, s32 sceneId, s32 spawn) {
    s32 pad;
    SceneTableEntry* scene = &gSceneTable[sceneId];

    scene->unk_D = 0;
    this->loadedScene = scene;
    this->sceneId = sceneId;
    this->sceneConfig = scene->drawConfig;
    this->sceneSegment = Play_LoadFile(this, &scene->segment);
    scene->unk_D = 0;
    gSegments[0x02] = OS_K0_TO_PHYSICAL(this->sceneSegment);
    Play_InitScene(this, spawn);
    Room_SetupFirstRoom(this, &this->roomCtx);

    if(sceneId == SCENE_MITURIN || sceneId == SCENE_MITURIN_BS)
    {
        SET_WEEKEVENTREG(WEEKEVENTREG_WOODFALL_TEMPLE_RISEN);
    }
}

void Play_GetScreenPos(PlayState* this, Vec3f* worldPos, Vec3f* screenPos) {
    f32 invW;

    // screenPos temporarily stores the projectedPos
    Actor_GetProjectedPos(this, worldPos, screenPos, &invW);

    screenPos->x = (SCREEN_WIDTH / 2) + (screenPos->x * invW * (SCREEN_WIDTH / 2));
    screenPos->y = (SCREEN_HEIGHT / 2) - (screenPos->y * invW * (SCREEN_HEIGHT / 2));
}

s16 Play_CreateSubCamera(PlayState* this) {
    s16 subCamId;

    for (subCamId = CAM_ID_SUB_FIRST; subCamId < NUM_CAMS; subCamId++) {
        if (this->cameraPtrs[subCamId] == NULL) {
            break;
        }
    }

    // if no subCameras available
    if (subCamId == NUM_CAMS) {
        return CAM_ID_NONE;
    }

    this->cameraPtrs[subCamId] = &this->subCameras[subCamId - CAM_ID_SUB_FIRST];
    Camera_Init(this->cameraPtrs[subCamId], &this->view, &this->colCtx, this);
    this->cameraPtrs[subCamId]->camId = subCamId;

    return subCamId;
}

s16 Play_GetActiveCamId(PlayState* this) {
    return this->activeCamId;
}

s32 Play_ChangeCameraStatus(PlayState* this, s16 camId, s16 status) {
    s16 camIdx = (camId == CAM_ID_NONE) ? this->activeCamId : camId;

    if (status == CAM_STATUS_ACTIVE) {
        this->activeCamId = camIdx;
    }

    return Camera_ChangeStatus(this->cameraPtrs[camIdx], status);
}

void Play_ClearCamera(PlayState* this, s16 camId) {
    s16 camIdx = (camId == CAM_ID_NONE) ? this->activeCamId : camId;

    if (this->cameraPtrs[camIdx] != NULL) {
        Camera_ChangeStatus(this->cameraPtrs[camIdx], CAM_STATUS_INACTIVE);
        this->cameraPtrs[camIdx] = NULL;
    }
}

void Play_ClearAllSubCameras(PlayState* this) {
    s16 subCamId;

    for (subCamId = CAM_ID_SUB_FIRST; subCamId < NUM_CAMS; subCamId++) {
        if (this->cameraPtrs[subCamId] != NULL) {
            Play_ClearCamera(this, subCamId);
        }
    }

    this->activeCamId = CAM_ID_MAIN;
}

Camera* Play_GetCamera(PlayState* this, s16 camId) {
    s16 camIdx = (camId == CAM_ID_NONE) ? this->activeCamId : camId;

    return this->cameraPtrs[camIdx];
}

/**
 * @return bit-packed success if each of the params were applied
 */
s32 Play_SetCameraAtEye(PlayState* this, s16 camId, Vec3f* at, Vec3f* eye) {
    s32 successfullySet = 0;
    s16 camIdx = (camId == CAM_ID_NONE) ? this->activeCamId : camId;
    Camera* camera = this->cameraPtrs[camIdx];

    successfullySet |= Camera_SetViewParam(camera, CAM_VIEW_AT, at);
    successfullySet <<= 1;
    successfullySet |= Camera_SetViewParam(camera, CAM_VIEW_EYE, eye);

    camera->dist = Math3D_Vec3f_DistXYZ(at, eye);

    if (camera->focalActor != NULL) {
        camera->focalActorAtOffset.x = at->x - camera->focalActor->world.pos.x;
        camera->focalActorAtOffset.y = at->y - camera->focalActor->world.pos.y;
        camera->focalActorAtOffset.z = at->z - camera->focalActor->world.pos.z;
    } else {
        camera->focalActorAtOffset.x = camera->focalActorAtOffset.y = camera->focalActorAtOffset.z = 0.0f;
    }

    camera->atLerpStepScale = 0.01f;

    return successfullySet;
}

/**
 * @return bit-packed success if each of the params were applied
 */
s32 Play_SetCameraAtEyeUp(PlayState* this, s16 camId, Vec3f* at, Vec3f* eye, Vec3f* up) {
    s32 successfullySet = 0;
    s16 camIdx = (camId == CAM_ID_NONE) ? this->activeCamId : camId;
    Camera* camera = this->cameraPtrs[camIdx];

    successfullySet |= Camera_SetViewParam(camera, CAM_VIEW_AT, at);
    successfullySet <<= 1;
    successfullySet |= Camera_SetViewParam(camera, CAM_VIEW_EYE, eye);
    successfullySet <<= 1;
    successfullySet |= Camera_SetViewParam(camera, CAM_VIEW_UP, up);

    camera->dist = Math3D_Vec3f_DistXYZ(at, eye);

    if (camera->focalActor != NULL) {
        camera->focalActorAtOffset.x = at->x - camera->focalActor->world.pos.x;
        camera->focalActorAtOffset.y = at->y - camera->focalActor->world.pos.y;
        camera->focalActorAtOffset.z = at->z - camera->focalActor->world.pos.z;
    } else {
        camera->focalActorAtOffset.x = camera->focalActorAtOffset.y = camera->focalActorAtOffset.z = 0.0f;
    }

    camera->atLerpStepScale = 0.01f;

    return successfullySet;
}

/**
 * @return true if the fov was successfully set
 */
s32 Play_SetCameraFov(PlayState* this, s16 camId, f32 fov) {
    s32 successfullySet = Camera_SetViewParam(this->cameraPtrs[camId], CAM_VIEW_FOV, &fov) & 1;

    if (1) {}
    return successfullySet;
}

s32 Play_SetCameraRoll(PlayState* this, s16 camId, s16 roll) {
    s16 camIdx = (camId == CAM_ID_NONE) ? this->activeCamId : camId;
    Camera* camera = this->cameraPtrs[camIdx];

    camera->roll = roll;

    return 1;
}

void Play_CopyCamera(PlayState* this, s16 destCamId, s16 srcCamId) {
    s16 srcCamId2 = (srcCamId == CAM_ID_NONE) ? this->activeCamId : srcCamId;
    s16 destCamId1 = (destCamId == CAM_ID_NONE) ? this->activeCamId : destCamId;

    Camera_Copy(this->cameraPtrs[destCamId1], this->cameraPtrs[srcCamId2]);
}

// Same as Play_ChangeCameraSetting but also calls Camera_InitFocalActorSettings
s32 func_80169A50(PlayState* this, s16 camId, Player* player, s16 setting) {
    Camera* camera;
    s16 camIdx = (camId == CAM_ID_NONE) ? this->activeCamId : camId;

    camera = this->cameraPtrs[camIdx];
    Camera_InitFocalActorSettings(camera, &player->actor);
    return Camera_ChangeSetting(camera, setting);
}

s32 Play_ChangeCameraSetting(PlayState* this, s16 camId, s16 setting) {
    return Camera_ChangeSetting(Play_GetCamera(this, camId), setting);
}

// Related to bosses and fishing
void func_80169AFC(PlayState* this, s16 camId, s16 timer) {
    s16 camIdx = (camId == CAM_ID_NONE) ? this->activeCamId : camId;
    s16 i;

    Play_ClearCamera(this, camIdx);

    for (i = CAM_ID_SUB_FIRST; i < NUM_CAMS; i++) {
        if (this->cameraPtrs[i] != NULL) {
            Play_ClearCamera(this, i);
        }
    }

    if (timer <= 0) {
        Play_ChangeCameraStatus(this, CAM_ID_MAIN, CAM_STATUS_ACTIVE);
        this->cameraPtrs[CAM_ID_MAIN]->childCamId = this->cameraPtrs[CAM_ID_MAIN]->doorTimer2 = 0;
    }
}

s16 Play_GetCameraUID(PlayState* this, s16 camId) {
    Camera* camera = this->cameraPtrs[camId];

    if (camera != NULL) {
        return camera->uid;
    } else {
        return -1;
    }
}

// Unused in both MM and OoT, purpose is very unclear
s16 func_80169BF8(PlayState* this, s16 camId, s16 uid) {
    Camera* camera = this->cameraPtrs[camId];

    if (camera != NULL) {
        return 0;
    } else if (camera->uid != uid) {
        return 0;
    } else if (camera->status != CAM_STATUS_ACTIVE) {
        return 2;
    } else {
        return 1;
    }
}

u16 Play_GetActorCsCamSetting(PlayState* this, s32 csCamDataIndex) {
    ActorCsCamInfo* actorCsCamList = &this->actorCsCamList[csCamDataIndex];

    return actorCsCamList->setting;
}

Vec3s* Play_GetActorCsCamFuncData(PlayState* this, s32 csCamDataIndex) {
    ActorCsCamInfo* actorCsCamList = &this->actorCsCamList[csCamDataIndex];

    return Lib_SegmentedToVirtual(actorCsCamList->actorCsCamFuncData);
}

/**
 * Converts the number of a scene to its "original" equivalent, the default version of the area which the player first
 * enters.
 */
s16 Play_GetOriginalSceneId(s16 sceneId) {
    // Inverted Stone Tower Temple -> Stone Tower Temple
    if (sceneId == SCENE_INISIE_R) {
        return SCENE_INISIE_N;
    }

    // Purified Southern Swamp -> Poisoned Sothern Swamp
    if (sceneId == SCENE_20SICHITAI2) {
        return SCENE_20SICHITAI;
    }

    // Spring Mountain Village -> Winter Mountain Village
    if (sceneId == SCENE_10YUKIYAMANOMURA2) {
        return SCENE_10YUKIYAMANOMURA;
    }

    // Spring Goron Village -> Winter Goron Village
    if (sceneId == SCENE_11GORONNOSATO2) {
        return SCENE_11GORONNOSATO;
    }

    // Spring Path to Goron Village -> Winter Path to Goron Village
    if (sceneId == SCENE_17SETUGEN2) {
        return SCENE_17SETUGEN;
    }

    // Inverted Stone Tower -> Stone Tower
    if (sceneId == SCENE_F41) {
        return SCENE_F40;
    }

    return sceneId;
}

/**
 * Copies the flags set in ActorContext over to the current scene's CycleSceneFlags, usually using the original scene
 * number. Exception for Inverted Stone Tower Temple, which uses its own.
 */
void Play_SaveCycleSceneFlags(PlayState* this) {
    CycleSceneFlags* cycleSceneFlags;

    cycleSceneFlags = &gSaveContext.cycleSceneFlags[Play_GetOriginalSceneId(this->sceneId)];
    cycleSceneFlags->chest = this->actorCtx.sceneFlags.chest;
    cycleSceneFlags->switch0 = this->actorCtx.sceneFlags.switches[0];
    cycleSceneFlags->switch1 = this->actorCtx.sceneFlags.switches[1];

    if (this->sceneId == SCENE_INISIE_R) { // Inverted Stone Tower Temple
        cycleSceneFlags = &gSaveContext.cycleSceneFlags[this->sceneId];
    }

    cycleSceneFlags->collectible = this->actorCtx.sceneFlags.collectible[0];
    cycleSceneFlags->clearedRoom = this->actorCtx.sceneFlags.clearedRoom;
}

void Play_SetRespawnData(PlayState* this, s32 respawnMode, u16 entrance, s32 roomIndex, s32 playerParams, Vec3f* pos,
                         s16 yaw) {

    gSaveContext.respawn[respawnMode].entrance = Entrance_Create(entrance >> 9, 0, entrance & 0xF);
    gSaveContext.respawn[respawnMode].roomIndex = roomIndex;
    gSaveContext.respawn[respawnMode].pos = *pos;
    gSaveContext.respawn[respawnMode].yaw = yaw;
    gSaveContext.respawn[respawnMode].playerParams = playerParams;
    gSaveContext.respawn[respawnMode].tempSwitchFlags = this->actorCtx.sceneFlags.switches[2];
    gSaveContext.respawn[respawnMode].unk_18 = this->actorCtx.sceneFlags.collectible[1];
    gSaveContext.respawn[respawnMode].tempCollectFlags = this->actorCtx.sceneFlags.collectible[2];
}

void Play_SetupRespawnPoint(PlayState* this, s32 respawnMode, s32 playerParams) {
    Player* player = GET_PLAYER(this);

    if (this->sceneId != SCENE_KAKUSIANA) { // Grottos
        Play_SetRespawnData(this, respawnMode, ((void)0, gSaveContext.save.entrance), this->roomCtx.curRoom.num,
                            playerParams, &player->actor.world.pos, player->actor.shape.rot.y);
    }
}

// Override respawn data in Sakon's Hideout
void func_80169ECC(PlayState* this) {
    if (this->sceneId == SCENE_SECOM) {
        this->nextEntrance = ENTRANCE(IKANA_CANYON, 6);
        gSaveContext.respawnFlag = -7;
    }
}

// Gameplay_TriggerVoidOut ?
// Used by Player, Ikana_Rotaryroom, Bji01, Kakasi, LiftNuts, Test4, Warptag, WarpUzu, Roomtimer
void func_80169EFC(PlayState* this) {
    gSaveContext.respawn[RESPAWN_MODE_DOWN].tempSwitchFlags = this->actorCtx.sceneFlags.switches[2];
    gSaveContext.respawn[RESPAWN_MODE_DOWN].unk_18 = this->actorCtx.sceneFlags.collectible[1];
    gSaveContext.respawn[RESPAWN_MODE_DOWN].tempCollectFlags = this->actorCtx.sceneFlags.collectible[2];
    this->nextEntrance = gSaveContext.respawn[RESPAWN_MODE_DOWN].entrance;
    gSaveContext.respawnFlag = 1;
    func_80169ECC(this);
    this->transitionTrigger = TRANS_TRIGGER_START;
    this->transitionType = TRANS_TYPE_FADE_BLACK;
}

// Gameplay_LoadToLastEntrance ?
// Used by game_over and Test7
void func_80169F78(PlayState* this) {
    this->nextEntrance = gSaveContext.respawn[RESPAWN_MODE_TOP].entrance;
    gSaveContext.respawnFlag = -1;
    func_80169ECC(this);
    this->transitionTrigger = TRANS_TRIGGER_START;
    this->transitionType = TRANS_TYPE_FADE_BLACK;
}

// Gameplay_TriggerRespawn ?
// Used for void by Wallmaster, Deku Shrine doors. Also used by Player, Kaleido, DoorWarp1
void func_80169FDC(PlayState* this) {
    func_80169F78(this);
}

s32 Play_CamIsNotFixed(PlayState* this) {
    return this->roomCtx.curRoom.roomShape->base.type != ROOM_SHAPE_TYPE_IMAGE;
}

s32 FrameAdvance_IsEnabled(PlayState* this) {
    return this->frameAdvCtx.enabled != false;
}

// Unused, unchanged from OoT, which uses it only in one Camera function.
/**
 * @brief Tests if \p actor is a door and the sides are different rooms.
 *
 * @param[in] thisx GameState, promoted to play inside.
 * @param[in] actor Actor to test.
 * @param[out] yaw Facing angle of the actor, or reverse if in the back room.
 * @return true if \p actor is a door and the sides are in different rooms, false otherwise
 */
s32 func_8016A02C(PlayState* this, Actor* actor, s16* yaw) {
    TransitionActorEntry* transitionActor;
    s8 frontRoom;

    if (actor->category != ACTORCAT_DOOR) {
        return false;
    }

    transitionActor = &this->transitionActors.list[(u16)actor->params >> 10];
    frontRoom = transitionActor->sides[0].room;
    if (frontRoom == transitionActor->sides[1].room) {
        return false;
    }

    if (frontRoom == actor->room) {
        *yaw = actor->shape.rot.y;
    } else {
        *yaw = actor->shape.rot.y + 0x8000;
    }

    return true;
}

// Unused
/**
 * @brief Tests if \p pos is underwater.
 *
 * @param[in] this PlayState
 * @param[in] pos position to test
 * @return true if inside a waterbox and not above a void.
 */
s32 Play_IsUnderwater(PlayState* this, Vec3f* pos) {
    WaterBox* waterBox;
    CollisionPoly* poly;
    Vec3f waterSurfacePos;
    s32 bgId;

    waterSurfacePos = *pos;

    if ((WaterBox_GetSurface1(this, &this->colCtx, waterSurfacePos.x, waterSurfacePos.z, &waterSurfacePos.y,
                              &waterBox) == true) &&
        (pos->y < waterSurfacePos.y) &&
        (BgCheck_EntityRaycastFloor3(&this->colCtx, &poly, &bgId, &waterSurfacePos) != BGCHECK_Y_MIN)) {
        return true;
    } else {
        return false;
    }
}

s32 Play_IsDebugCamEnabled(void) {
    return gDbgCamEnabled;
}

// A mapping from playerCsIds to sGlobalCamDataSettings indices.
s16 sPlayerCsIdToCsCamId[] = {
    CS_CAM_ID_GLOBAL_ITEM_OCARINA,        // PLAYER_CS_ID_ITEM_OCARINA
    CS_CAM_ID_GLOBAL_ITEM_GET,            // PLAYER_CS_ID_ITEM_GET
    CS_CAM_ID_GLOBAL_ITEM_BOTTLE,         // PLAYER_CS_ID_ITEM_BOTTLE
    CS_CAM_ID_GLOBAL_ITEM_SHOW,           // PLAYER_CS_ID_ITEM_SHOW
    CS_CAM_ID_GLOBAL_WARP_PAD_MOON,       // PLAYER_CS_ID_WARP_PAD_MOON
    CS_CAM_ID_GLOBAL_MASK_TRANSFORMATION, // PLAYER_CS_ID_MASK_TRANSFORMATION
    CS_CAM_ID_GLOBAL_DEATH,               // PLAYER_CS_ID_DEATH
    CS_CAM_ID_GLOBAL_REVIVE,              // PLAYER_CS_ID_REVIVE
    CS_CAM_ID_GLOBAL_SONG_WARP,           // PLAYER_CS_ID_SONG_WARP
    CS_CAM_ID_GLOBAL_WARP_PAD_ENTRANCE,   // PLAYER_CS_ID_WARP_PAD_ENTRANCE
};

// Used by Player
/**
 * Extract the common cutscene ids used by Player from the scene and set the cutscene ids in this->playerCsIds.
 * If a playerCsId is not present in the scene, then that particular id is set to CS_ID_NONE.
 * Otherwise, if there is an CutsceneEntry where csCamId matches the appropriate element of sPlayerCsIdToCsCamId,
 * set the corresponding playerActorCsId (and possibly change its priority for the zeroth one).
 */
void Play_AssignPlayerCsIdsFromScene(PlayState* this, s32 spawnCsId) {
    s32 i;
    s16* curPlayerCsId = this->playerCsIds;
    s16* csCamId = sPlayerCsIdToCsCamId;

    for (i = 0; i < ARRAY_COUNT(this->playerCsIds); i++, curPlayerCsId++, csCamId++) {
        CutsceneEntry* csEntry;
        s32 curCsId;

        *curPlayerCsId = CS_ID_NONE;

        for (curCsId = spawnCsId; curCsId != CS_ID_NONE; curCsId = csEntry->additionalCsId) {
            csEntry = CutsceneManager_GetCutsceneEntry(curCsId);

            if (csEntry->csCamId == *csCamId) {
                if ((csEntry->csCamId == CS_CAM_ID_GLOBAL_ITEM_OCARINA) && (csEntry->priority == 700)) {
                    csEntry->priority = 550;
                }
                *curPlayerCsId = curCsId;
                break;
            }
        }
    }
}

// Set values to fill screen
void Play_FillScreen(PlayState* this, s16 fillScreenOn, u8 red, u8 green, u8 blue, u8 alpha) {
    R_PLAY_FILL_SCREEN_ON = fillScreenOn;
    R_PLAY_FILL_SCREEN_R = red;
    R_PLAY_FILL_SCREEN_G = green;
    R_PLAY_FILL_SCREEN_B = blue;
    R_PLAY_FILL_SCREEN_ALPHA = alpha;
}

// u16 sOverridePlayerCsIds[] = {
//     PLAYER_CS_ID_MASK_TRANSFORMATION,
//     PLAYER_CS_ID_SONG_WARP,
//     PLAYER_CS_ID_WARP_PAD_ENTRANCE,
//     PLAYER_CS_ID_ITEM_OCARINA
// };

bool Play_IsChangingArea(PlayState* this) {
    return (this->transitionTrigger != TRANS_TRIGGER_OFF) || (this->transitionMode != TRANS_MODE_OFF);
}

void Play_Init(GameState* thisx) {
    PlayState* this = (PlayState*)thisx;
    GraphicsContext* gfxCtx = this->state.gfxCtx;
    // s32 pad;
    uintptr_t zAlloc;
    s32 zAllocSize;
    Player* player;
    s32 i;
    s32 spawn;
    u8 sceneLayer;
    // u32 scene_bgm;
    s32 scene = gSaveContext.save.entrance >> 9;

    // gSaveCopy = gSaveContext.save;
    // u32 index;
    // s16 cur_cs_id;
    // u8 override_cutscene = false;
    // u8 was_playing_last_hours = Audio_IsFinalHours();

    if ((gSaveContext.respawnFlag == -4) || (gSaveContext.respawnFlag == -0x63)) {
        if (CHECK_EVENTINF(EVENTINF_TRIGGER_DAYTELOP)) {
            CLEAR_EVENTINF(EVENTINF_TRIGGER_DAYTELOP);
            STOP_GAMESTATE(&this->state);
            SET_NEXT_GAMESTATE(&this->state, DayTelop_Init, sizeof(DayTelopState));
            return;
        }

        gSaveContext.unk_3CA7 = 1;
        if (gSaveContext.respawnFlag == -0x63) {
            gSaveContext.respawnFlag = 2;
        }
    } else {
        gSaveContext.unk_3CA7 = 0;
    }

    if (gSaveContext.save.entrance == -1) {
        gSaveContext.save.entrance = 0;
        STOP_GAMESTATE(&this->state);
        SET_NEXT_GAMESTATE(&this->state, TitleSetup_Init, sizeof(TitleSetupState));
        return;
    }

    if ((gSaveContext.nextCutsceneIndex == 0xFFEF) || (gSaveContext.nextCutsceneIndex == 0xFFF0)) {
        // scene = gSaveContext.save.entrance >> 9;
        spawn = (gSaveContext.save.entrance >> 4) & 0x1F;

        if (CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE)) {
            if (scene == ENTR_SCENE_MOUNTAIN_VILLAGE_WINTER) {
                scene = ENTR_SCENE_MOUNTAIN_VILLAGE_SPRING;
            } else if (scene == ENTR_SCENE_GORON_VILLAGE_WINTER) {
                scene = ENTR_SCENE_GORON_VILLAGE_SPRING;
            } else if (scene == ENTR_SCENE_PATH_TO_GORON_VILLAGE_WINTER) {
                scene = ENTR_SCENE_PATH_TO_GORON_VILLAGE_SPRING;
            } else if ((scene == ENTR_SCENE_SNOWHEAD) || (scene == ENTR_SCENE_PATH_TO_SNOWHEAD) ||
                       (scene == ENTR_SCENE_PATH_TO_MOUNTAIN_VILLAGE) || (scene == ENTR_SCENE_GORON_SHRINE) ||
                       (scene == ENTR_SCENE_GORON_RACETRACK)) {
                gSaveContext.nextCutsceneIndex = 0xFFF0;
            }
        }

        if (CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE)) {
            if (scene == ENTR_SCENE_SOUTHERN_SWAMP_POISONED) {
                scene = ENTR_SCENE_SOUTHERN_SWAMP_CLEARED;
            } else if (scene == ENTR_SCENE_WOODFALL) {
                gSaveContext.nextCutsceneIndex = 0xFFF1;
            }
        }

        if (CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE) && (scene == ENTR_SCENE_IKANA_CANYON)) {
            gSaveContext.nextCutsceneIndex = 0xFFF2;
        }

        if (CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE) &&
            ((scene == ENTR_SCENE_GREAT_BAY_COAST) || (scene == ENTR_SCENE_ZORA_CAPE))) {
            gSaveContext.nextCutsceneIndex = 0xFFF0;
        }

        // "First cycle" Termina Field
        if (INV_CONTENT(ITEM_OCARINA_OF_TIME) != ITEM_OCARINA_OF_TIME) {
            if ((scene == ENTR_SCENE_TERMINA_FIELD) &&
                (gSaveContext.save.entrance != ENTRANCE(TERMINA_FIELD, 10))) {
                gSaveContext.nextCutsceneIndex = 0xFFF4;
            }
        }
        //! FAKE:
        gSaveContext.save.entrance = Entrance_Create(scene, spawn, gSaveContext.save.entrance & 0xF);
    }
    
    GameState_Realloc(&this->state, 0);
    KaleidoManager_Init(this);
    ShrinkWindow_Init();
    View_Init(&this->view, gfxCtx);
    Audio_SetExtraFilter(0);
    Quake_Init();
    Distortion_Init(this);

    for (i = 0; i < ARRAY_COUNT(this->cameraPtrs); i++) {
        this->cameraPtrs[i] = NULL;
    }

    Camera_Init(&this->mainCamera, &this->view, &this->colCtx, this);
    Camera_ChangeStatus(&this->mainCamera, CAM_STATUS_ACTIVE);

    for (i = 0; i < ARRAY_COUNT(this->subCameras); i++) {
        Camera_Init(&this->subCameras[i], &this->view, &this->colCtx, this);
        Camera_ChangeStatus(&this->subCameras[i], CAM_STATUS_INACTIVE);
    }

    this->cameraPtrs[CAM_ID_MAIN] = &this->mainCamera;
    this->cameraPtrs[CAM_ID_MAIN]->uid = CAM_ID_MAIN;
    this->activeCamId = CAM_ID_MAIN;

    Camera_OverwriteStateFlags(&this->mainCamera, CAM_STATE_0 | CAM_STATE_CHECK_WATER | CAM_STATE_2 | CAM_STATE_3 |
                                                      CAM_STATE_4 | CAM_STATE_DISABLE_MODE_CHANGE | CAM_STATE_6);
    Sram_Alloc(&this->state, &this->sramCtx);
    Regs_InitData(this);
    Message_Init(this);
    GameOver_Init(this);
    SoundSource_InitAll(this);
    EffFootmark_Init(this);
    Effect_Init(this);
    EffectSS_Init(this, 100);
    CollisionCheck_InitContext(this, &this->colChkCtx);
    AnimTaskQueue_Reset(&this->animTaskQueue);
    Cutscene_InitContext(this, &this->csCtx);

    if (gSaveContext.nextCutsceneIndex != 0xFFEF) {
        gSaveContext.save.cutsceneIndex = gSaveContext.nextCutsceneIndex;
        gSaveContext.nextCutsceneIndex = 0xFFEF;
    }

    if (gSaveContext.save.cutsceneIndex == 0xFFFD) {
        gSaveContext.save.cutsceneIndex = 0;
    }

    if (gSaveContext.nextDayTime != NEXT_TIME_NONE) {
        gSaveContext.save.time = gSaveContext.nextDayTime;
        gSaveContext.skyboxTime = gSaveContext.nextDayTime;
    }

    if ((CURRENT_TIME >= CLOCK_TIME(18, 0)) || (CURRENT_TIME < CLOCK_TIME(6, 30))) {
        gSaveContext.save.isNight = true;
    } else {
        gSaveContext.save.isNight = false;
    }

    // func_800EDDB0(this);

    if (((gSaveContext.gameMode != GAMEMODE_NORMAL) && (gSaveContext.gameMode != GAMEMODE_TITLE_SCREEN)) ||
        (gSaveContext.save.cutsceneIndex >= 0xFFF0)) {
        gSaveContext.nayrusLoveTimer = 0;
        Magic_Reset(this);
        gSaveContext.sceneLayer = (gSaveContext.save.cutsceneIndex & 0xF) + 1;

        // Set saved cutscene to 0 so it doesn't immediately play, but instead let the `CutsceneManager` handle it.
        gSaveContext.save.cutsceneIndex = 0;
    } else {
        gSaveContext.sceneLayer = 0;
    }

    sceneLayer = gSaveContext.sceneLayer;

    Play_SpawnScene(this, Entrance_GetSceneIdAbsolute(gSaveContext.save.entrance + gSaveContext.sceneLayer),
        Entrance_GetSpawnNum(gSaveContext.save.entrance + gSaveContext.sceneLayer));
    KaleidoScopeCall_Init(this);
    Interface_Init(this);

    // if(scene == ENTR_SCENE_MOUNTAIN_VILLAGE_SPRING || scene == ENTR_SCENE_MOUNTAIN_VILLAGE_WINTER)
    // {
    //     // Fault_AddHangupPrintfAndCrash("SHIT");
    //     Chaos_RandomizeMountainVillageClimb(this);
    // }

    if (gSaveContext.nextDayTime != NEXT_TIME_NONE) {
        if (gSaveContext.nextDayTime == NEXT_TIME_DAY) {
            gSaveContext.save.day++;
            gSaveContext.save.eventDayCount++;
            gSaveContext.dogIsLost = true;
            gSaveContext.nextDayTime = NEXT_TIME_DAY_SET;
        } else {
            gSaveContext.nextDayTime = NEXT_TIME_NIGHT_SET;
        }
    }

    Play_InitMotionBlur();

    R_PAUSE_BG_PRERENDER_STATE = PAUSE_BG_PRERENDER_OFF;
    R_PICTO_PHOTO_STATE = PICTO_PHOTO_STATE_OFF;

    PreRender_Init(&this->pauseBgPreRender);
    PreRender_SetValuesSave(&this->pauseBgPreRender, gCfbWidth, gCfbHeight, NULL, NULL, NULL);
    PreRender_SetValues(&this->pauseBgPreRender, gCfbWidth, gCfbHeight, NULL, NULL);

    this->unk_18E64 = gWorkBuffer;
    this->pictoPhotoI8 = gHiBuffer.pictoPhotoI8;
    this->unk_18E68 = gHiBuffer.D_80784600;
    this->unk_18E58 = gHiBuffer.D_80784600;
    this->unk_18E60 = gHiBuffer.D_80784600;
    gTransitionTileState = TRANS_TILE_OFF;
    this->transitionMode = TRANS_MODE_OFF;
    D_801D0D54 = false;

    FrameAdvance_Init(&this->frameAdvCtx);
    Rand_Seed(osGetTime());
    Matrix_Init(&this->state);

    this->state.main = Play_Main;
    this->state.destroy = Play_Destroy;

    this->transitionTrigger = TRANS_TRIGGER_END;
    this->worldCoverAlpha = 0;
    this->bgCoverAlpha = 0;
    this->haltAllActors = false;
    this->soaringCsOrSoTCsPlaying = false;

    if (gSaveContext.gameMode != GAMEMODE_TITLE_SCREEN) {
        if (gSaveContext.nextTransitionType == TRANS_NEXT_TYPE_DEFAULT) {
            this->transitionType = (Entrance_GetTransitionFlags(gSaveContext.save.entrance + sceneLayer) >> 7) & 0x7F;
        } else {
            this->transitionType = gSaveContext.nextTransitionType;
            gSaveContext.nextTransitionType = TRANS_NEXT_TYPE_DEFAULT;
        }
    } else {
        this->transitionType = TRANS_TYPE_FADE_BLACK;
    }

    TransitionFade_Init(&this->unk_18E48);
    TransitionFade_SetType(&this->unk_18E48, TRANS_INSTANCE_TYPE_FADE_FLASH);
    TransitionFade_SetColor(&this->unk_18E48, RGBA8(160, 160, 160, 255));
    TransitionFade_Start(&this->unk_18E48);
    VisMono_Init(&sPlayVisMono);

    gPlayVisMonoColor.a = 0;
    sPlayVisFbufInstance = &sPlayVisFbuf;
    VisFbuf_Init(sPlayVisFbufInstance);
    sPlayVisFbufInstance->lodProportion = 0.0f;
    sPlayVisFbufInstance->mode = VIS_FBUF_MODE_GENERAL;
    sPlayVisFbufInstance->primColor.r = 0;
    sPlayVisFbufInstance->primColor.g = 0;
    sPlayVisFbufInstance->primColor.b = 0;
    sPlayVisFbufInstance->primColor.a = 0;
    sPlayVisFbufInstance->envColor.r = 0;
    sPlayVisFbufInstance->envColor.g = 0;
    sPlayVisFbufInstance->envColor.b = 0;
    sPlayVisFbufInstance->envColor.a = 0;
    CutsceneFlags_UnsetAll(this);

    THA_GetRemaining(&this->state.tha);
    zAllocSize = THA_GetRemaining(&this->state.tha);
    zAlloc = (uintptr_t)THA_AllocTailAlign16(&this->state.tha, zAllocSize);

    ZeldaArena_Init((void*)((zAlloc + 0xf) & ~0xF), (zAllocSize - ((zAlloc + 0xf) & ~0xF)) + zAlloc);

    Actor_InitContext(this, &this->actorCtx, this->linkActorEntry);
    // Object_InitObjectTableFaultClient(&this->objectCtx);

    while (!Room_ProcessRoomRequest(this, &this->roomCtx)) {}

    if ((CURRENT_DAY != 0) &&
        ((this->roomCtx.curRoom.type == ROOM_TYPE_DUNGEON) || (this->roomCtx.curRoom.type == ROOM_TYPE_BOSS))) {
        Actor_Spawn(&this->actorCtx, this, ACTOR_EN_TEST4, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0);
    }

    player = GET_PLAYER(this);

    Camera_InitFocalActorSettings(&this->mainCamera, &player->actor);
    gDbgCamEnabled = false;

    if (PLAYER_GET_BG_CAM_INDEX(&player->actor) != 0xFF) {
        Camera_ChangeActorCsCamIndex(&this->mainCamera, PLAYER_GET_BG_CAM_INDEX(&player->actor));
    }

    if(Chaos_IsCodeInActiveList(CHAOS_CODE_BAD_CONNECTION) && 
        gChaosContext.link.bad_connection_mode == CHAOS_BAD_CONNECTION_ROLLBACK)
    {
        /* force creating a new snapshot after a transition */
        // gChaosContext.link.snapshot_timer = 0;
        // gChaosContext.link.bad_connection_timer = 2;
        // bzero(&gChaosContext.link.player_snapshot, sizeof(gChaosContext.link.player_snapshot));
        // bzero(&gChaosContext.link.child_snapshot, sizeof(gChaosContext.link.child_snapshot));
        // bzero(&gChaosContext.link.arrow_snapshot, sizeof(gChaosContext.link.arrow_snapshot));
        // bzero(&gChaosContext.link.parent_snapshot, sizeof(gChaosContext.link.parent_snapshot));
        Chaos_NukeSnapshots();
    }

    gChaosContext.env.wind_actor = NULL;
    
    CutsceneManager_StoreCamera(&this->mainCamera);
    Interface_SetSceneRestrictions(this);
    Cutscene_HandleEntranceTriggers(this);
    Environment_PlaySceneSequence(this);
    gSaveContext.seqId = this->sceneSequences.seqId;
    gSaveContext.ambienceId = this->sceneSequences.ambienceId;
    AnimTaskQueue_Update(this, &this->animTaskQueue);
    // Cutscene_HandleEntranceTriggers(this);
    gSaveContext.respawnFlag = 0;
    sBombersNotebookOpen = false;
    BombersNotebook_Init(&sBombersNotebook);

    if(gChaosContext.screen_slayer == CHAOS_SCREEN_SLAYER_STATE_WAIT_FOR_TRANSITION)
    {
        gChaosContext.screen_slayer = CHAOS_SCREEN_SLAYER_STATE_READY;
    }

    // cur_cs_id = CutsceneManager_GetCurrentCsId();
    // override_cutscene = /* this->csCtx.state == CS_STATE_IDLE || */ cur_cs_id == CS_ID_NONE;

    // for(index = 0; index < ARRAY_COUNT(sOverridePlayerCsIds); index++)
    // {
    //     override_cutscene |= cur_cs_id == this->playerCsIds[sOverridePlayerCsIds[index]];
    // }

    // if(Environment_IsFinalHours(this) && override_cutscene)
    // {
    //     if ((this->sceneId != SCENE_00KEIKOKU || gSaveContext.sceneLayer != 1) /* && !was_playing_last_hours*/ )
    //     {
    //         // SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_AMBIENCE, 0);
    //         // // SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0);
    //         // if(this->sceneSequences.seqId != NA_BGM_GENERAL_SFX)
    //         // {
    //             // AudioSeq_ProcessSeqCmds();
    //             // AudioSeq_ResetActiveSequences();
    //         // }
    //         Audio_ClearObjSoundMainBgmSeqId();
    //         Audio_ClearPrevMainBgmSeqId();
    //         Audio_PlaySceneSequence(NA_BGM_FINAL_HOURS, CURRENT_DAY - 1);
    //         // Audio_StartSceneSequence(NA_BGM_FINAL_HOURS);
    //         // SEQCMD_SET_SEQPLAYER_IO(SEQ_PLAYER_BGM_MAIN, 4, CURRENT_DAY - 1);
    //     }
    // }
}
