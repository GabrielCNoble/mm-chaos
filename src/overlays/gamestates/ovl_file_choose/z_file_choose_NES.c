/*
 * File: z_file_choose_NES.c
 * Overlay: ovl_file_choose
 * Description:
 */

#include "z_file_select.h"
#include "overlays/gamestates/ovl_opening/z_opening.h"
#include "z64rumble.h"
#include "z64save.h"
#include "z64shrink_window.h"
#include "z64view.h"
#include "assets/interface/parameter_static/parameter_static.h"
#include "assets/misc/title_static/title_static.h"
#include "assets/interface/message_static/message_static.h"
#include "chaos_fuckery.h"
#include "libc/string.h"
#include "fault.h"


s32 D_808144F10 = 100;
f32 D_808144F14 = 8.0f;
f32 D_808144F18 = 100.0f;
s32 D_808144F1C = 0;
f32 wave_angle = 0;

extern struct ChaosConfig gChaosConfigs[];

static Gfx sScreenFillSetupDL[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetOtherMode(G_AD_DISABLE | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE |
                         G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_1PRIMITIVE,
                     G_AC_NONE | G_ZS_PIXEL | G_RM_CLD_SURF | G_RM_CLD_SURF2),
    gsDPSetCombineMode(G_CC_PRIMITIVE, G_CC_PRIMITIVE),
    gsSPEndDisplayList(),
};

s16 sFileInfoBoxPartWidths[] = {
    36, 36, 36, 36, 24, 28, 28,
};
 
struct ChaosConfigMenuPartDef
{
    // s16 tex_u;
    // s16 tex_v;

    struct
    {
        s16 tex_u;
        s16 tex_v;

    } tex_coords[4];

    s16     width;
} sChaosConfigMenuDef[] = {
    /* top half */
    {
        {
            {TC_10_5(0, 0),  TC_10_5(0, 0)},
            {TC_10_5(35, 0), TC_10_5(0, 0)},
            {TC_10_5(0, 0),  TC_10_5(40, 0)},
            {TC_10_5(35, 0), TC_10_5(40, 0)}
        },
        36
    },

    {
        {
            {TC_10_5(0, 0),  TC_10_5(0, 0)},
            {TC_10_5(35, 0), TC_10_5(0, 0)},
            {TC_10_5(0, 0),  TC_10_5(40, 0)},
            {TC_10_5(35, 0), TC_10_5(40, 0)}
        },
        36
    },

    {
        {
            {TC_10_5(0, 0),  TC_10_5(0, 0)},
            {TC_10_5(35, 0), TC_10_5(0, 0)},
            {TC_10_5(0, 0),  TC_10_5(40, 0)},
            {TC_10_5(35, 0), TC_10_5(40, 0)}
        },
        36
    },

    {
        {
            {TC_10_5(34, 0),  TC_10_5(0, 0)},
            {TC_10_5(35, 0), TC_10_5(0, 0)},
            {TC_10_5(34, 0),  TC_10_5(40, 0)},
            {TC_10_5(35, 0), TC_10_5(40, 0)}
        },
        36
    },

    {
        {
            {TC_10_5(0, 0),  TC_10_5(0, 0)},
            {TC_10_5(35, 0), TC_10_5(0, 0)},
            {TC_10_5(0, 0),  TC_10_5(40, 0)},
            {TC_10_5(35, 0), TC_10_5(40, 0)}
        },
        36
    },

    {
        {
            {TC_10_5(0, 0),  TC_10_5(0, 0)},
            {TC_10_5(23, 0), TC_10_5(0, 0)},
            {TC_10_5(0, 0),  TC_10_5(40, 0)},
            {TC_10_5(23, 0), TC_10_5(40, 0)}
        },
        24
    },




    /* bottom half */
    {
        {
            {TC_10_5(0, 0),  TC_10_5(40, 0)},
            {TC_10_5(35, 0), TC_10_5(40, 0)},
            {TC_10_5(0, 0),  TC_10_5(20, 0)},
            {TC_10_5(35, 0), TC_10_5(20, 0)}
        },
        36
    },

    {
        {
            {TC_10_5(0, 0),  TC_10_5(40, 0)},
            {TC_10_5(35, 0), TC_10_5(40, 0)},
            {TC_10_5(0, 0),  TC_10_5(20, 0)},
            {TC_10_5(35, 0), TC_10_5(20, 0)}
        },
        36
    },

    {
        {
            {TC_10_5(0, 0),  TC_10_5(40, 0)},
            {TC_10_5(35, 0), TC_10_5(40, 0)},
            {TC_10_5(0, 0),  TC_10_5(20, 0)},
            {TC_10_5(35, 0), TC_10_5(20, 0)}
        },
        36
    },

    {
        {
            {TC_10_5(34, 0),  TC_10_5(40, 0)},
            {TC_10_5(35, 0), TC_10_5(40, 0)},
            {TC_10_5(34, 0),  TC_10_5(20, 0)},
            {TC_10_5(35, 0), TC_10_5(20, 0)}
        },
        36
    },

    {
        {
            {TC_10_5(0, 0),  TC_10_5(40, 0)},
            {TC_10_5(35, 0), TC_10_5(40, 0)},
            {TC_10_5(0, 0),  TC_10_5(20, 0)},
            {TC_10_5(35, 0), TC_10_5(20, 0)}
        },
        36
    },

    {
        {
            {TC_10_5(0, 0),  TC_10_5(40, 0)},
            {TC_10_5(23, 0), TC_10_5(40, 0)},
            {TC_10_5(0, 0),  TC_10_5(20, 0)},
            {TC_10_5(23, 0), TC_10_5(20, 0)}
        },
        24
    },
};

// s16 sChaosConfigBoxPartWidths[] = {
//     36, 36, 36, 36, 36, 24
// };

s16 sWindowContentColors[] = { 100, 150, 255 };

s16 sFileSelectSkyboxRotation = 0;

s16 sWalletFirstDigit[] = {
    1, // tens (Default Wallet)
    0, // hundreds (Adult Wallet)
    0, // hundreds (Giant Wallet)
};

void FileSelect_IncrementConfigMode(FileSelectState* this) {
    this->configMode++;
}

void FileSelect_Noop1(void) {
}

void FileSelect_Noop2(FileSelectState* this) {
}

void FileSelect_InitModeUpdate(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    if (this->configMode == CM_FADE_IN_START) {
        if (gSaveContext.options.optionId != 0xA51D) { // Magic number?
            this->configMode++;
        } else {
            this->menuMode = FS_MENU_MODE_CONFIG;
            this->configMode = CM_FADE_IN_START;
            this->titleLabel = FS_TITLE_SELECT_FILE;
            this->nextTitleLabel = FS_TITLE_OPEN_FILE;
        }
    }

    if (this->configMode == CM_FADE_IN_END) {
        this->screenFillAlpha -= 40;
        if (this->screenFillAlpha <= 0) {
            this->screenFillAlpha = 0;
            this->configMode++; // CM_MAIN_MENU
        }
    } else if (this->configMode == CM_MAIN_MENU) {
        FileSelect_IncrementConfigMode(this); // CM_SETUP_COPY_SOURCE
    } else {
        this->screenFillAlpha += 40;
        if (this->screenFillAlpha >= 255) {
            this->screenFillAlpha = 255;
            this->menuMode = FS_MENU_MODE_CONFIG;
            this->configMode = CM_FADE_IN_START;
            this->titleLabel = FS_TITLE_SELECT_FILE;
            this->nextTitleLabel = FS_TITLE_OPEN_FILE;
        }
    }
}

void FileSelect_InitModeDraw(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    Gfx_SetupDL39_Opa(this->state.gfxCtx);
    FileSelect_Noop2(this);
}

void FileSelect_SetView(FileSelectState* this, f32 eyeX, f32 eyeY, f32 eyeZ) {
    Vec3f eye;
    Vec3f lookAt;
    Vec3f up;

    eye.x = eyeX;
    eye.y = eyeY;
    eye.z = eyeZ;

    lookAt.x = lookAt.y = lookAt.z = 0.0f;

    up.x = up.z = 0.0f;
    up.y = 1.0f;

    View_LookAt(&this->view, &eye, &lookAt, &up);
    View_Apply(&this->view, VIEW_ALL | VIEW_FORCE_VIEWING | VIEW_FORCE_VIEWPORT | VIEW_FORCE_PROJECTION_PERSPECTIVE);
}

Gfx* FileSelect_DrawTexQuadIA8(Gfx* gfx, TexturePtr texture, s16 width, s16 height, s16 point) {
    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, width, height, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSP1Quadrangle(gfx++, point, point + 2, point + 3, point + 1, 0);

    return gfx;
}

/**
 * Fade in the menu window and title label.
 * If a file is occupied fade in the name, name box, and connector.
 * Fade in the copy erase and options button according to the window alpha.
 */
void FileSelect_FadeInMenuElements(FileSelectState* this) {
    SramContext* sramCtx = &this->sramCtx;
    s16 i;

    this->titleAlpha[FS_TITLE_CUR] += 20;
    this->windowAlpha += 16;

    for (i = 0; i < 3; i++) {
        this->fileButtonAlpha[i] = this->windowAlpha;

        if (!gSaveContext.flashSaveAvailable) {
            if (NO_FLASH_SLOT_OCCUPIED(sramCtx, i)) {
                this->nameBoxAlpha[i] = this->nameAlpha[i] = this->windowAlpha;
                this->connectorAlpha[i] += 20;
                if (this->connectorAlpha[i] >= 255) {
                    this->connectorAlpha[i] = 255;
                }
            }
        } else if (SLOT_OCCUPIED(this, i)) {
            this->nameBoxAlpha[i] = this->nameAlpha[i] = this->windowAlpha;
            this->connectorAlpha[i] += 20;

            if (this->connectorAlpha[i] >= 255) {
                this->connectorAlpha[i] = 255;
            }
        }
    }

    this->actionButtonAlpha[FS_BTN_ACTION_COPY] = this->actionButtonAlpha[FS_BTN_ACTION_ERASE] =
        this->optionButtonAlpha = this->windowAlpha;
}

/**
 * Converts a numerical value to hundreds-tens-ones digits
 */
void FileSelect_SplitNumber(u16 value, u16* hundreds, u16* tens, u16* ones) {
    *hundreds = 0;
    *tens = 0;
    *ones = value;

    do {
        if ((*ones - 100) < 0) {
            break;
        }
        (*hundreds)++;
        *ones -= 100;
    } while (true);

    do {
        if ((*ones - 10) < 0) {
            break;
        }
        (*tens)++;
        *ones -= 10;
    } while (true);
}

// Start of Config Mode Update Functions

/**
 * Reduce the alpha of the black screen fill to create a fade in effect.
 * Additionally, slide the window from the right to the center of the screen.
 * Update function for `CM_FADE_IN_START`
 */
void FileSelect_StartFadeIn(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    FileSelect_FadeInMenuElements(this);
    this->screenFillAlpha -= 40;
    this->windowPosX -= 20;

    if (this->windowPosX <= -94) {
        this->windowPosX = -94;
        this->configMode = CM_FADE_IN_END;
        this->screenFillAlpha = 0;
    }
}

/**
 * Finish fading in the remaining menu elements.
 * Fade in the controls text at the bottom of the screen.
 * Update function for `CM_FADE_IN_END`
 */
void FileSelect_FinishFadeIn(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->controlsAlpha += 20;
    FileSelect_FadeInMenuElements(this);

    if (this->titleAlpha[FS_TITLE_CUR] >= 255) {
        this->titleAlpha[FS_TITLE_CUR] = 255;
        this->controlsAlpha = 255;
        this->windowAlpha = 200;
        this->configMode = CM_MAIN_MENU;
    }
}

u8 sEmptyName[] = { 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E };

/**
 * Update the cursor and wait for the player to select a button to change menus accordingly.
 * If an empty file is selected, enter the name entry config mode.
 * If an occupied file is selected, enter the `Select` menu mode.
 * If copy, erase, or options is selected, set config mode accordingly.
 * Lastly, set any warning labels if appropriate.
 * Update function for `CM_MAIN_MENU`
 */
void FileSelect_UpdateMainMenu(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;
    Input* input = CONTROLLER1(&this->state);

    if (CHECK_BTN_ALL(input->press.button, BTN_START) || CHECK_BTN_ALL(input->press.button, BTN_A)) {
        if (this->buttonIndex <= FS_BTN_MAIN_FILE_3) {
            struct save_info_t *save_info = &this->save_info[this->buttonIndex];
            if (!gSaveContext.flashSaveAvailable) {
                if (!NO_FLASH_SLOT_OCCUPIED(sramCtx, this->buttonIndex)) {
                    Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
                    this->configMode = CM_ROTATE_TO_NAME_ENTRY;
                    this->kbdButton = FS_KBD_BTN_NONE;
                    this->charPage = FS_CHAR_PAGE_HIRA;
                    if (gSaveContext.options.language != LANGUAGE_JPN) {
                        this->charPage = FS_CHAR_PAGE_ENG;
                    }
                    this->kbdX = 0;
                    this->kbdY = 0;
                    this->charIndex = 0;
                    this->charBgAlpha = 0;
                    this->newFileNameCharCount = 0;
                    this->nameEntryBoxPosX = 120;
                    this->nameEntryBoxAlpha = 0;
                    Lib_MemCpy(&save_info->fileName, &sEmptyName, ARRAY_COUNT(sEmptyName));
                } else {
                    Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
                    this->actionTimer = 4;
                    this->selectMode = SM_FADE_MAIN_TO_SELECT;
                    this->selectedFileIndex = this->buttonIndex;
                    this->menuMode = FS_MENU_MODE_SELECT;
                    this->nextTitleLabel = FS_TITLE_OPEN_FILE;
                }
            } else if (!SLOT_OCCUPIED(this, this->buttonIndex)) {
                Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
                this->configMode = CM_ROTATE_TO_NAME_ENTRY;
                this->kbdButton = FS_KBD_BTN_NONE;
                this->charPage = FS_CHAR_PAGE_HIRA;
                if (gSaveContext.options.language != LANGUAGE_JPN) {
                    this->charPage = FS_CHAR_PAGE_ENG;
                }
                this->kbdX = 0;
                this->kbdY = 0;
                this->charIndex = 0;
                this->charBgAlpha = 0;
                this->newFileNameCharCount = 0;
                this->nameEntryBoxPosX = 120;
                this->nameEntryBoxAlpha = 0;
                Lib_MemCpy(&save_info->fileName, &sEmptyName, ARRAY_COUNT(sEmptyName));
            } else {
                Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
                this->actionTimer = 4;
                this->selectMode = SM_FADE_MAIN_TO_SELECT;
                this->selectedFileIndex = this->buttonIndex;
                this->menuMode = FS_MENU_MODE_SELECT;
                this->nextTitleLabel = FS_TITLE_OPEN_FILE;
            }
        } else if (this->warningLabel == FS_WARNING_NONE) {
            Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
            this->prevConfigMode = this->configMode;

            if (this->buttonIndex == FS_BTN_MAIN_COPY) {
                this->configMode = CM_SETUP_COPY_SOURCE;
                this->nextTitleLabel = FS_TITLE_COPY_FROM;
            } else if (this->buttonIndex == FS_BTN_MAIN_ERASE) {
                this->configMode = CM_SETUP_ERASE_SELECT;
                this->nextTitleLabel = FS_TITLE_ERASE_FILE;
            } else {
                this->configMode = CM_MAIN_TO_OPTIONS;
                this->kbdButton = FS_KBD_BTN_HIRA;
                this->kbdX = 0;
                this->kbdY = 0;
                this->charBgAlpha = 0;
                this->newFileNameCharCount = 0;
                this->nameEntryBoxPosX = 120;
            }
            this->actionTimer = 4;
        } else {
            Audio_PlaySfx(NA_SE_SY_FSEL_ERROR);
        }
    } else if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
        gSaveContext.gameMode = GAMEMODE_TITLE_SCREEN;
        STOP_GAMESTATE(&this->state);
        SET_NEXT_GAMESTATE(&this->state, TitleSetup_Init, sizeof(TitleSetupState));
    } else {
        if (ABS_ALT(this->stickAdjY) > 30) {
            Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
            if (this->stickAdjY > 30) {
                this->buttonIndex--;
                if (this->buttonIndex == FS_BTN_MAIN_FILE_3) {
                    this->buttonIndex = FS_BTN_MAIN_FILE_2;
                }
                if (this->buttonIndex < FS_BTN_MAIN_FILE_1) {
                    this->buttonIndex = FS_BTN_MAIN_OPTIONS;
                }
            } else {
                this->buttonIndex++;
                if (this->buttonIndex == FS_BTN_MAIN_FILE_3) {
                    this->buttonIndex = FS_BTN_MAIN_COPY;
                }
                if (this->buttonIndex > FS_BTN_MAIN_OPTIONS) {
                    this->buttonIndex = FS_BTN_MAIN_FILE_1;
                }
            }
        }

        if (this->buttonIndex == FS_BTN_MAIN_COPY) {
            if (!gSaveContext.flashSaveAvailable) {
                if (!NO_FLASH_SLOT_OCCUPIED(sramCtx, 0) && !NO_FLASH_SLOT_OCCUPIED(sramCtx, 1) &&
                    !NO_FLASH_SLOT_OCCUPIED(sramCtx, 2)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_FILE_COPY;
                    this->emptyFileTextAlpha = 255;
                } else if (NO_FLASH_SLOT_OCCUPIED(sramCtx, 0) && NO_FLASH_SLOT_OCCUPIED(sramCtx, 1) &&
                           NO_FLASH_SLOT_OCCUPIED(sramCtx, 2)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_EMPTY_FILES;
                    this->emptyFileTextAlpha = 255;
                } else {
                    this->warningLabel = FS_WARNING_NONE;
                }
            } else {
                if (!SLOT_OCCUPIED(this, 0) && !SLOT_OCCUPIED(this, 1) && !SLOT_OCCUPIED(this, 2)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_FILE_COPY;
                    this->emptyFileTextAlpha = 255;
                } else if (SLOT_OCCUPIED(this, 0) && SLOT_OCCUPIED(this, 1) && SLOT_OCCUPIED(this, 2)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_EMPTY_FILES;
                    this->emptyFileTextAlpha = 255;
                } else {
                    this->warningLabel = FS_WARNING_NONE;
                }
            }
        } else if (this->buttonIndex == FS_BTN_MAIN_ERASE) {
            if (!gSaveContext.flashSaveAvailable) {
                if (!NO_FLASH_SLOT_OCCUPIED(sramCtx, 0) && !NO_FLASH_SLOT_OCCUPIED(sramCtx, 1) &&
                    !NO_FLASH_SLOT_OCCUPIED(sramCtx, 2)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_FILE_ERASE;
                    this->emptyFileTextAlpha = 255;
                } else {
                    this->warningLabel = FS_WARNING_NONE;
                }
            } else {
                if (!SLOT_OCCUPIED(this, 0) && !SLOT_OCCUPIED(this, 1) && !SLOT_OCCUPIED(this, 2)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_FILE_ERASE;
                    this->emptyFileTextAlpha = 255;
                } else {
                    this->warningLabel = FS_WARNING_NONE;
                }
            }
        } else {
            this->warningLabel = FS_WARNING_NONE;
        }
    }
}

/**
 * Update function for `CM_UNUSED_31`
 */
void FileSelect_UnusedCM31(GameState* thisx) {
}

/**
 * Delay the next config mode from running until `D_80814564` reaches 254.
 * Because the timer increments by 2, the delay is 127 frames (assuming the value was not changed by reg editor).
 * Unused in the final game, was possibly used for debugging.
 * Update function for `CM_UNUSED_DELAY`
 */
void FileSelect_UnusedCMDelay(GameState* thisx) {
    static s16 D_80814564 = 0;
    FileSelectState* this = (FileSelectState*)thisx;

    D_80814564 += 2;

    if (D_80814564 == 254) {
        this->configMode = this->nextConfigMode;
        D_80814564 = 0;
    }
}

/**
 * Rotate the window from the main menu to the name entry menu.
 * Update function for `CM_ROTATE_TO_NAME_ENTRY`
 */
void FileSelect_RotateToNameEntry(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->windowRot += 50.0f;

    if (this->windowRot >= 314.0f) {
        this->windowRot = 314.0f;
        this->configMode = CM_START_NAME_ENTRY;
    }
}

/**
 * Rotate the window from the main menu to the options menu.
 * Update function for `CM_MAIN_TO_OPTIONS`
 */
void FileSelect_RotateToOptions(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->windowRot += 50.0f;

    if (this->windowRot >= 314.0f) {
        this->windowRot = 314.0f;
        this->configMode = CM_START_OPTIONS;
    }
}

/**
 * Rotate the window from the options menu to the main menu.
 * Update function for `CM_NAME_ENTRY_TO_MAIN` and `CM_OPTIONS_TO_MAIN`
 */
void FileSelect_RotateToMain(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->windowRot += 50.0f;

    if (this->windowRot >= 628.0f) {
        this->windowRot = 0.0f;
        this->configMode = CM_MAIN_MENU;
    }
}

void (*sConfigModeUpdateFuncs[])(GameState*) = {
    // Main Menu
    FileSelect_StartFadeIn,    // CM_FADE_IN_START
    FileSelect_FinishFadeIn,   // CM_FADE_IN_END
    FileSelect_UpdateMainMenu, // CM_MAIN_MENU
    // Copy File
    FileSelect_SetupCopySource,      // CM_SETUP_COPY_SOURCE
    FileSelect_SelectCopySource,     // CM_SELECT_COPY_SOURCE
    FileSelect_SetupCopyDest1,       // CM_SETUP_COPY_DEST_1
    FileSelect_SetupCopyDest2,       // CM_SETUP_COPY_DEST_2
    FileSelect_SelectCopyDest,       // CM_SELECT_COPY_DEST
    FileSelect_ExitToCopySource1,    // CM_EXIT_TO_COPY_SOURCE_1
    FileSelect_ExitToCopySource2,    // CM_EXIT_TO_COPY_SOURCE_2
    FileSelect_SetupCopyConfirm1,    // CM_SETUP_COPY_CONFIRM_1
    FileSelect_SetupCopyConfirm2,    // CM_SETUP_COPY_CONFIRM_2
    FileSelect_CopyConfirm,          // CM_COPY_CONFIRM
    FileSelect_CopyWaitForFlashSave, // CM_COPY_WAIT_FOR_FLASH_SAVE
    FileSelect_ReturnToCopyDest,     // CM_RETURN_TO_COPY_DEST
    FileSelect_CopyAnim1,            // CM_COPY_ANIM_1
    FileSelect_CopyAnim2,            // CM_COPY_ANIM_2
    FileSelect_CopyAnim3,            // CM_COPY_ANIM_3
    FileSelect_CopyAnim4,            // CM_COPY_ANIM_4
    FileSelect_CopyAnim5,            // CM_COPY_ANIM_5
    FileSelect_ExitCopyToMain,       // CM_COPY_RETURN_MAIN
    // Erase File
    FileSelect_SetupEraseSelect,      // CM_SETUP_ERASE_SELECT
    FileSelect_EraseSelect,           // CM_ERASE_SELECT
    FileSelect_SetupEraseConfirm1,    // CM_SETUP_ERASE_CONFIRM_1
    FileSelect_SetupEraseConfirm2,    // CM_SETUP_ERASE_CONFIRM_2
    FileSelect_EraseConfirm,          // CM_ERASE_CONFIRM
    FileSelect_ExitToEraseSelect1,    // CM_EXIT_TO_ERASE_SELECT_1
    FileSelect_ExitToEraseSelect2,    // CM_EXIT_TO_ERASE_SELECT_2
    FileSelect_EraseAnim1,            // CM_ERASE_ANIM_1
    FileSelect_EraseWaitForFlashSave, // CM_ERASE_WAIT_FOR_FLASH_SAVE
    FileSelect_EraseAnim2,            // CM_ERASE_ANIM_2
    FileSelect_EraseAnim3,            // CM_ERASE_ANIM_3
    FileSelect_ExitEraseToMain,       // CM_EXIT_ERASE_TO_MAIN
    FileSelect_UnusedCM31,            // CM_UNUSED_31
    // New File Name Entry
    FileSelect_RotateToNameEntry,         // CM_ROTATE_TO_NAME_ENTRY
    FileSelect_StartNameEntry,            // CM_START_NAME_ENTRY
    FileSelect_UpdateKeyboardCursor,      // CM_NAME_ENTRY
    FileSelect_NameEntryWaitForFlashSave, // CM_NAME_ENTRY_WAIT_FOR_FLASH_SAVE
    FileSelect_RotateToMain,              // CM_NAME_ENTRY_TO_MAIN
    // Options
    FileSelect_RotateToOptions,         // CM_MAIN_TO_OPTIONS
    FileSelect_StartOptions,            // CM_START_OPTIONS
    FileSelect_UpdateOptionsMenu,       // CM_OPTIONS_MENU
    FileSelect_OptionsWaitForFlashSave, // CM_OPTIONS_WAIT_FOR_FLASH_SAVE
    FileSelect_RotateToMain,            // CM_OPTIONS_TO_MAIN
    // Possible Debug
    FileSelect_UnusedCMDelay, // CM_UNUSED_DELAY
};

s16 sCursorAlphaTargets[] = { 70, 200 };

/**
 * Updates the alpha of the cursor to make it pulsate.
 */
void FileSelect_PulsateCursor(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    s32 alphaStep =
        ABS_ALT(this->highlightColor[3] - sCursorAlphaTargets[this->highlightPulseDir]) / this->highlightTimer;

    if (this->highlightColor[3] >= sCursorAlphaTargets[this->highlightPulseDir]) {
        this->highlightColor[3] -= alphaStep;
    } else {
        this->highlightColor[3] += alphaStep;
    }

    this->highlightTimer--;

    if (this->highlightTimer == 0) {
        this->highlightColor[3] = sCursorAlphaTargets[this->highlightPulseDir];
        this->highlightTimer = 20;
        this->highlightPulseDir ^= 1;
    }
}

void FileSelect_ConfigModeUpdate(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    sConfigModeUpdateFuncs[this->configMode](&this->state);
}

void FileSelect_SetWindowVtx(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    s16 i;
    s16 j;
    s16 x;
    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    this->windowVtx = GRAPH_ALLOC(this->state.gfxCtx, 80 * sizeof(Vtx));
    tmp = this->windowPosX - 90;

    for (x = 0, i = 0; i < 4; i++) {
        tmp += 0x40;
        tmp2 = (i == 3) ? 0x30 : 0x40;

        for (j = 0, tmp3 = 0x50; j < 5; j++, x += 4, tmp3 -= 0x20) {
            this->windowVtx[x].v.ob[0] = this->windowVtx[x + 2].v.ob[0] = tmp;

            this->windowVtx[x + 1].v.ob[0] = this->windowVtx[x + 3].v.ob[0] = tmp2 + tmp;

            this->windowVtx[x].v.ob[1] = this->windowVtx[x + 1].v.ob[1] = tmp3;

            this->windowVtx[x + 2].v.ob[1] = this->windowVtx[x + 3].v.ob[1] = tmp3 - 0x20;

            this->windowVtx[x].v.ob[2] = this->windowVtx[x + 1].v.ob[2] = this->windowVtx[x + 2].v.ob[2] =
                this->windowVtx[x + 3].v.ob[2] = 0;

            this->windowVtx[x].v.flag = this->windowVtx[x + 1].v.flag = this->windowVtx[x + 2].v.flag =
                this->windowVtx[x + 3].v.flag = 0;

            this->windowVtx[x].v.tc[0] = this->windowVtx[x].v.tc[1] = this->windowVtx[x + 1].v.tc[1] =
                this->windowVtx[x + 2].v.tc[0] = 0;

            this->windowVtx[x + 1].v.tc[0] = this->windowVtx[x + 3].v.tc[0] = tmp2 << 5;

            this->windowVtx[x + 2].v.tc[1] = this->windowVtx[x + 3].v.tc[1] = 32 << 5;

            this->windowVtx[x].v.cn[0] = this->windowVtx[x + 2].v.cn[0] = this->windowVtx[x].v.cn[1] =
                this->windowVtx[x + 2].v.cn[1] = this->windowVtx[x].v.cn[2] = this->windowVtx[x + 2].v.cn[2] =
                    this->windowVtx[x + 1].v.cn[0] = this->windowVtx[x + 3].v.cn[0] = this->windowVtx[x + 1].v.cn[1] =
                        this->windowVtx[x + 3].v.cn[1] = this->windowVtx[x + 1].v.cn[2] =
                            this->windowVtx[x + 3].v.cn[2] = this->windowVtx[x].v.cn[3] =
                                this->windowVtx[x + 2].v.cn[3] = this->windowVtx[x + 1].v.cn[3] =
                                    this->windowVtx[x + 3].v.cn[3] = 255;
        }
    }
}

s16 D_80814620[] = { 8, 8, 8, 0 };
s16 D_80814628[] = { 12, 12, 12, 0 };
s16 D_80814630[] = { 12, 12, 12, 0 };
s16 D_80814638[] = { 88, 104, 120, 940, 944, 948 };
s16 D_80814644[] = { 88, 104, 120, 944 };
s16 D_8081464C[] = { 940, 944 };

/* confirm button vertex offset */
s16 D_80814650[] = { 940, 944, 948 };

/*
 * fileSelect->windowContentVtx[0]    -> Title Label (4)
 *
 * fileSelect->windowContentVtx[4]    -> File 1 InfoBox (28)
 * fileSelect->windowContentVtx[32]   -> File 2 InfoBox (28)
 * fileSelect->windowContentVtx[60]   -> File 3 InfoBox (28)
 *
 * ** FILE 1 **
 *
 * fileSelect->windowContentVtx[88]   -> File Button
 * fileSelect->windowContentVtx[92]   -> File Name Box
 * fileSelect->windowContentVtx[96]   -> Connectors
 * fileSelect->windowContentVtx[100]  -> Blank Button (Owl Save)
 *
 * ** FILE 2 **
 *
 * fileSelect->windowContentVtx[104]  -> Repeat of File 1 above
 *
 * ** FILE 3 **
 *
 * fileSelect->windowContentVtx[120]  -> Repeat of File 1 above
 *
 * ** FILE 1 Info **
 *
 * fileSelect->windowContentVtx[136]  -> File Name (32)
 * fileSelect->windowContentVtx[168]  -> File Name Shadow (32)
 * fileSelect->windowContentVtx[200]  -> Rupee Digits (12)
 * fileSelect->windowContentVtx[212]  -> Rupee Digits Shadow (12)
 * fileSelect->windowContentVtx[224]  -> Mask Count (8)
 * fileSelect->windowContentVtx[232]  -> Mask Count Shadow (8)
 * fileSelect->windowContentVtx[240]  -> Hearts (80)
 * fileSelect->windowContentVtx[320]  -> Remains (16)
 * fileSelect->windowContentVtx[336]  -> Rupee Icon (4)
 * fileSelect->windowContentVtx[340]  -> Heart Piece Count (4)
 * fileSelect->windowContentVtx[344]  -> Mask Text (8)
 * fileSelect->windowContentVtx[352]  -> Owl Save Icon (4)
 * fileSelect->windowContentVtx[356]  -> Day Text (8)
 * fileSelect->windowContentVtx[364]  -> Time Digits (20)
 * fileSelect->windowContentVtx[384]  -> Time Digits Shadow (20)
 *
 * ** FILE 2 Info **
 *
 * fileSelect->windowContentVtx[404]  -> Repeat of File 1 above
 *
 * ** FILE 3 Info **
 *
 * fileSelect->windowContentVtx[672]  -> Repeat of File 1 above
 *
 * fileSelect->windowContentVtx[940]  -> Action buttons (copy/erase/yes/quit)
 * fileSelect->windowContentVtx[944]  -> Action buttons (copy/erase/yes/quit)
 * fileSelect->windowContentVtx[948]  -> Option Button
 * fileSelect->windowContentVtx[952]  -> Highlight over currently selected button
 * fileSelect->windowContentVtx[956]  -> Warning labels
 */

void FileSelect_SetWindowContentVtx(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    Vtx *chaos_config_vtx;
    u16 vtxId;
    s16 file_index;
    s16 digit_index;
    s16 heart_index;
    s16 mask_index;
    s16 remains_index;
    s16 button_index;
    s16 option_index;
    s16 box_part;
    s16 j;
    s16 i;
    s16 spAC;
    u16 spA4[3];
    u16* ptr;
    s32 posY;
    s32 relPosY;
    s32 tempPosY;
    s32 posX;
    s32 index;

    // this->windowContentVtx = GRAPH_ALLOC(this->state.gfxCtx, FILE_SELECT_WINDOW_CONTENT_TOTAL_VERT_COUNT * sizeof(Vtx));
    this->ui_contents = GRAPH_ALLOC(this->state.gfxCtx, sizeof(union FileSelectUI));

    for(index = 0; index < sizeof(union FileSelectUI) / sizeof(Vtx); index += 4)
    {
        this->ui_contents->vertices[index + 0].v.ob[0] = 0x12c;
        this->ui_contents->vertices[index + 0].v.ob[1] = 0;
        this->ui_contents->vertices[index + 0].v.ob[2] = 0;
        this->ui_contents->vertices[index + 0].v.tc[0] = 0;
        this->ui_contents->vertices[index + 0].v.tc[1] = 0;
        this->ui_contents->vertices[index + 0].v.cn[0] = 255;
        this->ui_contents->vertices[index + 0].v.cn[1] = 255;
        this->ui_contents->vertices[index + 0].v.cn[2] = 255;
        this->ui_contents->vertices[index + 0].v.cn[3] = 255;

        this->ui_contents->vertices[index + 1].v.ob[0] = this->ui_contents->vertices[index + 0].v.ob[0] + 16;
        this->ui_contents->vertices[index + 1].v.ob[1] = 0;
        this->ui_contents->vertices[index + 1].v.ob[2] = 0;
        this->ui_contents->vertices[index + 1].v.tc[0] = 0x200;
        this->ui_contents->vertices[index + 1].v.tc[1] = 0;
        this->ui_contents->vertices[index + 1].v.cn[0] = 255;
        this->ui_contents->vertices[index + 1].v.cn[1] = 255;
        this->ui_contents->vertices[index + 1].v.cn[2] = 255;
        this->ui_contents->vertices[index + 1].v.cn[3] = 255;

        this->ui_contents->vertices[index + 2].v.ob[0] = 0x12c;
        this->ui_contents->vertices[index + 2].v.ob[1] = this->ui_contents->vertices[index + 1].v.ob[1] - 16;
        this->ui_contents->vertices[index + 2].v.ob[2] = 0;
        this->ui_contents->vertices[index + 2].v.tc[0] = 0;
        this->ui_contents->vertices[index + 2].v.tc[1] = 0x200;
        this->ui_contents->vertices[index + 2].v.cn[0] = 255;
        this->ui_contents->vertices[index + 2].v.cn[1] = 255;
        this->ui_contents->vertices[index + 2].v.cn[2] = 255;
        this->ui_contents->vertices[index + 2].v.cn[3] = 255;

        this->ui_contents->vertices[index + 3].v.ob[0] = this->ui_contents->vertices[index + 0].v.ob[0] + 16;
        this->ui_contents->vertices[index + 3].v.ob[1] = this->ui_contents->vertices[index + 0].v.ob[1] - 16;
        this->ui_contents->vertices[index + 3].v.ob[2] = 0;
        this->ui_contents->vertices[index + 3].v.tc[0] = 0x200;
        this->ui_contents->vertices[index + 3].v.tc[1] = 0x200;
        this->ui_contents->vertices[index + 3].v.cn[0] = 255;
        this->ui_contents->vertices[index + 3].v.cn[1] = 255;
        this->ui_contents->vertices[index + 3].v.cn[2] = 255;
        this->ui_contents->vertices[index + 3].v.cn[3] = 255;
    }

    // Initialize all windowContentVtx
    // for (vtxId = 0; vtxId < FILE_SELECT_WINDOW_CONTENT_TOTAL_VERT_COUNT; vtxId += 4) {
    //     // x-coord (left)
    //     this->windowContentVtx[vtxId + 0].v.ob[0] = 0x12C;
    //     this->windowContentVtx[vtxId + 0].v.ob[1] = 0;
    //     this->windowContentVtx[vtxId + 0].v.ob[2] = 0;

    //     this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 16;
    //     this->windowContentVtx[vtxId + 1].v.ob[1] = 0;
    //     this->windowContentVtx[vtxId + 1].v.ob[2] = 0;

    //     this->windowContentVtx[vtxId + 2].v.ob[0] = 0x12C;
    //     this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 16;
    //     this->windowContentVtx[vtxId + 2].v.ob[2] = 0;
        
    //     this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 16;
    //     this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 16;
    //     this->windowContentVtx[vtxId + 3].v.ob[2] = 0;

    //     // flag
    //     this->windowContentVtx[vtxId + 0].v.flag = 0;
    //     this->windowContentVtx[vtxId + 1].v.flag = 0;
    //     this->windowContentVtx[vtxId + 2].v.flag = 0;
    //     this->windowContentVtx[vtxId + 3].v.flag = 0;

    //     // texture coordinates
    //     this->windowContentVtx[vtxId + 0].v.tc[0] = 0;
    //     this->windowContentVtx[vtxId + 0].v.tc[1] = 0;

    //     this->windowContentVtx[vtxId + 1].v.tc[0] = 0x200;
    //     this->windowContentVtx[vtxId + 1].v.tc[1] = 0;

    //     this->windowContentVtx[vtxId + 2].v.tc[0] = 0;
    //     this->windowContentVtx[vtxId + 2].v.tc[1] = 0x200;

    //     this->windowContentVtx[vtxId + 3].v.tc[0] = 0x200;
    //     this->windowContentVtx[vtxId + 3].v.tc[1] = 0x200;

    //     // alpha
    //     this->windowContentVtx[vtxId + 0].v.cn[0] = 255;
    //     this->windowContentVtx[vtxId + 0].v.cn[1] = 255;
    //     this->windowContentVtx[vtxId + 0].v.cn[2] = 255;
    //     this->windowContentVtx[vtxId + 0].v.cn[3] = 255;

    //     this->windowContentVtx[vtxId + 1].v.cn[0] = 255;
    //     this->windowContentVtx[vtxId + 1].v.cn[1] = 255;
    //     this->windowContentVtx[vtxId + 1].v.cn[2] = 255;
    //     this->windowContentVtx[vtxId + 1].v.cn[3] = 255;

    //     this->windowContentVtx[vtxId + 2].v.cn[0] = 255;
    //     this->windowContentVtx[vtxId + 2].v.cn[1] = 255;
    //     this->windowContentVtx[vtxId + 2].v.cn[2] = 255;
    //     this->windowContentVtx[vtxId + 2].v.cn[3] = 255;

    //     this->windowContentVtx[vtxId + 3].v.cn[0] = 255;
    //     this->windowContentVtx[vtxId + 3].v.cn[1] = 255;
    //     this->windowContentVtx[vtxId + 3].v.cn[2] = 255;
    //     this->windowContentVtx[vtxId + 3].v.cn[3] = 255;
    // }

    /** Title Label **/

    /* top left */
    // this->windowContentVtx[0].v.ob[0] = this->windowPosX;
    // this->windowContentVtx[0].v.ob[1] = 0x48;

    // /* top right */
    // this->windowContentVtx[1].v.ob[0] = this->windowContentVtx[0].v.ob[0] + 0x80;
    // this->windowContentVtx[1].v.ob[1] = 0x48;
    // this->windowContentVtx[1].v.tc[0] = 0x1000;
    
    // /* bottom left */
    // this->windowContentVtx[2].v.ob[0] = this->windowPosX;
    // this->windowContentVtx[2].v.ob[1] = this->windowContentVtx[0].v.ob[1] - 0x10;

    // /* bottom right */
    // this->windowContentVtx[3].v.ob[0] = this->windowContentVtx[0].v.ob[0] + 0x80;
    // this->windowContentVtx[3].v.ob[1] = this->windowContentVtx[0].v.ob[1] - 0x10;
    // this->windowContentVtx[3].v.tc[0] = 0x1000;

    /* top left */
    this->ui_contents->title_label[0].v.ob[0] = this->windowPosX;
    this->ui_contents->title_label[0].v.ob[1] = 0x48;

    // /* top right */
    this->ui_contents->title_label[1].v.ob[0] = this->ui_contents->title_label[0].v.ob[0] + 0x80;
    this->ui_contents->title_label[1].v.ob[1] = 0x48;
    this->ui_contents->title_label[1].v.tc[0] = 0x1000;
    
    /* bottom left */
    this->ui_contents->title_label[2].v.ob[0] = this->windowPosX;
    this->ui_contents->title_label[2].v.ob[1] = this->ui_contents->title_label[0].v.ob[1] - 0x10;

    /* bottom right */
    this->ui_contents->title_label[3].v.ob[0] = this->ui_contents->title_label[0].v.ob[0] + 0x80;
    this->ui_contents->title_label[3].v.ob[1] = this->ui_contents->title_label[0].v.ob[1] - 0x10;
    this->ui_contents->title_label[3].v.tc[0] = 0x1000;


    /** File InfoBox **/

    // Loop through 3 files
    // for (vtxId = 4, i = 0; i < 1; i++) {
    vtxId = 4;
    posX = this->windowPosX - 6;

    // Loop through 7 textures
    for (box_part = 0; box_part < 7; box_part++, vtxId += 4) 
    {
        /* top-left corner */
        // this->windowContentVtx[vtxId + 0].v.ob[0] = posX;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = this->fileNamesY[0] + 0x2C;

        // /* top-right corner */
        // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + sFileInfoBoxPartWidths[j];
        // this->windowContentVtx[vtxId + 1].v.ob[1] = this->fileNamesY[0] + 0x2C;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = sFileInfoBoxPartWidths[j] << 5;

        // /* bottom-left corner */
        // this->windowContentVtx[vtxId + 2].v.ob[0] = posX;
        // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x38;
        // this->windowContentVtx[vtxId + 2].v.tc[1] = 0x700;
        
        // /* bottom-right corner */
        // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + sFileInfoBoxPartWidths[j];            
        // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x38;
        // this->windowContentVtx[vtxId + 3].v.tc[0] = sFileInfoBoxPartWidths[j] << 5;
        // this->windowContentVtx[vtxId + 3].v.tc[1] = 0x700;


        this->ui_contents->file_info_box[box_part][0].v.ob[0] = posX;
        this->ui_contents->file_info_box[box_part][0].v.ob[1] = this->fileNamesY[0] + 0x2C;
        /* top-right corner */
        this->ui_contents->file_info_box[box_part][1].v.ob[0] = this->ui_contents->file_info_box[box_part][0].v.ob[0] + sFileInfoBoxPartWidths[box_part];
        this->ui_contents->file_info_box[box_part][1].v.ob[1] = this->fileNamesY[0] + 0x2C;
        this->ui_contents->file_info_box[box_part][1].v.tc[0] = sFileInfoBoxPartWidths[box_part] << 5;
        /* bottom-left corner */
        this->ui_contents->file_info_box[box_part][2].v.ob[0] = posX;
        this->ui_contents->file_info_box[box_part][2].v.ob[1] = this->ui_contents->file_info_box[box_part][0].v.ob[1] - 0x38;
        this->ui_contents->file_info_box[box_part][2].v.tc[1] = 0x700;
        /* bottom-right corner */
        this->ui_contents->file_info_box[box_part][3].v.ob[0] = this->ui_contents->file_info_box[box_part][0].v.ob[0] + sFileInfoBoxPartWidths[box_part];            
        this->ui_contents->file_info_box[box_part][3].v.ob[1] = this->ui_contents->file_info_box[box_part][0].v.ob[1] - 0x38;
        this->ui_contents->file_info_box[box_part][3].v.tc[0] = sFileInfoBoxPartWidths[box_part] << 5;
        this->ui_contents->file_info_box[box_part][3].v.tc[1] = 0x700;

        // Update X position
        posX += sFileInfoBoxPartWidths[box_part];
    }

    posX = this->windowPosX - 6; 
    /* chaos options box (top portion) */
    for (box_part = 0; box_part < 6; box_part++, vtxId += 4) 
    {
        struct ChaosConfigMenuPartDef *menu_def = sChaosConfigMenuDef + box_part;
        /* top-left corner */
        // this->windowContentVtx[vtxId + 0].v.ob[0] = posX;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = this->fileNamesY[0] + 0x2C;
        // this->windowContentVtx[vtxId + 0].v.tc[0] = menu_def->tex_coords[0].tex_u;
        // this->windowContentVtx[vtxId + 0].v.tc[1] = menu_def->tex_coords[0].tex_v;
        // /* top-right corner */
        // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + menu_def->width;
        // this->windowContentVtx[vtxId + 1].v.ob[1] = this->fileNamesY[0] + 0x2C;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = menu_def->tex_coords[1].tex_u;
        // this->windowContentVtx[vtxId + 1].v.tc[1] = menu_def->tex_coords[1].tex_v;
        // /* bottom-left corner */
        // this->windowContentVtx[vtxId + 2].v.ob[0] = posX;
        // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x28;
        // this->windowContentVtx[vtxId + 2].v.tc[0] = menu_def->tex_coords[2].tex_u;
        // this->windowContentVtx[vtxId + 2].v.tc[1] = menu_def->tex_coords[2].tex_v;
        // /* bottom-right corner */
        // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + menu_def->width;
        // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x28;
        // this->windowContentVtx[vtxId + 3].v.tc[0] = menu_def->tex_coords[3].tex_u;
        // this->windowContentVtx[vtxId + 3].v.tc[1] = menu_def->tex_coords[3].tex_v;


        this->ui_contents->chaos_options_box_top[box_part][0].v.ob[0] = posX;
        this->ui_contents->chaos_options_box_top[box_part][0].v.ob[1] = this->fileNamesY[0] + 0x2C;
        this->ui_contents->chaos_options_box_top[box_part][0].v.tc[0] = menu_def->tex_coords[0].tex_u;
        this->ui_contents->chaos_options_box_top[box_part][0].v.tc[1] = menu_def->tex_coords[0].tex_v;
        /* top-right corner */
        this->ui_contents->chaos_options_box_top[box_part][1].v.ob[0] = this->ui_contents->chaos_options_box_top[box_part][0].v.ob[0] + menu_def->width;
        this->ui_contents->chaos_options_box_top[box_part][1].v.ob[1] = this->fileNamesY[0] + 0x2C;
        this->ui_contents->chaos_options_box_top[box_part][1].v.tc[0] = menu_def->tex_coords[1].tex_u;
        this->ui_contents->chaos_options_box_top[box_part][1].v.tc[1] = menu_def->tex_coords[1].tex_v;
        /* bottom-left corner */
        this->ui_contents->chaos_options_box_top[box_part][2].v.ob[0] = posX;
        this->ui_contents->chaos_options_box_top[box_part][2].v.ob[1] = this->ui_contents->chaos_options_box_top[box_part][0].v.ob[1] - 0x28;
        this->ui_contents->chaos_options_box_top[box_part][2].v.tc[0] = menu_def->tex_coords[2].tex_u;
        this->ui_contents->chaos_options_box_top[box_part][2].v.tc[1] = menu_def->tex_coords[2].tex_v;
        /* bottom-right corner */
        this->ui_contents->chaos_options_box_top[box_part][3].v.ob[0] = this->ui_contents->chaos_options_box_top[box_part][0].v.ob[0] + menu_def->width;
        this->ui_contents->chaos_options_box_top[box_part][3].v.ob[1] = this->ui_contents->chaos_options_box_top[box_part][0].v.ob[1] - 0x28;
        this->ui_contents->chaos_options_box_top[box_part][3].v.tc[0] = menu_def->tex_coords[3].tex_u;
        this->ui_contents->chaos_options_box_top[box_part][3].v.tc[1] = menu_def->tex_coords[3].tex_v;

        // Update X position
        posX += menu_def->width;
    }

    posX = this->windowPosX - 6; 
    /* chaos options box (bottom portion) */
    for (box_part = 0; box_part < 6; box_part++, vtxId += 4) 
    {
        struct ChaosConfigMenuPartDef *menu_def = sChaosConfigMenuDef + box_part + 6;

        // /* top-left corner */
        // this->windowContentVtx[vtxId + 0].v.ob[0] = posX;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = this->fileNamesY[0] + 4;
        // this->windowContentVtx[vtxId + 0].v.tc[0] = menu_def->tex_coords[0].tex_u;
        // this->windowContentVtx[vtxId + 0].v.tc[1] = menu_def->tex_coords[0].tex_v;
        // /* top-right corner */
        // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + menu_def->width;
        // this->windowContentVtx[vtxId + 1].v.ob[1] = this->fileNamesY[0] + 4;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = menu_def->tex_coords[1].tex_u;
        // this->windowContentVtx[vtxId + 1].v.tc[1] = menu_def->tex_coords[1].tex_v;
        // /* bottom-left corner */
        // this->windowContentVtx[vtxId + 2].v.ob[0] = posX;
        // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x46;
        // this->windowContentVtx[vtxId + 2].v.tc[0] = menu_def->tex_coords[2].tex_u;
        // this->windowContentVtx[vtxId + 2].v.tc[1] = menu_def->tex_coords[2].tex_v;
        // /* bottom-right corner */
        // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + menu_def->width;            
        // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x46;
        // this->windowContentVtx[vtxId + 3].v.tc[0] = menu_def->tex_coords[3].tex_u;
        // this->windowContentVtx[vtxId + 3].v.tc[1] = menu_def->tex_coords[3].tex_v;

        /* top-left corner */
        this->ui_contents->chaos_options_box_bottom[box_part][0].v.ob[0] = posX;
        this->ui_contents->chaos_options_box_bottom[box_part][0].v.ob[1] = this->fileNamesY[0] + 4;
        this->ui_contents->chaos_options_box_bottom[box_part][0].v.tc[0] = menu_def->tex_coords[0].tex_u;
        this->ui_contents->chaos_options_box_bottom[box_part][0].v.tc[1] = menu_def->tex_coords[0].tex_v;
        /* top-right corner */
        this->ui_contents->chaos_options_box_bottom[box_part][1].v.ob[0] = this->ui_contents->chaos_options_box_bottom[box_part][0].v.ob[0] + menu_def->width;
        this->ui_contents->chaos_options_box_bottom[box_part][1].v.ob[1] = this->fileNamesY[0] + 4;
        this->ui_contents->chaos_options_box_bottom[box_part][1].v.tc[0] = menu_def->tex_coords[1].tex_u;
        this->ui_contents->chaos_options_box_bottom[box_part][1].v.tc[1] = menu_def->tex_coords[1].tex_v;
        /* bottom-left corner */
        this->ui_contents->chaos_options_box_bottom[box_part][2].v.ob[0] = posX;
        this->ui_contents->chaos_options_box_bottom[box_part][2].v.ob[1] = this->ui_contents->chaos_options_box_bottom[box_part][0].v.ob[1] - 0x46;
        this->ui_contents->chaos_options_box_bottom[box_part][2].v.tc[0] = menu_def->tex_coords[2].tex_u;
        this->ui_contents->chaos_options_box_bottom[box_part][2].v.tc[1] = menu_def->tex_coords[2].tex_v;
        /* bottom-right corner */
        this->ui_contents->chaos_options_box_bottom[box_part][3].v.ob[0] = this->ui_contents->chaos_options_box_bottom[box_part][0].v.ob[0] + menu_def->width;            
        this->ui_contents->chaos_options_box_bottom[box_part][3].v.ob[1] = this->ui_contents->chaos_options_box_bottom[box_part][0].v.ob[1] - 0x46;
        this->ui_contents->chaos_options_box_bottom[box_part][3].v.tc[0] = menu_def->tex_coords[3].tex_u;
        this->ui_contents->chaos_options_box_bottom[box_part][3].v.tc[1] = menu_def->tex_coords[3].tex_v;

        // Update X position
        posX += menu_def->width;
    }

    vtxId += 8;

    // File Buttons

    posX = this->windowPosX - 6;
    posY = 44;

    // Loop through 3 files
    for (file_index = 0; file_index < 2; file_index++, vtxId += 16, posY -= 0x10) {

        /* File Button */

        // /* top-left corner */
        // this->windowContentVtx[vtxId + 0].v.ob[0] = posX;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = this->buttonYOffsets[j] + posY;
        
        // /* top-right corner */
        // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x40;
        // this->windowContentVtx[vtxId + 1].v.ob[1] = this->buttonYOffsets[j] + posY;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = 0x800;

        // /* bottom-left corner */
        // this->windowContentVtx[vtxId + 2].v.ob[0] = posX;
        // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x10;

        // /* bottom-right corner */
        // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x40;
        // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x10;        
        // this->windowContentVtx[vtxId + 3].v.tc[0] = 0x800;

        /* top-left corner */
        this->ui_contents->files[file_index].file_button[0].v.ob[0] = posX;
        this->ui_contents->files[file_index].file_button[0].v.ob[1] = this->buttonYOffsets[file_index] + posY;
        /* top-right corner */
        this->ui_contents->files[file_index].file_button[1].v.ob[0] = this->ui_contents->files[file_index].file_button[0].v.ob[0] + 0x40;
        this->ui_contents->files[file_index].file_button[1].v.ob[1] = this->buttonYOffsets[file_index] + posY;
        this->ui_contents->files[file_index].file_button[1].v.tc[0] = 0x800;
        /* bottom-left corner */
        this->ui_contents->files[file_index].file_button[2].v.ob[0] = posX;
        this->ui_contents->files[file_index].file_button[2].v.ob[1] = this->ui_contents->files[file_index].file_button[0].v.ob[1] - 0x10;
        /* bottom-right corner */
        this->ui_contents->files[file_index].file_button[3].v.ob[0] = this->ui_contents->files[file_index].file_button[0].v.ob[0] + 0x40;
        this->ui_contents->files[file_index].file_button[3].v.ob[1] = this->ui_contents->files[file_index].file_button[0].v.ob[1] - 0x10;        
        this->ui_contents->files[file_index].file_button[3].v.tc[0] = 0x800;

        /* File Name Box */

        // /* top left-corner */
        // this->windowContentVtx[vtxId + 4].v.ob[0] = posX + 0x40;
        // this->windowContentVtx[vtxId + 4].v.ob[1] = this->buttonYOffsets[j] + posY;

        // /* top right-corner */
        // this->windowContentVtx[vtxId + 5].v.ob[0] = this->windowContentVtx[vtxId + 4].v.ob[0] + 0x6C;
        // this->windowContentVtx[vtxId + 5].v.ob[1] = this->buttonYOffsets[j] + posY;
        // this->windowContentVtx[vtxId + 5].v.tc[0] = 0xD80;

        // /* bottom-left corner */
        // this->windowContentVtx[vtxId + 6].v.ob[0] = posX + 0x40;
        // this->windowContentVtx[vtxId + 6].v.ob[1] = this->windowContentVtx[vtxId + 4].v.ob[1] - 0x10;

        // /* bottom-right corner */
        // this->windowContentVtx[vtxId + 7].v.ob[0] = this->windowContentVtx[vtxId + 4].v.ob[0] + 0x6C;
        // this->windowContentVtx[vtxId + 7].v.ob[1] = this->windowContentVtx[vtxId + 4].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 7].v.tc[0] = 0xD80;
        /* top left-corner */
        this->ui_contents->files[file_index].file_name_box[0].v.ob[0] = posX + 0x40;
        this->ui_contents->files[file_index].file_name_box[0].v.ob[1] = this->buttonYOffsets[file_index] + posY;
        /* top right-corner */
        this->ui_contents->files[file_index].file_name_box[1].v.ob[0] = this->ui_contents->files[file_index].file_name_box[0].v.ob[0] + 0x6C;
        this->ui_contents->files[file_index].file_name_box[1].v.ob[1] = this->buttonYOffsets[file_index] + posY;
        this->ui_contents->files[file_index].file_name_box[1].v.tc[0] = 0xD80;
        /* bottom-left corner */
        this->ui_contents->files[file_index].file_name_box[2].v.ob[0] = posX + 0x40;
        this->ui_contents->files[file_index].file_name_box[2].v.ob[1] = this->ui_contents->files[file_index].file_name_box[0].v.ob[1] - 0x10;
        /* bottom-right corner */
        this->ui_contents->files[file_index].file_name_box[3].v.ob[0] = this->ui_contents->files[file_index].file_name_box[0].v.ob[0] + 0x6C;
        this->ui_contents->files[file_index].file_name_box[3].v.ob[1] = this->ui_contents->files[file_index].file_name_box[0].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].file_name_box[3].v.tc[0] = 0xD80;

        /* Connectors */

        // /* top-left corner */
        // this->windowContentVtx[vtxId + 8].v.ob[0] = posX + 0x34;
        // this->windowContentVtx[vtxId + 8].v.ob[1] = this->buttonYOffsets[j] + posY;
        
        // /* top-right corner */
        // this->windowContentVtx[vtxId + 9].v.ob[0] = this->windowContentVtx[vtxId + 8].v.ob[0] + 0x18;
        // this->windowContentVtx[vtxId + 9].v.ob[1] = this->buttonYOffsets[j] + posY;
        // this->windowContentVtx[vtxId + 9].v.tc[0] = 0x300;
        
        // /* bottom-left corner */
        // this->windowContentVtx[vtxId + 10].v.ob[1] = this->windowContentVtx[vtxId + 8].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 10].v.ob[0] = posX + 0x34;

        // /* bottom-right corner */
        // this->windowContentVtx[vtxId + 11].v.ob[1] = this->windowContentVtx[vtxId + 8].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 11].v.ob[0] = this->windowContentVtx[vtxId + 8].v.ob[0] + 0x18;
        // this->windowContentVtx[vtxId + 11].v.tc[0] = 0x300;

        /* top-left corner */
        this->ui_contents->files[file_index].connectors[0].v.ob[0] = posX + 0x34;
        this->ui_contents->files[file_index].connectors[0].v.ob[1] = this->buttonYOffsets[file_index] + posY;
        /* top-right corner */
        this->ui_contents->files[file_index].connectors[1].v.ob[0] = this->ui_contents->files[file_index].connectors[0].v.ob[0] + 0x18;
        this->ui_contents->files[file_index].connectors[1].v.ob[1] = this->buttonYOffsets[file_index] + posY;
        this->ui_contents->files[file_index].connectors[1].v.tc[0] = 0x300;
        /* bottom-left corner */
        this->ui_contents->files[file_index].connectors[2].v.ob[1] = this->ui_contents->files[file_index].connectors[0].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].connectors[2].v.ob[0] = posX + 0x34;
        /* bottom-right corner */
        this->ui_contents->files[file_index].connectors[3].v.ob[1] = this->ui_contents->files[file_index].connectors[0].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].connectors[3].v.ob[0] = this->ui_contents->files[file_index].connectors[0].v.ob[0] + 0x18;
        this->ui_contents->files[file_index].connectors[3].v.tc[0] = 0x300;

        /* Blank Button (Owl Save) */

        // /* top-left corner */
        // this->windowContentVtx[vtxId + 12].v.ob[0] = posX + 0xA9;
        // this->windowContentVtx[vtxId + 12].v.ob[1] = this->buttonYOffsets[j] + posY;

        // /* top-right corner */
        // this->windowContentVtx[vtxId + 13].v.ob[0] = this->windowContentVtx[vtxId + 12].v.ob[0] + 0x34;
        // this->windowContentVtx[vtxId + 13].v.ob[1] = this->buttonYOffsets[j] + posY;
        // this->windowContentVtx[vtxId + 13].v.tc[0] = 0x680;
        
        // /* bottom-left corner */
        // this->windowContentVtx[vtxId + 14].v.ob[1] = this->windowContentVtx[vtxId + 12].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 14].v.ob[0] = posX + 0xA9;

        // /* bottom-right corner */
        // this->windowContentVtx[vtxId + 15].v.ob[0] = this->windowContentVtx[vtxId + 12].v.ob[0] + 0x34;
        // this->windowContentVtx[vtxId + 15].v.ob[1] = this->windowContentVtx[vtxId + 12].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 15].v.tc[0] = 0x680;

        /* top-left corner */
        this->ui_contents->files[file_index].owl_save[0].v.ob[0] = posX + 0xA9;
        this->ui_contents->files[file_index].owl_save[0].v.ob[1] = this->buttonYOffsets[file_index] + posY;
        /* top-right corner */
        this->ui_contents->files[file_index].owl_save[1].v.ob[0] = this->ui_contents->files[file_index].owl_save[0].v.ob[0] + 0x34;
        this->ui_contents->files[file_index].owl_save[1].v.ob[1] = this->buttonYOffsets[file_index] + posY;
        this->ui_contents->files[file_index].owl_save[1].v.tc[0] = 0x680;
        /* bottom-left corner */
        this->ui_contents->files[file_index].owl_save[2].v.ob[1] = this->ui_contents->files[file_index].owl_save[0].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].owl_save[2].v.ob[0] = posX + 0xA9;
        /* bottom-right corner */
        this->ui_contents->files[file_index].owl_save[3].v.ob[0] = this->ui_contents->files[file_index].owl_save[0].v.ob[0] + 0x34;
        this->ui_contents->files[file_index].owl_save[3].v.ob[1] = this->ui_contents->files[file_index].owl_save[0].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].owl_save[3].v.tc[0] = 0x680;
    }

    posY = 44;

    // Loop through 3 files
    for (file_index = 0; file_index < 2; file_index++, posY -= 16) {
        struct save_info_t *save_info = &this->save_info[file_index + 2];
        if (!gSaveContext.flashSaveAvailable) {
            // Should skip vtxId
            // vtxId += 268;
            continue;
        }

        // Account for owl-save offset

        spAC = file_index;
        if (save_info->isOwlSave || save_info->is_crash_save) {
            spAC = file_index + 2;
        }

        /* File name */

        posX = this->windowPosX - 6;

        if ((this->configMode == 0x10) && (file_index == this->copyDestFileIndex)) {
            relPosY = this->fileNamesY[file_index] + 0x2C;
        } else if (((this->configMode == 0x11) || (this->configMode == 0x12)) && (file_index == this->copyDestFileIndex)) {
            relPosY = this->buttonYOffsets[file_index] + posY;
        } else {
            relPosY = posY + this->buttonYOffsets[file_index] + this->fileNamesY[file_index];
        }

        tempPosY = relPosY - 2;

        // Loop through 8 characters of file name
        for (i = 0; i < 8; i++, vtxId += 4) {

            index = save_info->fileName[i];

            /* File Name */

            // x-coord (left)
            // this->windowContentVtx[vtxId + 0].v.ob[0] = D_80814280[index] + posX + 0x4E;
            // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY;

            // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0xB;
            // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

            // this->windowContentVtx[vtxId + 2].v.ob[0] = D_80814280[index] + posX + 0x4E;
            // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0xC;
            
            // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0xB;            
            // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0xC;

            this->ui_contents->files[file_index].file_name[i][0].v.ob[0] = D_80814280[index] + posX + 0x4E;
            this->ui_contents->files[file_index].file_name[i][0].v.ob[1] = tempPosY;

            this->ui_contents->files[file_index].file_name[i][1].v.ob[0] = this->ui_contents->files [file_index].file_name[i][0].v.ob[0] + 0xB;
            this->ui_contents->files[file_index].file_name[i][1].v.ob[1] = tempPosY;

            this->ui_contents->files[file_index].file_name[i][2].v.ob[0] = D_80814280[index] + posX + 0x4E;
            this->ui_contents->files[file_index].file_name[i][2].v.ob[1] = this->ui_contents->files[file_index].file_name[i][0].v.ob[1] - 0xC;
            
            this->ui_contents->files[file_index].file_name[i][3].v.ob[0] = this->ui_contents->files[file_index].file_name[i][0].v.ob[0] + 0xB;            
            this->ui_contents->files[file_index].file_name[i][3].v.ob[1] = this->ui_contents->files[file_index].file_name[i][0].v.ob[1] - 0xC;

            /* File Name Shadow */

            // this->windowContentVtx[vtxId + 32].v.ob[0] = D_80814280[index] + posX + 0x4F;
            // this->windowContentVtx[vtxId + 32].v.ob[1] = tempPosY - 1;

            // this->windowContentVtx[vtxId + 33].v.ob[0] = this->windowContentVtx[vtxId + 32].v.ob[0] + 0xB;
            // this->windowContentVtx[vtxId + 33].v.ob[1] = tempPosY - 1;

            // this->windowContentVtx[vtxId + 34].v.ob[0] = D_80814280[index] + posX + 0x4F;
            // this->windowContentVtx[vtxId + 34].v.ob[1] = this->windowContentVtx[vtxId + 32].v.ob[1] - 0xC;
            
            // this->windowContentVtx[vtxId + 35].v.ob[0] = this->windowContentVtx[vtxId + 32].v.ob[0] + 0xB;
            // this->windowContentVtx[vtxId + 35].v.ob[1] = this->windowContentVtx[vtxId + 32].v.ob[1] - 0xC;

            this->ui_contents->files[file_index].file_name_shadow[i][0].v.ob[0] = D_80814280[index] + posX + 0x4F;
            this->ui_contents->files[file_index].file_name_shadow[i][0].v.ob[1] = tempPosY - 1;

            this->ui_contents->files[file_index].file_name_shadow[i][1].v.ob[0] = this->ui_contents->files[file_index].file_name_shadow[i][0].v.ob[0] + 0xB;
            this->ui_contents->files[file_index].file_name_shadow[i][1].v.ob[1] = tempPosY - 1;

            this->ui_contents->files[file_index].file_name_shadow[i][2].v.ob[0] = D_80814280[index] + posX + 0x4F;
            this->ui_contents->files[file_index].file_name_shadow[i][2].v.ob[1] = this->ui_contents->files[file_index].file_name_shadow[i][0].v.ob[1] - 0xC;
            
            this->ui_contents->files[file_index].file_name_shadow[i][3].v.ob[0] = this->ui_contents->files[file_index].file_name_shadow[i][0].v.ob[0] + 0xB;
            this->ui_contents->files[file_index].file_name_shadow[i][3].v.ob[1] = this->ui_contents->files[file_index].file_name_shadow[i][0].v.ob[1] - 0xC;

            // Update X position
            posX += 10;
        }
        // Account for the shadow
        vtxId += 32;

        /* Rupee Digits */

        posX = this->windowPosX + 14;
        tempPosY = relPosY - 0x18;

        FileSelect_SplitNumber(save_info->rupees, &spA4[0], &spA4[1], &spA4[2]);

        index = sWalletFirstDigit[save_info->walletUpgrades];

        ptr = &spA4[index];

        for (digit_index = 0; digit_index < 3; digit_index++, vtxId += 4, ptr++) {

            /* Rupee Digits */
            // this->windowContentVtx[vtxId + 0].v.ob[0] = D_80814280[*ptr] + posX;
            // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY;

            // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + D_80814628[i];
            // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

            // this->windowContentVtx[vtxId + 2].v.ob[0] = D_80814280[*ptr] + posX;
            // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - D_80814630[i];

            // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + D_80814628[i];
            // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - D_80814630[i];

            this->ui_contents->files[file_index].rupee_digits[digit_index][0].v.ob[0] = D_80814280[*ptr] + posX;
            this->ui_contents->files[file_index].rupee_digits[digit_index][0].v.ob[1] = tempPosY;

            this->ui_contents->files[file_index].rupee_digits[digit_index][1].v.ob[0] = this->ui_contents->files[file_index].rupee_digits[digit_index][0].v.ob[0] + D_80814628[digit_index];
            this->ui_contents->files[file_index].rupee_digits[digit_index][1].v.ob[1] = tempPosY;

            this->ui_contents->files[file_index].rupee_digits[digit_index][2].v.ob[0] = D_80814280[*ptr] + posX;
            this->ui_contents->files[file_index].rupee_digits[digit_index][2].v.ob[1] = this->ui_contents->files[file_index].rupee_digits[digit_index][0].v.ob[1] - D_80814630[digit_index];

            this->ui_contents->files[file_index].rupee_digits[digit_index][3].v.ob[0] = this->ui_contents->files[file_index].rupee_digits[digit_index][0].v.ob[0] + D_80814628[digit_index];
            this->ui_contents->files[file_index].rupee_digits[digit_index][3].v.ob[1] = this->ui_contents->files[file_index].rupee_digits[digit_index][0].v.ob[1] - D_80814630[digit_index];

            /* Rupee Digits Shadow */
            this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][0].v.ob[0] = this->ui_contents->files[file_index].rupee_digits[digit_index][0].v.ob[0] + 1;
            this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][0].v.ob[1] = tempPosY - 1;

            this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][1].v.ob[1] = tempPosY - 1;
            this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][1].v.ob[0] = this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][0].v.ob[0] + D_80814628[digit_index];

            this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][2].v.ob[0] = this->ui_contents->files[file_index].rupee_digits[digit_index][0].v.ob[0] + 1;
            this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][2].v.ob[1] = this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][0].v.ob[1] - D_80814630[digit_index];

            this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][3].v.ob[0] = this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][0].v.ob[0] + D_80814628[digit_index];
            this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][3].v.ob[1] = this->ui_contents->files[file_index].rupee_digits_shadow[digit_index][0].v.ob[1] - D_80814630[digit_index];

            // Update X position
            posX += D_80814620[digit_index];
        }

        // Account for the shadow
        vtxId += 12;

        /* Mask Count */

        posX = this->windowPosX + 42;
        tempPosY = relPosY - 0x2A;

        FileSelect_SplitNumber(save_info->maskCount, &spA4[0], &spA4[1], &spA4[2]);

        for (digit_index = 0; digit_index < 2; digit_index++, vtxId += 4) {

            /* Mask Count */
            // this->windowContentVtx[vtxId + 0].v.ob[0] = D_80814280[spA4[i]] + posX;
            // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY;

            // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + D_80814628[i];
            // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

            // this->windowContentVtx[vtxId + 2].v.ob[0] = D_80814280[spA4[i]] + posX;
            // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - D_80814630[i];
            
            // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + D_80814628[i];
            // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - D_80814630[i];

            s16 offset_index = digit_index + 1;

            this->ui_contents->files[file_index].mask_count_digits[digit_index][0].v.ob[0] = D_80814280[spA4[offset_index]] + posX;
            this->ui_contents->files[file_index].mask_count_digits[digit_index][0].v.ob[1] = tempPosY;

            this->ui_contents->files[file_index].mask_count_digits[digit_index][1].v.ob[0] = this->ui_contents->files[file_index].mask_count_digits[digit_index][0].v.ob[0] + D_80814628[offset_index];
            this->ui_contents->files[file_index].mask_count_digits[digit_index][1].v.ob[1] = tempPosY;

            this->ui_contents->files[file_index].mask_count_digits[digit_index][2].v.ob[0] = D_80814280[spA4[offset_index]] + posX;
            this->ui_contents->files[file_index].mask_count_digits[digit_index][2].v.ob[1] = this->ui_contents->files[file_index].mask_count_digits[digit_index][0].v.ob[1] - D_80814630[offset_index];
            
            this->ui_contents->files[file_index].mask_count_digits[digit_index][3].v.ob[0] = this->ui_contents->files[file_index].mask_count_digits[digit_index][0].v.ob[0] + D_80814628[offset_index];
            this->ui_contents->files[file_index].mask_count_digits[digit_index][3].v.ob[1] = this->ui_contents->files[file_index].mask_count_digits[digit_index][0].v.ob[1] - D_80814630[offset_index];

            /* Mask Count Shadow */
            // this->windowContentVtx[vtxId + 8].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 1;
            // this->windowContentVtx[vtxId + 8].v.ob[1] = tempPosY - 1;

            // this->windowContentVtx[vtxId + 9].v.ob[0] = this->windowContentVtx[vtxId + 8].v.ob[0] + D_80814628[i];
            // this->windowContentVtx[vtxId + 9].v.ob[1] = tempPosY - 1;

            // this->windowContentVtx[vtxId + 10].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 1;
            // this->windowContentVtx[vtxId + 10].v.ob[1] = this->windowContentVtx[vtxId + 8].v.ob[1] - D_80814630[i];
            
            // this->windowContentVtx[vtxId + 11].v.ob[0] = this->windowContentVtx[vtxId + 8].v.ob[0] + D_80814628[i];
            // this->windowContentVtx[vtxId + 11].v.ob[1] = this->windowContentVtx[vtxId + 8].v.ob[1] - D_80814630[i];

            this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][0].v.ob[0] = this->ui_contents->files[file_index].mask_count_digits[digit_index][0].v.ob[0] + 1;
            this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][0].v.ob[1] = tempPosY - 1;

            this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][1].v.ob[0] = this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][0].v.ob[0] + D_80814628[offset_index];
            this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][1].v.ob[1] = tempPosY - 1;

            this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][2].v.ob[0] = this->ui_contents->files[file_index].mask_count_digits[digit_index][0].v.ob[0] + 1;
            this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][2].v.ob[1] = this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][0].v.ob[1] - D_80814630[offset_index];
            
            this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][3].v.ob[0] = this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][0].v.ob[0] + D_80814628[offset_index];
            this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][3].v.ob[1] = this->ui_contents->files[file_index].mask_count_digits_shadow[digit_index][0].v.ob[1] - D_80814630[offset_index];

            // Update X position
            posX += D_80814620[offset_index];
        }

        // Account for the shadow
        vtxId += 8;

        /* Hearts */

        posX = this->windowPosX + 63;
        tempPosY = relPosY - 0x10;

        // Loop through 20 hearts
        for (heart_index = 0; heart_index < 20; heart_index++, vtxId += 4, posX += 9) {
            // this->windowContentVtx[vtxId + 0].v.ob[0] = posX;
            // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY;

            // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0xA;
            // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

            // this->windowContentVtx[vtxId + 2].v.ob[0] = posX;
            // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0xA;
            
            // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0xA;
            // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0xA;

            this->ui_contents->files[file_index].hearts[heart_index][0].v.ob[0] = posX;
            this->ui_contents->files[file_index].hearts[heart_index][0].v.ob[1] = tempPosY;

            this->ui_contents->files[file_index].hearts[heart_index][1].v.ob[0] = this->ui_contents->files[file_index].hearts[heart_index][0].v.ob[0] + 0xA;
            this->ui_contents->files[file_index].hearts[heart_index][1].v.ob[1] = tempPosY;

            this->ui_contents->files[file_index].hearts[heart_index][2].v.ob[0] = posX;
            this->ui_contents->files[file_index].hearts[heart_index][2].v.ob[1] = this->ui_contents->files[file_index].hearts[heart_index][0].v.ob[1] - 0xA;
            
            this->ui_contents->files[file_index].hearts[heart_index][3].v.ob[0] = this->ui_contents->files[file_index].hearts[heart_index][0].v.ob[0] + 0xA;
            this->ui_contents->files[file_index].hearts[heart_index][3].v.ob[1] = this->ui_contents->files[file_index].hearts[heart_index][0].v.ob[1] - 0xA;

            // New row of hearts next iteration
            if (i == 9) {
                posX = this->windowPosX + (63 - 9);
                tempPosY -= 8;
            }
        }

        /* Quest Remains */

        posX = this->windowPosX + 64;
        tempPosY = relPosY - 0x20;

        // Loop through 4 Remains
        for (remains_index = 0; remains_index < 4; remains_index++, vtxId += 4, posX += 0x18) {
            // this->windowContentVtx[vtxId + 0].v.ob[0] = posX;
            // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY;

            // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x14;
            // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;
            // this->windowContentVtx[vtxId + 1].v.tc[0] = 0x400;

            // this->windowContentVtx[vtxId + 2].v.ob[0] = posX;
            // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x14;
            // this->windowContentVtx[vtxId + 2].v.tc[1] = 0x400;
            
            // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x14;
            // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x14;
            // this->windowContentVtx[vtxId + 3].v.tc[0] = 0x400;
            // this->windowContentVtx[vtxId + 3].v.tc[1] = 0x400;

            this->ui_contents->files[file_index].remains_masks[remains_index][0].v.ob[0] = posX;
            this->ui_contents->files[file_index].remains_masks[remains_index][0].v.ob[1] = tempPosY;

            this->ui_contents->files[file_index].remains_masks[remains_index][1].v.ob[0] = this->ui_contents->files[file_index].remains_masks[remains_index][0].v.ob[0] + 0x14;
            this->ui_contents->files[file_index].remains_masks[remains_index][1].v.ob[1] = tempPosY;
            this->ui_contents->files[file_index].remains_masks[remains_index][1].v.tc[0] = 0x400;

            this->ui_contents->files[file_index].remains_masks[remains_index][2].v.ob[0] = posX;
            this->ui_contents->files[file_index].remains_masks[remains_index][2].v.ob[1] = this->ui_contents->files[file_index].remains_masks[remains_index][0].v.ob[1] - 0x14;
            this->ui_contents->files[file_index].remains_masks[remains_index][2].v.tc[1] = 0x400;
            
            this->ui_contents->files[file_index].remains_masks[remains_index][3].v.ob[0] = this->ui_contents->files[file_index].remains_masks[remains_index][0].v.ob[0] + 0x14;
            this->ui_contents->files[file_index].remains_masks[remains_index][3].v.ob[1] = this->ui_contents->files[file_index].remains_masks[remains_index][0].v.ob[1] - 0x14;
            this->ui_contents->files[file_index].remains_masks[remains_index][3].v.tc[0] = 0x400;
            this->ui_contents->files[file_index].remains_masks[remains_index][3].v.tc[1] = 0x400;
        }

        /* Rupee Icon */

        // posX = this->windowPosX - 1;
        tempPosY = relPosY - 0x15;

        // this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowPosX - 1;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY;

        // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x10;
        // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = 0x200;

        // this->windowContentVtx[vtxId + 2].v.ob[0] = this->windowPosX - 1;
        // this->windowContentVtx[vtxId + 2].v.tc[1] = 0x200;
        
        
        // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x10;
        // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 3].v.tc[0] = 0x200;
        // this->windowContentVtx[vtxId + 3].v.tc[1] = 0x200;

        this->ui_contents->files[file_index].rupee_icon[0].v.ob[0] = this->windowPosX - 1;
        this->ui_contents->files[file_index].rupee_icon[0].v.ob[1] = tempPosY;

        this->ui_contents->files[file_index].rupee_icon[1].v.ob[0] = this->ui_contents->files[file_index].rupee_icon[0].v.ob[0] + 0x10;
        this->ui_contents->files[file_index].rupee_icon[1].v.ob[1] = tempPosY;
        this->ui_contents->files[file_index].rupee_icon[1].v.tc[0] = 0x200;
        
        this->ui_contents->files[file_index].rupee_icon[2].v.ob[0] = this->windowPosX - 1;
        this->ui_contents->files[file_index].rupee_icon[2].v.ob[1] = this->ui_contents->files[file_index].rupee_icon[0].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].rupee_icon[2].v.tc[1] = 0x200;
        
        this->ui_contents->files[file_index].rupee_icon[3].v.ob[0] = this->ui_contents->files[file_index].rupee_icon[0].v.ob[0] + 0x10;
        this->ui_contents->files[file_index].rupee_icon[3].v.ob[1] = this->ui_contents->files[file_index].rupee_icon[0].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].rupee_icon[3].v.tc[0] = 0x200;
        this->ui_contents->files[file_index].rupee_icon[3].v.tc[1] = 0x200;
        

        // texture coordinates
        
        

        vtxId += 4;

        /* Heart Piece Count */

        // posX = this->windowPosX + 0x27;
        tempPosY = relPosY - 0x15;

        // this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowPosX + 0x27;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY;

        // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x18;
        // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = 0x300;

        // this->windowContentVtx[vtxId + 2].v.ob[0] = this->windowPosX + 0x27;
        // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 2].v.tc[1] = this->windowContentVtx[vtxId + 3].v.tc[1] = 0x200;
        
        // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x18;
        // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x10;

        this->ui_contents->files[file_index].heart_container_pieces[0].v.ob[0] = this->windowPosX + 0x27;
        this->ui_contents->files[file_index].heart_container_pieces[0].v.ob[1] = tempPosY;

        this->ui_contents->files[file_index].heart_container_pieces[1].v.ob[0] = this->ui_contents->files[file_index].heart_container_pieces[0].v.ob[0] + 0x18;
        this->ui_contents->files[file_index].heart_container_pieces[1].v.ob[1] = tempPosY;
        this->ui_contents->files[file_index].heart_container_pieces[1].v.tc[0] = this->ui_contents->files[file_index].heart_container_pieces[3].v.tc[0] = 0x300;

        this->ui_contents->files[file_index].heart_container_pieces[2].v.ob[0] = this->windowPosX + 0x27;
        this->ui_contents->files[file_index].heart_container_pieces[2].v.ob[1] = this->ui_contents->files[file_index].heart_container_pieces[0].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].heart_container_pieces[2].v.tc[1] = this->ui_contents->files[file_index].heart_container_pieces[3].v.tc[1] = 0x200;
        
        this->ui_contents->files[file_index].heart_container_pieces[3].v.ob[0] = this->ui_contents->files[file_index].heart_container_pieces[0].v.ob[0] + 0x18;
        this->ui_contents->files[file_index].heart_container_pieces[3].v.ob[1] = this->ui_contents->files[file_index].heart_container_pieces[0].v.ob[1] - 0x10;
        
        

        vtxId += 4;

        /* Mask Text */

        // posX = this->windowPosX - 10;
        tempPosY = relPosY - 0x27;

        // this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowPosX - 10;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY;

        // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x40;
        // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = 0x800;

        // this->windowContentVtx[vtxId + 2].v.ob[0] = this->windowPosX - 10;
        // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 2].v.tc[1] = 0x200;

        // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x40;
        // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 3].v.tc[0] = 0x800;
        // this->windowContentVtx[vtxId + 3].v.tc[1] = 0x200;

        // this->windowContentVtx[vtxId + 4].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 1;
        // this->windowContentVtx[vtxId + 4].v.ob[1] = tempPosY - 1;

        // this->windowContentVtx[vtxId + 5].v.ob[0] = this->windowContentVtx[vtxId + 4].v.ob[0] + 0x40;
        // this->windowContentVtx[vtxId + 5].v.ob[1] = tempPosY - 1;
        // this->windowContentVtx[vtxId + 5].v.tc[0] = 0x800;

        // this->windowContentVtx[vtxId + 6].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 1;
        // this->windowContentVtx[vtxId + 6].v.ob[1] = this->windowContentVtx[vtxId + 4].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 6].v.tc[1] = 0x200;

        // this->windowContentVtx[vtxId + 7].v.ob[0] = this->windowContentVtx[vtxId + 4].v.ob[0] + 0x40;
        // this->windowContentVtx[vtxId + 7].v.ob[1] = this->windowContentVtx[vtxId + 4].v.ob[1] - 0x10;
        // this->windowContentVtx[vtxId + 7].v.tc[0] = 0x800;
        // this->windowContentVtx[vtxId + 7].v.tc[1] = 0x200;

        // Fault_AddHangupPrintfAndCrash("HERE");

        // this->windowContentVtx[vtxId + 6].v.ob[1] = this->windowContentVtx[vtxId + 7].v.ob[1] =
        //     this->windowContentVtx[vtxId + 4].v.ob[1] - 0x10;

        this->ui_contents->files[file_index].mask_text[0].v.ob[0] = this->windowPosX - 10;
        this->ui_contents->files[file_index].mask_text[0].v.ob[1] = tempPosY;

        this->ui_contents->files[file_index].mask_text[1].v.ob[0] = this->ui_contents->files[file_index].mask_text[0].v.ob[0] + 0x40;
        this->ui_contents->files[file_index].mask_text[1].v.ob[1] = tempPosY;
        this->ui_contents->files[file_index].mask_text[1].v.tc[0] = 0x800;

        this->ui_contents->files[file_index].mask_text[2].v.ob[0] = this->windowPosX - 10;
        this->ui_contents->files[file_index].mask_text[2].v.ob[1] = this->ui_contents->files[file_index].mask_text[0].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].mask_text[2].v.tc[1] = 0x200;

        this->ui_contents->files[file_index].mask_text[3].v.ob[0] = this->ui_contents->files[file_index].mask_text[0].v.ob[0] + 0x40;
        this->ui_contents->files[file_index].mask_text[3].v.ob[1] = this->ui_contents->files[file_index].mask_text[0].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].mask_text[3].v.tc[0] = 0x800;
        this->ui_contents->files[file_index].mask_text[3].v.tc[1] = 0x200;

        this->ui_contents->files[file_index].mask_text[4].v.ob[0] = this->ui_contents->files[file_index].mask_text[0].v.ob[0] + 1;
        this->ui_contents->files[file_index].mask_text[4].v.ob[1] = tempPosY - 1;

        this->ui_contents->files[file_index].mask_text[5].v.ob[0] = this->ui_contents->files[file_index].mask_text[4].v.ob[0] + 0x40;
        this->ui_contents->files[file_index].mask_text[5].v.ob[1] = tempPosY - 1;
        this->ui_contents->files[file_index].mask_text[5].v.tc[0] = 0x800;

        this->ui_contents->files[file_index].mask_text[6].v.ob[0] = this->ui_contents->files[file_index].mask_text[0].v.ob[0] + 1;
        this->ui_contents->files[file_index].mask_text[6].v.ob[1] = this->ui_contents->files[file_index].mask_text[4].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].mask_text[6].v.tc[1] = 0x200;

        this->ui_contents->files[file_index].mask_text[7].v.ob[0] = this->ui_contents->files[file_index].mask_text[4].v.ob[0] + 0x40;
        this->ui_contents->files[file_index].mask_text[7].v.ob[1] = this->ui_contents->files[file_index].mask_text[4].v.ob[1] - 0x10;
        this->ui_contents->files[file_index].mask_text[7].v.tc[0] = 0x800;
        this->ui_contents->files[file_index].mask_text[7].v.tc[1] = 0x200;

        vtxId += 8;

        /* Owl Save Icon */

        posX = this->windowPosX + 0xA3;

        if ((this->configMode == 0x10) && (file_index == this->copyDestFileIndex)) {
            tempPosY = this->fileNamesY[file_index] + 0x2C;
        } else if (((this->configMode == 0x11) || (this->configMode == 0x12)) && (file_index == this->copyDestFileIndex)) {
            tempPosY = this->buttonYOffsets[file_index] + posY;
        } else {
            tempPosY = posY + this->buttonYOffsets[file_index] + this->fileNamesY[file_index];
        }


        // this->windowContentVtx[vtxId + 0].v.ob[0] = posX + 0xE;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY - 2;

        // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x18;
        // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY - 2;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = 0x300;
        

        // this->windowContentVtx[vtxId + 2].v.ob[0] = posX + 0xE;
        // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0xC;
        // this->windowContentVtx[vtxId + 2].v.tc[1] = 0x180;
        
        
        // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x18;
        // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0xC;
        // this->windowContentVtx[vtxId + 3].v.tc[1] = 0x180;
        // this->windowContentVtx[vtxId + 3].v.tc[0] = 0x300;

        // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
        //     this->windowContentVtx[vtxId + 0].v.ob[1] - 0xC;

        this->ui_contents->files[file_index].owl_icon[0].v.ob[0] = posX + 0xE;
        this->ui_contents->files[file_index].owl_icon[0].v.ob[1] = tempPosY - 2;

        this->ui_contents->files[file_index].owl_icon[1].v.ob[0] = this->ui_contents->files[file_index].owl_icon[0].v.ob[0] + 0x18;
        this->ui_contents->files[file_index].owl_icon[1].v.ob[1] = tempPosY - 2;
        this->ui_contents->files[file_index].owl_icon[1].v.tc[0] = 0x300;
        
        this->ui_contents->files[file_index].owl_icon[2].v.ob[0] = posX + 0xE;
        this->ui_contents->files[file_index].owl_icon[2].v.ob[1] = this->ui_contents->files[file_index].owl_icon[0].v.ob[1] - 0xC;
        this->ui_contents->files[file_index].owl_icon[2].v.tc[1] = 0x180;
        
        this->ui_contents->files[file_index].owl_icon[3].v.ob[0] = this->ui_contents->files[file_index].owl_icon[0].v.ob[0] + 0x18;
        this->ui_contents->files[file_index].owl_icon[3].v.ob[1] = this->ui_contents->files[file_index].owl_icon[0].v.ob[1] - 0xC;
        this->ui_contents->files[file_index].owl_icon[3].v.tc[0] = 0x300;
        this->ui_contents->files[file_index].owl_icon[3].v.tc[1] = 0x180;

        this->ui_contents->files[file_index].crash_icon[0].v.ob[0] = posX + 8;
        this->ui_contents->files[file_index].crash_icon[0].v.ob[1] = tempPosY - 2;

        this->ui_contents->files[file_index].crash_icon[1].v.ob[0] = this->ui_contents->files[file_index].crash_icon[0].v.ob[0] + 32;
        this->ui_contents->files[file_index].crash_icon[1].v.ob[1] = tempPosY - 2;
        this->ui_contents->files[file_index].crash_icon[1].v.tc[0] = TC_10_5(47, 0);
        
        this->ui_contents->files[file_index].crash_icon[2].v.ob[0] = posX + 8;
        this->ui_contents->files[file_index].crash_icon[2].v.ob[1] = this->ui_contents->files[file_index].crash_icon[0].v.ob[1] - 0xC;
        this->ui_contents->files[file_index].crash_icon[2].v.tc[1] = TC_10_5(15, 0);
        
        this->ui_contents->files[file_index].crash_icon[3].v.ob[0] = this->ui_contents->files[file_index].crash_icon[0].v.ob[0] + 32;
        this->ui_contents->files[file_index].crash_icon[3].v.ob[1] = this->ui_contents->files[file_index].crash_icon[0].v.ob[1] - 0xC;
        this->ui_contents->files[file_index].crash_icon[3].v.tc[0] = TC_10_5(47, 0);
        this->ui_contents->files[file_index].crash_icon[3].v.tc[1] = TC_10_5(15, 0);

        vtxId += 4;


        /* Day Text */
        for (i = 0; i < 2; i++, vtxId += 4) 
        {
            // this->windowContentVtx[vtxId + 0].v.ob[0] = 2 + posX + i;
            // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY - i - 0x12;

            // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x30;
            // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY - i - 0x12;
            // this->windowContentVtx[vtxId + 1].v.tc[0] = 0x600;

            // this->windowContentVtx[vtxId + 2].v.ob[0] = 2 + posX + i;
            // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x18;
            // this->windowContentVtx[vtxId + 2].v.tc[1] = 0x300;

            // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x30;
            // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x18;
            // this->windowContentVtx[vtxId + 3].v.tc[0] = 0x600;
            // this->windowContentVtx[vtxId + 3].v.tc[1] = 0x300;

            this->ui_contents->files[file_index].day_text[i][0].v.ob[0] = 2 + posX + i;
            this->ui_contents->files[file_index].day_text[i][0].v.ob[1] = tempPosY - i - 0x12;

            this->ui_contents->files[file_index].day_text[i][1].v.ob[0] = this->ui_contents->files[file_index].day_text[i][0].v.ob[0] + 0x30;
            this->ui_contents->files[file_index].day_text[i][1].v.ob[1] = tempPosY - i - 0x12;
            this->ui_contents->files[file_index].day_text[i][1].v.tc[0] = 0x600;

            this->ui_contents->files[file_index].day_text[i][2].v.ob[0] = 2 + posX + i;
            this->ui_contents->files[file_index].day_text[i][2].v.ob[1] = this->ui_contents->files[file_index].day_text[i][0].v.ob[1] - 0x18;
            this->ui_contents->files[file_index].day_text[i][2].v.tc[1] = 0x300;

            this->ui_contents->files[file_index].day_text[i][3].v.ob[0] = this->ui_contents->files[file_index].day_text[i][0].v.ob[0] + 0x30;
            this->ui_contents->files[file_index].day_text[i][3].v.ob[1] = this->ui_contents->files[file_index].day_text[i][0].v.ob[1] - 0x18;
            this->ui_contents->files[file_index].day_text[i][3].v.tc[0] = 0x600;
            this->ui_contents->files[file_index].day_text[i][3].v.tc[1] = 0x300;
        }

        /* Time Digits */

        posX += 6;
        index = vtxId;

        for (digit_index = 0; digit_index < 5; digit_index++, vtxId += 4, posX += 8) {

            // this->windowContentVtx[vtxId + 0].v.ob[0] = posX;
            // this->windowContentVtx[vtxId + 2].v.ob[0] = posX;

            // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0xC;
            // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0xC;

            // this->windowContentVtx[vtxId + 0].v.ob[1] = tempPosY - 0x2A;
            // this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY - 0x2A;

            // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0xC;
            // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0xC;

            // this->windowContentVtx[vtxId + 0x14].v.ob[0] = posX + 1;
            // this->windowContentVtx[vtxId + 0x16].v.ob[0] = posX + 1;

            // this->windowContentVtx[vtxId + 0x15].v.ob[0] = this->windowContentVtx[vtxId + 0x14].v.ob[0] + 0xC;
            // this->windowContentVtx[vtxId + 0x17].v.ob[0] = this->windowContentVtx[vtxId + 0x14].v.ob[0] + 0xC;

            // this->windowContentVtx[vtxId + 0x14].v.ob[1] = tempPosY - 0x2B;
            // this->windowContentVtx[vtxId + 0x15].v.ob[1] = tempPosY - 0x2B;

            // this->windowContentVtx[vtxId + 0x16].v.ob[1] = this->windowContentVtx[vtxId + 0x14].v.ob[1] - 0xC;
            // this->windowContentVtx[vtxId + 0x17].v.ob[1] = this->windowContentVtx[vtxId + 0x14].v.ob[1] - 0xC;

            this->ui_contents->files[file_index].time_digits[digit_index][0].v.ob[0] = posX;
            this->ui_contents->files[file_index].time_digits[digit_index][0].v.ob[1] = tempPosY - 0x2A;

            this->ui_contents->files[file_index].time_digits[digit_index][1].v.ob[0] = this->ui_contents->files[file_index].time_digits[digit_index][0].v.ob[0] + 0xC;
            this->ui_contents->files[file_index].time_digits[digit_index][1].v.ob[1] = tempPosY - 0x2A;

            this->ui_contents->files[file_index].time_digits[digit_index][2].v.ob[0] = posX;
            this->ui_contents->files[file_index].time_digits[digit_index][2].v.ob[1] = this->ui_contents->files[file_index].time_digits[digit_index][0].v.ob[1] - 0xC;
            
            this->ui_contents->files[file_index].time_digits[digit_index][3].v.ob[0] = this->ui_contents->files[file_index].time_digits[digit_index][0].v.ob[0] + 0xC;
            this->ui_contents->files[file_index].time_digits[digit_index][3].v.ob[1] = this->ui_contents->files[file_index].time_digits[digit_index][0].v.ob[1] - 0xC;
            

            this->ui_contents->files[file_index].time_digits_shadow[digit_index][0].v.ob[0] = posX + 1;
            this->ui_contents->files[file_index].time_digits_shadow[digit_index][0].v.ob[1] = tempPosY - 0x2B;

            this->ui_contents->files[file_index].time_digits_shadow[digit_index][1].v.ob[0] = this->ui_contents->files[file_index].time_digits_shadow[digit_index][0].v.ob[0] + 0xC;
            this->ui_contents->files[file_index].time_digits_shadow[digit_index][1].v.ob[1] = tempPosY - 0x2B;

            this->ui_contents->files[file_index].time_digits_shadow[digit_index][2].v.ob[0] = posX + 1;
            this->ui_contents->files[file_index].time_digits_shadow[digit_index][2].v.ob[1] = this->ui_contents->files[file_index].time_digits_shadow[digit_index][0].v.ob[1] - 0xC;
            
            this->ui_contents->files[file_index].time_digits_shadow[digit_index][3].v.ob[0] = this->ui_contents->files[file_index].time_digits_shadow[digit_index][0].v.ob[0] + 0xC;
            this->ui_contents->files[file_index].time_digits_shadow[digit_index][3].v.ob[1] = this->ui_contents->files[file_index].time_digits_shadow[digit_index][0].v.ob[1] - 0xC;
        }

        // Adjust the colon to the right
        // this->windowContentVtx[index + 8].v.ob[0] = this->windowContentVtx[index + 8].v.ob[0] + 3;
        // this->windowContentVtx[index + 10].v.ob[0] = this->windowContentVtx[index + 8].v.ob[0] + 3;

        // this->windowContentVtx[index + 9].v.ob[0] = this->windowContentVtx[index + 8].v.ob[0] + 0xC;
        // this->windowContentVtx[index + 11].v.ob[0] = this->windowContentVtx[index + 8].v.ob[0] + 0xC;

        // this->windowContentVtx[index + 0x1C].v.ob[0] = this->windowContentVtx[index + 8].v.ob[0] + 1;
        // this->windowContentVtx[index + 0x1E].v.ob[0] = this->windowContentVtx[index + 8].v.ob[0] + 1;

        // this->windowContentVtx[index + 0x1D].v.ob[0] = this->windowContentVtx[index + 0x1C].v.ob[0] + 0xC;
        // this->windowContentVtx[index + 0x1F].v.ob[0] = this->windowContentVtx[index + 0x1C].v.ob[0] + 0xC;

        vtxId += 20;
    }

    posX = this->windowPosX - 6;
    posY = -0xC;

    for (button_index = 0; button_index < 3; button_index++, vtxId += 4, posY -= FILE_SELECT_BUTTON_HEIGHT) {

        // this->windowContentVtx[vtxId + 0].v.ob[0] = posX;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = this->buttonYOffsets[j + 3] + posY;

        // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + FILE_SELECT_BUTTON_WIDTH;
        // this->windowContentVtx[vtxId + 1].v.ob[1] = this->buttonYOffsets[j + 3] + posY;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = 0x800;

        // this->windowContentVtx[vtxId + 2].v.ob[0] = posX;
        // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - FILE_SELECT_BUTTON_HEIGHT;
        
        // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + FILE_SELECT_BUTTON_WIDTH;
        // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - FILE_SELECT_BUTTON_HEIGHT;
        // this->windowContentVtx[vtxId + 3].v.tc[0] = 0x800;

        this->ui_contents->buttons[button_index][0].v.ob[0] = posX;
        this->ui_contents->buttons[button_index][0].v.ob[1] = this->buttonYOffsets[button_index + 3] + posY;

        this->ui_contents->buttons[button_index][1].v.ob[0] = this->ui_contents->buttons[button_index][0].v.ob[0] + FILE_SELECT_BUTTON_WIDTH;
        this->ui_contents->buttons[button_index][1].v.ob[1] = this->buttonYOffsets[button_index + 3] + posY;
        this->ui_contents->buttons[button_index][1].v.tc[0] = 0x800;

        this->ui_contents->buttons[button_index][2].v.ob[0] = posX;
        this->ui_contents->buttons[button_index][2].v.ob[1] = this->ui_contents->buttons[button_index][0].v.ob[1] - FILE_SELECT_BUTTON_HEIGHT;
        
        this->ui_contents->buttons[button_index][3].v.ob[0] = this->ui_contents->buttons[button_index][0].v.ob[0] + FILE_SELECT_BUTTON_WIDTH;
        this->ui_contents->buttons[button_index][3].v.ob[1] = this->ui_contents->buttons[button_index][0].v.ob[1] - FILE_SELECT_BUTTON_HEIGHT;
        this->ui_contents->buttons[button_index][3].v.tc[0] = 0x800;
    }

    // this->windowContentVtx[vtxId + 0].v.ob[0] = posX;
    // this->windowContentVtx[vtxId + 0].v.ob[1] = this->buttonYOffsets[6] - 0x34;

    // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + FILE_SELECT_BUTTON_WIDTH;
    // this->windowContentVtx[vtxId + 1].v.ob[1] = this->buttonYOffsets[6] - 0x34;
    // this->windowContentVtx[vtxId + 1].v.tc[0] = 0x800;

    // this->windowContentVtx[vtxId + 2].v.ob[0] = posX;
    // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - FILE_SELECT_BUTTON_HEIGHT;

    // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + FILE_SELECT_BUTTON_WIDTH;
    // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - FILE_SELECT_BUTTON_HEIGHT;
    // this->windowContentVtx[vtxId + 3].v.tc[0] = 0x800;

    vtxId += 4;

    if (((this->menuMode == FS_MENU_MODE_CONFIG) && (this->configMode >= CM_MAIN_MENU)) ||
        ((this->menuMode == FS_MENU_MODE_SELECT) && (this->selectMode == SM_CONFIRM_FILE))) 
    {
        Vtx *button_vertices;

        if (this->menuMode == FS_MENU_MODE_CONFIG) 
        {
            if ((this->configMode == CM_SELECT_COPY_SOURCE) || (this->configMode == CM_SELECT_COPY_DEST) || 
                (this->configMode == CM_ERASE_SELECT)) 
            {
                // j = D_80814644[this->buttonIndex];
                
                if(this->buttonIndex < 2)
                {
                    button_vertices = this->ui_contents->files[this->buttonIndex].file_button;
                }
                else
                {
                    button_vertices = this->ui_contents->buttons[1];
                }
            } 
            else if ((this->configMode == CM_ERASE_CONFIRM) || (this->configMode == CM_COPY_CONFIRM)) 
            {
                // j = D_8081464C[this->buttonIndex];
                button_vertices = this->ui_contents->buttons[this->buttonIndex];
            } 
            else 
            {
                if(this->buttonIndex < 3)
                {
                    button_vertices = this->ui_contents->files[this->buttonIndex].file_button;
                }
                else
                {
                    button_vertices = this->ui_contents->buttons[this->buttonIndex - 3];
                }
            }
        } 
        else 
        {
            // j = D_80814650[this->confirmButtonIndex];
            button_vertices = this->ui_contents->buttons[this->confirmButtonIndex];
        }

        // this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowPosX - 0xA;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[j].v.ob[1] + 4;

        // this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x48;
        // this->windowContentVtx[vtxId + 1].v.ob[1] = this->windowContentVtx[j].v.ob[1] + 4;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = 0x900;

        // this->windowContentVtx[vtxId + 2].v.ob[0] = this->windowPosX - 0xA;
        // this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x18;
        // this->windowContentVtx[vtxId + 2].v.tc[1] = 0x300;

        // this->windowContentVtx[vtxId + 3].v.ob[0] = this->windowContentVtx[vtxId + 0].v.ob[0] + 0x48;
        // this->windowContentVtx[vtxId + 3].v.ob[1] = this->windowContentVtx[vtxId + 0].v.ob[1] - 0x18;        
        // this->windowContentVtx[vtxId + 3].v.tc[0] = 0x900;
        // this->windowContentVtx[vtxId + 3].v.tc[1] = 0x300;

        this->ui_contents->highlighted_option[0].v.ob[0] = this->windowPosX - 0xA;
        this->ui_contents->highlighted_option[0].v.ob[1] = button_vertices->v.ob[1] + 4;

        this->ui_contents->highlighted_option[1].v.ob[0] = this->ui_contents->highlighted_option[0].v.ob[0] + 0x48;
        this->ui_contents->highlighted_option[1].v.ob[1] = button_vertices->v.ob[1] + 4;
        this->ui_contents->highlighted_option[1].v.tc[0] = 0x900;

        this->ui_contents->highlighted_option[2].v.ob[0] = this->windowPosX - 0xA;
        this->ui_contents->highlighted_option[2].v.ob[1] = this->ui_contents->highlighted_option[0].v.ob[1] - 0x18;
        this->ui_contents->highlighted_option[2].v.tc[1] = 0x300;

        this->ui_contents->highlighted_option[3].v.ob[0] = this->ui_contents->highlighted_option[0].v.ob[0] + 0x48;
        this->ui_contents->highlighted_option[3].v.ob[1] = this->ui_contents->highlighted_option[0].v.ob[1] - 0x18;        
        this->ui_contents->highlighted_option[3].v.tc[0] = 0x900;
        this->ui_contents->highlighted_option[3].v.tc[1] = 0x300;
    }

    // this->windowContentVtx[vtxId + 4].v.ob[0] = this->windowPosX + 0x3A;
    // this->windowContentVtx[vtxId + 4].v.ob[1] = this->windowContentVtx[D_80814638[this->warningButtonIndex]].v.ob[1];

    // this->windowContentVtx[vtxId + 5].v.ob[0] = this->windowContentVtx[vtxId + 4].v.ob[0] + 0x80;
    // this->windowContentVtx[vtxId + 5].v.ob[1] = this->windowContentVtx[D_80814638[this->warningButtonIndex]].v.ob[1];
    // this->windowContentVtx[vtxId + 5].v.tc[0] = 0x1000;

    // this->windowContentVtx[vtxId + 6].v.ob[0] = this->windowPosX + 0x3A;
    // this->windowContentVtx[vtxId + 6].v.ob[1] = this->windowContentVtx[vtxId + 4].v.ob[1] - 0x10;

    // this->windowContentVtx[vtxId + 7].v.ob[0] = this->windowContentVtx[vtxId + 4].v.ob[0] + 0x80;
    // this->windowContentVtx[vtxId + 7].v.ob[1] = this->windowContentVtx[vtxId + 4].v.ob[1] - 0x10;
    // this->windowContentVtx[vtxId + 7].v.tc[0] = 0x1000;

    if(this->menuMode == FS_MENU_MODE_CHAOS_CONFIG)
    {
        /* effect config buttons */
        posX = this->windowPosX - 4;
        // posY = 0x09;
        posY = 0x18;
        vtxId = FILE_SELECT_WINDOW_CONTENT_VERT_COUNT; 
         
        for(option_index = 0; option_index < FILE_SELECT_CHAOS_SETTINGS_MAX_VISIBLE_SETTINGS; option_index++, vtxId += 4)
        {
            // /* top-left corner */
            // this->windowContentVtx[vtxId + 0].v.ob[0] = -FILE_SELECT_CHAOS_SETTING_OPTION_WIDTH / 2 + 2;
            // this->windowContentVtx[vtxId + 0].v.ob[1] = posY;
            // this->windowContentVtx[vtxId + 0].v.tc[0] = TC_10_5(0, 0);
            // this->windowContentVtx[vtxId + 0].v.tc[1] = TC_10_5(0, 0); 

            // /* top-right corner */
            // this->windowContentVtx[vtxId + 1].v.ob[0] = FILE_SELECT_CHAOS_SETTING_OPTION_WIDTH / 2 + 2;
            // this->windowContentVtx[vtxId + 1].v.ob[1] = posY;
            // this->windowContentVtx[vtxId + 1].v.tc[0] = TC_10_5(51, 0);
            // this->windowContentVtx[vtxId + 1].v.tc[1] = TC_10_5(0, 0);

            // /* bottom-left corner */
            // this->windowContentVtx[vtxId + 2].v.ob[0] = -FILE_SELECT_CHAOS_SETTING_OPTION_WIDTH / 2 + 2;
            // this->windowContentVtx[vtxId + 2].v.ob[1] = posY - FILE_SELECT_CHAOS_SETTING_OPTION_HEIGHT;
            // this->windowContentVtx[vtxId + 2].v.tc[0] = TC_10_5(0, 0);
            // this->windowContentVtx[vtxId + 2].v.tc[1] = TC_10_5(15, 0);
            
            // /* bottom-right corner */
            // this->windowContentVtx[vtxId + 3].v.ob[0] = FILE_SELECT_CHAOS_SETTING_OPTION_WIDTH / 2 + 2;
            // this->windowContentVtx[vtxId + 3].v.ob[1] = posY - FILE_SELECT_CHAOS_SETTING_OPTION_HEIGHT;
            // this->windowContentVtx[vtxId + 3].v.tc[0] = TC_10_5(51, 0);
            // this->windowContentVtx[vtxId + 3].v.tc[1] = TC_10_5(15, 0);

            /* top-left corner */
            this->ui_contents->chaos_options[option_index][0].v.ob[0] = -FILE_SELECT_CHAOS_SETTING_OPTION_WIDTH / 2 + 2;
            this->ui_contents->chaos_options[option_index][0].v.ob[1] = posY;
            this->ui_contents->chaos_options[option_index][0].v.tc[0] = TC_10_5(0, 0);
            this->ui_contents->chaos_options[option_index][0].v.tc[1] = TC_10_5(0, 0); 
            /* top-right corner */
            this->ui_contents->chaos_options[option_index][1].v.ob[0] = FILE_SELECT_CHAOS_SETTING_OPTION_WIDTH / 2 + 2;
            this->ui_contents->chaos_options[option_index][1].v.ob[1] = posY;
            this->ui_contents->chaos_options[option_index][1].v.tc[0] = TC_10_5(51, 0);
            this->ui_contents->chaos_options[option_index][1].v.tc[1] = TC_10_5(0, 0);
            /* bottom-left corner */
            this->ui_contents->chaos_options[option_index][2].v.ob[0] = -FILE_SELECT_CHAOS_SETTING_OPTION_WIDTH / 2 + 2;
            this->ui_contents->chaos_options[option_index][2].v.ob[1] = posY - FILE_SELECT_CHAOS_SETTING_OPTION_HEIGHT;
            this->ui_contents->chaos_options[option_index][2].v.tc[0] = TC_10_5(0, 0);
            this->ui_contents->chaos_options[option_index][2].v.tc[1] = TC_10_5(15, 0);
            /* bottom-right corner */
            this->ui_contents->chaos_options[option_index][3].v.ob[0] = FILE_SELECT_CHAOS_SETTING_OPTION_WIDTH / 2 + 2;
            this->ui_contents->chaos_options[option_index][3].v.ob[1] = posY - FILE_SELECT_CHAOS_SETTING_OPTION_HEIGHT;
            this->ui_contents->chaos_options[option_index][3].v.tc[0] = TC_10_5(51, 0);
            this->ui_contents->chaos_options[option_index][3].v.tc[1] = TC_10_5(15, 0);

            posY -= FILE_SELECT_CHAOS_SETTING_OPTION_HEIGHT + 1;
        }

        if(this->chaos_config_option_index >= 0)
        {
            /* highlighted chaos option */ 
            Vtx *chaos_config_vtx = this->ui_contents->chaos_options[this->chaos_config_option_index];

            // /* top-left corner */
            // this->windowContentVtx[vtxId + 0].v.ob[0] = chaos_config_vtx[0].v.ob[0] - 2;
            // this->windowContentVtx[vtxId + 0].v.ob[1] = chaos_config_vtx[0].v.ob[1] + 2;
            // this->windowContentVtx[vtxId + 0].v.tc[0] = TC_10_5(0, 0);
            // this->windowContentVtx[vtxId + 0].v.tc[1] = TC_10_5(0, 0);

            // /* top-right corner */
            // this->windowContentVtx[vtxId + 1].v.ob[0] = chaos_config_vtx[1].v.ob[0] + 2;
            // this->windowContentVtx[vtxId + 1].v.ob[1] = chaos_config_vtx[1].v.ob[1] + 2;
            // this->windowContentVtx[vtxId + 1].v.tc[0] = TC_10_5(52, 0);
            // this->windowContentVtx[vtxId + 1].v.tc[1] = TC_10_5(0, 0);

            // /* bottom-left corner */
            // this->windowContentVtx[vtxId + 2].v.ob[0] = chaos_config_vtx[2].v.ob[0] - 2;
            // this->windowContentVtx[vtxId + 2].v.ob[1] = chaos_config_vtx[2].v.ob[1] - 2;
            // this->windowContentVtx[vtxId + 2].v.tc[0] = TC_10_5(0, 0);
            // this->windowContentVtx[vtxId + 2].v.tc[1] = TC_10_5(16, 0);
            
            // /* bottom-right corner */
            // this->windowContentVtx[vtxId + 3].v.ob[0] = chaos_config_vtx[3].v.ob[0] + 2;            
            // this->windowContentVtx[vtxId + 3].v.ob[1] = chaos_config_vtx[3].v.ob[1] - 2;
            // this->windowContentVtx[vtxId + 3].v.tc[0] = TC_10_5(52, 0);
            // this->windowContentVtx[vtxId + 3].v.tc[1] = TC_10_5(16, 0);            


            /* top-left corner */
            this->ui_contents->chaos_highlighted_option[0].v.ob[0] = chaos_config_vtx[0].v.ob[0] - 2;
            this->ui_contents->chaos_highlighted_option[0].v.ob[1] = chaos_config_vtx[0].v.ob[1] + 2;
            this->ui_contents->chaos_highlighted_option[0].v.tc[0] = TC_10_5(0, 0);
            this->ui_contents->chaos_highlighted_option[0].v.tc[1] = TC_10_5(0, 0);
            /* top-right corner */
            this->ui_contents->chaos_highlighted_option[1].v.ob[0] = chaos_config_vtx[1].v.ob[0] + 2;
            this->ui_contents->chaos_highlighted_option[1].v.ob[1] = chaos_config_vtx[1].v.ob[1] + 2;
            this->ui_contents->chaos_highlighted_option[1].v.tc[0] = TC_10_5(52, 0);
            this->ui_contents->chaos_highlighted_option[1].v.tc[1] = TC_10_5(0, 0);
            /* bottom-left corner */
            this->ui_contents->chaos_highlighted_option[2].v.ob[0] = chaos_config_vtx[2].v.ob[0] - 2;
            this->ui_contents->chaos_highlighted_option[2].v.ob[1] = chaos_config_vtx[2].v.ob[1] - 2;
            this->ui_contents->chaos_highlighted_option[2].v.tc[0] = TC_10_5(0, 0);
            this->ui_contents->chaos_highlighted_option[2].v.tc[1] = TC_10_5(16, 0);
            /* bottom-right corner */
            this->ui_contents->chaos_highlighted_option[3].v.ob[0] = chaos_config_vtx[3].v.ob[0] + 2;            
            this->ui_contents->chaos_highlighted_option[3].v.ob[1] = chaos_config_vtx[3].v.ob[1] - 2;
            this->ui_contents->chaos_highlighted_option[3].v.tc[0] = TC_10_5(52, 0);
            this->ui_contents->chaos_highlighted_option[3].v.tc[1] = TC_10_5(16, 0);            
        }

        vtxId += 4;

        {
            /* chaos config description textbox */
            f32 scale = 1.0f;

            if(this->chaos_config_description_alpha < 180)
            {
                scale = (f32)this->chaos_config_description_alpha / 160.0f;
            }
            else if(this->chaos_config_description_alpha < 200)
            {
                scale = (f32)(180 - (this->chaos_config_description_alpha - 180)) / 160.0f;
            }
            // vtxId += 4;

            // /* top-left corner */
            // this->windowContentVtx[vtxId + 0].v.ob[0] = -0x90 * scale;
            // this->windowContentVtx[vtxId + 0].v.ob[1] = 0x30 * scale;
            // this->windowContentVtx[vtxId + 0].v.tc[0] = TC_10_5(0, 0);
            // this->windowContentVtx[vtxId + 0].v.tc[1] = TC_10_5(0, 0);

            // /* top-right corner */
            // this->windowContentVtx[vtxId + 1].v.ob[0] = 0x90 * scale;
            // this->windowContentVtx[vtxId + 1].v.ob[1] = 0x30 * scale;
            // this->windowContentVtx[vtxId + 1].v.tc[0] = TC_10_5(255, 0);
            // this->windowContentVtx[vtxId + 1].v.tc[1] = TC_10_5(0, 0);

            // /* bottom-left corner */
            // this->windowContentVtx[vtxId + 2].v.ob[0] = -0x90 * scale;
            // this->windowContentVtx[vtxId + 2].v.ob[1] = -0x30 * scale;
            // this->windowContentVtx[vtxId + 2].v.tc[0] = TC_10_5(0, 0);
            // this->windowContentVtx[vtxId + 2].v.tc[1] = TC_10_5(63, 0);
            
            // /* bottom-right corner */
            // this->windowContentVtx[vtxId + 3].v.ob[0] = 0x90 * scale;            
            // this->windowContentVtx[vtxId + 3].v.ob[1] = -0x30 * scale;
            // this->windowContentVtx[vtxId + 3].v.tc[0] = TC_10_5(255, 0);
            // this->windowContentVtx[vtxId + 3].v.tc[1] = TC_10_5(63, 0);

            /* top-left corner */
            this->ui_contents->chaos_text_box[0].v.ob[0] = -0x90 * scale;
            this->ui_contents->chaos_text_box[0].v.ob[1] = 0x30 * scale;
            this->ui_contents->chaos_text_box[0].v.tc[0] = TC_10_5(0, 0);
            this->ui_contents->chaos_text_box[0].v.tc[1] = TC_10_5(0, 0);

            /* top-right corner */
            this->ui_contents->chaos_text_box[1].v.ob[0] = 0x90 * scale;
            this->ui_contents->chaos_text_box[1].v.ob[1] = 0x30 * scale;
            this->ui_contents->chaos_text_box[1].v.tc[0] = TC_10_5(255, 0);
            this->ui_contents->chaos_text_box[1].v.tc[1] = TC_10_5(0, 0);

            /* bottom-left corner */
            this->ui_contents->chaos_text_box[2].v.ob[0] = -0x90 * scale;
            this->ui_contents->chaos_text_box[2].v.ob[1] = -0x30 * scale;
            this->ui_contents->chaos_text_box[2].v.tc[0] = TC_10_5(0, 0);
            this->ui_contents->chaos_text_box[2].v.tc[1] = TC_10_5(63, 0);
            
            /* bottom-right corner */
            this->ui_contents->chaos_text_box[3].v.ob[0] = 0x90 * scale;            
            this->ui_contents->chaos_text_box[3].v.ob[1] = -0x30 * scale;
            this->ui_contents->chaos_text_box[3].v.tc[0] = TC_10_5(255, 0);
            this->ui_contents->chaos_text_box[3].v.tc[1] = TC_10_5(63, 0);

            vtxId += 4;
        }


        // posX += FILE_SELECT_CHAOS_SETTING_TAB_WIDTH / 2;
        // posY = 0x1a;

        // vtxId = FILE_SELECT_WINDOW_CONTENT_TOTAL_VERT_COUNT - 12;

        // for(i = 0; i < 2; i++, vtxId += 4)
        // {
        //     /* top-left corner */
        //     this->windowContentVtx[vtxId + 0].v.ob[0] = posX - FILE_SELECT_CHAOS_SETTING_TAB_WIDTH / 2 + 2;
        //     this->windowContentVtx[vtxId + 0].v.ob[1] = posY;
        //     this->windowContentVtx[vtxId + 0].v.tc[0] = TC_10_5(0, 0);
        //     this->windowContentVtx[vtxId + 0].v.tc[1] = TC_10_5(0, 0); 

        //     /* top-right corner */
        //     this->windowContentVtx[vtxId + 1].v.ob[0] = posX + FILE_SELECT_CHAOS_SETTING_TAB_WIDTH / 2 + 2;
        //     this->windowContentVtx[vtxId + 1].v.ob[1] = posY;
        //     this->windowContentVtx[vtxId + 1].v.tc[0] = TC_10_5(51, 0);
        //     this->windowContentVtx[vtxId + 1].v.tc[1] = TC_10_5(0, 0);

        //     /* bottom-left corner */
        //     this->windowContentVtx[vtxId + 2].v.ob[0] = posX - FILE_SELECT_CHAOS_SETTING_TAB_WIDTH / 2 + 2;
        //     this->windowContentVtx[vtxId + 2].v.ob[1] = posY - FILE_SELECT_CHAOS_SETTING_TAB_HEIGHT;
        //     this->windowContentVtx[vtxId + 2].v.tc[0] = TC_10_5(0, 0);
        //     this->windowContentVtx[vtxId + 2].v.tc[1] = TC_10_5(15, 0);
            
        //     /* bottom-right corner */
        //     this->windowContentVtx[vtxId + 3].v.ob[0] = posX + FILE_SELECT_CHAOS_SETTING_TAB_WIDTH / 2 + 2;
        //     this->windowContentVtx[vtxId + 3].v.ob[1] = posY - FILE_SELECT_CHAOS_SETTING_TAB_HEIGHT;
        //     this->windowContentVtx[vtxId + 3].v.tc[0] = TC_10_5(51, 0);
        //     this->windowContentVtx[vtxId + 3].v.tc[1] = TC_10_5(15, 0);

        //     posX += FILE_SELECT_CHAOS_SETTING_TAB_WIDTH + 1;
        // }


        // chaos_config_vtx = this->windowContentVtx + (FILE_SELECT_WINDOW_CONTENT_TOTAL_VERT_COUNT - 12) + this->chaos_config_tab_index * 4;
        // /* top-left corner */
        // this->windowContentVtx[vtxId + 0].v.ob[0] = chaos_config_vtx[0].v.ob[0] - 2;
        // this->windowContentVtx[vtxId + 0].v.ob[1] = chaos_config_vtx[0].v.ob[1] + 2;
        // this->windowContentVtx[vtxId + 0].v.tc[0] = TC_10_5(0, 0);
        // this->windowContentVtx[vtxId + 0].v.tc[1] = TC_10_5(0, 0);

        // /* top-right corner */
        // this->windowContentVtx[vtxId + 1].v.ob[0] = chaos_config_vtx[1].v.ob[0] + 2;
        // this->windowContentVtx[vtxId + 1].v.ob[1] = chaos_config_vtx[1].v.ob[1] + 2;
        // this->windowContentVtx[vtxId + 1].v.tc[0] = TC_10_5(52, 0);
        // this->windowContentVtx[vtxId + 1].v.tc[1] = TC_10_5(0, 0);

        // /* bottom-left corner */
        // this->windowContentVtx[vtxId + 2].v.ob[0] = chaos_config_vtx[2].v.ob[0] - 2;
        // this->windowContentVtx[vtxId + 2].v.ob[1] = chaos_config_vtx[2].v.ob[1] - 2;
        // this->windowContentVtx[vtxId + 2].v.tc[0] = TC_10_5(0, 0);
        // this->windowContentVtx[vtxId + 2].v.tc[1] = TC_10_5(16, 0);
        
        // /* bottom-right corner */
        // this->windowContentVtx[vtxId + 3].v.ob[0] = chaos_config_vtx[3].v.ob[0] + 2;            
        // this->windowContentVtx[vtxId + 3].v.ob[1] = chaos_config_vtx[3].v.ob[1] - 2;
        // this->windowContentVtx[vtxId + 3].v.tc[0] = TC_10_5(52, 0);
        // this->windowContentVtx[vtxId + 3].v.tc[1] = TC_10_5(16, 0);            
    }
}

u16 D_80814654[] = {
    0x88,
    0x194,
    0x2A0,
};
TexturePtr sFileSelRemainsTextures[] = {
    gFileSelOdolwasRemainsTex,
    gFileSelGohtsRemainsTex,
    gFileSelGyorgsRemainsTex,
    gFileSelTwinmoldsRemainsTex,
};
TexturePtr sFileSelDayENGTextures[] = {
    gFileSelFirstDayENGTex,
    gFileSelFirstDayENGTex,
    gFileSelSecondDayENGTex,
    gFileSelFinalDayENGTex,
};
TexturePtr sFileSelHeartPieceTextures[] = {
    gFileSel0QuarterHeartENGTex,
    gFileSel1QuarterHeartENGTex,
    gFileSel2QuarterHeartENGTex,
    gFileSel3QuarterHeartENGTex,
};
static TexturePtr sHeartTextures[2][5] = {
    {
        gHeartEmptyTex,
        gHeartQuarterTex,
        gHeartHalfTex,
        gHeartThreeQuarterTex,
        gHeartFullTex,
    },
    {
        gDefenseHeartEmptyTex,
        gDefenseHeartQuarterTex,
        gDefenseHeartHalfTex,
        gDefenseHeartThreeQuarterTex,
        gDefenseHeartFullTex,
    },
};
u8 sHealthToQuarterHeartCount[] = {
    0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3,
};
s16 sFileSelRupeePrimColors[3][3] = {
    { 200, 255, 100 }, // Default Wallet
    { 170, 170, 255 }, // Adult Wallet
    { 255, 105, 105 }, // Giant Wallet
};
s16 sFileSelRupeeEnvColors[3][3] = {
    { 0, 80, 0 },   // Default Wallet
    { 10, 10, 80 }, // Adult Wallet
    { 40, 10, 0 },  // Giant Wallet
};
static s16 sHeartPrimColors[2][3] = {
    { 255, 70, 50 },
    { 200, 0, 0 },
};
static s16 sHeartEnvColors[2][3] = {
    { 50, 40, 60 },
    { 255, 255, 255 },
};

void FileSelect_DrawFileInfo(GameState* thisx, s16 fileIndex) {
    FileSelectState* this = (FileSelectState*)thisx;
    Font* font = &this->font;
    // s16 j;
    s16 vtxOffset;
    s32 heartType;
    s16 heart_count;
    s16 heart_index;
    s16 remains_index;
    s16 i;
    s16 sp20C;
    s16 health;
    s16 timeDigits[5];
    u16 digits[3]; // rupees and mask count
    u8 quarterHeartCount;
    struct save_info_t *save_info = &this->save_info[fileIndex];

    OPEN_DISPS(this->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

    // sp20C = fileIndex;

    {
        // draw file name
        if (this->nameAlpha[fileIndex] != 0) {
            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + (4 * 8)], 32, 0);
            gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].file_name_shadow, 32, 0);
            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 0, 0, 0, this->nameAlpha[fileIndex]);

            for (vtxOffset = 0, i = 0; vtxOffset < (4 * 8); i++, vtxOffset += 4) {
                FileSelect_DrawTexQuadI4(this->state.gfxCtx,
                                        font->fontBuf + save_info->fileName[i] * FONT_CHAR_TEX_SIZE, vtxOffset);
            }

            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex]], 32, 0);
            gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].file_name, 32, 0);
            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 255, 255, 255, this->nameAlpha[fileIndex]);

            for (vtxOffset = 0, i = 0; vtxOffset < (4 * 8); i++, vtxOffset += 4) {
                FileSelect_DrawTexQuadI4(this->state.gfxCtx,
                                        font->fontBuf + save_info->fileName[i] * FONT_CHAR_TEX_SIZE, vtxOffset);
            }
        }
    }

    if ((fileIndex == this->selectedFileIndex) || (fileIndex == this->copyDestFileIndex)) 
    {
        if (this->save_info[fileIndex + 2].isOwlSave || this->save_info[fileIndex + 2].is_crash_save) 
        {
            save_info = &this->save_info[fileIndex + 2];
        }

        {
            /* rupee digits and shadow */
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, sFileSelRupeePrimColors[save_info->walletUpgrades][0],
                            sFileSelRupeePrimColors[save_info->walletUpgrades][1],
                            sFileSelRupeePrimColors[save_info->walletUpgrades][2], this->fileInfoAlpha[fileIndex]);
            gDPSetEnvColor(POLY_OPA_DISP++, sFileSelRupeeEnvColors[save_info->walletUpgrades][0],
                        sFileSelRupeeEnvColors[save_info->walletUpgrades][1],
                        sFileSelRupeeEnvColors[save_info->walletUpgrades][2], 255);

            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0xC8], 4, 0);
            gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].rupee_icon, 4, 0);

            gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelRupeeTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                                G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                            PRIMITIVE, 0);

            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 0, 0, 0, this->fileInfoAlpha[fileIndex]);
            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0x4C], 12, 0);
            gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].rupee_digits_shadow, 12, 0);

            FileSelect_SplitNumber((u16)save_info->rupees, &digits[0], &digits[1], &digits[2]);

            for (vtxOffset = 0, i = sWalletFirstDigit[save_info->walletUpgrades]; i < 3; i++, vtxOffset += 4) {
                FileSelect_DrawTexQuadI4(this->state.gfxCtx, font->fontBuf + digits[i] * FONT_CHAR_TEX_SIZE, vtxOffset);
            }

            if (save_info->rupees == gUpgradeCapacities[4][save_info->walletUpgrades]) {
                gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 120, 255, 0, this->fileInfoAlpha[fileIndex]);
            } else if (save_info->rupees != 0) {
                gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
            } else {
                gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 100, 100, 100, this->fileInfoAlpha[fileIndex]);
            }

            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0x40], 12, 0);
            gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].rupee_digits, 12, 0);

            for (vtxOffset = 0, i = sWalletFirstDigit[save_info->walletUpgrades]; i < 3; i++, vtxOffset += 4) {
                FileSelect_DrawTexQuadI4(this->state.gfxCtx, font->fontBuf + digits[i] * FONT_CHAR_TEX_SIZE, vtxOffset);
            }
        }

        {

            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                            PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 255, 0, 0, this->fileInfoAlpha[fileIndex]);
            gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);

            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0xCC], 4, 0);

            gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].heart_container_pieces, 4, 0);

            POLY_OPA_DISP = FileSelect_DrawTexQuadIA8(POLY_OPA_DISP, sFileSelHeartPieceTextures[save_info->heartPieceCount], 0x18, 0x10, 0);
        }

        // Fault_AddHangupPrintfAndCrash("HERE");
        // gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, sHeartPrimColors[heartType][0], sHeartPrimColors[heartType][1],
        //                 sHeartPrimColors[heartType][2], this->fileInfoAlpha[fileIndex]);
        // gDPSetEnvColor(POLY_OPA_DISP++, sHeartEnvColors[heartType][0], sHeartEnvColors[heartType][1],
        //                sHeartEnvColors[heartType][2], 255);

        {
            /* hearts */
            if (save_info->defenseHearts == 0) {
                heartType = 0;
            } else {
                heartType = 1;
            }

            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, sHeartPrimColors[heartType][0], sHeartPrimColors[heartType][1],
                            sHeartPrimColors[heartType][2], this->fileInfoAlpha[fileIndex]);
            gDPSetEnvColor(POLY_OPA_DISP++, sHeartEnvColors[heartType][0], sHeartEnvColors[heartType][1],
                        sHeartEnvColors[heartType][2], 255);

            heart_count = save_info->healthCapacity / 0x10;

            health = save_info->health;
            if (health <= 0x30) {
                health = 0x30;
            }

            quarterHeartCount = 4;
            for (vtxOffset = 0, heart_index = 0; heart_index < heart_count; heart_index++, vtxOffset += 4) 
            {
                if (health < 0x10) 
                {
                    if (health != 0) 
                    {
                        quarterHeartCount = sHealthToQuarterHeartCount[health];
                        health = 0;
                    } 
                    else 
                    {
                        quarterHeartCount = 0;
                    }
                } 
                else 
                {
                    health -= 0x10;
                }

                // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0x68 + vtxOffset], 4, 0);
                gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].hearts[heart_index], 4, 0);
                POLY_OPA_DISP = FileSelect_DrawTexQuadIA8(POLY_OPA_DISP, sHeartTextures[heartType][quarterHeartCount], 0x10, 0x10, 0);
            }

            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0x68 + vtxOffset], 4, 0);
            // POLY_OPA_DISP =
            //     FileSelect_DrawTexQuadIA8(POLY_OPA_DISP, sHeartTextures[heartType][quarterHeartCount], 0x10, 0x10, 0);
        }

        {
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
            gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);

            for (vtxOffset = 0, remains_index = 0; remains_index < 4; remains_index++, vtxOffset += 4) 
            {
                if (save_info->questItems & gBitFlags[remains_index])
                {
                    // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0xB8 + vtxOffset], 4, 0);
                    gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].remains_masks[remains_index], 4, 0);
                    gDPLoadTextureBlock(POLY_OPA_DISP++, sFileSelRemainsTextures[remains_index], G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0,
                                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                        G_TX_NOLOD, G_TX_NOLOD);
                    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
                }
            }
        }

        {
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                            PRIMITIVE, 0);
 
            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 0, 0, 0, this->fileInfoAlpha[fileIndex]);
            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0xD0], 8, 0);
            gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].mask_text, 8, 0);
            gDPLoadTextureBlock_4b(POLY_OPA_DISP++, gFileSelMASKSENGTex, G_IM_FMT_I, 64, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 4, 6, 7, 5, 0);
            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

            gDPPipeSync(POLY_OPA_DISP++);

            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 0, 0, 0, this->fileInfoAlpha[fileIndex]);
            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0x60], 8, 0);
            gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].mask_count_digits_shadow, 8, 0);

            FileSelect_SplitNumber(save_info->maskCount, &digits[0], &digits[1], &digits[2]);

            for (vtxOffset = 0, i = 1; i < 3; i++, vtxOffset += 4) {
                FileSelect_DrawTexQuadI4(this->state.gfxCtx, font->fontBuf + digits[i] * FONT_CHAR_TEX_SIZE, vtxOffset);
            }

            gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 255, 255, 255, this->fileInfoAlpha[fileIndex]);

            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0x58], 8, 0);
            gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[fileIndex].mask_count_digits, 8, 0);

            for (vtxOffset = 0, i = 1; i < 3; i++, vtxOffset += 4) {

                FileSelect_DrawTexQuadI4(this->state.gfxCtx, font->fontBuf + digits[i] * FONT_CHAR_TEX_SIZE, vtxOffset);
            }
        }
    }

    if ((this->save_info[fileIndex + 2].isOwlSave || this->save_info[fileIndex + 2].is_crash_save) && this->chaos_config_box_alpha == 0) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

        gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 255, 255, 255, this->nameAlpha[fileIndex]);        

        if(this->save_info[fileIndex + 2].is_crash_save)
        {
            gSPVertex(POLY_OPA_DISP++, this->ui_contents->files[fileIndex].crash_icon, 4, 0);
            gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelCrashSaveTex, G_IM_FMT_IA, G_IM_SIZ_8b, 48, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                                G_TX_NOLOD);
        }
        else
        {
            gSPVertex(POLY_OPA_DISP++, this->ui_contents->files[fileIndex].owl_icon, 4, 0);
            gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelOwlSaveIconTex, G_IM_FMT_RGBA, G_IM_SIZ_32b, 24, 12, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                                G_TX_NOLOD);
        }
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

        // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0xDC], 8, 0);
        gSPVertex(POLY_OPA_DISP++, this->ui_contents->files[fileIndex].day_text, 8, 0);

        gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 0, 0, 0, this->fileInfoAlpha[fileIndex]);

        gDPLoadTextureBlock_4b(POLY_OPA_DISP++, sFileSelDayENGTextures[save_info->day], G_IM_FMT_I, 48, 24, 0,
                               G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                               G_TX_NOLOD, G_TX_NOLOD);
        gSP1Quadrangle(POLY_OPA_DISP++, 4, 6, 7, 5, 0);

        gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

        timeDigits[0] = 0;
        timeDigits[1] = TIME_TO_MINUTES_F(save_info->time) / 60.0f;

        while (timeDigits[1] >= 10) {
            timeDigits[0]++;
            timeDigits[1] -= 10;
        }

        timeDigits[3] = 0;
        timeDigits[4] = (s32)TIME_TO_MINUTES_F(save_info->time) % 60;

        while (timeDigits[4] >= 10) {
            timeDigits[3]++;
            timeDigits[4] -= 10;
        }
        timeDigits[2] = 0x41;

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                          PRIMITIVE, 0);

        gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 0, 0, 0, this->fileInfoAlpha[fileIndex]);
        // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0xF8], 20, 0);
        gSPVertex(POLY_OPA_DISP++, this->ui_contents->files[fileIndex].time_digits_shadow, 20, 0);

        for (i = 0, vtxOffset = 0; i < 5; i++, vtxOffset += 4) {
            FileSelect_DrawTexQuadI4(this->state.gfxCtx, font->fontBuf + timeDigits[i] * FONT_CHAR_TEX_SIZE, vtxOffset);
        }

        gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
        // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80814654[fileIndex] + 0xE4], 20, 0);
        gSPVertex(POLY_OPA_DISP++, this->ui_contents->files[fileIndex].time_digits, 20, 0);

        for (i = 0, vtxOffset = 0; i < 5; i++, vtxOffset += 4) {
            FileSelect_DrawTexQuadI4(this->state.gfxCtx, font->fontBuf + timeDigits[i] * FONT_CHAR_TEX_SIZE, vtxOffset);
        }
    }

    gDPPipeSync(POLY_OPA_DISP++);

    CLOSE_DISPS(this->state.gfxCtx);
}

TexturePtr sFileInfoBoxTextures[] = {
    gFileSelFileInfoBox0Tex, gFileSelFileInfoBox1Tex,      gFileSelFileInfoBox2Tex,      gFileSelFileInfoBox3Tex,
    gFileSelFileInfoBox4Tex, gFileSelFileExtraInfoBox0Tex, gFileSelFileExtraInfoBox1Tex,
};

TexturePtr sChaosConfigBoxTopTextures[] = {
    gFileSelFileInfoBox0Tex, 
    gFileSelFileInfoBox1Tex,      
    gFileSelFileInfoBox2Tex,
    gFileSelFileInfoBox2Tex,
    gFileSelFileInfoBox3Tex,
    gFileSelFileInfoBox4Tex
};

// TexturePtr sChaosConfigBoxBottomTextures[] = {
//     gFileSelFileExtraInfoBox0Tex, 
//     gFileSelFileExtraInfoBox0Tex,      
//     gFileSelFileExtraInfoBox0Tex,      
//     gFileSelFileExtraInfoBox0Tex,
//     gFileSelFileExtraInfoBox0Tex
//     // gFileSelFileInfoBox0Tex,
//     // gFileSelFileInfoBox1Tex,
//     // gFileSelFileInfoBox2Tex,
//     // gFileSelFileInfoBox3Tex,
//     // gFileSelFileInfoBox4Tex,
// };

TexturePtr sTitleLabels[] = {
    gFileSelPleaseSelectAFileENGTex, gFileSelOpenThisFileENGTex,    gFileSelCopyWhichFileENGTex,
    gFileSelCopyToWhichFileENGTex,   gFileSelAreYouSureCopyENGTex,  gFileSelFileCopiedENGTex,
    gFileSelEraseWhichFileENGTex,    gFileSelAreYouSureEraseENGTex, gFileSelFileErasedENGTex,
    gChaosConfigEngTex
};

TexturePtr sWarningLabels[] = {
    gFileSelNoFileToCopyENGTex, gFileSelNoFileToEraseENGTex, gFileSelNoEmptyFileENGTex,
    gFileSelFileEmptyENGTex,    gFileSelFileInUseENGTex,
};

TexturePtr sFileButtonTextures[] = {
    gFileSelFile1ButtonENGTex,
    gFileSelFile2ButtonENGTex,
    gFileSelFile3ButtonENGTex,
};

TexturePtr sActionButtonTextures[] = {
    gFileSelCopyButtonENGTex,
    gFileSelEraseButtonENGTex,
    gFileSelYesButtonENGTex,
    gFileSelQuitButtonENGTex,
    gFileSelOptionsButtonENGTex
};

/**
 * Draw most window contents including buttons, labels, and icons.
 * Does not include anything from the keyboard and settings windows.
 */
void FileSelect_DrawWindowContents(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    GfxPrint gfx_print;
    s16 fileIndex;
    s16 temp;
    s16 i;
    s16 quadVtxIndex;

    static Color_RGB16 sIconPrimColors[] = {
        { 0, 80, 200 },
        { 50, 130, 255 },
    };
    static Color_RGB16 sIconEnvColors[] = {
        { 0, 0, 0 },
        { 0, 130, 255 },
    };
    static s16 sIconPrimR = 0;
    static s16 sIconPrimG = 80;
    static s16 sIconPrimB = 200;
    static s16 sIconFlashTimer = 12;
    static s16 sIconFlashColorIndex = 0;
    static s16 sIconEnvR = 0;
    static s16 sIconEnvG = 0;
    static s16 sIconEnvB = 0;

    s16 primR;
    s16 primG;
    s16 primB;
    s16 envR;
    s16 envG;
    s16 envB;

    OPEN_DISPS(this->state.gfxCtx);

    // draw title label
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->titleAlpha[FS_TITLE_CUR]);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

    // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[0], 4, 0);
    gSPVertex(POLY_OPA_DISP++, &this->ui_contents->title_label, 4, 0);
    gDPLoadTextureBlock(POLY_OPA_DISP++, sTitleLabels[this->titleLabel], G_IM_FMT_IA, G_IM_SIZ_8b, 128, 16, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

    // draw next title label
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->titleAlpha[FS_TITLE_NEXT]);
    gDPLoadTextureBlock(POLY_OPA_DISP++, sTitleLabels[this->nextTitleLabel], G_IM_FMT_IA, G_IM_SIZ_8b, 128, 16, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
 
    temp = 4;

    gDPPipeSync(POLY_OPA_DISP++);
    // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[temp], 28, 0);
    gSPVertex(POLY_OPA_DISP++, &this->ui_contents->file_info_box, 28, 0);
    // draw file info box (large box when a file is selected)
    for (fileIndex = 0; fileIndex < 3; fileIndex++ /*, temp += 28 */) {
        if (fileIndex < 2) {
            struct save_info_t *save_info = &this->save_info[fileIndex + 2];
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2], this->fileInfoAlpha[fileIndex]);
            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[temp], 28, 0);

            for (quadVtxIndex = 0, i = 0; i < 7; i++, quadVtxIndex += 4) {
                if ((i < 5) || ((save_info->isOwlSave || save_info->is_crash_save) && (i >= 5))) {
                    gDPLoadTextureBlock(POLY_OPA_DISP++, sFileInfoBoxTextures[i], G_IM_FMT_IA, G_IM_SIZ_16b,
                                        sFileInfoBoxPartWidths[i], 56, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                    gSP1Quadrangle(POLY_OPA_DISP++, quadVtxIndex, quadVtxIndex + 2, quadVtxIndex + 3, quadVtxIndex + 1, 0);
                }
            }
        }
    }

    gDPPipeSync(POLY_OPA_DISP++);

    if(this->chaos_config_box_alpha > 0)
    {
        Gfx *text_draw_list;
        u32 pos_x;
        u32 pos_y;
        u32 config_count;

        /* draw chaos config box */
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2], this->chaos_config_box_alpha);
        // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[temp + 28], 24, 0);
        gSPVertex(POLY_OPA_DISP++, &this->ui_contents->chaos_options_box_top, 24, 0);

        for (quadVtxIndex = 0, i = 0; i < 6; i++, quadVtxIndex += 4) 
        {   
            struct ChaosConfigMenuPartDef *menu_def = sChaosConfigMenuDef + i;
            gDPPipeSync(POLY_OPA_DISP++);
            gDPLoadTextureBlock(POLY_OPA_DISP++, sChaosConfigBoxTopTextures[i], G_IM_FMT_IA, G_IM_SIZ_16b,
                                menu_def->width, 56, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 
                                G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

            gSP1Quadrangle(POLY_OPA_DISP++, quadVtxIndex, quadVtxIndex + 2, quadVtxIndex + 3, quadVtxIndex + 1, 0);
        }

        gDPPipeSync(POLY_OPA_DISP++);
        // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[temp + 52], 24, 0);
        gSPVertex(POLY_OPA_DISP++, &this->ui_contents->chaos_options_box_bottom, 24, 0);

        for (quadVtxIndex = 0, i = 0; i < 6; i++, quadVtxIndex += 4) 
        {   
            struct ChaosConfigMenuPartDef *menu_def = sChaosConfigMenuDef + i + 6;
            gDPPipeSync(POLY_OPA_DISP++);
            gDPLoadTextureBlock(POLY_OPA_DISP++, sChaosConfigBoxTopTextures[i], G_IM_FMT_IA, G_IM_SIZ_16b,
                                menu_def->width, 56, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 
                                G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

            gSP1Quadrangle(POLY_OPA_DISP++, quadVtxIndex, quadVtxIndex + 2, quadVtxIndex + 3, quadVtxIndex + 1, 0);
        }
 

        // /* draw tabs */
        // gDPPipeSync(POLY_OPA_DISP++);
        // gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelBlankButtonTex, G_IM_FMT_IA, G_IM_SIZ_16b, 52, 16, 0,
        //             G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[FILE_SELECT_WINDOW_CONTENT_TOTAL_VERT_COUNT - 12], 12, 0);
        // // gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, 60, 76, 300, 192);
        // gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, 0, 0, 600, 600);
        // for(quadVtxIndex = 0, i = 0; i < 2; i++, quadVtxIndex += 4)
        // {
        //     u8 tab_alpha = this->chaos_config_box_alpha;
        //     if(i != this->chaos_config_tab_index)
        //     {
        //         tab_alpha /= 4;
        //     }
            
        //     gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2], tab_alpha);
        //     gSP1Quadrangle(POLY_OPA_DISP++, quadVtxIndex, quadVtxIndex + 2, quadVtxIndex + 3, quadVtxIndex + 1, 0);
        // }

        // if(this->chaos_config_option_index < 0)
        // {
        //     // draw highlight over currently selected button
        //     // quadVtxIndex = FILE_SELECT_CHAOS_SETTINGS_MAX_VISIBLE_SETTINGS * 4;
        //     // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[FILE_SELECT_WINDOW_CONTENT_TOTAL_VERT_COUNT - 4], 4, 0);
        //     gDPPipeSync(POLY_OPA_DISP++);
        //     gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, 
        //                     TEXEL0, 0, PRIMITIVE, 0);
        //     gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->highlightColor[0], this->highlightColor[1],
        //                     this->highlightColor[2], this->highlightColor[3]);
        //     gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelBlankButtonTex, G_IM_FMT_IA, G_IM_SIZ_16b, 52, 16, 0,
        //                         G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        //     gSP1Quadrangle(POLY_OPA_DISP++, 8, 10, 11, 9, 0);
        //     gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
        //               ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        //     gDPPipeSync(POLY_OPA_DISP++);
        // }


        config_count = FILE_SELECT_CHAOS_SETTINGS_MAX_VISIBLE_SETTINGS;

        if(config_count > CHAOS_CONFIG_LAST)
        {
            config_count = CHAOS_CONFIG_LAST;
        } 

        switch(this->chaos_config_tab_index)
        {
            case 0:
            {
                /* draw chaos config buttons */
                gDPPipeSync(POLY_OPA_DISP++);
                gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelBlankButtonTex, G_IM_FMT_IA, G_IM_SIZ_16b, 52, 16, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2], this->chaos_config_box_alpha);
                // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[FILE_SELECT_WINDOW_CONTENT_VERT_COUNT], 
                //     FILE_SELECT_CHAOS_SETTINGS_MAX_VISIBLE_SETTINGS * 4, 0);
                gSPVertex(POLY_OPA_DISP++, &this->ui_contents->chaos_options, FILE_SELECT_CHAOS_SETTINGS_MAX_VISIBLE_SETTINGS * 4, 0);
                gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, 60, 76, 300, 192);
                // gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, 0, 0, 600, 600);
                for(quadVtxIndex = 0, i = 0; i < config_count; i++, quadVtxIndex += 4)
                {
                    gSP1Quadrangle(POLY_OPA_DISP++, quadVtxIndex, quadVtxIndex + 2, quadVtxIndex + 3, quadVtxIndex + 1, 0);
                }
                gDPPipeSync(POLY_OPA_DISP++);
                gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, 0, 0, 300, 192);

                if(this->chaos_config_option_index >= 0)
                {
                    // draw highlight over currently selected button
                    // quadVtxIndex = FILE_SELECT_CHAOS_SETTINGS_MAX_VISIBLE_SETTINGS * 4;
                    // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[FILE_SELECT_WINDOW_CONTENT_VERT_COUNT + quadVtxIndex], 8, 0);
                    gSPVertex(POLY_OPA_DISP++, &this->ui_contents->chaos_highlighted_option, 4, 0);
                    gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, 
                                    TEXEL0, 0, PRIMITIVE, 0);
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->highlightColor[0], this->highlightColor[1],
                                    this->highlightColor[2], this->highlightColor[3]);
                    gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelBlankButtonTex, G_IM_FMT_IA, G_IM_SIZ_16b, 52, 16, 0,
                                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
                }


                /* draw chaos config text */
                text_draw_list = POLY_XLU_DISP;
                gSPDisplayList(POLY_OPA_DISP++, text_draw_list);
                gDPSetScissor(text_draw_list++, G_SC_NON_INTERLACE, 60, 76, 400, 192);
                // gDPSetScissor(text_draw_list++, G_SC_NON_INTERLACE, 0, 0, 600, 600);
                GfxPrint_Init(&gfx_print);
                GfxPrint_Open(&gfx_print, text_draw_list);
                gDPSetDepthSource(gfx_print.dList++, G_ZS_PRIM);
                gDPSetPrimDepth(gfx_print.dList++, 0x4000, 0);     
                // gDPSetEnvColor(gfx_print.dList++, 0, 0, 0, 10);
                // GfxPrint_SetBasePosPx(&gfx_print, 0, 0); 
                gfx_print.flags |= GFXP_FLAG_BLEND;
                // pos_x = 60 << 2;
                // pos_y = (96 + 16) << 2;

                pos_x = 65 << 2;
                // pos_y = ((240 - (this->windowContentVtx[FILE_SELECT_WINDOW_CONTENT_VERT_COUNT].v.ob[1] + 120)) << 2) + 8;
                pos_y = ((240 - (this->ui_contents->chaos_options[0][0].v.ob[1] + 120)) << 2) + 8;
                gfx_print.wrap_start = 66 << 2;
                gfx_print.wrap_size = 198 << 2;
                // for(quadVtxIndex = 0, i = 0; i < config_count; i++, pos_y += (FILE_SELECT_CHAOS_SETTING_OPTION_HEIGHT + 4) * 3)
                for(i = 0; i < config_count; i++, pos_y += ((FILE_SELECT_CHAOS_SETTING_OPTION_HEIGHT + 1 ) << 2) + 2)
                {
                    u32 config_index = i + this->chaos_config_scroll;
                    struct ChaosConfig *config = gChaosConfigs + config_index;
                    // Vtx *vertex = &this->windowContentVtx[FILE_SELECT_WINDOW_CONTENT_VERT_COUNT + quadVtxIndex];

                    gfx_print.posX = pos_x;
                    gfx_print.posY = pos_y & 0xfffffffc;

                    if(Chaos_GetConfigFlag(config_index))
                    {
                        GfxPrint_SetColor(&gfx_print, 255, 255, 255, 220 - this->chaos_config_description_alpha);
                    }
                    else
                    {
                        GfxPrint_SetColor(&gfx_print, 120, 120, 120, 220 - this->chaos_config_description_alpha);
                    }

                    // gfx_print.posY += (FILE_SELECT_CHAOS_SETTING_OPTION_HEIGHT << 1) >> ((text_size.lines - 1));
                    GfxPrint_Printf(&gfx_print, config->label);
                }

                text_draw_list = GfxPrint_Close(&gfx_print);
                gDPPipeSync(text_draw_list++);
                gSPEndDisplayList(text_draw_list++);
                POLY_XLU_DISP = text_draw_list;
            }
            break;
        }

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, 0, 0, 0xffff, 0xffff);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetOtherMode(POLY_OPA_DISP++, G_AD_PATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE |
                            G_TD_CLAMP | G_TP_PERSP | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                            G_AC_NONE | G_ZS_PIXEL | G_RM_XLU_SURF | G_RM_XLU_SURF2);
        gDPPipeSync(POLY_OPA_DISP++);


        // gDPLoadTextureBlock_4b(POLY_OPA_DISP++, this->textbox_segment, G_IM_FMT_I, 128, 64, 0, G_TX_MIRROR | G_TX_WRAP,
        //                        G_TX_NOMIRROR | G_TX_WRAP, 7, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        // gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, 200);
        // gDPSetDepthSource(POLY_OPA_DISP++, G_ZS_PRIM);
        // gDPSetPrimDepth(POLY_OPA_DISP++, 0, 10);
        // gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        // gDPSetRenderMode(POLY_OPA_DISP++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
        // // gDPLoadTextureBlock(POLY_OPA_DISP++, gMessageDefaultBackgroundTex, G_IM_FMT_IA, G_IM_SIZ_16b, 128, 64, 0,
        // //                     G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        // gSP1Quadrangle(POLY_OPA_DISP++, 4, 6, 7, 5, 0);
        // gDPPipeSync(POLY_OPA_DISP++);

        // text_draw_list = POLY_XLU_DISP;
        // gSPDisplayList(POLY_OPA_DISP++, text_draw_list);
        // // gDPSetScissor(text_draw_list++, G_SC_NON_INTERLACE, 60, 76, 400, 192);
        // GfxPrint_Init(&gfx_print);
        // GfxPrint_Open(&gfx_print, text_draw_list);
        // GfxPrint_SetColor(&gfx_print, 255, 255, 255, 255);
        // // gDPSetPrimDepth(gfx_print.dList++, 0, 0);
        // // gfx_print.posX = 64 << 2;
        // // gfx_print.posY = 50 << 2;
        // GfxPrint_SetPos(&gfx_print, 10, 5);
        // GfxPrint_Printf(&gfx_print, "SHIT");

        // text_draw_list = GfxPrint_Close(&gfx_print);
        // gSPEndDisplayList(text_draw_list++);
        // POLY_XLU_DISP = text_draw_list;
    }

    temp += 28 * 3;

    for (i = 0; i < 2; i++, temp += 16) {
        if (i < 2) {
            // draw file button
            // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[temp], 16, 0);
            gSPVertex(POLY_OPA_DISP++, &this->ui_contents->files[i].file_button, 16, 0);

            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sWindowContentColors[0], sWindowContentColors[1],
                            sWindowContentColors[2], this->fileButtonAlpha[i]);
            gDPLoadTextureBlock(POLY_OPA_DISP++, sFileButtonTextures[i], G_IM_FMT_IA, G_IM_SIZ_16b, 64, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

            // draw file name box
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sWindowContentColors[0], sWindowContentColors[1],
                            sWindowContentColors[2], this->nameBoxAlpha[i]);
            gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelFileNameBoxTex, G_IM_FMT_IA, G_IM_SIZ_16b, 108, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 4, 6, 7, 5, 0);

            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sWindowContentColors[0], sWindowContentColors[1],
                            sWindowContentColors[2], this->connectorAlpha[i]);
            gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelConnectorTex, G_IM_FMT_IA, G_IM_SIZ_8b, 24, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 8, 10, 11, 9, 0);

            if (this->save_info[i + 2].isOwlSave || this->save_info[i + 2].is_crash_save) {
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sWindowContentColors[0], sWindowContentColors[1],
                                sWindowContentColors[2], this->nameBoxAlpha[i]);
                gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelBlankButtonTex, G_IM_FMT_IA, G_IM_SIZ_16b, 52, 16, 0,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                    G_TX_NOLOD, G_TX_NOLOD);
                gSP1Quadrangle(POLY_OPA_DISP++, 12, 14, 15, 13, 0);
            }
        }
    }

    // draw file info
    for (fileIndex = 0; fileIndex < 2; fileIndex++) {
        FileSelect_DrawFileInfo(&this->state, fileIndex);
    }

    {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                        ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
        // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[0x3AC], 20, 0);
        gSPVertex(POLY_OPA_DISP++, this->ui_contents->buttons, 20, 0);
        // temp = this->confirmButtonTexIndices[FS_BTN_CONFIRM_OPTIONS];

        // draw primary action buttons (copy/erase)
        for (quadVtxIndex = 0, i = 0; i < 2; i++, quadVtxIndex += 4) {
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                            this->actionButtonAlpha[i]);
            gDPLoadTextureBlock(POLY_OPA_DISP++, sActionButtonTextures[i], G_IM_FMT_IA, G_IM_SIZ_16b, 64, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                                G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, quadVtxIndex, quadVtxIndex + 2, quadVtxIndex + 3, quadVtxIndex + 1, 0);
        }
    }
    
    {
        // draw options button
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                        this->optionButtonAlpha);
        gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelOptionsButtonENGTex, G_IM_FMT_IA, G_IM_SIZ_16b, 64, 16, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        // gSP1Quadrangle(POLY_OPA_DISP++, 8, 10, 11, 9, 0);
        gSP1Quadrangle(POLY_OPA_DISP++, quadVtxIndex, quadVtxIndex + 2, quadVtxIndex + 3, quadVtxIndex + 1, 0);
    

        // draw confirm buttons (yes/quit/options)
        for (quadVtxIndex = 0, i = FS_BTN_CONFIRM_YES; i <= FS_BTN_CONFIRM_OPTIONS; i++, quadVtxIndex += 4) {
            temp = this->confirmButtonTexIndices[i];
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                            this->confirmButtonAlpha[i]);
            gDPLoadTextureBlock(POLY_OPA_DISP++, sActionButtonTextures[temp], G_IM_FMT_IA, G_IM_SIZ_16b, 64, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                                G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, quadVtxIndex, quadVtxIndex + 2, quadVtxIndex + 3, quadVtxIndex + 1, 0);
        }
    }

    quadVtxIndex += 4;

    // draw highlight over currently selected button
    if (((this->menuMode == FS_MENU_MODE_CONFIG) &&
         ((this->configMode == CM_MAIN_MENU) || (this->configMode == CM_SELECT_COPY_SOURCE) ||
          (this->configMode == CM_SELECT_COPY_DEST) || (this->configMode == CM_COPY_CONFIRM) ||
          (this->configMode == CM_ERASE_SELECT) || (this->configMode == CM_ERASE_CONFIRM))) ||
        ((this->menuMode == FS_MENU_MODE_SELECT) && (this->selectMode == SM_CONFIRM_FILE))) {
        
        gSPVertex(POLY_OPA_DISP++, this->ui_contents->highlighted_option, 4, 0);
        gDPPipeSync(POLY_OPA_DISP++);

        gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                          PRIMITIVE, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->highlightColor[0], this->highlightColor[1],
                        this->highlightColor[2], this->highlightColor[3]);
        gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelBigButtonHighlightTex, G_IM_FMT_I, G_IM_SIZ_8b, 72, 24, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        // gSP1Quadrangle(POLY_OPA_DISP++, 12, 14, 15, 13, 0);
        // gSP1Quadrangle(POLY_OPA_DISP++, quadVtxIndex, quadVtxIndex + 2, quadVtxIndex + 3, quadVtxIndex + 1, 0);
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    }
    quadVtxIndex += 4;

    // draw warning labels
    if (this->warningLabel > FS_WARNING_NONE) {
        gDPPipeSync(POLY_OPA_DISP++);

        gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->emptyFileTextAlpha);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
        gDPLoadTextureBlock(POLY_OPA_DISP++, sWarningLabels[this->warningLabel], G_IM_FMT_IA, G_IM_SIZ_8b, 128, 16, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        // gSP1Quadrangle(POLY_OPA_DISP++, 16, 18, 19, 17, 0);
        gSP1Quadrangle(POLY_OPA_DISP++, quadVtxIndex, quadVtxIndex + 2, quadVtxIndex + 3, quadVtxIndex + 1, 0);
    }

    gDPPipeSync(POLY_OPA_DISP++);

    if(this->chaos_config_description_alpha > 0)
    {
        Gfx *text_draw_list;
        struct ChaosConfig *config = gChaosConfigs + this->chaos_config_option_index + this->chaos_config_scroll;
        GfxTextSize text_size;
        s16 pos_x;
        s16 pos_y;
        // quadVtxIndex = FILE_SELECT_CHAOS_SETTINGS_MAX_VISIBLE_SETTINGS * 4;
        // gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[FILE_SELECT_WINDOW_CONTENT_VERT_COUNT + quadVtxIndex], 8, 0);
        gSPVertex(POLY_OPA_DISP++, this->ui_contents->chaos_text_box, 4, 0);

        gDPLoadTextureBlock_4b(POLY_OPA_DISP++, this->textbox_segment, G_IM_FMT_I, 128, 64, 0, G_TX_MIRROR | G_TX_WRAP,
                               G_TX_NOMIRROR | G_TX_WRAP, 7, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, this->chaos_config_description_alpha);
        gDPSetDepthSource(POLY_OPA_DISP++, G_ZS_PRIM);
        gDPSetPrimDepth(POLY_OPA_DISP++, 0, 0);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetRenderMode(POLY_OPA_DISP++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
        gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, 0, 0, 0xffff, 0xffff);

        // gSP1Quadrangle(POLY_OPA_DISP++, 4, 6, 7, 5, 0);
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        gDPPipeSync(POLY_OPA_DISP++);

        if(this->chaos_config_description_alpha == 200 && this->chaos_config_text_flash_timer == 0)
        {
            u32 icon_x = 156;
            u32 icon_y = 164;

            primR = ABS_ALT(sIconPrimR - sIconPrimColors[sIconFlashColorIndex].r) / sIconFlashTimer;
            primG = ABS_ALT(sIconPrimG - sIconPrimColors[sIconFlashColorIndex].g) / sIconFlashTimer;
            primB = ABS_ALT(sIconPrimB - sIconPrimColors[sIconFlashColorIndex].b) / sIconFlashTimer;

            if (sIconPrimR >= sIconPrimColors[sIconFlashColorIndex].r) {
                sIconPrimR -= primR;
            } else {
                sIconPrimR += primR;
            }

            if (sIconPrimG >= sIconPrimColors[sIconFlashColorIndex].g) {
                sIconPrimG -= primG;
            } else {
                sIconPrimG += primG;
            }

            if (sIconPrimB >= sIconPrimColors[sIconFlashColorIndex].b) {
                sIconPrimB -= primB;
            } else {
                sIconPrimB += primB;
            }

            envR = ABS_ALT(sIconEnvR - sIconEnvColors[sIconFlashColorIndex].r) / sIconFlashTimer;
            envG = ABS_ALT(sIconEnvG - sIconEnvColors[sIconFlashColorIndex].g) / sIconFlashTimer;
            envB = ABS_ALT(sIconEnvB - sIconEnvColors[sIconFlashColorIndex].b) / sIconFlashTimer;

            if (sIconEnvR >= sIconEnvColors[sIconFlashColorIndex].r) {
                sIconEnvR -= envR;
            } else {
                sIconEnvR += envR;
            }

            if (sIconEnvG >= sIconEnvColors[sIconFlashColorIndex].g) {
                sIconEnvG -= envG;
            } else {
                sIconEnvG += envG;
            }

            if (sIconEnvB >= sIconEnvColors[sIconFlashColorIndex].b) {
                sIconEnvB -= envB;
            } else {
                sIconEnvB += envB;
            }

            sIconFlashTimer--;
            if (sIconFlashTimer == 0) {
                sIconPrimR = sIconPrimColors[sIconFlashColorIndex].r;
                sIconPrimG = sIconPrimColors[sIconFlashColorIndex].g;
                sIconPrimB = sIconPrimColors[sIconFlashColorIndex].b;
                sIconEnvR = sIconEnvColors[sIconFlashColorIndex].r;
                sIconEnvG = sIconEnvColors[sIconFlashColorIndex].g;
                sIconEnvB = sIconEnvColors[sIconFlashColorIndex].b;
                sIconFlashTimer = 36;
                sIconFlashColorIndex ^= 1;
            }

            // if(FileSelect_IsAtEndOfChaosConfigDetails(thisx))
            // {
            //     gDPLoadTextureBlock_4b(POLY_OPA_DISP++, this->textbox_segment + FS_MSG_END_TEX_OFFSET, G_IM_FMT_I, 16, 16, 0, G_TX_MIRROR | G_TX_WRAP,
            //                    G_TX_NOMIRROR | G_TX_WRAP, 7, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            // }
            // else
            // {
            //     gDPLoadTextureBlock_4b(POLY_OPA_DISP++, this->textbox_segment + FS_MSG_CONTINUE_TEX_OFFSET, G_IM_FMT_I, 16, 16, 0, G_TX_MIRROR | G_TX_WRAP,
            //                    G_TX_NOMIRROR | G_TX_WRAP, 7, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            // }
 
            gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                          ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sIconPrimR, sIconPrimG, sIconPrimB, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, sIconEnvR, sIconEnvG, sIconEnvB, 255);

            gDPLoadTextureBlock_4b(POLY_OPA_DISP++, this->font.iconBuf, G_IM_FMT_I, 16, 16, 0, G_TX_NOMIRROR | G_TX_CLAMP,
                                   G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

            // gSPTextureRectangle(POLY_OPA_DISP++, icon_x << 2, icon_y << 2, (s16)(icon_x + 0.75f * 16.0f) << 2, (s16)(icon_y + 0.75f * 16.0f) << 2, 0, 0, 0, 1024.0f / 0.75f, 1024.0f / 0.75f);
            gSPTextureRectangle(POLY_OPA_DISP++, (icon_x << 2), (icon_y << 2), ((s16)(icon_x + 16) << 2), ((s16)(icon_y + 16) << 2), G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            gDPPipeSync(POLY_OPA_DISP++);

            text_draw_list = POLY_XLU_DISP;
            gSPDisplayList(POLY_OPA_DISP++, text_draw_list);
            gDPSetScissor(text_draw_list++, G_SC_NON_INTERLACE, 0, 0, 400, 400);
            GfxPrint_Init(&gfx_print);
            GfxPrint_Open(&gfx_print, text_draw_list);
            gDPSetDepthSource(gfx_print.dList++, G_ZS_PRIM);
            gDPSetPrimDepth(gfx_print.dList++, 0x4000, 0);     
            // gDPSetEnvColor(gfx_print.dList++, 0, 0, 0, 10);
            // GfxPrint_SetBasePosPx(&gfx_print, 0, 0); 
            gfx_print.flags |= GFXP_FLAG_BLEND;
            gfx_print.flags &= ~GFXP_FLAG_SHADOW;
            pos_x = 30 << 2;
            pos_y = 92 << 2; 
            gfx_print.wrap_start = pos_x;
            gfx_print.wrap_size = 272 << 2;
            
            // GfxPrint_CalcWrappedTextDimensions(&gfx_print, &text_size, config->description);
            gfx_print.posX = pos_x;
            gfx_print.posY = pos_y;
            gfx_print.y_increment = 12 << 2;

            GfxPrint_SetColor(&gfx_print, 255, 255, 255, this->chaos_config_description_alpha);
            GfxPrint_PrintfWrap(&gfx_print, this->chaos_config_text_buffer);

            text_draw_list = GfxPrint_Close(&gfx_print);
            gDPPipeSync(text_draw_list++);
            gSPEndDisplayList(text_draw_list++);
            POLY_XLU_DISP = text_draw_list;
        }
    }

    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIDECALA, G_CC_MODULATEIDECALA);

    CLOSE_DISPS(this->state.gfxCtx);
}

void FileSelect_ConfigModeDraw(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    OPEN_DISPS(this->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);
    Gfx_SetupDL42_Opa(this->state.gfxCtx);
    FileSelect_SetView(this, 0.0f, 0.0f, 64.0f);
    FileSelect_SetWindowVtx(&this->state);
    FileSelect_SetWindowContentVtx(&this->state);

    if ((this->configMode != CM_NAME_ENTRY) && (this->configMode != CM_START_NAME_ENTRY)) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                        this->windowAlpha);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

        Matrix_Translate(0.0f, /* Math_SinF(fmodf(wave_angle, 2.0f * 3.14159265f)) * 30.0f */ 0.0f, -93.6f, MTXMODE_NEW);
        Matrix_Scale(0.78f, 0.78f, 0.78f, MTXMODE_APPLY);
        // wave_angle += 0.05f;

        if (this->windowRot != 0) {
            Matrix_RotateXFApply(this->windowRot / 100.0f);
        }

        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, this->state.gfxCtx);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[0], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow1DL);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[32], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow2DL);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[64], 16, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow3DL);

        gDPPipeSync(POLY_OPA_DISP++);

        FileSelect_DrawWindowContents(&this->state);
    }

    // draw name entry menu
    if ((this->configMode >= CM_ROTATE_TO_NAME_ENTRY) && (this->configMode <= CM_NAME_ENTRY_TO_MAIN)) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                        this->windowAlpha);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

        Matrix_Translate(Rand_Centered() * 10.0f, Rand_Centered() * 10.0f, -93.6f, MTXMODE_NEW);
        Matrix_Scale(0.78f, 0.78f, 0.78f, MTXMODE_APPLY);
        Matrix_RotateXFApply((this->windowRot - 314.0f) / 100.0f + Rand_Centered() * 0.1f);
        Matrix_RotateYF(Rand_Centered() * 0.1f, MTXMODE_APPLY);
        Matrix_RotateZF(Rand_Centered() * 0.1f, MTXMODE_APPLY);

        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, this->state.gfxCtx);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[0], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow1DL);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[32], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow2DL);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[64], 16, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow3DL);

        gDPPipeSync(POLY_OPA_DISP++);

        FileSelect_DrawNameEntry(&this->state);
    }

    // draw options menu
    if ((this->configMode >= CM_MAIN_TO_OPTIONS) && (this->configMode <= CM_OPTIONS_TO_MAIN)) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                        this->windowAlpha);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

        Matrix_Translate(0.0f, 0.0f, -93.6f, MTXMODE_NEW);
        Matrix_Scale(0.78f, 0.78f, 0.78f, MTXMODE_APPLY);
        Matrix_RotateXFApply((this->windowRot - 314.0f) / 100.0f);

        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, this->state.gfxCtx);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[0], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow1DL);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[32], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow2DL);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[64], 16, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow3DL);

        gDPPipeSync(POLY_OPA_DISP++);

        FileSelect_DrawOptions(&this->state);
    }

    gDPPipeSync(POLY_OPA_DISP++);

    FileSelect_SetView(this, 0.0f, 0.0f, 64.0f);

    CLOSE_DISPS(this->state.gfxCtx);
}

/**
 * Fade out the main menu elements to transition to select mode.
 * Update function for `SM_FADE_MAIN_TO_SELECT`
 */
void FileSelect_FadeMainToSelect(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;
    s16 i;

    for (i = 0; i < 3; i++) {
        if (i != this->buttonIndex) {
            // this->fileButtonAlpha[i] -= 50;
            this->fileButtonAlpha[i] -= 100;

            if(this->fileButtonAlpha[i] < 0)
            {
                this->fileButtonAlpha[i] = 0;
            }

            this->actionButtonAlpha[FS_BTN_ACTION_COPY] = this->fileButtonAlpha[i];
            this->actionButtonAlpha[FS_BTN_ACTION_ERASE] = this->fileButtonAlpha[i];
            this->optionButtonAlpha = this->fileButtonAlpha[i];

            if (!gSaveContext.flashSaveAvailable) {
                if (NO_FLASH_SLOT_OCCUPIED(sramCtx, i)) {
                    this->nameAlpha[i] = this->nameBoxAlpha[i] = this->fileButtonAlpha[i];
                    // this->connectorAlpha[i] -= 255 / 4;
                    this->connectorAlpha[i] -= 100;
                }
            } else {
                if (SLOT_OCCUPIED(this, i)) {
                    this->nameAlpha[i] = this->fileButtonAlpha[i];
                    this->nameBoxAlpha[i] = this->fileButtonAlpha[i];
                    // this->connectorAlpha[i] -= 255 / 4;
                    this->connectorAlpha[i] -= 100;
                }
            }

            if(this->connectorAlpha[i] < 0)
            {
                this->connectorAlpha[i] = 0;
            }
        }
    }

    this->titleAlpha[FS_TITLE_CUR] -= 255 / 4;
    this->titleAlpha[FS_TITLE_NEXT] += 255 / 4;
    this->actionTimer--;

    if (this->actionTimer == 0) {
        this->actionTimer = 4;
        // this->selectMode++; // SM_MOVE_FILE_TO_TOP
        this->selectMode = SM_MOVE_FILE_TO_TOP;
        this->confirmButtonIndex = FS_BTN_CONFIRM_YES;
        this->titleAlpha[FS_TITLE_CUR] = 0;
        Sram_LoadChaosConfig(&this->sramCtx, this->buttonIndex);
    }
}

// Amount to move by to reach the top of the screen
s16 sFileYOffsets[] = { 0, 16, 32 };

/**
 * Moves the selected file to the top of the window.
 * Update function for `SM_MOVE_FILE_TO_TOP`
 */
void FileSelect_MoveSelectedFileToTop(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    s32 yStep;

    yStep = ABS_ALT(this->buttonYOffsets[this->buttonIndex] - sFileYOffsets[this->buttonIndex]) / this->actionTimer;
    this->buttonYOffsets[this->buttonIndex] += yStep;
    this->actionTimer--;

    if ((this->actionTimer == 0) || (this->buttonYOffsets[this->buttonIndex] == sFileYOffsets[this->buttonIndex])) {
        this->buttonYOffsets[FS_BTN_SELECT_YES] = -4;
        this->buttonYOffsets[FS_BTN_SELECT_QUIT] = -4;
        this->buttonYOffsets[FS_BTN_SELECT_OPTIONS] = -4;
        this->actionTimer = 4;
        // this->selectMode++; // SM_FADE_IN_FILE_INFO
        this->selectMode = SM_FADE_IN_FILE_INFO;
    }
}

/**
 * Fade in the file info for the selected file.
 * Update function for `SM_FADE_IN_FILE_INFO`
 */
void FileSelect_FadeInFileInfo(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->fileInfoAlpha[this->buttonIndex] += 50;
    this->nameBoxAlpha[this->buttonIndex] -= 100;
    this->chaos_config_box_alpha -= 100;

    if (this->nameBoxAlpha[this->buttonIndex] <= 0) {
        this->nameBoxAlpha[this->buttonIndex] = 0;
    }

    if(this->chaos_config_box_alpha < 0)
    {
        this->chaos_config_box_alpha = 0;
    }

    this->actionTimer--;

    if (this->actionTimer == 0) {
        this->fileInfoAlpha[this->buttonIndex] = 200;
        this->actionTimer = 4;
        // this->selectMode++; // SM_CONFIRM_FILE
        // Sram_LoadChaosConfig(&this->sramCtx, this->buttonIndex);
        this->selectMode = SM_CONFIRM_FILE;
    }

    this->confirmButtonAlpha[FS_BTN_CONFIRM_YES] = this->fileInfoAlpha[this->buttonIndex];
    this->confirmButtonAlpha[FS_BTN_CONFIRM_QUIT] = this->fileInfoAlpha[this->buttonIndex];
    this->confirmButtonAlpha[FS_BTN_CONFIRM_OPTIONS] = this->fileInfoAlpha[this->buttonIndex];
}

/**
 * Update the cursor and handle the option that the player picks for confirming the selected file.
 * Update function for `SM_CONFIRM_FILE`
 */
void FileSelect_ConfirmFile(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    Input* input = CONTROLLER1(&this->state);
    u32 button_index = this->confirmButtonIndex;

    if (CHECK_BTN_ALL(input->press.button, BTN_START) || (CHECK_BTN_ALL(input->press.button, BTN_A))) {
        if (this->confirmButtonIndex == FS_BTN_CONFIRM_YES) {
            Rumble_Request(300.0f, 180, 20, 100);
            Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
            this->selectMode = SM_FADE_OUT;
            Audio_MuteAllSeqExceptSystemAndOcarina(15);
        }
        else if(this->confirmButtonIndex == FS_BTN_CONFIRM_OPTIONS)
        {
            Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
            this->menuMode = FS_MENU_MODE_CHAOS_CONFIG;
            this->chaos_config_mode = CCM_FADE_SELECT_TO_CONFIG;
            this->nextTitleLabel = FS_TITLE_CHAOS_CONFIG;
            this->actionTimer = 4;
        }
        else 
        { // FS_BTN_CONFIRM_QUIT
            Audio_PlaySfx(NA_SE_SY_FSEL_CLOSE);
            // this->selectMode++; // SM_FADE_OUT_FILE_INFO
            this->selectMode = SM_FADE_OUT_FILE_INFO;
        }
    } else if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
        Audio_PlaySfx(NA_SE_SY_FSEL_CLOSE);
        // this->selectMode++; // SM_FADE_OUT_FILE_INFO
        this->selectMode = SM_FADE_OUT_FILE_INFO;
    } 
    else if (this->stickAdjY < -30) 
    {
        Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
        button_index = (button_index + 1) % FS_BTN_CONFIRM_LAST;
    }
    else if (this->stickAdjY > 30)
    {
        Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
        button_index = (button_index - 1) % (-FS_BTN_CONFIRM_LAST);
    }

    this->confirmButtonIndex = button_index;
}

/**
 * Fade out the file info for the selected file before returning to the main menu.
 * Update function for `SM_FADE_OUT_FILE_INFO`
 */
void FileSelect_FadeOutFileInfo(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->fileInfoAlpha[this->buttonIndex] -= 200 / 4;
    this->nameBoxAlpha[this->buttonIndex] += 200 / 4;
    this->actionTimer--;

    if (this->actionTimer == 0) {
        this->buttonYOffsets[FS_BTN_SELECT_YES] = 0;
        this->buttonYOffsets[FS_BTN_SELECT_QUIT] = 0;
        this->buttonYOffsets[FS_BTN_SELECT_OPTIONS] = 0;
        this->nameBoxAlpha[this->buttonIndex] = 200;
        this->fileInfoAlpha[this->buttonIndex] = 0;
        this->nextTitleLabel = FS_TITLE_SELECT_FILE;
        this->actionTimer = 4;
        this->selectMode++;
    }

    this->confirmButtonAlpha[FS_BTN_CONFIRM_YES] = this->fileInfoAlpha[this->buttonIndex];
    this->confirmButtonAlpha[FS_BTN_CONFIRM_QUIT] = this->fileInfoAlpha[this->buttonIndex];
    this->confirmButtonAlpha[FS_BTN_CONFIRM_OPTIONS] = this->fileInfoAlpha[this->buttonIndex];
}

/**
 * Move the selected file back to the slot position then go to config mode for the main menu.
 * Update function for `SM_MOVE_FILE_TO_SLOT`
 */
void FileSelect_MoveSelectedFileToSlot(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;
    s32 yStep;
    s16 i;

    yStep = ABS_ALT(this->buttonYOffsets[this->buttonIndex]) / this->actionTimer;
    this->buttonYOffsets[this->buttonIndex] -= yStep;

    if (this->buttonYOffsets[this->buttonIndex] <= 0) {
        this->buttonYOffsets[this->buttonIndex] = 0;
    }

    for (i = 0; i < 3; i++) {
        if (i != this->buttonIndex) {
            this->fileButtonAlpha[i] += 200 / 4;

            if (this->fileButtonAlpha[i] >= 200) {
                this->fileButtonAlpha[i] = 200;
            }

            this->actionButtonAlpha[FS_BTN_ACTION_COPY] = this->fileButtonAlpha[i];
            this->actionButtonAlpha[FS_BTN_ACTION_ERASE] = this->fileButtonAlpha[i];
            this->optionButtonAlpha = this->fileButtonAlpha[i];

            if (!gSaveContext.flashSaveAvailable) {
                if (NO_FLASH_SLOT_OCCUPIED(sramCtx, i)) {
                    this->nameBoxAlpha[i] = this->nameAlpha[i] = this->fileButtonAlpha[i];
                    this->connectorAlpha[i] += 255 / 4;
                }
            } else {
                if (SLOT_OCCUPIED(this, i)) {
                    this->nameBoxAlpha[i] = this->nameAlpha[i] = this->fileButtonAlpha[i];
                    this->connectorAlpha[i] += 255 / 4;
                }
            }
        }
    }

    this->titleAlpha[FS_TITLE_CUR] -= 255 / 4;
    this->titleAlpha[FS_TITLE_NEXT] += 255 / 4;
    this->actionTimer--;

    if (this->actionTimer == 0) {
        this->titleAlpha[FS_TITLE_CUR] = 255;
        this->titleAlpha[FS_TITLE_NEXT] = 0;
        this->titleLabel = this->nextTitleLabel;
        this->actionTimer = 4;
        this->menuMode = FS_MENU_MODE_CONFIG;
        this->configMode = CM_MAIN_MENU;
        this->nextConfigMode = CM_MAIN_MENU;
        this->selectMode = SM_FADE_MAIN_TO_SELECT;
    }
}

/**
 * Fill the screen with black to fade out.
 * Update function for `SM_FADE_OUT`
 */
void FileSelect_FadeOut(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->screenFillAlpha += 40;

    if (this->screenFillAlpha >= 255) {
        this->screenFillAlpha = 255;
        // this->selectMode++; // SM_LOAD_GAME
        this->selectMode = SM_LOAD_GAME;
    }
}

/**
 * Load the save for the appropriate file and start the game.
 * Update function for `SM_LOAD_GAME`
 */
void FileSelect_LoadGame(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    u16 i;

    gSaveContext.fileNum = this->buttonIndex;
    Sram_OpenSave(this, &this->sramCtx);
    gSaveContext.gameMode = GAMEMODE_NORMAL;
    STOP_GAMESTATE(&this->state);
    SET_NEXT_GAMESTATE(&this->state, Play_Init, sizeof(PlayState));

    // if(gSaveContext.save.is_crash_save)
    // {

    // }
    // else
    // {

        gSaveContext.respawnFlag = 0;
        gSaveContext.respawn[RESPAWN_MODE_DOWN].entrance = ENTR_LOAD_OPENING;
    // }

    gSaveContext.seqId = (u8)NA_BGM_DISABLED;
    gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
    gSaveContext.showTitleCard = true;
    gSaveContext.dogParams = 0;

    for (i = 0; i < TIMER_ID_MAX; i++) {
        gSaveContext.timerStates[i] = TIMER_STATE_OFF;
    }

    gSaveContext.prevHudVisibility = HUD_VISIBILITY_ALL;
    gSaveContext.nayrusLoveTimer = 0;
    gSaveContext.healthAccumulator = 0;
    gSaveContext.magicFlag = 0;
    gSaveContext.forcedSeqId = 0;
    gSaveContext.skyboxTime = CLOCK_TIME(0, 0);
    gSaveContext.nextTransitionType = TRANS_NEXT_TYPE_DEFAULT;
    gSaveContext.cutsceneTrigger = 0;
    gSaveContext.chamberCutsceneNum = 0;
    gSaveContext.nextDayTime = NEXT_TIME_NONE;
    gSaveContext.retainWeatherMode = false;

    gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_ENABLED;

    gSaveContext.hudVisibilityForceButtonAlphasByStatus = false;
    gSaveContext.nextHudVisibility = HUD_VISIBILITY_IDLE;
    gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
    gSaveContext.hudVisibilityTimer = 0;

    gSaveContext.save.saveInfo.playerData.tatlTimer = 0;

    if(Chaos_GetConfigFlag(CHAOS_CONFIG_DETERMINISTIC_EFFECT_RNG))
    {
        /* deterministic rng enabled, reinitialize rng and timers so they always start similar */
        Chaos_InitRng();
    }
}

void FileSelect_FadeChaosConfigToSelect(GameState *thisx)
{
    FileSelectState* this = (FileSelectState*)thisx;
}

void (*sSelectModeUpdateFuncs[])(GameState*) = {
    FileSelect_FadeMainToSelect,       // SM_FADE_MAIN_TO_SELECT
    FileSelect_MoveSelectedFileToTop,  // SM_MOVE_FILE_TO_TOP
    FileSelect_FadeInFileInfo,         // SM_FADE_IN_FILE_INFO
    FileSelect_ConfirmFile,            // SM_CONFIRM_FILE
    FileSelect_FadeOutFileInfo,        // SM_FADE_OUT_FILE_INFO
    FileSelect_MoveSelectedFileToSlot, // SM_MOVE_FILE_TO_SLOT
    FileSelect_FadeOut,                // SM_FADE_OUT
    FileSelect_LoadGame,               // SM_LOAD_GAME
    FileSelect_FadeChaosConfigToSelect
};

void FileSelect_SelectModeUpdate(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    sSelectModeUpdateFuncs[this->selectMode](&this->state);
}

void FileSelect_SelectModeDraw(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    OPEN_DISPS(this->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);

    Gfx_SetupDL42_Opa(this->state.gfxCtx);
    FileSelect_SetView(this, 0.0f, 0.0f, 64.0f);
    FileSelect_SetWindowVtx(&this->state);
    FileSelect_SetWindowContentVtx(&this->state);

    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                    this->windowAlpha);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

    Matrix_Translate(0.0f, 0.0f, -93.6f, MTXMODE_NEW);
    Matrix_Scale(0.78f, 0.78f, 0.78f, MTXMODE_APPLY);
    Matrix_RotateXFApply(this->windowRot / 100.0f);
    // Matrix_RotateYF((Rand_ZeroOne() * 2.0f - 1.0f) * 0.05f, MTXMODE_APPLY);
    // Matrix_RotateZF((Rand_ZeroOne() * 2.0f - 1.0f) * 0.05f, MTXMODE_APPLY);
    // gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(this->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, this->state.gfxCtx);

    gSPVertex(POLY_OPA_DISP++, &this->windowVtx[0], 32, 0);
    gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow1DL);

    gSPVertex(POLY_OPA_DISP++, &this->windowVtx[32], 32, 0);
    gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow2DL);

    gSPVertex(POLY_OPA_DISP++, &this->windowVtx[64], 16, 0);
    gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow3DL);

    FileSelect_DrawWindowContents(&this->state);
    gDPPipeSync(POLY_OPA_DISP++);
    FileSelect_SetView(this, 0.0f, 0.0f, 64.0f);

    CLOSE_DISPS(this->state.gfxCtx);
}

void FileSelect_FadeSelectToChaosConfig(GameState *thisx)
{
    FileSelectState* this = (FileSelectState*)thisx;

    this->chaos_config_box_alpha += 50;
    this->fileInfoAlpha[this->buttonIndex] -= 100;
    if(this->fileInfoAlpha[this->buttonIndex] <= 0)
    {
        this->fileInfoAlpha[this->buttonIndex] = 0;
    }

    this->actionTimer--;

    if(this->actionTimer == 0)
    {
        this->actionTimer = 4;
        this->chaos_config_mode = CCM_CHAOS_OPTIONS;
        this->chaos_config_box_alpha = 200;
        this->chaos_config_option_index = 0;
        this->chaos_config_scroll = 0;

    }
    this->chaos_config_tab_index = 0;
    this->confirmButtonAlpha[FS_BTN_CONFIRM_YES] = this->fileInfoAlpha[this->buttonIndex];
    this->confirmButtonAlpha[FS_BTN_CONFIRM_QUIT] = this->fileInfoAlpha[this->buttonIndex];
    this->confirmButtonAlpha[FS_BTN_CONFIRM_OPTIONS] = this->fileInfoAlpha[this->buttonIndex];
}
 
void FileSelect_ChaosOptions(GameState *thisx)
{
    FileSelectState *this = (FileSelectState *)thisx;
    Input *input = CONTROLLER1(&this->state);
    // u32 option_index = this->chaos_config_option_index;
    if(CHECK_BTN_ALL(input->press.button, BTN_B))
    {
        Audio_PlaySfx(NA_SE_SY_FSEL_CLOSE);
        Sram_SaveChaosConfig(&this->sramCtx, this->buttonIndex);
        // this->chaos_config_mode = CCM_CHAOS_WAIT_FOR_FLASH_SAVE;
        this->menuMode = FS_MENU_MODE_SELECT;
        this->selectMode = SM_FADE_IN_FILE_INFO;
        this->nextTitleLabel = FS_TITLE_OPEN_FILE;
        this->actionTimer = 4;
    }
    else if(CHECK_BTN_ALL(input->press.button, BTN_A))
    {
        if(this->chaos_config_option_index >= 0)
        {
            u32 config_index = this->chaos_config_option_index + this->chaos_config_scroll;
            Chaos_SetConfigFlag(config_index, !Chaos_GetConfigFlag(config_index));
            Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
        }
    }
    else if(CHECK_BTN_ANY(input->press.button, BTN_L | BTN_R))
    {
        this->chaos_config_mode = CCM_FADE_IN_CONFIG_DETAILS;
        this->actionTimer = 4;
    }
    else if(this->stickAdjY < -30) 
    {
        if(this->chaos_config_option_index == FILE_SELECT_CHAOS_SETTINGS_MAX_VISIBLE_SETTINGS - 1)
        {
            if(this->chaos_config_scroll + FILE_SELECT_CHAOS_SETTINGS_MAX_VISIBLE_SETTINGS < CHAOS_CONFIG_LAST)
            {
                this->chaos_config_scroll++;
                Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
            }
        } 
        else if(this->chaos_config_option_index < CHAOS_CONFIG_LAST - 1)
        {
            this->chaos_config_option_index++;
            Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
        }
    }
    else if(this->stickAdjY > 30)
    {
        if(this->chaos_config_option_index == 0)
        {
            if(this->chaos_config_scroll > 0)
            {
                this->chaos_config_scroll--;
                Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
            }
            // else
            // {
            //     this->chaos_config_option_index = -1;
            // }
        }
        else
        {
            if(this->chaos_config_option_index > 0)
            {
                this->chaos_config_option_index--;
                Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
            }
        }
    }
}

u32 FileSelect_IsAtEndOfChaosConfigDetails(GameState *thisx)
{
    FileSelectState *this = (FileSelectState *)thisx;
    struct ChaosConfig *config = gChaosConfigs + this->chaos_config_option_index + this->chaos_config_scroll;
    return config->description[this->chaos_config_description_text_offset] == '\0';
} 

u32 FileSelect_AdvanceChaosConfigDetails(GameState *thisx)
{
    FileSelectState *this = (FileSelectState *)thisx;
    struct ChaosConfig *config = gChaosConfigs + this->chaos_config_option_index + this->chaos_config_scroll;
    this->chaos_config_text_buffer_offset = 0;

    while(config->description[this->chaos_config_description_text_offset] != '\n' &&
            config->description[this->chaos_config_description_text_offset] != '\0' && 
            this->chaos_config_text_buffer_offset < sizeof(this->chaos_config_text_buffer) - 1)
    {
        this->chaos_config_text_buffer[this->chaos_config_text_buffer_offset] = config->description[this->chaos_config_description_text_offset];
        this->chaos_config_description_text_offset++;
        this->chaos_config_text_buffer_offset++;
    }

    if(config->description[this->chaos_config_description_text_offset] == '\n')
    {
        this->chaos_config_description_text_offset++;
    }

    this->chaos_config_text_buffer[this->chaos_config_text_buffer_offset] = '\0';
    this->chaos_config_text_buffer_offset++;

    return FileSelect_IsAtEndOfChaosConfigDetails(thisx);
}

void FileSelect_FadeInConfigDetails(GameState *thisx)
{
    FileSelectState *this = (FileSelectState *)thisx;
    this->chaos_config_description_alpha += 12;

    if(this->chaos_config_description_alpha > 200)
    {
        this->chaos_config_description_alpha = 200;
        this->chaos_config_mode = CCM_CHAOS_CONFIG_DETAILS;
        this->actionTimer = 0;
        this->chaos_config_description_text_offset = 0;
        this->chaos_config_text_buffer_offset = 0;

        if(FileSelect_AdvanceChaosConfigDetails(thisx))
        {
            Audio_PlaySfx(NA_SE_SY_MESSAGE_END);
            Font_LoadMessageBoxEndIcon(&this->font, 1);
        }
        else
        {
            Font_LoadMessageBoxEndIcon(&this->font, 0);
        }
    }
}

void FileSelect_ChaosConfigDetails(GameState *thisx)
{
    FileSelectState *this = (FileSelectState *)thisx;
    Input *input = CONTROLLER1(&this->state);

    if(CHECK_BTN_ANY(input->press.button, BTN_B))
    {
        this->chaos_config_mode = CCM_FADE_OUT_CONFIG_DETAILS;
        this->actionTimer = 4;
        Audio_PlaySfx(NA_SE_SY_DECIDE);
    }
    else if(CHECK_BTN_ANY(input->press.button, BTN_A))
    {
        if(FileSelect_IsAtEndOfChaosConfigDetails(thisx))
        {
            this->chaos_config_mode = CCM_FADE_OUT_CONFIG_DETAILS;
            this->actionTimer = 4;
            Audio_PlaySfx(NA_SE_SY_DECIDE);
            return;
        }
        else
        {
            if(!FileSelect_AdvanceChaosConfigDetails(thisx))
            {
                Font_LoadMessageBoxEndIcon(&this->font, 0);
            }
            Audio_PlaySfx(NA_SE_SY_MESSAGE_PASS);
            this->chaos_config_text_flash_timer = 6;
        }
    }

    if(this->chaos_config_text_flash_timer > 0)
    {
        this->chaos_config_text_flash_timer--;

        if(FileSelect_IsAtEndOfChaosConfigDetails(thisx) && this->chaos_config_text_flash_timer == 0)
        {
            Audio_PlaySfx(NA_SE_SY_MESSAGE_END);
            Font_LoadMessageBoxEndIcon(&this->font, 1);
        }
    }
}

void FileSelect_FadeOutConfigDetails(GameState *thisx)
{
    FileSelectState *this = (FileSelectState *)thisx;
    this->chaos_config_description_alpha = 0;
    this->actionTimer = 0;
    this->chaos_config_mode = CCM_CHAOS_OPTIONS;
}

void FileSelect_ChaosWaitForFlashSave(GameState *thisx)
{
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;

    Sram_UpdateWriteToFlashDefault(sramCtx);

    if (sramCtx->status == 0) {
        this->menuMode = FS_MENU_MODE_SELECT;
        this->selectMode = SM_FADE_IN_FILE_INFO;
        this->nextTitleLabel = FS_TITLE_OPEN_FILE;
        this->actionTimer = 4;
    }
}

void (*sChaosConfigModeUpdateFuncs[])(GameState *thisx) = {
    FileSelect_FadeSelectToChaosConfig,
    FileSelect_ChaosOptions,
    FileSelect_FadeInConfigDetails,
    FileSelect_ChaosConfigDetails,
    FileSelect_FadeOutConfigDetails,
    FileSelect_ChaosWaitForFlashSave
};

void FileSelect_ChaosConfigModeUpdate(GameState *thisx)
{
    FileSelectState *this = (FileSelectState *)thisx;
    sChaosConfigModeUpdateFuncs[this->chaos_config_mode](thisx);
}

void FileSelect_ChaosConfigModeDraw(GameState *thisx)
{
    FileSelectState *this = (FileSelectState *)thisx;
    u32 quad_offset = 0;
    u32 index;
    OPEN_DISPS(this->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);

    Gfx_SetupDL42_Opa(this->state.gfxCtx);
    FileSelect_SetView(this, 0.0f, 0.0f, 64.0f);
    FileSelect_SetWindowVtx(&this->state);
    FileSelect_SetWindowContentVtx(&this->state);

    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                    this->windowAlpha);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

    Matrix_Translate(0.0f, 0.0f, -93.6f, MTXMODE_NEW);
    Matrix_Scale(0.78f, 0.78f, 0.78f, MTXMODE_APPLY);
    Matrix_RotateXFApply(this->windowRot / 100.0f);
    // gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(this->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, this->state.gfxCtx);

    gSPVertex(POLY_OPA_DISP++, &this->windowVtx[0], 32, 0);
    gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow1DL);

    gSPVertex(POLY_OPA_DISP++, &this->windowVtx[32], 32, 0);
    gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow2DL);

    gSPVertex(POLY_OPA_DISP++, &this->windowVtx[64], 16, 0);
    gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow3DL);

    FileSelect_DrawWindowContents(&this->state);
    gDPPipeSync(POLY_OPA_DISP++);

    FileSelect_SetView(this, 0.0f, 0.0f, 64.0f);

    CLOSE_DISPS(this->state.gfxCtx);
}



void FileSelect_UpdateAndDrawSkybox(FileSelectState* this) {
    s32 pad;
    f32 eyeX;
    f32 eyeY;
    f32 eyeZ;
    SkyboxContext *skybox_ctx = &this->skyboxCtx;

    OPEN_DISPS(this->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);

    eyeX = 1000.0f * Math_CosS(sFileSelectSkyboxRotation) - 1000.0f * Math_SinS(sFileSelectSkyboxRotation) + Rand_ZeroOne() * 500.0f;
    eyeY = -700.0f + Rand_ZeroOne() * 500.0f;
    eyeZ = 1000.0f * Math_SinS(sFileSelectSkyboxRotation) + 1000.0f * Math_CosS(sFileSelectSkyboxRotation) + Rand_ZeroOne() * 500.0f;

    FileSelect_SetView(this, eyeX, eyeY, eyeZ);
    Skybox_Draw(&this->skyboxCtx, this->state.gfxCtx, SKYBOX_NORMAL_SKY, this->envCtx.skyboxBlend, eyeX, eyeY, eyeZ);

    gDPSetTextureLUT(POLY_OPA_DISP++, G_TT_NONE);

    sFileSelectSkyboxRotation += -0xA;
    skybox_ctx->rot.x += (Rand_ZeroOne() - 0.4f) * 0.2f;
    skybox_ctx->rot.y += (Rand_ZeroOne() - 0.4f) * 0.2f;
    skybox_ctx->rot.z += (Rand_ZeroOne() - 0.4f) * 0.2f;

    CLOSE_DISPS(this->state.gfxCtx);
}

void (*gFileSelectDrawFuncs[])(GameState*) = {
    FileSelect_InitModeDraw,   // FS_MENU_MODE_INIT
    FileSelect_ConfigModeDraw, // FS_MENU_MODE_CONFIG
    FileSelect_SelectModeDraw, // FS_MENU_MODE_SELECT
    FileSelect_ChaosConfigModeDraw
};
void (*gFileSelectUpdateFuncs[])(GameState*) = {
    FileSelect_InitModeUpdate,   // FS_MENU_MODE_INIT
    FileSelect_ConfigModeUpdate, // FS_MENU_MODE_CONFIG
    FileSelect_SelectModeUpdate, // FS_MENU_MODE_SELECT
    FileSelect_ChaosConfigModeUpdate
};

TexturePtr D_808147B4[] = { gFileSelPleaseWaitENGTex, gFileSelDecideCancelENGTex, gFileSelDecideSaveENGTex, gChaosConfigDetailsEngTex };
s16 D_808147C0[] = { 144, 144, 152 };
s16 D_808147C8[] = { 90, 90, 86 };

void FileSelect_Main(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    Input* input = CONTROLLER1(&this->state);
    GfxPrint gfx_print;
    Gfx *text_draw_list;
    s32 texIndex;
    u32 footer_offset = 0;
    s32 pad;

    func_8012CF0C(this->state.gfxCtx, 0, 1, 0, 0, 0);

    OPEN_DISPS(this->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x01, this->staticSegment);
    gSPSegment(POLY_OPA_DISP++, 0x02, this->parameterSegment);
    gSPSegment(POLY_OPA_DISP++, 0x06, this->titleSegment);

    this->stickAdjX = input->rel.stick_x;
    this->stickAdjY = input->rel.stick_y;

    if (this->stickAdjX < -30) {
        if (this->stickXDir == -1) {
            this->inputTimerX--;
            if (this->inputTimerX < 0) {
                this->inputTimerX = 5;
            } else {
                this->stickAdjX = 0;
            }
        } else {
            this->inputTimerX = 10;
            this->stickXDir = -1;
        }
    } else if (this->stickAdjX > 30) {
        if (this->stickXDir == 1) {
            this->inputTimerX--;
            if (this->inputTimerX < 0) {
                this->inputTimerX = 5;
            } else {
                this->stickAdjX = 0;
            }
        } else {
            this->inputTimerX = 10;
            this->stickXDir = 1;
        }
    } else {
        this->stickXDir = 0;
    }

    if (this->stickAdjY < -30) {
        if (this->stickYDir == -1) {
            this->inputTimerY--;
            if (this->inputTimerY < 0) {
                this->inputTimerY = 5;
            } else {
                this->stickAdjY = 0;
            }
        } else {
            this->inputTimerY = 10;
            this->stickYDir = -1;
        }
    } else if (this->stickAdjY > 30) {
        if (this->stickYDir == 1) {
            this->inputTimerY--;
            if (this->inputTimerY < 0) {
                this->inputTimerY = 5;
            } else {
                this->stickAdjY = 0;
            }
        } else {
            this->inputTimerY = 10;
            this->stickYDir = 1;
        }
    } else {
        this->stickYDir = 0;
    }

    this->emptyFileTextAlpha = 0;

    FileSelect_PulsateCursor(&this->state);
    gFileSelectUpdateFuncs[this->menuMode](&this->state);
    FileSelect_UpdateAndDrawSkybox(this);
    gFileSelectDrawFuncs[this->menuMode](&this->state);

    text_draw_list = POLY_XLU_DISP;
    gSPDisplayList(POLY_OPA_DISP++, text_draw_list);
    GfxPrint_Init(&gfx_print);
    GfxPrint_Open(&gfx_print, text_draw_list);
    gfx_print.flags |= GFXP_FLAG_BLEND; 

    GfxPrint_SetPos(&gfx_print, 10, 28);
    GfxPrint_SetColor(&gfx_print, 255, 255, 255, 255 - this->screenFillAlpha);
    GfxPrint_Printf(&gfx_print, "Chaos Edition v%d.%d.%d", CHAOS_MAJOR_VERSION, CHAOS_MINOR_VERSION, CHAOS_PATCH_VERSION);

    text_draw_list = GfxPrint_Close(&gfx_print);
    gDPPipeSync(text_draw_list++);
    gSPEndDisplayList(text_draw_list++);
    POLY_XLU_DISP = text_draw_list;

    Gfx_SetupDL39_Opa(this->state.gfxCtx);

    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 100, 255, 255, this->controlsAlpha);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

    if (this->sramCtx.status > 0) {
        texIndex = 0;
    } else if ((this->configMode >= CM_MAIN_TO_OPTIONS) && (this->configMode <= CM_OPTIONS_TO_MAIN) ||
                this->menuMode == FS_MENU_MODE_CHAOS_CONFIG) {
        texIndex = 2;
    } else {
        texIndex = 1;
    }

    footer_offset = this->chaos_config_box_alpha / 3;

    gDPLoadTextureBlock(POLY_OPA_DISP++, D_808147B4[texIndex], G_IM_FMT_IA, G_IM_SIZ_8b, D_808147C0[texIndex], 16, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                        G_TX_NOLOD);

    gSPTextureRectangle(POLY_OPA_DISP++, (D_808147C8[texIndex] - footer_offset) << 2, 204 << 2,
                        (D_808147C8[texIndex] + D_808147C0[texIndex] - footer_offset) << 2, (204 + 16) << 2, G_TX_RENDERTILE, 0, 0,
                        1 << 10, 1 << 10);

    if(this->menuMode == FS_MENU_MODE_CHAOS_CONFIG)
    {
        gDPPipeSync(POLY_OPA_DISP++);                    
        gDPLoadTextureBlock(POLY_OPA_DISP++, gChaosConfigDetailsEngTex, G_IM_FMT_IA, G_IM_SIZ_8b, 80, 16, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                        G_TX_NOLOD);

        gSPTextureRectangle(POLY_OPA_DISP++, 180 << 2, 204 << 2,
                        (180 + 80) << 2, (204 + 16) << 2, G_TX_RENDERTILE, 0, 0,
                        1 << 10, 1 << 10);
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gSPDisplayList(POLY_OPA_DISP++, sScreenFillSetupDL);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, this->screenFillAlpha);
    gSPDisplayList(POLY_OPA_DISP++, D_0E000000.fillRect);

    CLOSE_DISPS(this->state.gfxCtx);
}

void FileSelect_InitContext(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    EnvironmentContext* envCtx = &this->envCtx;

    Sram_Alloc(&this->state, &this->sramCtx);
    func_801457CC(&this->state, &this->sramCtx);

    this->menuMode = FS_MENU_MODE_INIT;

    this->buttonIndex = this->selectMode = this->selectedFileIndex = this->copyDestFileIndex =
        this->confirmButtonIndex = 0;
 
    this->confirmButtonTexIndices[FS_BTN_CONFIRM_OPTIONS] = 4;
    this->confirmButtonTexIndices[FS_BTN_CONFIRM_YES] = 2;
    this->confirmButtonTexIndices[FS_BTN_CONFIRM_QUIT] = 3;
    this->titleLabel = FS_TITLE_SELECT_FILE;
    this->nextTitleLabel = FS_TITLE_OPEN_FILE;

    this->screenFillAlpha = 255;
    this->highlightPulseDir = 1;
    this->unk_244F4 = 0xC;
    this->highlightColor[0] = 155;
    this->highlightColor[1] = 255;
    this->highlightColor[2] = 255;
    this->highlightColor[3] = 70;
    this->configMode = CM_FADE_IN_START;
    this->windowRot = 0.0f;

    this->stickXDir = this->inputTimerX = 0;
    this->stickYDir = this->inputTimerY = 0;

    this->kbdX = this->kbdY = this->charIndex = 0;

    this->kbdButton = FS_KBD_BTN_NONE;

    this->windowColor[0] = 100;
    this->windowColor[1] = 150;
    this->windowColor[2] = 255;

    this->windowAlpha = 0;
    this->titleAlpha[FS_TITLE_CUR] = 0;
    this->titleAlpha[FS_TITLE_NEXT] = 0;
    this->fileButtonAlpha[0] = 0;
    this->fileButtonAlpha[1] = 0;
    this->fileButtonAlpha[2] = 0;
    this->nameBoxAlpha[0] = 0;
    this->nameBoxAlpha[1] = 0;
    this->nameBoxAlpha[2] = 0;
    this->nameAlpha[0] = 0;
    this->nameAlpha[1] = 0;
    this->nameAlpha[2] = 0;
    this->connectorAlpha[0] = 0;
    this->connectorAlpha[1] = 0;
    this->connectorAlpha[2] = 0;
    this->fileInfoAlpha[0] = 0;
    this->fileInfoAlpha[1] = 0;
    this->fileInfoAlpha[2] = 0;
    this->actionButtonAlpha[FS_BTN_ACTION_COPY] = 0;
    this->actionButtonAlpha[FS_BTN_ACTION_ERASE] = 0;
    this->confirmButtonAlpha[FS_BTN_CONFIRM_YES] = 0;
    this->confirmButtonAlpha[FS_BTN_CONFIRM_QUIT] = 0;
    this->confirmButtonAlpha[FS_BTN_CONFIRM_OPTIONS] = 0;
    this->optionButtonAlpha = 0;
    this->nameEntryBoxAlpha = 0;
    this->controlsAlpha = 0;
    this->emptyFileTextAlpha = 0;
    this->chaos_config_scroll = 0;
    this->chaos_config_box_alpha = 0;
    this->chaos_config_description_alpha = 0;

    this->windowPosX = 6;
    this->actionTimer = 4;
    this->warningLabel = FS_WARNING_NONE;

    this->warningButtonIndex = 0;
    this->buttonYOffsets[0] = 0;
    this->buttonYOffsets[1] = 0;
    this->buttonYOffsets[2] = 0;
    this->buttonYOffsets[3] = 0;
    this->buttonYOffsets[4] = 0;
    this->buttonYOffsets[5] = 0;
    this->buttonYOffsets[6] = 0;
    this->fileNamesY[0] = 0;
    this->fileNamesY[1] = 0;
    this->fileNamesY[2] = 0;

    this->unk_2451E[0] = 0;
    this->unk_2451E[1] = 3;
    this->unk_2451E[2] = 6;
    this->unk_2451E[3] = 8;
    this->unk_2451E[4] = 10;
    this->highlightTimer = 20;

    ShrinkWindow_Letterbox_SetSizeTarget(0);

    gSaveContext.skyboxTime = 0;
    gSaveContext.save.time = CLOCK_TIME(0, 0);

    Skybox_Init(&this->state, &this->skyboxCtx, 1);
    R_TIME_SPEED = 10;

    envCtx->changeSkyboxState = CHANGE_SKYBOX_INACTIVE;
    envCtx->changeSkyboxTimer = 0;
    envCtx->changeLightEnabled = false;
    envCtx->changeLightTimer = 0;
    envCtx->skyboxDmaState = 0;
    envCtx->skybox1Index = 99;
    envCtx->skybox2Index = 99;
    envCtx->lightConfig = 0;
    envCtx->changeLightNextConfig = 0;
    envCtx->lightSetting = 0;
    envCtx->skyboxConfig = SKYBOX_CONFIG_2;
    envCtx->skyboxDisabled = 0;
    envCtx->skyboxBlend = 0;
    envCtx->glareAlpha = 0.0f;
    envCtx->lensFlareAlphaScale = 0.0f;

    gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_ENABLED;
}

void FileSelect_Destroy(GameState* this) {
    ShrinkWindow_Destroy();
}

void FileSelect_Init(GameState* thisx) {
    s32 pad;
    FileSelectState* this = (FileSelectState*)thisx;
    size_t size;

    GameState_SetFramerateDivisor(&this->state, 1);
    Matrix_Init(&this->state);
    ShrinkWindow_Init();
    View_Init(&this->view, this->state.gfxCtx);
    this->state.main = FileSelect_Main;
    this->state.destroy = FileSelect_Destroy;
    FileSelect_InitContext(&this->state);
    Font_LoadOrderedFont(&this->font);

    size = SEGMENT_ROM_SIZE(title_static);
    this->staticSegment = THA_AllocTailAlign16(&this->state.tha, size);
    DmaMgr_RequestSync(this->staticSegment, SEGMENT_ROM_START(title_static), size);

    size = SEGMENT_ROM_SIZE(parameter_static);
    this->parameterSegment = THA_AllocTailAlign16(&this->state.tha, size);
    DmaMgr_RequestSync(this->parameterSegment, SEGMENT_ROM_START(parameter_static), size);

    size = gObjectTable[OBJECT_MAG].vromEnd - gObjectTable[OBJECT_MAG].vromStart;
    this->titleSegment = THA_AllocTailAlign16(&this->state.tha, size);
    DmaMgr_RequestSync(this->titleSegment, gObjectTable[OBJECT_MAG].vromStart, size);

    this->textbox_segment = THA_AllocTailAlign16(&this->state.tha, 0x4000);
    DmaMgr_RequestSync(this->textbox_segment, SEGMENT_ROM_START(message_static), 0x1000);
    // DmaMgr_RequestSync(this->textbox_segment + FS_MSG_CONTINUE_TEX_OFFSET, SEGMENT_ROM_START(message_static) + 0x5000, 0x80);
    // DmaMgr_RequestSync(this->textbox_segment + FS_MSG_END_TEX_OFFSET, SEGMENT_ROM_START(message_static) + 0x5080, 0x80);

    Audio_SetSpec(0xA);
    // Setting ioData to 1 and writing it to ioPort 7 will skip the harp intro
    Audio_PlaySequenceWithSeqPlayerIO(SEQ_PLAYER_BGM_MAIN, NA_BGM_FILE_SELECT, 0, 7, 1);
}
