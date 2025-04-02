#include "chaos_fuckery.h"
#include "functions.h"
#include "regs.h"
#include "libu64/gfxprint.h"
#include "gfxalloc.h"
#include "variables.h"
#include "z64.h"
#include "z64cutscene.h"
#include "z64lifemeter.h"
#include "z64scene.h"
#include "z64object.h"
#include "zelda_arena.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "assets/objects/object_yukimura_obj/object_yukimura_obj.h"
#include "overlays/actors/ovl_En_Arrow/z_en_arrow.h"
#include "fault.h"
 
// For dance party
#include "overlays/actors/ovl_En_Rd/z_en_rd.h"

ChaosContext    gChaosContext; 
u64             gChaosRngState = 1;
u32             gDisplayEffectInfo = 0;
u32             gChaosEffectPageIndex = 0;
u32             gAcceptPageChange = 0;
u32             gPlayerAction;
u32             gPlayerUpperAction;
u32             gRngInitialized = 0;
extern u32      gSceneIndex;
extern u32      gEntranceIndex;
ArrowType       gCurrentArrowType = 0;
u32             gChangeArrowType = 0;
u32             gPlayInitCount = 0;
u32             gCurrentBgmId;

u32             gPlayerActionChangeCount = 0;
u32             gPlayerActionChanges[64];
u32             gForcePause = false;


Vtx gMountainVillageLadderFragment[] = {
    VTX(-CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH, CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_HEIGHT,-8, 0, 		-16 * 256, 0, 0, 120, 154),
	VTX(-CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH,-CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_HEIGHT,-8, 0, 		-64	  , 0, 0, 120, 154),
	VTX( CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH,-CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_HEIGHT,-8, 32 * 64, -64	  , 0, 0, 120, 154),
	VTX( CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH, CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_HEIGHT,-8, 32 * 64, -16 * 256, 0, 0, 120, 154),
};

Gfx gMountainVillageLadderSetupDL[]  = {
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPPipeSync(),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, TEXEL0, PRIMITIVE, 0, COMBINED, 0, 0, 0, 0, COMBINED),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2),
    gsDPPipeSync(),
    gsDPSetTextureLUT(G_TT_NONE),
    gsDPLoadTextureBlock(object_yukimura_obj_Tex_0009D8, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 16, 0, G_TX_MIRROR |
                         G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 5, 4, G_TX_NOLOD, G_TX_NOLOD),
    gsDPSetPrimColor(0, 0x80, 255, 255, 255, 255),
    gsSPLoadGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
};

Gfx gMountainVillageLadderDL[ARRAY_COUNT(gMountainVillageLadderSetupDL) + sizeof(Gfx) + sizeof(Gfx) * CHAOS_MAX_MOUNTAIN_VILLAGE_LADDER_SEGS * 2];
u32 gMountainVillageLadderSegmentCount;
Vtx gMountainVillageLadderSegments[CHAOS_MAX_MOUNTAIN_VILLAGE_LADDER_SEGS][4];

u16 gSimonSaysKeyMap[] = {
    /* [CHAOS_SIMON_SAYS_KEY_DUP]       = */ BTN_DUP,
    /* [CHAOS_SIMON_SAYS_KEY_DRIGHT]    = */ BTN_DRIGHT,
    /* [CHAOS_SIMON_SAYS_KEY_DDOWN]     = */ BTN_DDOWN,
    /* [CHAOS_SIMON_SAYS_KEY_DLEFT]     = */ BTN_DLEFT,
};

const char *gSimonSaysKeyStrs[] = {
    /* [CHAOS_SIMON_SAYS_KEY_DUP]       = */ "D-UP",
    /* [CHAOS_SIMON_SAYS_KEY_DRIGHT]    = */ "D-RIGHT",
    /* [CHAOS_SIMON_SAYS_KEY_DDOWN]     = */ "D-DOWN",
    /* [CHAOS_SIMON_SAYS_KEY_DLEFT]     = */ "D-LEFT",
};

struct SimonSaysConfig gSimonSaysConfigs[] = {
    /* [SIMON_SAYS_CONFIG_PRESS_KEY_OR_DIE] */ {
        "Press %s or die", true, 10
    },
    /* [SIMON_SAYS_CONFIG_PRESS_KEY_TO_DIE] */ {
        "Press %s to die", false, 10
    },
    /* [SIMON_SAYS_CONFIG_PRESS_KEY_AND_DIE] */ {
        "Press %s and die", false, 10
    },
    /* [SIMON_SAYS_CONFIG_DO_NOT_PRESS_KEY_TO_NOT_NOT_DIE] */ {
        "Do not press %s to not not die", true, 3
    },
    /* [SIMON_SAYS_CONFIG_DO_NOT_PRESS_KEY_OR_DIE] */ {
        "Do not press %s or die", false, 6
    },
    /* [SIMON_SAYS_CONFIG_DO_NOT_PRESS_KEY_TO_NOT_DIE] */ {
        "Do not press %s to not die", false, 4
    },
    /* [SIMON_SAYS_CONFIG_DO_NOT_NOT_PRESS_KEY_TO_DIE] */ {
        "Do not not press %s to die", false, 4
    },
    /* [SIMON_SAYS_CONFIG_DO_NOT_NOT_PRESS_KEY_TO_NOT_DIE] */ {
        "Do not not press %s to not die", true, 4
    },
    /* [SIMON_SAYS_CONFIG_DO_NOT_NOT_PRESS_KEY_OR_DIE] */ {
        "Do not not press %s or die", true, 4
    },
    /* [SIMON_SAYS_CONFIG_PRESS_KEY_NOT_TO_NOT_DIE] */ {
        "Press %s not to not die", false, 4
    },
};

// Vec3s           gBeybladePose[] = {
//     /* [PLAYER_LIMB_NONE] */            {0, 3000, 0},
//     /* [PLAYER_LIMB_ROOT] */            {0, 0, 0},
//     /* [PLAYER_LIMB_WAIST] */           {0, 0, 0},
//     /* [PLAYER_LIMB_LOWER_ROOT] */      {0, 0, 0},
//     /* [PLAYER_LIMB_RIGHT_THIGH] */     {0, -0x7000, 0},
//     /* [PLAYER_LIMB_RIGHT_SHIN] */      {0, 0, 0},
//     /* [PLAYER_LIMB_RIGHT_FOOT] */      {0, 0, 0},
//     /* [PLAYER_LIMB_LEFT_THIGH] */      {0, -0x2000, 0},
//     /* [PLAYER_LIMB_LEFT_SHIN] */       {0, 0, 0},
//     /* [PLAYER_LIMB_LEFT_FOOT] */       {0, 0, 0},
//     /* [PLAYER_LIMB_UPPER_ROOT] */      {0, 0, 0},
//     /* [PLAYER_LIMB_HEAD] */            {0x8000, 0, 0},
//     /* [PLAYER_LIMB_HAT] */             {0, 0, 0},
//     /* [PLAYER_LIMB_COLLAR] */          {0, 0, 0},
//     /* [PLAYER_LIMB_LEFT_SHOULDER] */   {0, 0x0c00, 0xb400},
//     /* [PLAYER_LIMB_LEFT_FOREARM] */    {0, 0, 0},
//     /* [PLAYER_LIMB_LEFT_HAND] */       {0, 0, 0xc000},
//     /* [PLAYER_LIMB_RIGHT_SHOULDER] */  {0, 0x4000, 0},
//     /* [PLAYER_LIMB_RIGHT_FOREARM] */   {0, 0, 0},
//     /* [PLAYER_LIMB_RIGHT_HAND] */      {0, 0, 0xc000},
//     /* [PLAYER_LIMB_SHEATH] */          {0, 0, 0},
//     /* [PLAYER_LIMB_TORSO] */           {0, 0, 0},
// };

// JointIndex gBeybladeJointIndices[] = {
//     /* [PLAYER_LIMB_NONE] */            {0,  1,  2},
//     /* [PLAYER_LIMB_ROOT] */            {3,  4,  5},
//     /* [PLAYER_LIMB_WAIST] */           {6,  7,  8},
//     /* [PLAYER_LIMB_LOWER_ROOT] */      {9,  10, 11},
//     /* [PLAYER_LIMB_RIGHT_THIGH] */     {12, 13, 14},
//     /* [PLAYER_LIMB_RIGHT_SHIN] */      {15, 16, 17},
//     /* [PLAYER_LIMB_RIGHT_FOOT] */      {18, 19, 20},
//     /* [PLAYER_LIMB_LEFT_THIGH] */      {21, 22, 23},
//     /* [PLAYER_LIMB_LEFT_SHIN] */       {24, 25, 26},
//     /* [PLAYER_LIMB_LEFT_FOOT] */       {27, 28, 29},
//     /* [PLAYER_LIMB_UPPER_ROOT] */      {30, 31, 32},
//     /* [PLAYER_LIMB_HEAD] */            {33, 34, 35},
//     /* [PLAYER_LIMB_HAT] */             {36, 37, 38},
//     /* [PLAYER_LIMB_COLLAR] */          {39, 40, 41},
//     /* [PLAYER_LIMB_LEFT_SHOULDER] */   {42, 43, 44},
//     /* [PLAYER_LIMB_LEFT_FOREARM] */    {45, 46, 47},
//     /* [PLAYER_LIMB_LEFT_HAND] */       {48, 49, 50},
//     /* [PLAYER_LIMB_RIGHT_SHOULDER] */  {51, 52, 53},
//     /* [PLAYER_LIMB_RIGHT_FOREARM] */   {54, 55, 56},
//     /* [PLAYER_LIMB_RIGHT_HAND] */      {57, 58, 59},
//     /* [PLAYER_LIMB_SHEATH] */          {60, 61, 62},
//     /* [PLAYER_LIMB_TORSO] */           {63, 64, 65},
// };

// AnimationHeader gBeybladeAnimation = {
//     1, gBeybladePose, gBeybladeJointIndices, 66
// };

s16             gVertPosRandList[256];
s16             gTexCoordRandList[256];
u8              gColorRandList[256];

struct ChaosConfig gChaosConfigs[CHAOS_CONFIG_LAST] = {
    /* [CHAOS_CONFIG_BEER_GOGGLES_BLUR] = */ {
        /* .label = */          "Beer goggles motion blur ",  
        /* .description = */    "If enabled, beer goggles effect will use the game's motion blur effect."
    },
    /* [CHAOS_CONFIG_IKANA_CLIMB_TREE_ACTOR_CHASE] = */{
        /* .label = */          "Ikana canyon actor chase", 
        /* .description = */    "If enabled, Ikana canyon hookshotable trees will chase the player when actor chase activates."
    },
    /* [CHAOS_CONFIG_STONE_TOWER_CLIMB_ACTOR_CHASE] */{
        /* .label = */          "Stone Tower actor chase", 
        /* .description = */    "If enabled, hookshot posts and movable block switches in Stone Tower climb will chase the player when actor chase activates."
    },
    /* [CHAOS_CONFIG_DETERMINISTIC_EFFECT_RNG] */{
        /* .label = */          "Deterministic effect rng", 
        /* .description = */    "If enabled, the chaos system will use a separate rng from the game for everything, which will always be initialized to a fixed value.\n"
                                "This means two instances of the game should produce the same effects, with the same configuration, as long as the gameplay in both\n"
                                "remain similar enough to not impose different effect restrictions when an effect is to be activated.\n"
                                "The downside is that random things in the game will have no influence over which effects are activated.\n"
                                "This is mostly useful for people that want to race against one another, so the game doesn't give an unfair advantage to anyone from the get-go."
    },
    /* [CHAOS_CONFIG_USE_DISRUPTIVE_EFFECT_PROB] */{
        /* .label = */          "Disruptive effect prob", 
        /* .description = */    "If enabled, the chaos system will increase the probability of disruptive effects based on how long the player is not affectable by disruptive effects.\n"
                                "This means going through room/scene transitions, warping, getting into cutscenes or talking to npcs too often will result in more annoying effects.\n"
    },
    /* [CHAOS_CONFIG_ALLOW_BEER_GOGGLES_AND_SILENT_FIELD] */{
        /* .label =  */         "Allow alcoholic blidness",
        /* .description */      "If enabled, beer goggles and silent field effects will be allowed to activate at the same time. Together, they tend to make seeing impossible."
    },
    /* [CHAOS_CONFIG_ALLOW_ENEMY_INFIGHTING] */ {
        /* .label = */          "Enemy infighting",
                                "If enabled, chaos spawned enemies will be able to deal damage to others, and will retaliate others if attacked.\n"
                                "This often means enemies will stop paying attention to the player for some time."
    },
    /* [CHAOS_CONFIG_DPAD_DOWN_TO_DIE] */ {
        /* .label = */          "D-Down to die",
        /* .description */      "If enabled, holding D-Pad down will kill Link after 5 seconds. Useful for getting out of softlocks."
    },
    /* [CHAOS_CONFIG_USE_PERIODIC_EFFECT_PROB] */ {
        /* .label = */          "Periodic effect prob",
        /* .description = */    "If enabled, the chaos system will periodically change the probability of disruptive effects to activate.\n"
                                "This means annoying effects will start hitting one after another for a while, and then the game will step back and give the player some time to breathe, before doing it again.\n"
                                "This probability also affects the behavior of some effects."
    },
    /* [CHAOS_CONFIG_RANDOM_MOUNTAIN_VILLAGE_CLIMB] = */ {
        /* .label = */          "Rand mntn. village climb",
        /* .description = */    "If enabled, the mountain village ladder will be randomized at every scene load, making lens of truth absolutely mandatory."
    },
    /* [CHAOS_CONFIG_GIVE_FIERCE_DEITY_MASK] = */{
        /* .label = */          "Random Fierce Deity mask",
        /* .description = */    "If enabled, the random Fierce Deity effect will add Fierce Deity's mask to inventory instead of forcefully transforming Link when activating.\n"
                                "The mask will also be removed from the inventory/C-buttons when deactivating, and Link will be transformed back to human."
    },
    /* [CHAOS_CONFIG_ALLOW_UNDERWATER_OCARINA] = */{
        /* .label = */          "Underwater ocarina",
        /* .description = */    "If enabled, playing ocarina underwater will be possible. This is so it's possible to counter the fast time effect when underwater.\n"
    },
};


struct ChaosConfigSetup gChaosConfigSetups[] = {
    {0, 4, 4, Chaos_SetV044ConfigDefaults},
    {0, 4, 5, Chaos_SetV045ConfigDefaults},
    {0, 4, 6, Chaos_SetV046ConfigDefaults},
    {0, 5, 0, Chaos_SetV050ConfigDefaults},
    {0, 5, 3, Chaos_SetV053ConfigDefaults},
};

PlayerAnimationHeader *gImaginaryFriendAnimations[] = {
    &gPlayerAnim_cl_tewofuru,
    &gPlayerAnim_link_kei_wait,
    &gPlayerAnim_link_keirei,
};

struct ChaosCodeDef gChaosCodeDefs[] = {
    /* [CHAOS_CODE_NONE]                     = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1), 0),
    /* [CHAOS_CODE_LOW_GRAVITY]              = */ CHAOS_CODE_DEF(10, 18, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.02f),
    /* [CHAOS_CODE_CHANGE_HEALTH]            = */ CHAOS_CODE_DEF(1,  10, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.04f),
    /* [CHAOS_CODE_CHANGE_RUPEE]             = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.04f),
    /* [CHAOS_CODE_ACTOR_CHASE]              = */ CHAOS_CODE_DEF(6,  15, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0), 0.01f),
    /* [CHAOS_CODE_YEET]                     = */ CHAOS_CODE_DEF(8,  17, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.01f),
    /* [CHAOS_CODE_POKE]                     = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1), 0.017f),
    /* [CHAOS_CODE_MOON_DANCE]               = */ CHAOS_CODE_DEF(5,  30, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.006f),
    /* [CHAOS_CODE_ONE_HIT_KO]               = */ CHAOS_CODE_DEF(8,  23, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.006f),
    /* [CHAOS_CODE_RANDOM_KNOCKBACK]         = */ CHAOS_CODE_DEF(8,  15, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1), 0.008f),
    /* [CHAOS_CODE_ICE_TRAP]                 = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1), 0.015f),
    /* [CHAOS_CODE_TIMER_UP]                 = */ CHAOS_CODE_DEF(10, 20, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.011f),
    /* [CHAOS_CODE_SHOCK]                    = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1), 0.015f),
    /* [CHAOS_CODE_EARTHQUAKE]               = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.06f),
    /* [CHAOS_CODE_BOMB_ARROWS]              = */ CHAOS_CODE_DEF(15, 25, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.02f),
    /* [CHAOS_CODE_WEIRD_ARROWS]             = */ CHAOS_CODE_DEF(15, 25, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.02f),
    /* [CHAOS_CODE_BUCKSHOT_ARROWS]          = */ CHAOS_CODE_DEF(15, 25, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.02f),
    /* [CHAOS_CODE_RANDOM_BOMB_TIMER]        = */ CHAOS_CODE_DEF(10, 15, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.028f),
    /* [CHAOS_CODE_LOVELESS_MARRIAGE]        = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0), 0.0055f),
    /* [CHAOS_CODE_WEIRD_UI]                 = */ CHAOS_CODE_DEF(8,  15, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1), 0.04f),
    /* [CHAOS_CODE_BEER_GOGGLES]             = */ CHAOS_CODE_DEF(25, 55, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.0044f),
    /* [CHAOS_CODE_CHANGE_MAGIC]             = */ CHAOS_CODE_DEF(1,  10, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.04f),
    /* [CHAOS_CODE_INVINCIBLE]               = */ CHAOS_CODE_DEF(8,  23, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.005f),
    /* [CHAOS_CODE_SYKE]                     = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1), 0.000089f),
    /* [CHAOS_CODE_DIE]                      = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1), 0.00001f),
    /* [CHAOS_CODE_TRAP_FLAP]                = */ CHAOS_CODE_DEF(10, 20, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.02f),
    // /* [CHAOS_CODE_TEXTBOX]                  = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1), 0.008f),
    /* [CHAOS_CODE_SLIPPERY_FLOORS]          = */ CHAOS_CODE_DEF(10, 20, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.012f),
    /* [CHAOS_CODE_SLOW_DOWN]                = */ CHAOS_CODE_DEF(5,  12, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.006f),
    /* [CHAOS_CODE_ENTRANCE_RANDO]           = */ CHAOS_CODE_DEF(5,  15, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.008f),
    /* [CHAOS_CODE_TERRIBLE_MUSIC]           = */ CHAOS_CODE_DEF(15, 35, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.01f),
    /* [CHAOS_CODE_INCREDIBLE_KNOCKBACK]     = */ CHAOS_CODE_DEF(10, 21, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.008f),
    /* [CHAOS_CODE_RANDOM_SCALING]           = */ CHAOS_CODE_DEF(10, 21, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.007f),
    /* [CHAOS_CODE_BIG_BROTHER]              = */ CHAOS_CODE_DEF(25, 70, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.002f),
    /* [CHAOS_CODE_OUT_OF_SHAPE]             = */ CHAOS_CODE_DEF(5,  12, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1), 0.008f),
    // /* [CHAOS_CODE_TUNIC_COLOR]              = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.006f),
    /* [CHAOS_CODE_WEIRD_SKYBOX]             = */ CHAOS_CODE_DEF(10, 15, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.006f),
    /* [CHAOS_CODE_PLAY_OCARINA]             = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1), 0.004f),
    /* [CHAOS_CODE_RANDO_FIERCE_DEITY]       = */ CHAOS_CODE_DEF(50, 150,CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1), 0.001f),
    /* [CHAOS_CODE_CHICKEN_ARISE]            = */ CHAOS_CODE_DEF(15, 23, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0), 0.0055f),
    /* [CHAOS_CODE_STARFOX]                  = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0), 0.0055f),
    /* [CHAOS_CODE_SWAP_HEAL_AND_HURT]       = */ CHAOS_CODE_DEF(5,  25, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.003f),
    /* [CHAOS_CODE_JUNK_ITEM]                = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1), 0.0055f),
    /* [CHAOS_CODE_RANDOM_HEALTH_UP]         = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.001f),
    /* [CHAOS_CODE_RANDOM_HEALTH_DOWN]       = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.001f),
    /* [CHAOS_CODE_IMAGINARY_FRIENDS]        = */ CHAOS_CODE_DEF(5,  12, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1), 0.008f),
    /* [CHAOS_CODE_WALLMASTER]               = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0), 0.0055f),
    /* [CHAOS_CODE_REDEADASS_GROOVE]         = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0), 0.0065f),
    /* [CHAOS_CODE_SCALE_RANDOM_LIMB]        = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.0016f),
    /* [CHAOS_CODE_LIFTOFF]                  = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1), 0.00018f),
    /* [CHAOS_CODE_UNSTABLE_ROOMS]           = */ CHAOS_CODE_DEF(15, 25, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 0.005f),
    /* [CHAOS_CODE_HEART_SNAKE]              = */ CHAOS_CODE_DEF(25, 65, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 0.0038f),
    /* [CHAOS_CODE_FAST_TIME]                = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 0.0012f),
    /* [CHAOS_CODE_MOON_CRASH]               = */ CHAOS_CODE_DEF(0,  0,  CHAOS_CODE_RESTRICTION_FLAG_MASK(1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1), 0.0006f),
    /* [CHAOS_CODE_SPEEDBOOST]               = */ CHAOS_CODE_DEF(15, 20, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1), 0.0035f),
    /* [CHAOS_CODE_BILLBOARD_ACTORS]         = */ CHAOS_CODE_DEF(21, 36, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.0045f),
    /* [CHAOS_CODE_SIGNPOST]                 = */ CHAOS_CODE_DEF(35, 60, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.0035f),
    /* [CHAOS_CODE_SILENT_FIELD]             = */ CHAOS_CODE_DEF(20, 35, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0), 0.00075f),
    /* [CHAOS_CODE_BAD_CONNECTION]           = */ CHAOS_CODE_DEF(9, 18,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0), 0.0025f),
    /* [CHAOS_CODE_BEYBLADE]                 = */ CHAOS_CODE_DEF(20, 35, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 0.0025f),
    /* [CHAOS_CODE_DIRECTILE_DYSFUNCTION]    = */ CHAOS_CODE_DEF(5, 15,  CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 0.0045f),
    /* [CHAOS_CODE_LENGTH_CONTRACTION]       = */ CHAOS_CODE_DEF(15, 25, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 0.0025f),
    /* [CHAOS_CODE_FISH]                     = */ CHAOS_CODE_DEF(0, 0,   CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 0.0035f),
    /* [CHAOS_CODE_AIR_SUPPORT]              = */ CHAOS_CODE_DEF(25, 65, CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 0.0035f),
    /* [CHAOS_CODE_SIMON_SAYS]               = */ CHAOS_CODE_DEF(0, 0,   CHAOS_CODE_RESTRICTION_FLAG_MASK(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 0.0030f),
};
  
const char *gChaosCodeNames[] = {
    /* [CHAOS_CODE_NONE]                        = */ "NONE",
    /* [CHAOS_CODE_LOW_GRAVITY]                 = */ "Low gravity",
    /* [CHAOS_CODE_CHANGE_HEALTH]               = */ "Change health",
    /* [CHAOS_CODE_CHANGE_RUPEE]                = */ "Change rupee",
    /* [CHAOS_CODE_ACTOR_CHASE]                 = */ "Actor chase",
    /* [CHAOS_CODE_YEET]                        = */ "YEET",
    /* [CHAOS_CODE_POKE]                        = */ "Poke",
    /* [CHAOS_CODE_MOON_DANCE]                  = */ "Moon dance",
    /* [CHAOS_CODE_ONE_HIT_KO]                  = */ "One-hit KO",
    /* [CHAOS_CODE_RANDOM_KNOCKBACK]            = */ "Random knockback",
    /* [CHAOS_CODE_ICE_TRAP]                    = */ "Ice trap",
    /* [CHAOS_CODE_TIMER_UP]                    = */ "Timer up",
    /* [CHAOS_CODE_SHOCK]                       = */ "Shock",
    /* [CHAOS_CODE_EARTHQUAKE]                  = */ "Earthquake",
    /* [CHAOS_CODE_BOMB_ARROWS]                 = */ "Bomb arrows",
    /* [CHAOS_CODE_WEIRD_ARROWS]                = */ "Weird arrows",
    /* [CHAOS_CODE_BUCKSHOT_ARROWS]             = */ "Buckshot arrows",
    /* [CHAOS_CODE_RANDOM_BOMB_TIMER]           = */ "Random bomb timer",
    /* [CHAOS_CODE_LOVELESS_MARRIAGE]           = */ "Loveless marriage",
    /* [CHAOS_CODE_WEIRD_UI]                    = */ "Weird UI",
    /* [CHAOS_CODE_BEER_GOGGLES]                = */ "Beer goggles",
    /* [CHAOS_CODE_CHANGE_MAGIC]                = */ "Change magic",
    /* [CHAOS_CODE_INVINCIBLE]                  = */ "Invincible",
    /* [CHAOS_CODE_SYKE]                        = */ "SYKE",
    /* [CHAOS_CODE_DIE]                         = */ "DIE",
    /* [CHAOS_CODE_TRAP_FLAP]                   = */ "Trap flap",
    // /* [CHAOS_CODE_TEXTBOX]                     = */ "Textbox",
    /* [CHAOS_CODE_SLIPPERY_FLOORS]             = */ "Slippery floors",
    /* [CHAOS_CODE_SLOW_DOWN]                   = */ "Slow down",
    /* [CHAOS_CODE_ENTRANCE_RANDO]              = */ "Entrance rando",
    /* [CHAOS_CODE_TERRIBLE_MUSIC]              = */ "Terrible music",
    /* [CHAOS_CODE_INCREDIBLE_KNOCKBACK]        = */ "Incredible knockback",
    /* [CHAOS_CODE_RANDOM_SCALING]              = */ "Random scaling",
    /* [CHAOS_CODE_BIG_BROTHER]                 = */ "Big brother",
    /* [CHAOS_CODE_OUT_OF_SHAPE]                = */ "Out of shape",
    // /* [CHAOS_CODE_TUNIC_COLOR]                 = */ "Tunic color",
    /* [CHAOS_CODE_WEIRD_SKYBOX]                = */ "Weird skybox",
    /* [CHAOS_CODE_PLAY_OCARINA]                = */ "Play ocarina",
    /* [CHAOS_CODE_RANDO_FIERCE_DEITY]          = */ "Random fierce deity",
    /* [CHAOS_CODE_CHICKEN_ARISE]               = */ "Chicken arise",
    /* [CHAOS_CODE_STARFOX]                     = */ "Starfox",
    /* [CHAOS_CODE_SWAP_HEAL_AND_HURT]          = */ "Swap heal and hurt",
    /* [CHAOS_CODE_JUNK_ITEM]                   = */ "Junk item",
    /* [CHAOS_CODE_RANDOM_HEALTH_UP]            = */ "Random health up",
    /* [CHAOS_CODE_RANDOM_HEALTH_DOWN]          = */ "Random health down",
    /* [CHAOS_CODE_IMAGINARY_FRIENDS]           = */ "Imaginary friends",
    /* [CHAOS_CODE_WALLMASTER]                  = */ "Wallmaster",
    /* [CHAOS_CODE_REDEADASS_GROOVE]            = */ "Redeadass groove",
    /* [CHAOS_CODE_SCALE_RANDOM_LIMB]           = */ "Scale random limb",
    /* [CHAOS_CODE_LIFTOFF]                     = */ "Liftoff",
    /* [CHAOS_CODE_UNSTABLE_ROOMS]              = */ "Unstable rooms",
    /* [CHAOS_CODE_HEART_SNAKE]                 = */ "Heart snake",
    /* [CHAOS_CODE_FAST_TIME]                   = */ "Fast time",
    /* [CHAOS_CODE_MOON_CRASH]                  = */ "Moon crash",
    /* [CHAOS_CODE_SPEEDBOOST]                  = */ "Speed boost",
    /* [CHAOS_CODE_BILLBOARD_ACTORS]            = */ "Billboard actors",
    /* [CHAOS_CODE_SIGNPOST]                    = */ "Signpost",
    /* [CHAOS_CODE_SILENT_FIELD]                = */ "Silent field",
    /* [CHAOS_CODE_BAD_CONNECTION]              = */ "Bad connection",
    /* [CHAOS_CODE_BEYBLADE]                    = */ "Beyblade",
    /* [CHAOS_CODE_DIRECTILE_DYSFUNCTION]       = */ "Directile dysfunction",
    /* [CHAOS_CODE_LENGTH_CONTRACTION]          = */ "Length contraction",
    /* [CHAOS_CODE_FISH]                        = */ "Fish",
    /* [CHAOS_CODE_AIR_SUPPORT]                 = */ "Air support",
    /* [CHAOS_CODE_SIMON_SAYS]                  = */ "Simon says",
};

enum FAIRY_FOUNTAIN_EXITS
{
    FAIRY_FOUNTAIN_EXIT_NORTH_CLOCK_TOWN,
    FAIRY_FOUNTAIN_EXIT_WOODFALL,
    FAIRY_FOUNTAIN_EXIT_SNOWHEAD,
    FAIRY_FOUNTAIN_EXIT_SEA,
    FAIRY_FOUNTAIN_EXIT_IKANA_CANYON,
};

struct RandomSceneEntrances
{
    u8 *    entrances;
    u8      entrance_count;
};

u8 gMayorsResidenceEntrances[] =                    {0};
u8 gMagicHagsPotionShopEntrances[] =                {0};
u8 gRomaniRanchInteriorEntrances[] =                {0, 1};
u8 gHoneyAndDarlingsShopEntrances[] =               {0};
u8 gBeneathTheGraveyardEntrances[] =                {0, 1};
u8 gSouthernSwampEntrances[] =                      {0, 1, 2, 3, 4, 5, 7, 8, 9};
u8 gCuriosityShopEntrances[] =                      {0, 1};
u8 gGrottoEntrances[] =                             {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
u8 gOdowlaLairEntrances[] =                         {0};
u8 gIkanaCanyonEntrances[] =                        {0, 1, 2, 3, 5, 6, 8, 11, 12, 13, 14};
u8 gPirateFortressInnerWallsExteriorEntrances[] =   {0, 1, 2, 3, 4, 5, 6, 7, 8};
u8 gMilkBarEntrances[] =                            {0};
u8 gStoneTowerTempleEntrances[] =                   {0};
u8 gTreasureChestShopEntrances[] =                  {0};
u8 gInvertedStoneTowerTempleEntrances[] =           {0};
u8 gWoodfallTempleEntrances[] =                     {0, 2};
u8 gPathToMountainVillageEntrances[] =              {0};
u8 gAncientCastleOfIkanaEntrances[] =               {0, 1, 2, 3};
u8 gTownShootingGalleryEntrances[] =                {0};
u8 gSnowheadTempleEntrances[] =                     {0};
u8 gMilkRoadEntrances[] =                           {0, 1, 2, 3};
u8 gPirateFortressInteriorEntrances[] =             {0, 1, 2, 3, 4, 5, 6, 7, 9, 10};
u8 gSwampShootingGalleryEntrances[] =               {0};
u8 gPinackleRockEntrances[] =                       {0};
u8 gGreatFairyFontainEntrances[] = {
    FAIRY_FOUNTAIN_EXIT_NORTH_CLOCK_TOWN, 
    FAIRY_FOUNTAIN_EXIT_WOODFALL, 
    FAIRY_FOUNTAIN_EXIT_SNOWHEAD,
    FAIRY_FOUNTAIN_EXIT_SEA, 
    FAIRY_FOUNTAIN_EXIT_IKANA_CANYON
};
u8 gSwampSpiderHouseEntrances[] =                   {0};
u8 gOceansideSpiderHouseEntrances[] =               {0};
u8 gObservatoryEntrances[] =                        {0, 1};
u8 gDekuPalaceEntrances[] =                         {0, 2, 3, 4, 5};
u8 gMountainSmithyEntrances[] =                     {0};
u8 gTerminaFieldEntrances[] =                       {0, 3, 4, 5, 6, 7, 8, 9};
u8 gPostOfficeEntrances[] =                         {0};
u8 gMarineResearchLabEntrances[] =                  {0};
u8 gGoronShrineEntrances[] =                        {0};
u8 gZoraHallEntrances[] =                           {0, 1, 2, 3, 4, 5, 6};
u8 gTradingPostEntrances[] =                        {0};
u8 gRomaniRanchExteriorEntrances[] =                {0, 2, 3, 4, 5};
u8 gTwinmoldLairEntrances[] =                       {0};
u8 gGreatBayCoastEntrances[] =                      {0, 1, 2, 3, 4, 5, 7, 8, 12};
u8 gZoraCapeEntrances[] =                           {0, 1, 2, 3, 4, 5};
u8 gLotteryShopEntrances[] =                        {0};
u8 gPirateFortressOuterWallsEntrances[] =           {0, 1, 3, 4, 5, 6};
u8 gFishermansHutEntrances[] =                      {0};
u8 gGoronShopEntrances[] =                          {0};
u8 gDekuKingChamberEntrances[] =                    {0, 1};
u8 gPathToSouthernSwampEntrances[] =                {0, 1, 2};
u8 gDoggyRacetrackEntrances[] =                     {0};
u8 gCuccoShakEntrances[] =                          {0};
u8 gIkanaGraveyardEntrances[] =                     {0, 1, 2, 3, 4};
u8 gGohtLairEntrances[] =                           {0};
u8 gWoodfallEntrances[] =                           {0, 1, 2, 3};
u8 gGoronVillageEntrances[] =                       {0, 2, 3};
u8 gGreatBayTempleEntrances[] =                     {0};
u8 gWaterfalRapidsEntrances[] =                     {0};
u8 gBeneathTheWellEntrances[] =                     {0, 1};
u8 gZoraHallRoomsEntrances[] =                      {0, 1, 2, 3, 5};
u8 gGoronGraveyardEntrances[] =                     {0};
u8 gSakonHideoutEntrances[] =                       {0};
u8 gMountainVillageEntrances[] =                    {1, 2, 3, 4};
u8 gGhostHutEntrances[] =                           {0};
u8 gDekuShrineEntrances[] =                         {0};
u8 gPathToIkanaCanyon[] =                           {0, 1, 2};
u8 gSwordmanSchoolEntrances[] =                     {0};
u8 gMusicBoxHouseEntrances[] =                      {0};
u8 gTouristInformationEntrances[] =                 {0};
u8 gStoneTowerTempleOutsideEntrances[] =            {0, 2};
u8 gPathToSnowheadEntrances[] =                     {0, 1};
u8 gSnowheadEntrances[] =                           {0, 1, 2};
u8 gPathToGoronVillageEntrances[] =                 {0, 1, 2};
u8 gGyorgLairEntrances[] =                          {0};
u8 gSecretShrineEntrances[] =                       {0};
u8 gStockPotInnEntrances[] =                        {0, 1};
u8 gInsideClockTowerEntrances[] =                   {1, 5};
u8 gWoodsOfMysteryEntrances[] =                     {0};
u8 gBombShopEntrances[] =                           {0};
u8 gGormanTrackEntrances[] =                        {0, 3, 4};
u8 gGoronRacetrackEntrances[] =                     {0};
u8 gEastClocktownEntrances[] =                      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
u8 gWestClocktownEntrances[] =                      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
u8 gNorthClocktownEntrances[] =                     {0, 1, 2, 3, 4};
u8 gSouthClocktownEntrances[] =                     {0, 1, 2, 3, 4, 5, 6, 7, 8};
u8 gLaundryPoolEntrances[] =                        {0, 1};

u8 gBossEntrances[] = {
    ENTR_SCENE_ODOLWAS_LAIR, 
    ENTR_SCENE_GOHTS_LAIR, 
    ENTR_SCENE_GYORGS_LAIR,
    ENTR_SCENE_TWINMOLDS_LAIR
};

struct ChaosSpawnActorCodeDef gSpawnActorCodeDefs[] = {
    // { OBJECT_RR,            CHAOS_CODE_LOVELESS_MARRIAGE },
    { OBJECT_NIW,           CHAOS_CODE_CHICKEN_ARISE },
    { OBJECT_ARWING,        CHAOS_CODE_STARFOX },
    { OBJECT_WALLMASTER,    CHAOS_CODE_WALLMASTER },
    { OBJECT_RD,            CHAOS_CODE_REDEADASS_GROOVE }
};

#define RANDOM_ENTRANCES_DEF(entrances) {           \
    /* .entrances = */ entrances,                   \
    /* .entrance_count */ ARRAY_COUNT(entrances)    \
}                                                   \

#define RANDOM_ENTRANCES_EMPTY() {NULL, 0}
 
struct RandomSceneEntrances gEntrances[] = {
    /* [ENTR_SCENE_MAYORS_RESIDENCE] = */               RANDOM_ENTRANCES_DEF(gMayorsResidenceEntrances),
    /* [ENTR_SCENE_MAJORAS_LAIR] = */                   RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_MAGIC_HAGS_POTION_SHOP] = */         RANDOM_ENTRANCES_DEF(gMagicHagsPotionShopEntrances),
    /* [ENTR_SCENE_RANCH_HOUSE] = */                    RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_HONEY_AND_DARLINGS_SHOP] = */        RANDOM_ENTRANCES_DEF(gHoneyAndDarlingsShopEntrances),
    /* [ENTR_SCENE_BENEATH_THE_GRAVERYARD] = */         RANDOM_ENTRANCES_DEF(gBeneathTheGraveyardEntrances),
    /* [ENTR_SCENE_SOUTHERN_SWAMP_CLEARED] = */         RANDOM_ENTRANCES_DEF(gSouthernSwampEntrances),
    /* [ENTR_SCENE_CURIOSITY_SHOP] = */                 RANDOM_ENTRANCES_DEF(gCuriosityShopEntrances),
    /* [ENTR_SCENE_UNSET_08] = */                       RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_UNSET_09] = */                       RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_GROTTOS] = */                        RANDOM_ENTRANCES_DEF(gGrottoEntrances),
    /* [ENTR_SCENE_UNSET_0B] = */                       RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_UNSET_0C] = */                       RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_UNSET_0D] = */                       RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_CUTSCENE] = */                       RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_UNSET_0F] = */                       RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_IKANA_CANYON] = */                   RANDOM_ENTRANCES_DEF(gIkanaCanyonEntrances),
    /* [ENTR_SCENE_PIRATES_FORTRESS] = */               RANDOM_ENTRANCES_DEF(gPirateFortressInnerWallsExteriorEntrances),
    /* [ENTR_SCENE_MILK_BAR] = */                       RANDOM_ENTRANCES_DEF(gMilkBarEntrances),
    /* [ENTR_SCENE_STONE_TOWER_TEMPLE] = */             RANDOM_ENTRANCES_DEF(gStoneTowerTempleOutsideEntrances),
    /* [ENTR_SCENE_TREASURE_CHEST_SHOP] = */            RANDOM_ENTRANCES_DEF(gTreasureChestShopEntrances),
    /* [ENTR_SCENE_STONE_TOWER_TEMPLE_INVERTED] = */    RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_CLOCK_TOWER_ROOFTOP] = */            RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_OPENING_DUNGEON] = */                RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_WOODFALL_TEMPLE] = */                RANDOM_ENTRANCES_DEF(gWoodfallTempleEntrances),
    /* [ENTR_SCENE_PATH_TO_MOUNTAIN_VILLAGE] = */       RANDOM_ENTRANCES_DEF(gPathToMountainVillageEntrances),
    /* [ENTR_SCENE_IKANA_CASTLE] = */                   RANDOM_ENTRANCES_DEF(gAncientCastleOfIkanaEntrances),
    /* [ENTR_SCENE_DEKU_SCRUB_PLAYGROUND] = */          RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_ODOLWAS_LAIR] = */                   RANDOM_ENTRANCES_DEF(gOdowlaLairEntrances),
    /* [ENTR_SCENE_TOWN_SHOOTING_GALLERY] = */          RANDOM_ENTRANCES_DEF(gTownShootingGalleryEntrances),
    /* [ENTR_SCENE_SNOWHEAD_TEMPLE] = */                RANDOM_ENTRANCES_DEF(gSnowheadTempleEntrances),
    /* [ENTR_SCENE_MILK_ROAD] = */                      RANDOM_ENTRANCES_DEF(gMilkRoadEntrances),
    /* [ENTR_SCENE_PIRATES_FORTRESS_INTERIOR] = */      RANDOM_ENTRANCES_DEF(gPirateFortressInteriorEntrances),
    /* [ENTR_SCENE_SWAMP_SHOOTING_GALLERY] = */         RANDOM_ENTRANCES_DEF(gSwampShootingGalleryEntrances),
    /* [ENTR_SCENE_PINNACLE_ROCK] = */                  RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_FAIRY_FOUNTAIN] = */                 RANDOM_ENTRANCES_DEF(gGreatFairyFontainEntrances),
    /* [ENTR_SCENE_SWAMP_SPIDER_HOUSE] = */             RANDOM_ENTRANCES_DEF(gSwampSpiderHouseEntrances),
    /* [ENTR_SCENE_OCEANSIDE_SPIDER_HOUSE] = */         RANDOM_ENTRANCES_DEF(gOceansideSpiderHouseEntrances),
    /* [ENTR_SCENE_ASTRAL_OBSERVATORY] = */             RANDOM_ENTRANCES_DEF(gObservatoryEntrances),
    /* [ENTR_SCENE_MOON_DEKU_TRIAL] = */                RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_DEKU_PALACE] = */                    RANDOM_ENTRANCES_DEF(gDekuPalaceEntrances),
    /* [ENTR_SCENE_MOUNTAIN_SMITHY] = */                RANDOM_ENTRANCES_DEF(gMountainSmithyEntrances),
    /* [ENTR_SCENE_TERMINA_FIELD] = */                  RANDOM_ENTRANCES_DEF(gTerminaFieldEntrances),
    /* [ENTR_SCENE_POST_OFFICE] = */                    RANDOM_ENTRANCES_DEF(gPostOfficeEntrances),
    /* [ENTR_SCENE_MARINE_RESEARCH_LAB] = */            RANDOM_ENTRANCES_DEF(gMarineResearchLabEntrances),
    /* [ENTR_SCENE_DAMPES_HOUSE] = */                   RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_UNSET_2E] = */                       RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_GORON_SHRINE] = */                   RANDOM_ENTRANCES_DEF(gGoronShrineEntrances),
    /* [ENTR_SCENE_ZORA_HALL] = */                      RANDOM_ENTRANCES_DEF(gZoraHallEntrances),
    /* [ENTR_SCENE_TRADING_POST] = */                   RANDOM_ENTRANCES_DEF(gTradingPostEntrances),
    /* [ENTR_SCENE_ROMANI_RANCH] = */                   RANDOM_ENTRANCES_DEF(gRomaniRanchExteriorEntrances),
    /* [ENTR_SCENE_TWINMOLDS_LAIR] = */                 RANDOM_ENTRANCES_DEF(gTwinmoldLairEntrances),
    /* [ENTR_SCENE_GREAT_BAY_COAST] = */                RANDOM_ENTRANCES_DEF(gGreatBayCoastEntrances),
    /* [ENTR_SCENE_ZORA_CAPE] = */                      RANDOM_ENTRANCES_DEF(gZoraCapeEntrances),
    /* [ENTR_SCENE_LOTTERY_SHOP] = */                   RANDOM_ENTRANCES_DEF(gLotteryShopEntrances),
    /* [ENTR_SCENE_UNSET_37] = */                       RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_PIRATES_FORTRESS_EXTERIOR] = */      RANDOM_ENTRANCES_DEF(gPirateFortressOuterWallsEntrances),
    /* [ENTR_SCENE_FISHERMANS_HUT] = */                 RANDOM_ENTRANCES_DEF(gFishermansHutEntrances),
    /* [ENTR_SCENE_GORON_SHOP] = */                     RANDOM_ENTRANCES_DEF(gGoronShopEntrances),
    /* [ENTR_SCENE_DEKU_KINGS_CHAMBER] = */             RANDOM_ENTRANCES_DEF(gDekuKingChamberEntrances),
    /* [ENTR_SCENE_MOON_GORON_TRIAL] = */               RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_ROAD_TO_SOUTHERN_SWAMP] = */         RANDOM_ENTRANCES_DEF(gPathToSouthernSwampEntrances),
    /* [ENTR_SCENE_DOGGY_RACETRACK] = */                RANDOM_ENTRANCES_DEF(gDoggyRacetrackEntrances),
    /* [ENTR_SCENE_CUCCO_SHACK] = */                    RANDOM_ENTRANCES_DEF(gCuccoShakEntrances),
    /* [ENTR_SCENE_IKANA_GRAVEYARD] = */                RANDOM_ENTRANCES_DEF(gIkanaGraveyardEntrances),
    /* [ENTR_SCENE_GOHTS_LAIR] = */                     RANDOM_ENTRANCES_DEF(gGohtLairEntrances),
    /* [ENTR_SCENE_SOUTHERN_SWAMP_POISONED] = */        RANDOM_ENTRANCES_DEF(gSouthernSwampEntrances),
    /* [ENTR_SCENE_WOODFALL] = */                       RANDOM_ENTRANCES_DEF(gWoodfallEntrances),
    /* [ENTR_SCENE_MOON_ZORA_TRIAL] = */                RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_GORON_VILLAGE_SPRING] = */           RANDOM_ENTRANCES_DEF(gGoronVillageEntrances),
    /* [ENTR_SCENE_GREAT_BAY_TEMPLE] = */               RANDOM_ENTRANCES_DEF(gGreatBayTempleEntrances),
    /* [ENTR_SCENE_WATERFALL_RAPIDS] = */               RANDOM_ENTRANCES_DEF(gWaterfalRapidsEntrances),
    /* [ENTR_SCENE_BENEATH_THE_WELL] = */               RANDOM_ENTRANCES_DEF(gBeneathTheWellEntrances),
    /* [ENTR_SCENE_ZORA_HALL_ROOMS] = */                RANDOM_ENTRANCES_DEF(gZoraHallRoomsEntrances),
    /* [ENTR_SCENE_GORON_VILLAGE_WINTER] = */           RANDOM_ENTRANCES_DEF(gGoronVillageEntrances),
    /* [ENTR_SCENE_GORON_GRAVERYARD] = */               RANDOM_ENTRANCES_DEF(gGoronGraveyardEntrances),
    /* [ENTR_SCENE_SAKONS_HIDEOUT] = */                 RANDOM_ENTRANCES_DEF(gSakonHideoutEntrances),
    /* [ENTR_SCENE_MOUNTAIN_VILLAGE_WINTER] = */        RANDOM_ENTRANCES_DEF(gMountainVillageEntrances),
    /* [ENTR_SCENE_GHOST_HUT] = */                      RANDOM_ENTRANCES_DEF(gGhostHutEntrances),
    /* [ENTR_SCENE_DEKU_SHRINE] = */                    RANDOM_ENTRANCES_DEF(gDekuShrineEntrances),
    /* [ENTR_SCENE_ROAD_TO_IKANA] = */                  RANDOM_ENTRANCES_DEF(gPathToIkanaCanyon),
    /* [ENTR_SCENE_SWORDMANS_SCHOOL] = */               RANDOM_ENTRANCES_DEF(gSwordmanSchoolEntrances),
    /* [ENTR_SCENE_MUSIC_BOX_HOUSE] = */                RANDOM_ENTRANCES_DEF(gMusicBoxHouseEntrances),
    /* [ENTR_SCENE_IGOS_DU_IKANAS_LAIR] = */            RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_TOURIST_INFORMATION] = */            RANDOM_ENTRANCES_DEF(gTouristInformationEntrances),
    /* [ENTR_SCENE_STONE_TOWER] = */                    RANDOM_ENTRANCES_DEF(gStoneTowerTempleOutsideEntrances),
    /* [ENTR_SCENE_STONE_TOWER_INVERTED] = */           RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_MOUNTAIN_VILLAGE_SPRING] = */        RANDOM_ENTRANCES_DEF(gMountainVillageEntrances),
    /* [ENTR_SCENE_PATH_TO_SNOWHEAD] = */               RANDOM_ENTRANCES_DEF(gPathToSnowheadEntrances),
    /* [ENTR_SCENE_SNOWHEAD] = */                       RANDOM_ENTRANCES_DEF(gSnowheadEntrances),
    /* [ENTR_SCENE_PATH_TO_GORON_VILLAGE_WINTER] = */   RANDOM_ENTRANCES_DEF(gPathToGoronVillageEntrances),
    /* [ENTR_SCENE_PATH_TO_GORON_VILLAGE_SPRING] = */   RANDOM_ENTRANCES_DEF(gPathToGoronVillageEntrances),
    /* [ENTR_SCENE_GYORGS_LAIR] = */                    RANDOM_ENTRANCES_DEF(gGyorgLairEntrances),
    /* [ENTR_SCENE_SECRET_SHRINE] = */                  RANDOM_ENTRANCES_DEF(gSecretShrineEntrances),
    /* [ENTR_SCENE_STOCK_POT_INN] = */                  RANDOM_ENTRANCES_DEF(gStockPotInnEntrances),
    /* [ENTR_SCENE_GREAT_BAY_CUTSCENE] = */             RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_CLOCK_TOWER_INTERIOR] = */           RANDOM_ENTRANCES_DEF(gInsideClockTowerEntrances),
    /* [ENTR_SCENE_WOODS_OF_MYSTERY] = */               RANDOM_ENTRANCES_DEF(gWoodsOfMysteryEntrances),
    /* [ENTR_SCENE_LOST_WOODS] = */                     RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_MOON_LINK_TRIAL] = */                RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_THE_MOON] = */                       RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_BOMB_SHOP] = */                      RANDOM_ENTRANCES_DEF(gBombShopEntrances),
    /* [ENTR_SCENE_GIANTS_CHAMBER] = */                 RANDOM_ENTRANCES_EMPTY(),
    /* [ENTR_SCENE_GORMAN_TRACK] = */                   RANDOM_ENTRANCES_DEF(gGormanTrackEntrances),
    /* [ENTR_SCENE_GORON_RACETRACK] = */                RANDOM_ENTRANCES_DEF(gGoronRacetrackEntrances),
    /* [ENTR_SCENE_EAST_CLOCK_TOWN] = */                RANDOM_ENTRANCES_DEF(gEastClocktownEntrances),
    /* [ENTR_SCENE_WEST_CLOCK_TOWN] = */                RANDOM_ENTRANCES_DEF(gWestClocktownEntrances),
    /* [ENTR_SCENE_NORTH_CLOCK_TOWN] = */               RANDOM_ENTRANCES_DEF(gNorthClocktownEntrances),
    /* [ENTR_SCENE_SOUTH_CLOCK_TOWN] = */               RANDOM_ENTRANCES_DEF(gSouthClocktownEntrances),
    /* [ENTR_SCENE_LAUNDRY_POOL] = */                   RANDOM_ENTRANCES_DEF(gLaundryPoolEntrances),
};

const char *gSceneNames[] = {
    /* [ENTR_SCENE_MAYORS_RESIDENCE] = */               "Mayor's residence",
    /* [ENTR_SCENE_MAJORAS_LAIR] = */                   "",
    /* [ENTR_SCENE_MAGIC_HAGS_POTION_SHOP] = */         "Magic potion shop",
    /* [ENTR_SCENE_RANCH_HOUSE] = */                    "Ranch house",
    /* [ENTR_SCENE_HONEY_AND_DARLINGS_SHOP] = */        "Honey & Darling's shop",
    /* [ENTR_SCENE_BENEATH_THE_GRAVERYARD] = */         "Beneath the graveyard",
    /* [ENTR_SCENE_SOUTHERN_SWAMP_CLEARED] = */         "Southern swamp",
    /* [ENTR_SCENE_CURIOSITY_SHOP] = */                 "Curiosity shop",
    /* [ENTR_SCENE_UNSET_08] = */                       "",
    /* [ENTR_SCENE_UNSET_09] = */                       "",
    /* [ENTR_SCENE_GROTTOS] = */                        "",
    /* [ENTR_SCENE_UNSET_0B] = */                       "",
    /* [ENTR_SCENE_UNSET_0C] = */                       "",
    /* [ENTR_SCENE_UNSET_0D] = */                       "",
    /* [ENTR_SCENE_CUTSCENE] = */                       "",
    /* [ENTR_SCENE_UNSET_0F] = */                       "",
    /* [ENTR_SCENE_IKANA_CANYON] = */                   "Ikana canyon",
    /* [ENTR_SCENE_PIRATES_FORTRESS] = */               "Pirate fortress, outer perimeter",
    /* [ENTR_SCENE_MILK_BAR] = */                       "Milk bar",
    /* [ENTR_SCENE_STONE_TOWER_TEMPLE] = */             "Stone Tower temple",
    /* [ENTR_SCENE_TREASURE_CHEST_SHOP] = */            "Treasure Chest shop",
    /* [ENTR_SCENE_STONE_TOWER_TEMPLE_INVERTED] = */    "",
    /* [ENTR_SCENE_CLOCK_TOWER_ROOFTOP] = */            "",
    /* [ENTR_SCENE_OPENING_DUNGEON] = */                "", 
    /* [ENTR_SCENE_WOODFALL_TEMPLE] = */                "Woodfall temple",
    /* [ENTR_SCENE_PATH_TO_MOUNTAIN_VILLAGE] = */       "Path to mountain village",
    /* [ENTR_SCENE_IKANA_CASTLE] = */                   "Ikana castle",
    /* [ENTR_SCENE_DEKU_SCRUB_PLAYGROUND] = */          "Deky scrub playground",
    /* [ENTR_SCENE_ODOLWAS_LAIR] = */                   "Odolwa's lair",
    /* [ENTR_SCENE_TOWN_SHOOTING_GALLERY] = */          "Town shooting gallery",
    /* [ENTR_SCENE_SNOWHEAD_TEMPLE] = */                "Snowhead temple",
    /* [ENTR_SCENE_MILK_ROAD] = */                      "Milk road",
    /* [ENTR_SCENE_PIRATES_FORTRESS_INTERIOR] = */      "Pirate fortress, interior",
    /* [ENTR_SCENE_SWAMP_SHOOTING_GALLERY] = */         "Swamp shooting gallery",
    /* [ENTR_SCENE_PINNACLE_ROCK] = */                  "Pinnacle rock",
    /* [ENTR_SCENE_FAIRY_FOUNTAIN] = */                 "Fairy's fountain",
    /* [ENTR_SCENE_SWAMP_SPIDER_HOUSE] = */             "Swamp spider house",
    /* [ENTR_SCENE_OCEANSIDE_SPIDER_HOUSE] = */         "Oceanside spider house",
    /* [ENTR_SCENE_ASTRAL_OBSERVATORY] = */             "Astral observatory",
    /* [ENTR_SCENE_MOON_DEKU_TRIAL] = */                "",
    /* [ENTR_SCENE_DEKU_PALACE] = */                    "Deku palace",
    /* [ENTR_SCENE_MOUNTAIN_SMITHY] = */                "Mountain smithy",
    /* [ENTR_SCENE_TERMINA_FIELD] = */                  "Termina field",
    /* [ENTR_SCENE_POST_OFFICE] = */                    "Post office",
    /* [ENTR_SCENE_MARINE_RESEARCH_LAB] = */            "Marine research lab",
    /* [ENTR_SCENE_DAMPES_HOUSE] = */                   "Dampe's house",
    /* [ENTR_SCENE_UNSET_2E] = */                       "",
    /* [ENTR_SCENE_GORON_SHRINE] = */                   "Goron shrine",
    /* [ENTR_SCENE_ZORA_HALL] = */                      "Zora hall",
    /* [ENTR_SCENE_TRADING_POST] = */                   "Trading post",
    /* [ENTR_SCENE_ROMANI_RANCH] = */                   "Romani ranch",
    /* [ENTR_SCENE_TWINMOLDS_LAIR] = */                 "Twinmold's lair",
    /* [ENTR_SCENE_GREAT_BAY_COAST] = */                "Great Bay coast",
    /* [ENTR_SCENE_ZORA_CAPE] = */                      "Zora cape",
    /* [ENTR_SCENE_LOTTERY_SHOP] = */                   "Lottery shop",
    /* [ENTR_SCENE_UNSET_37] = */                       "",
    /* [ENTR_SCENE_PIRATES_FORTRESS_EXTERIOR] = */      "Pirate fortress, inner perimeter",
    /* [ENTR_SCENE_FISHERMANS_HUT] = */                 "Fisherman's hut",
    /* [ENTR_SCENE_GORON_SHOP] = */                     "Goron shop",
    /* [ENTR_SCENE_DEKU_KINGS_CHAMBER] = */             "Deku king's chamber",
    /* [ENTR_SCENE_MOON_GORON_TRIAL] = */               "",
    /* [ENTR_SCENE_ROAD_TO_SOUTHERN_SWAMP] = */         "Road to Southern Swamp",
    /* [ENTR_SCENE_DOGGY_RACETRACK] = */                "Doggy racetrack",
    /* [ENTR_SCENE_CUCCO_SHACK] = */                    "Cucco shack",
    /* [ENTR_SCENE_IKANA_GRAVEYARD] = */                "Ikana graveyard",
    /* [ENTR_SCENE_GOHTS_LAIR] = */                     "Goht's lair",
    /* [ENTR_SCENE_SOUTHERN_SWAMP_POISONED] = */        "Southern Swamp",
    /* [ENTR_SCENE_WOODFALL] = */                       "Woodfall",
    /* [ENTR_SCENE_MOON_ZORA_TRIAL] = */                "",
    /* [ENTR_SCENE_GORON_VILLAGE_SPRING] = */           "Goron village",
    /* [ENTR_SCENE_GREAT_BAY_TEMPLE] = */               "Great bay temple",
    /* [ENTR_SCENE_WATERFALL_RAPIDS] = */               "Waterfal rapids",
    /* [ENTR_SCENE_BENEATH_THE_WELL] = */               "Beneath the well",
    /* [ENTR_SCENE_ZORA_HALL_ROOMS] = */                "Zora hall rooms",
    /* [ENTR_SCENE_GORON_VILLAGE_WINTER] = */           "Goron village",
    /* [ENTR_SCENE_GORON_GRAVERYARD] = */               "Goron graveyard",
    /* [ENTR_SCENE_SAKONS_HIDEOUT] = */                 "Sakon's hideout",
    /* [ENTR_SCENE_MOUNTAIN_VILLAGE_WINTER] = */        "Mountain village",
    /* [ENTR_SCENE_GHOST_HUT] = */                      "Ghost hut",
    /* [ENTR_SCENE_DEKU_SHRINE] = */                    "Deku shrine",
    /* [ENTR_SCENE_ROAD_TO_IKANA] = */                  "Road to Ikana",
    /* [ENTR_SCENE_SWORDMANS_SCHOOL] = */               "Swordman's school",
    /* [ENTR_SCENE_MUSIC_BOX_HOUSE] = */                "Music box house",
    /* [ENTR_SCENE_IGOS_DU_IKANAS_LAIR] = */            "Igos du Ikana's lair",
    /* [ENTR_SCENE_TOURIST_INFORMATION] = */            "Tourist information",
    /* [ENTR_SCENE_STONE_TOWER] = */                    "Stone Tower",
    /* [ENTR_SCENE_STONE_TOWER_INVERTED] = */           "",
    /* [ENTR_SCENE_MOUNTAIN_VILLAGE_SPRING] = */        "Mountain village",
    /* [ENTR_SCENE_PATH_TO_SNOWHEAD] = */               "Path to Snowhead",
    /* [ENTR_SCENE_SNOWHEAD] = */                       "Snowhead",
    /* [ENTR_SCENE_PATH_TO_GORON_VILLAGE_WINTER] = */   "Path to Goron village",
    /* [ENTR_SCENE_PATH_TO_GORON_VILLAGE_SPRING] = */   "Path to Goron village",
    /* [ENTR_SCENE_GYORGS_LAIR] = */                    "Gyorg's lair",
    /* [ENTR_SCENE_SECRET_SHRINE] = */                  "Secren shrine",
    /* [ENTR_SCENE_STOCK_POT_INN] = */                  "Stock Pot inn",
    /* [ENTR_SCENE_GREAT_BAY_CUTSCENE] = */             "",
    /* [ENTR_SCENE_CLOCK_TOWER_INTERIOR] = */           "Clock Tower interior",
    /* [ENTR_SCENE_WOODS_OF_MYSTERY] = */               "Woods of Mystery",
    /* [ENTR_SCENE_LOST_WOODS] = */                     "",
    /* [ENTR_SCENE_MOON_LINK_TRIAL] = */                "",
    /* [ENTR_SCENE_THE_MOON] = */                       "",
    /* [ENTR_SCENE_BOMB_SHOP] = */                      "Bomb shop",
    /* [ENTR_SCENE_GIANTS_CHAMBER] = */                 "",
    /* [ENTR_SCENE_GORMAN_TRACK] = */                   "Gorman track",
    /* [ENTR_SCENE_GORON_RACETRACK] = */                "Goron racetrack",
    /* [ENTR_SCENE_EAST_CLOCK_TOWN] = */                "East Clock Town",
    /* [ENTR_SCENE_WEST_CLOCK_TOWN] = */                "West Clock Town",
    /* [ENTR_SCENE_NORTH_CLOCK_TOWN] = */               "North Clock Town",
    /* [ENTR_SCENE_SOUTH_CLOCK_TOWN] = */               "South Clock Town",
    /* [ENTR_SCENE_LAUNDRY_POOL] = */                   "Laundry pool",
};

u8 gOdolwaLairRooms[] = {0};
u8 gIgosDuIkanaLairRooms[] = {0};
u8 gPirateFortressInteriorRooms[] = {0, 1, 2};
u8 gStoneTowerTempleRooms[] = {10};
u8 gWoodfallTempleRooms[] = {8, 7};

struct
{
    u16     scene_index;
    u8      room_count;
    u8 *    room_indices;

} gNoLowGravRooms[] = {
    {ENTR_SCENE_ODOLWAS_LAIR,               ARRAY_COUNT(gOdolwaLairRooms),              gOdolwaLairRooms},
    {ENTR_SCENE_IGOS_DU_IKANAS_LAIR,        ARRAY_COUNT(gIgosDuIkanaLairRooms),         gIgosDuIkanaLairRooms},
    {ENTR_SCENE_PIRATES_FORTRESS_INTERIOR,  ARRAY_COUNT(gPirateFortressInteriorRooms),  gPirateFortressInteriorRooms},
    {ENTR_SCENE_STONE_TOWER_TEMPLE,         ARRAY_COUNT(gStoneTowerTempleRooms),        gStoneTowerTempleRooms},
    {ENTR_SCENE_WOODFALL_TEMPLE,            ARRAY_COUNT(gWoodfallTempleRooms),          gWoodfallTempleRooms},
};

/* xorshift* */
u32 Chaos_Rand(void)
{
    // u64 state = *rng_state;
    gChaosRngState ^= gChaosRngState >> 12;
    gChaosRngState ^= gChaosRngState << 25;
    gChaosRngState ^= gChaosRngState >> 27;
    return gChaosRngState * 0x2545F4914F6CDD1DULL;
}

f32 Chaos_ZeroOne(void)
{
    u32 rand_int = Chaos_Rand();
    union { u32 i; f32 f;} type_pun;
    type_pun.i = (rand_int >> 9) | 0x3F800000;
    return type_pun.f - 1.0f;
}

s16 Chaos_RandS16Offset(s16 base, s16 range)
{
    if(Chaos_GetConfigFlag(CHAOS_CONFIG_DETERMINISTIC_EFFECT_RNG))
    {
        return (s16)(Chaos_ZeroOne() * range) + base;
    }
    
    return Rand_S16Offset(base, range);
}

u32 Chaos_RandNext(void)
{
    if(Chaos_GetConfigFlag(CHAOS_CONFIG_DETERMINISTIC_EFFECT_RNG))
    {
        return Chaos_Rand();
    }
    
    return Rand_Next();
}

void Chaos_InitRng(void)
{
    if(!Chaos_GetConfigFlag(CHAOS_CONFIG_DETERMINISTIC_EFFECT_RNG))
    {
        gChaosRngState = Rand_Next();
    }
    else
    {
        gChaosRngState = 1;
        gChaosContext.chaos_timer = 15;
        gChaosContext.code_elapsed_usec = 0;
        gChaosContext.chaos_elapsed_usec = 0;
        gChaosContext.prev_update_counter = osGetTime();
    }
}

// static u16 gTestObjectIds[] = {
//     OBJECT_RR,
//     OBJECT_NIW,
//     OBJECT_ARWING,
//     OBJECT_WALLMASTER
// };

void Chaos_Init(void)
{
    u32 index;
    // u64 prev_end = 0;
    // f32 probability_scale = 0.0f;
    // gChaosRngState = Rand_Next();
    gChaosContext.moon.pitch = 0.0f;
    gChaosContext.moon.yaw = 0.0f;
    gChaosContext.active_code_count = 0;
    gChaosContext.enabled_code_count = 0;
    gChaosContext.chaos_timer = 15;
    gChaosContext.code_elapsed_usec = 0;
    gChaosContext.chaos_elapsed_usec = 0;
    gChaosContext.prev_update_counter = osGetTime();
    gChaosContext.update_enabled = 0;
    gChaosContext.interruption_probability_scale = 1.0f;
    gChaosContext.periodic_probability_scale = 1.0f;
    gChaosContext.periodic_probability_scale_target = 1.0f;
    gChaosContext.periodic_probability_update_timer = 0;
    // gChaosContext.cur_spawn_actor_code = CHAOS_CODE_NONE;
    // gChaosContext.spawn_actor_code_was_activated = CHAOS_CODE_NONE;
    // gChaosContext.update_spawn_actor_code = false;
    gChaosContext.queued_spawn_actor_code = CHAOS_CODE_NONE;
    gChaosContext.loaded_object_id = 0;
    gChaosContext.chaos_keep_size = 0;
    gChaosContext.need_update_distribution = false;
    gChaosContext.hide_actors = 0;
    gChaosContext.link.tunic_r = 30;
    gChaosContext.link.tunic_g = 105;
    gChaosContext.link.tunic_b = 27;
    gChaosContext.link.beer_alpha = 0;
    gChaosContext.link.syke = false; 
    gChaosContext.link.out_of_shape_speed_scale = 1.0f;
    gChaosContext.link.speed_boost_speed_scale = 1.0f;
    gChaosContext.link.imaginary_friends_speed_scale = 1.0f;
    gChaosContext.link.cur_animation = NULL;
    gChaosContext.link.cur_animation_frame = 0;
    gChaosContext.link.ear_scales[0].x = 1.0f;
    gChaosContext.link.ear_scales[0].z = 1.0f;
    gChaosContext.link.ear_scales[1].x = 1.0f;
    gChaosContext.link.ear_scales[1].z = 1.0f;
    gChaosContext.link.out_of_shape_state = CHAOS_OUT_OF_SHAPE_STATE_NONE;
    gChaosContext.link.beer_goggles_state = CHAOS_BEER_GOGGLES_STATE_NONE;
    gChaosContext.link.fierce_deity_state = CHAOS_RANDOM_FIERCE_DEITY_STATE_NONE;
    gChaosContext.link.imaginary_friends_state = CHAOS_IMAGINARY_FRIENDS_STATE_NONE;
    // gChaosContext.link.sneeze_state = CHAOS_SNEEZE_STATE_NONE;
    gChaosContext.link.liftoff_state = CHAOS_LIFTOFF_STATE_NONE;
    gChaosContext.link.liftoff_timer = 0;
    gChaosContext.link.fierce_deity_counter = 0;
    gChaosContext.link.imaginary_friends_anim_index = 0;

    gChaosContext.chicken.cucco.niwType = NIW_TYPE_CHAOS;
    gChaosContext.chicken.cucco.actor.update = EnNiw_Update;
    gChaosContext.chicken.cucco.actor.draw = EnNiw_Draw;

    gChaosContext.link.simon_says_state = CHAOS_SIMON_SAYS_STATE_IDLE;
    
    for(index = 0; index < CHAOS_CODE_LAST; index++)
    {
        gChaosContext.active_code_indices[index] = INVALID_CODE_INDEX;
        gChaosContext.enabled_code_indices[index] = INVALID_CODE_INDEX;
    }

    for(index = 0; index < ARRAY_COUNT(gSpawnActorCodeDefs); index++)
    {
        u32 object_id = gSpawnActorCodeDefs[index].object;
        size_t size = gObjectTable[object_id].vromEnd - gObjectTable[object_id].vromStart;
        if(size > gChaosContext.chaos_keep_size)
        {
            gChaosContext.chaos_keep_size = size;
            gChaosContext.chaos_keep_largest_object = object_id;
        }
    }

    for(index = 0; index < ARRAY_COUNT(gChaosContext.link.limb_scales); index++)
    {
        gChaosContext.link.limb_scales[index] = 1.0f;
    }
    gChaosContext.link.temp_limb_scale = 1.0f;
    

    gChaosContext.chaos_keep_size = (gChaosContext.chaos_keep_size + 0x3ff) & (~0x3ff);
    gChaosContext.room.vert_list_list[0] = NULL;
    gChaosContext.room.vert_list_list[1] = NULL;
    gChaosContext.room.weirdness_behavior = 0;

    for(index = 0; index < ARRAY_COUNT(gVertPosRandList); index++)
    {
        gVertPosRandList[index] = (s16)(Rand_Next() % 7) - 3;
        gTexCoordRandList[index] = Rand_S16Offset(-48, 96);
        gColorRandList[index] = Rand_S16Offset(-5, 10);
    }

    gChaosContext.time.fast_time_state = CHAOS_FAST_TIME_STATE_NONE;
    gChaosContext.env.fog_lerp = 0.0f;
    gChaosContext.env.length_contraction_scale = 1.0f;
}

u8 Chaos_RandomCode(void)
{
    u32 rand_index = Chaos_Rand();
    // s32 search_index = gChaosContext.enabled_code_count >> 1;
    // s32 search_index_offset = search_index >> 1;
    s32 search_left = 0;
    s32 search_right = gChaosContext.enabled_code_count - 1;


    while(search_left <= search_right)
    {
        s32 search_index = (search_right + search_left) >> 1;
        struct ChaosCodeSlot *slot = gChaosContext.enabled_codes + search_index;

        if(rand_index < slot->range_start)
        {
            search_right = search_index - 1;
        }
        else if(rand_index > slot->range_end)
        {
            search_left = search_index + 1;
        }
        else
        {
            return slot->code;
        }
    }

    // if(search_index_offset <= 0)
    // {
    //     search_index_offset = 1;
    // }

    // while(search_index >= 0 && search_index < gChaosContext.enabled_code_count)
    // {
    //     struct ChaosCodeSlot *slot = gChaosContext.enabled_codes + search_index;
    //     if(rand_index < slot->range_start)
    //     {
    //         search_index -= search_index_offset;
    //     }
    //     else if(rand_index > slot->range_end)
    //     {
    //         search_index += search_index_offset;
    //     }
    //     else if(rand_index >= slot->range_start && 
    //             rand_index <= slot->range_end)
    //     {
    //         return slot->code;
    //     }

    //     if(search_index_offset > 1)
    //     {
    //         search_index_offset >>= 1;
    //     }
    // }

    return CHAOS_CODE_NONE;
}
 
void Chaos_UpdateChaos(PlayState *playstate)
{
    OSTime              update_counter = osGetTime();
    u32                 code_elapsed_seconds;
    u32                 chaos_elapsed_seconds;
    u32                 slot_index;
    u32                 next_rand;
    u32                 code_add_result;
    u32                 elapsed_usec;
    Vec3f               camera_velocity0;
    Vec3f               camera_velocity1;
    u8                  next_code; 
    u8                  next_code_timer;
    struct ChaosCode *  last_code = NULL;
    Player *            player = GET_PLAYER(playstate);
    Camera *            camera = Play_GetCamera(playstate, CAM_ID_MAIN);

    if(!gRngInitialized)
    {
        Chaos_InitRng();
        gRngInitialized = true;
    }

    if(gChaosContext.need_update_distribution)
    {
        Chaos_UpdateCodeDistribution();
        gChaosContext.need_update_distribution = false;
    }

    Math_Vec3f_Diff(&camera->eye, &gChaosContext.view.prev_camera_pos, &camera_velocity0);
    gChaosContext.view.prev_camera_pos = camera->eye;

    Math_Vec3f_Lerp(&gChaosContext.view.prev_camera_velocity[0], &gChaosContext.view.prev_camera_velocity[1], 0.5f, &camera_velocity1);
    gChaosContext.view.prev_camera_velocity[1] = gChaosContext.view.prev_camera_velocity[0];
    gChaosContext.view.prev_camera_velocity[0] = gChaosContext.view.camera_velocity;
    Math_Vec3f_Lerp(&camera_velocity1, &camera_velocity0, 0.5f, &gChaosContext.view.camera_velocity);
    // gChaosContext.view.camera_velocity.x = (gChaosContext.view.camera_velocity.x + camera_velocity.x) * 0.5f;
    // gChaosContext.view.camera_velocity.y = (gChaosContext.view.camera_velocity.y + camera_velocity.y) * 0.5f;
    // gChaosContext.view.camera_velocity.z = (gChaosContext.view.camera_velocity.z + camera_velocity.z) * 0.5f;
    // gChaosContext.view.prev_camera_velocity = gChaosContext.view.camera_velocity;

    if(Chaos_CanUpdateChaos(playstate))
    {   
        Chaos_UpdateEnabledChaosEffectsAndEntrances(playstate);

        if(update_counter < gChaosContext.prev_update_counter)
        {
            /* PARANOID: cpu counter overflow */
            gChaosContext.prev_update_counter = update_counter;
        }
 
        elapsed_usec = (OS_CYCLES_TO_USEC(update_counter) - OS_CYCLES_TO_USEC(gChaosContext.prev_update_counter));
        gChaosContext.code_elapsed_usec += elapsed_usec;
        gChaosContext.prev_update_counter = update_counter;

        code_elapsed_seconds = gChaosContext.code_elapsed_usec / 1000000;

        if(Chaos_IsCodeActive(CHAOS_CODE_TIMER_UP))
        {
            elapsed_usec *= 2;
            gChaosContext.chaos_elapsed_usec += elapsed_usec;
            chaos_elapsed_seconds = gChaosContext.chaos_elapsed_usec / 1000000;
        }
        else 
        {
            gChaosContext.chaos_elapsed_usec = gChaosContext.code_elapsed_usec;
            chaos_elapsed_seconds = code_elapsed_seconds;
        }

        /* check effect spawned actors and kill those that are out of view for too long */
        slot_index = 0;
        while(slot_index < gChaosContext.actors.spawned_actors)
        {
            struct ChaosActor *chaos_actor = gChaosContext.actors.slots + slot_index;
            chaos_actor->actor->flags |= ACTOR_FLAG_CHAOS;
            if(!(chaos_actor->actor->flags & ACTOR_FLAG_INSIDE_CULLING_VOLUME))
            {
                if(code_elapsed_seconds >= chaos_actor->timer)
                {
                    Chaos_KillActorAtIndex(slot_index);
                    continue;
                }

                chaos_actor->timer -= code_elapsed_seconds;
            }
            else
            {
                chaos_actor->timer = ACTOR_DESPAWN_TIMER;
            }
            slot_index++;
        }

        /* advance effect timers */
        slot_index = 0;
        while(slot_index < gChaosContext.active_code_count)
        {
            struct ChaosCode *code = gChaosContext.active_codes + slot_index;
            u16 restrictions = gChaosCodeDefs[code->code].restrictions & CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_MASK;

            if(code_elapsed_seconds >= code->timer)
            {
                code->timer = 0;
                if(!(restrictions & gChaosContext.effect_restrictions))
                {
                    Chaos_DeactivateCodeAtIndex(slot_index);
                }
                else
                {
                    slot_index++;
                }
                continue;
            }

            code->timer -= code_elapsed_seconds;
            slot_index++;
        } 

        gChaosContext.code_elapsed_usec -= code_elapsed_seconds * 1000000;

        // if(Chaos_GetConfigFlag(CHAOS_CONFIG_CHAOS))
        // {
        // return;  
        // }

        if(chaos_elapsed_seconds > 0)
        {
            u32 restrictions = CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TIME_STOPPED |
                                CHAOS_CODE_RESTRICTION_FLAG_AFFECT_CUTSCENE |
                                CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TRANSITION;

            if(gChaosContext.effect_restrictions & restrictions)
            {
                Chaos_StepUpInterruptionProbabilityScale(chaos_elapsed_seconds);
            }

            gChaosContext.chaos_elapsed_usec -= chaos_elapsed_seconds * 1000000;

            if(chaos_elapsed_seconds > gChaosContext.chaos_timer)
            {
                gChaosContext.chaos_timer = 0;
            }
            else
            {
                gChaosContext.chaos_timer -= chaos_elapsed_seconds;
            }

            if(Chaos_GetConfigFlag(CHAOS_CONFIG_USE_PERIODIC_EFFECT_PROB))
            {
                if(chaos_elapsed_seconds > gChaosContext.periodic_probability_update_timer)
                {
                    gChaosContext.periodic_probability_update_timer = 0;
                }
                else
                {
                    gChaosContext.periodic_probability_update_timer -= chaos_elapsed_seconds;
                }

                Chaos_UpdatePeriodicProbabilityScale();
            }
            else
            {
                gChaosContext.periodic_probability_scale = 1.0f;
            }
        
            if(gChaosContext.chaos_timer == 0)  
            {
                u32 attempts = 0;
                struct ChaosCodeDef *code_def;
                code_add_result = CHAOS_ADD_RESULT_NO_SLOTS;
                // gChaosContext.chaos_timer = MIN_CHAOS_TIMER + Rand_Next() % (MAX_CHAOS_TIMER - MIN_CHAOS_TIMER);
                // next_rand = Rand_Next();
                gChaosContext.chaos_timer = MIN_CHAOS_TIMER + Chaos_RandNext() % (MAX_CHAOS_TIMER - MIN_CHAOS_TIMER);
                next_rand = Chaos_RandNext();
                do
                {
                    u16 effect_restrictions;
                    u8 time_delta;
                    if(gChaosContext.queued_spawn_actor_code != CHAOS_CODE_NONE &&
                        Object_IsLoaded(&playstate->objectCtx, gChaosContext.chaos_keep_slot))
                    {
                        /* there's an actor spawn effect "queued" and the object
                        is already done loading, so activate it */
                        next_code = gChaosContext.queued_spawn_actor_code;
                        gChaosContext.queued_spawn_actor_code = CHAOS_CODE_NONE;
                    }
                    else
                    {
                        next_code = Chaos_RandomCode();
                    }

                    time_delta = gChaosCodeDefs[next_code].max_time - gChaosCodeDefs[next_code].min_time;

                    if(time_delta > 0)
                    {
                        next_code_timer = gChaosCodeDefs[next_code].min_time + next_rand % time_delta;
                    }
                    else
                    {
                        next_code_timer = 0;
                    }

                    effect_restrictions = gChaosCodeDefs[next_code].restrictions & CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_MASK;
                    attempts++;

                    if(effect_restrictions & gChaosContext.effect_restrictions)
                    {
                        continue;
                    }

                    /* TODO: create a effect exclusion list for each effect, to avoid a bunch of branching here */
                    switch(next_code)
                    {
                        case CHAOS_CODE_TERRIBLE_MUSIC:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_BEER_GOGGLES))
                            {
                                /* beer goggles also screws with sound tempo/frequency */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_BEER_GOGGLES:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_TERRIBLE_MUSIC))
                            {
                                /* terrible music also screws with sound tempo/frequency */
                                continue;
                            }
                        break;

                        // case CHAOS_CODE_OUT_OF_SHAPE:
                        //     if(Chaos_IsCodeInActiveList(CHAOS_CODE_IMAGINARY_FRIENDS))
                        //     {
                        //         continue;
                        //     }

                        //     if(Chaos_IsCodeInActiveList(CHAOS_CODE_SILENT_FIELD) && 
                        //         !Chaos_GetConfigFlag(CHAOS_CONFIG_ALLOW_BEER_GOGGLES_AND_SILENT_FIELD))
                        //     {
                        //         continue;
                        //     }
                        // break;

                        // case CHAOS_CODE_SILENT_FIELD:
                        //     if(Chaos_IsCodeInActiveList(CHAOS_CODE_SILENT_FIELD) && 
                        //         !Chaos_GetConfigFlag(CHAOS_CONFIG_ALLOW_BEER_GOGGLES_AND_SILENT_FIELD))
                        //     {
                        //         continue;
                        //     }
                        // break;

                        // case CHAOS_CODE_IMAGINARY_FRIENDS:
                        //     if(Chaos_IsCodeInActiveList(CHAOS_CODE_OUT_OF_SHAPE))
                        //     {
                        //         continue;
                        //     }
                        // break;

                        case CHAOS_CODE_RANDOM_HEALTH_UP:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_HEART_SNAKE))
                            {
                                continue;
                            }

                            if(gSaveContext.save.saveInfo.playerData.healthCapacity >= 
                                LIFEMETER_FULL_HEART_HEALTH * CHAOS_MAX_HEART_CONTAINERS)
                            {
                                continue;
                            }
                        break;

                        case CHAOS_CODE_RANDOM_HEALTH_DOWN:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_HEART_SNAKE))
                            {
                                continue;
                            }

                            if(gSaveContext.save.saveInfo.playerData.healthCapacity == LIFEMETER_FULL_HEART_HEALTH)
                            {
                                continue;
                            }
                        break;

                        // case CHAOS_CODE_LOVELESS_MARRIAGE:
                        case CHAOS_CODE_CHICKEN_ARISE:
                        // case CHAOS_CODE_STARFOX:
                        case CHAOS_CODE_WALLMASTER:
                        case CHAOS_CODE_REDEADASS_GROOVE:
                        {
                            u32 index;
                            
                            for(index = 0; index < ARRAY_COUNT(gSpawnActorCodeDefs); index++)
                            {
                                struct ChaosSpawnActorCodeDef *code_def = gSpawnActorCodeDefs + index;

                                if(next_code == code_def->code && gChaosContext.loaded_object_id != code_def->object)
                                {
                                    /* object for next code is not loaded */

                                    next_code = CHAOS_CODE_NONE;

                                    if(Chaos_IsCodeInActiveList(CHAOS_CODE_CHICKEN_ARISE) || gChaosContext.actors.spawned_actors > 0)
                                    {
                                        /* there's still another code using the currently loaded object */
                                        break;
                                    }

                                    Object_RequestOverwrite(&playstate->objectCtx, gChaosContext.chaos_keep_slot, code_def->object);
                                    gChaosContext.loaded_object_id = code_def->object;
                                    gChaosContext.queued_spawn_actor_code = code_def->code;
                                    break;
                                }
                            }

                            if(next_code == CHAOS_CODE_NONE)
                            {
                                continue;
                            }
                        }
                        break;

                        case CHAOS_CODE_BUCKSHOT_ARROWS:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_BOMB_ARROWS))
                            {
                                /* bomb arrows and buckshot arrows don't mix very well */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_BOMB_ARROWS:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_BUCKSHOT_ARROWS))
                            {
                                /* bomb arrows and buckshot arrows don't mix very well */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_ONE_HIT_KO:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_CHANGE_HEALTH) || 
                               Chaos_IsCodeInActiveList(CHAOS_CODE_INVINCIBLE))
                            {
                                /* changing health would one-hit the player or not have any 
                                effect at all, so don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_CHANGE_HEALTH:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_ONE_HIT_KO) || 
                               Chaos_IsCodeInActiveList(CHAOS_CODE_INVINCIBLE) || 
                               Chaos_IsCodeInActiveList(CHAOS_CODE_SWAP_HEAL_AND_HURT) ||
                               gChaosContext.link.simon_says_state == CHAOS_SIMON_SAYS_STATE_WAIT_DEATH)
                            {
                                /* changing health would one-hit the player, not have any 
                                effect at all or potentially cancel a simon says death, so don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_INVINCIBLE:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_ONE_HIT_KO) || 
                               Chaos_IsCodeInActiveList(CHAOS_CODE_CHANGE_HEALTH) ||
                               gChaosContext.link.simon_says_state == CHAOS_SIMON_SAYS_STATE_WAIT_DEATH)
                            {
                                /* making the player invicible now would make both codes 
                                not have an effect, so don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_SWAP_HEAL_AND_HURT:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_CHANGE_HEALTH))
                            {
                                /* could potentially one-hit the player, so
                                don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_BIG_BROTHER:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_MOON_DANCE))
                            {
                                /* the moon won't be able to face the player if 
                                it starts dancing, so do it later */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_MOON_DANCE:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_BIG_BROTHER))
                            {
                                /* the moon won't be able to dance if it's
                                trying to face the player, so do it later */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_WEIRD_UI:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_HEART_SNAKE))
                            {
                                continue;
                            }
                        break;

                        case CHAOS_CODE_HEART_SNAKE:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_WEIRD_UI) || gChaosContext.link.simon_says_state != CHAOS_SIMON_SAYS_STATE_IDLE ||
                                gSaveContext.save.saveInfo.playerData.healthCapacity <= LIFEMETER_FULL_HEART_HEALTH * 3)
                            {
                                continue;
                            }
                        break;

                        case CHAOS_CODE_SIMON_SAYS:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_HEART_SNAKE) || Chaos_IsCodeInActiveList(CHAOS_CODE_CHANGE_HEALTH) || 
                                gChaosContext.link.simon_says_state != CHAOS_SIMON_SAYS_STATE_IDLE)
                            {
                                continue;
                            }
                        break;

                        case CHAOS_CODE_JUNK_ITEM:
                            if(gChaosContext.link.fierce_deity_state == CHAOS_RANDOM_FIERCE_DEITY_STATE_WAIT_FOR_FORM ||
                               gChaosContext.link.fierce_deity_state == CHAOS_RANDOM_FIERCE_DEITY_STATE_SWITCH)
                            {
                                continue;
                            }
                        break;

                        case CHAOS_CODE_MOON_CRASH:
                            if(Environment_IsFinalHours(playstate))
                            {
                                /* either the effect is still active or we're
                                in the real final hours */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_SLOW_DOWN:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_SPEEDBOOST))
                            {
                                continue;
                            }
                        break;

                        case CHAOS_CODE_SPEEDBOOST:
                            if(Chaos_IsCodeInActiveList(CHAOS_CODE_SLOW_DOWN))
                            {
                                continue;
                            }
                        break;
                    }

                    code_add_result = Chaos_ActivateCode(next_code, next_code_timer);
                }
                while(code_add_result == CHAOS_ADD_RESULT_ALREADY_ACTIVE || attempts >= 50);

                if(code_add_result == CHAOS_ADD_RESULT_OK)
                {
                    last_code = gChaosContext.active_codes + (gChaosContext.active_code_count - 1);

                    switch(last_code->code)
                    {
                        case CHAOS_CODE_MOON_DANCE:
                            do 
                            {
                                /* pick a random combination of moon dance move flags */
                                gChaosContext.moon.moon_dance = Chaos_RandNext() % CHAOS_MOON_MOVE_LAST;
                            }
                            while(gChaosContext.moon.moon_dance == 0);
                        break;

                        case CHAOS_CODE_HEART_SNAKE:
                            gChaosContext.ui.snake_state = CHAOS_SNAKE_GAME_STATE_INIT;
                        break;

                        case CHAOS_CODE_SIMON_SAYS:
                            gChaosContext.link.simon_says_state = CHAOS_SIMON_SAYS_STATE_START;
                        break;

                        case CHAOS_CODE_UNSTABLE_ROOMS:
                            do
                            {
                                gChaosContext.room.weirdness_behavior = Chaos_RandNext() % CHAOS_UNSTABLE_ROOMS_BEHAVIOR_LAST;
                            }
                            while(gChaosContext.room.weirdness_behavior == 0);
                            Chaos_StepDownInterruptionProbabilityScale();
                            gChaosContext.room.room_rotation_timer = 0;
                        break;

                        case CHAOS_CODE_RANDOM_KNOCKBACK:
                            /* deal knockback immediately */
                            gChaosContext.link.random_knockback_timer = 1;
                            Chaos_StepDownInterruptionProbabilityScale();
                        break;

                        case CHAOS_CODE_RANDOM_SCALING:
                            gChaosContext.link.temp_limb_scale = gChaosContext.link.limb_scales[0];
                            gChaosContext.link.scaled_limb_index = 0;
                            gChaosContext.link.random_scaling_mode = Chaos_RandNext() % CHAOS_RANDOM_SCALING_MODE_LAST;
                        break;

                        case CHAOS_CODE_TRAP_FLAP:
                            /* start yapping immediately */
                            gChaosContext.link.trap_flap_timer = 1;
                        break;

                        case CHAOS_CODE_TERRIBLE_MUSIC:
                            /* start screwing up the bgm immediately */
                            gChaosContext.bgm.change_timer = 1;
                        break;

                        case CHAOS_CODE_BIG_BROTHER:
                            gChaosContext.moon.big_brother_state = CHAOS_BIG_BROTHER_STATE_TRACKING;
                            gChaosContext.moon.eye_glow = 0.0f;
                        break;

                        // case CHAOS_CODE_TUNIC_COLOR:
                        // {
                        //     u32 color = Rand_Next() % 0xffffff;
                        //     gChaosContext.link.tunic_r = color;
                        //     gChaosContext.link.tunic_g = color >> 8;
                        //     gChaosContext.link.tunic_b = color >> 16;
                        // }
                        // break;

                        case CHAOS_CODE_MOON_CRASH:
                        {
                            Chaos_StartMoonCrash();

                            if(gSaveContext.save.chaos.moon_crash_count < 3)
                            {
                                gSaveContext.save.chaos.moon_crash_count++;
                            }

                            Chaos_StepDownInterruptionProbabilityScale();
                        }
                        break;

                        case CHAOS_CODE_LOVELESS_MARRIAGE:
                        case CHAOS_CODE_WALLMASTER:
                        case CHAOS_CODE_STARFOX:
                        case CHAOS_CODE_POKE:
                        case CHAOS_CODE_IMAGINARY_FRIENDS:
                        case CHAOS_CODE_OUT_OF_SHAPE:
                        case CHAOS_CODE_JUNK_ITEM:
                        case CHAOS_CODE_ICE_TRAP:
                        case CHAOS_CODE_SYKE:
                        case CHAOS_CODE_DIE:
                        case CHAOS_CODE_PLAY_OCARINA:
                        case CHAOS_CODE_SHOCK:
                        case CHAOS_CODE_REDEADASS_GROOVE:
                        case CHAOS_CODE_LIFTOFF:
                            Chaos_StepDownInterruptionProbabilityScale();
                        break;

                        case CHAOS_CODE_CHICKEN_ARISE:
                            gChaosContext.chicken.cucco.attackNiwSpawnTimer = 0;
                            gChaosContext.chicken.cucco.attackNiwCount = 0;
                            Chaos_StepDownInterruptionProbabilityScale();
                        break;

                        case CHAOS_CODE_BAD_CONNECTION:
                            gChaosContext.link.bad_connection_mode = Chaos_RandNext() % CHAOS_BAD_CONNECTION_LAST;
                            gChaosContext.link.snapshot_timer = 1;
                            gChaosContext.link.bad_connection_timer = 1;
                            Chaos_NukeSnapshots();
                            Chaos_StepDownInterruptionProbabilityScale();
                        break;
                        
                        case CHAOS_CODE_SILENT_FIELD:
                            gChaosContext.env.spawn_stalchilds = next_code_timer < 30;
                            gChaosContext.env.stalchild_count = 0;
                            gChaosContext.env.stalchild_spawn_timer = 1;
                        break;

                        case CHAOS_CODE_LENGTH_CONTRACTION:
                        {
                            s16 random_yaw = Chaos_RandNext() % 0xffff;
                            gChaosContext.env.length_contraction_axis.x = Math_SinS(random_yaw);
                            gChaosContext.env.length_contraction_axis.y = 0.0f;
                            gChaosContext.env.length_contraction_axis.z = Math_CosS(random_yaw);
                            gChaosContext.env.length_contraction_scale = 1.0f;
                        }
                        break;
                    }
                }
            }
        }
    }
}

#define ENABLED_EFFECTS_PER_PAGE    24
#define ENABLED_ENTRANCES_PER_PAGE  25

extern PlayerAnimationHeader *gImaginaryFriendAnimations[];

void Chaos_PrintCodes(PlayState *playstate, Input *input)
{
    Gfx* gfx;
    Gfx* polyOpa;
    Player *player = GET_PLAYER(playstate);
    Camera *camera = Play_GetCamera(playstate, CAM_ID_MAIN);
    GfxPrint gfx_print;
    u32 slot_index;
    u32 y_pos = 1;
    u32 enabled_effects_page_count = gChaosContext.enabled_code_count / ENABLED_EFFECTS_PER_PAGE;
    u32 enabled_entrance_page_count = gChaosContext.entrance.enabled_scene_count / ENABLED_ENTRANCES_PER_PAGE;
    u32 background_line_count = 2;
    u32 page_count;

    if(gChaosContext.enabled_code_count % ENABLED_EFFECTS_PER_PAGE)
    {
        enabled_effects_page_count++;
    }

    if(gChaosContext.entrance.enabled_scene_count % ENABLED_ENTRANCES_PER_PAGE)
    {
        enabled_entrance_page_count++;
    }

    page_count = enabled_effects_page_count + enabled_entrance_page_count + 1;

    if(CHECK_BTN_ANY(input->cur.button, BTN_DUP))
    {
        if(gDisplayEffectInfo == 0)
        {
            gChaosEffectPageIndex = 0;
        }

        gDisplayEffectInfo = 1;

        OPEN_DISPS(playstate->state.gfxCtx);

        gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, 80);
        gDPFillRectangle(OVERLAY_DISP++, 0, 0, SCREEN_WIDTH, 5 + ((2 + gChaosContext.active_code_count) << 3));
        gDPPipeSync(OVERLAY_DISP++);

        polyOpa = POLY_OPA_DISP;
        gfx = Gfx_Open(polyOpa);
        gSPDisplayList(OVERLAY_DISP++, gfx);

        GfxPrint_Init(&gfx_print);
        GfxPrint_Open(&gfx_print, gfx);
        GfxPrint_SetColor(&gfx_print, 255, 255, 255, 255);
        GfxPrint_SetPos(&gfx_print, 1, y_pos++);

        if(!CHECK_BTN_ANY(input->press.button, BTN_R | BTN_L))
        {
            gAcceptPageChange = 1;
        }
        

        if(gAcceptPageChange)
        {
            if(CHECK_BTN_ANY(input->press.button, BTN_L))
            {
                gChaosEffectPageIndex = (gChaosEffectPageIndex - 1) % -(page_count + 1);
                gAcceptPageChange = 0;
            }

            if(CHECK_BTN_ANY(input->press.button, BTN_R))
            {
                gChaosEffectPageIndex = (gChaosEffectPageIndex + 1) % (page_count + 1);
                gAcceptPageChange = 0;
            }
        }

        // gChaosEffectPageIndex = 0xffffffff;

        if(gChaosEffectPageIndex == 0)
        {
            GfxPrint_Printf(&gfx_print, "Chaos timer: %d", (u32)gChaosContext.chaos_timer);

            slot_index = 0;
            while(slot_index < gChaosContext.active_code_count)
            {
                struct ChaosCode *code = gChaosContext.active_codes + slot_index;
                GfxPrint_SetPos(&gfx_print, 1, y_pos++);
                GfxPrint_Printf(&gfx_print, "%s: %d", gChaosCodeNames[code->code], (u32)code->timer);
                slot_index++;
            }
        }
        else if(gChaosEffectPageIndex <= enabled_effects_page_count)
        {
            struct ChaosCodeSlot *first_slot;
            u32 display_effect_count;
            u32 first_effect_index = (gChaosEffectPageIndex - 1) * ENABLED_EFFECTS_PER_PAGE;
            first_effect_index = CLAMP_MAX(first_effect_index, gChaosContext.enabled_code_count);
            first_slot = gChaosContext.enabled_codes + first_effect_index;
            display_effect_count = gChaosContext.enabled_code_count - first_effect_index;
            display_effect_count = CLAMP_MAX(display_effect_count, ENABLED_EFFECTS_PER_PAGE);
            
            GfxPrint_Printf(&gfx_print, "Enabled effects: (%d/%d)", gChaosEffectPageIndex, enabled_effects_page_count);            
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "Interrupt prob scale: %f", gChaosContext.interruption_probability_scale);            
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "Periodic prob scale: %f (%d)", gChaosContext.periodic_probability_scale, gChaosContext.periodic_probability_update_timer);            
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            slot_index = 0;
            while(slot_index < display_effect_count)
            {
                struct ChaosCodeSlot *slot = first_slot + slot_index;
                f32 probability = ((f32)(slot->range_end - slot->range_start) / (f32)0xffffffff) * 100.0f;
                GfxPrint_SetColor(&gfx_print, 255, 255, Chaos_IsCodeActive(slot->code) ? 0 : 255, 255);
                GfxPrint_SetPos(&gfx_print, 1, y_pos);
                GfxPrint_Printf(&gfx_print, "%s", gChaosCodeNames[slot->code]);
                GfxPrint_SetPos(&gfx_print, 22, y_pos++);
                GfxPrint_Printf(&gfx_print, "(%.4f%%)", probability);
                slot_index++;
            }
        }
        else if(gChaosEffectPageIndex <= enabled_effects_page_count + enabled_entrance_page_count)
        {
            u32 display_entrance_count;
            u32 first_entrance_index = (gChaosEffectPageIndex - (enabled_effects_page_count + 1)) * ENABLED_ENTRANCES_PER_PAGE;
            first_entrance_index = CLAMP_MAX(first_entrance_index, gChaosContext.entrance.enabled_scene_count);
            // first_slot = gChaosContext.enabled_codes + first_effect_index;
            display_entrance_count = gChaosContext.entrance.enabled_scene_count - first_entrance_index;
            display_entrance_count = CLAMP_MAX(display_entrance_count, ENABLED_ENTRANCES_PER_PAGE);

            GfxPrint_Printf(&gfx_print, "Entrance rando scenes: (%d/%d)", 
                gChaosEffectPageIndex - enabled_effects_page_count, enabled_entrance_page_count);
            slot_index = 0;
            while(slot_index < display_entrance_count)
            {
                u8 scene_index = gChaosContext.entrance.enabled_scenes[first_entrance_index + slot_index];
                GfxPrint_SetPos(&gfx_print, 1, y_pos++);
                GfxPrint_Printf(&gfx_print, "%s", gSceneNames[scene_index]);
                slot_index++;
            }
        }
        else
        {
            u32 scene = gSaveContext.save.entrance >> 9;
            u32 entrance = (gSaveContext.save.entrance >> 4) & 0x1f;
            u32 layer = gSaveContext.save.entrance & 0xf;
            u32 index = 0;
            size_t largest_free_block;
            size_t total_free_size;
            size_t total_alloc_size;
            ZeldaArena_GetSizes(&largest_free_block, &total_free_size, &total_alloc_size);
 
            GfxPrint_Printf(&gfx_print, "Player state stuff");
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "stateFlags1: %08x", player->stateFlags1);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "stateFlags2: %08x", player->stateFlags2);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "stateFlags3: %08x", player->stateFlags3);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "player csId: %d player csAction: %d", player->csId, player->csAction);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "cutscene id: %d", CutsceneManager_GetCurrentCsId());
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "actionVar1: %d", player->av1.actionVar1);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "actionVar2: %d", player->av2.actionVar2);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            // GfxPrint_Printf(&gfx_print, "Action: %d, Upper action: %d", gPlayerAction, gPlayerUpperAction);
            GfxPrint_Printf(&gfx_print, "Upper action: %d, Action: ", gPlayerUpperAction);
            GfxPrint_Printf(&gfx_print, "%d", gPlayerActionChanges[0]);
            if(gPlayerActionChangeCount > 1)
            {
                for(index = 1; index < gPlayerActionChangeCount; index++)
                {
                    GfxPrint_Printf(&gfx_print, " -> %d", gPlayerActionChanges[index]);
                }
            }

            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "First person mode: %d", player->unk_AA5);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "Magic state: %d", gSaveContext.magicState);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "Scene: %d, entrance: %d", scene, entrance);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "layer: %d, room: %d", layer, playstate->roomCtx.curRoom.num);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "effect restrictions: %x", gChaosContext.effect_restrictions);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "game mode: %x", gSaveContext.gameMode);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "pause state: %x", playstate->pauseCtx.state);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "actor count: %x", gChaosContext.actors.spawned_actors);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "current boot: %x", player->currentBoots);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            // GfxPrint_Printf(&gfx_print, "wall poly: %d", (u32)(player->actor.wallPoly - playstate->colCtx.colHeader->polyList));
            // GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            // GfxPrint_Printf(&gfx_print, "camera setting: %x, camera mode: %x", camera->setting, camera->mode);
            // GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            // GfxPrint_Printf(&gfx_print, "bgm id: %d", AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) & 0xff);
            // GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            // GfxPrint_Printf(&gfx_print, "Chaos object slot size: %d bytes", gChaosContext.chaos_keep_size);
            // GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "Allocated: %d, Free: %d", (u32)total_alloc_size, (u32)total_free_size);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "%d", playstate->bButtonAmmoPlusOne);
            
        }

        gfx = GfxPrint_Close(&gfx_print);
        GfxPrint_Destroy(&gfx_print);
        gSPEndDisplayList(gfx++);
        Gfx_Close(polyOpa, gfx);
        POLY_OPA_DISP = gfx;
        CLOSE_DISPS(gfxCtx);
    }
    else
    {
        gDisplayEffectInfo = 0;
    }

    gPlayerActionChangeCount = 0;
}

void Chaos_AppendActionChange(PlayState *play, u32 action)
{
    if(gPlayerActionChangeCount < ARRAY_COUNT(gPlayerActionChanges))
    {
        gPlayerActionChanges[gPlayerActionChangeCount] = action;
        gPlayerActionChangeCount++;
    }
}

u8 Chaos_ActivateCode(u8 code, u8 seconds)
{
    if(gChaosContext.active_code_indices[code] != INVALID_CODE_INDEX)
    {
        return CHAOS_ADD_RESULT_ALREADY_ACTIVE;
    }

    if(gChaosContext.active_code_count < MAX_ACTIVE_CODES)
    {
        struct ChaosCode *slot = gChaosContext.active_codes + gChaosContext.active_code_count;
        slot->code = code;
        slot->timer = seconds;
        gChaosContext.active_code_indices[code] = gChaosContext.active_code_count;
        gChaosContext.active_code_count++;
        return CHAOS_ADD_RESULT_OK;
    }

    return CHAOS_ADD_RESULT_NO_SLOTS;
}

void Chaos_DeactivateCodeAtIndex(u8 index)
{
    u8 code;

    if(index < gChaosContext.active_code_count)
    {
        code = gChaosContext.active_codes[index].code;
        gChaosContext.active_code_indices[code] = INVALID_CODE_INDEX;
        gChaosContext.active_code_count--;

        switch(code)
        {
            case CHAOS_CODE_WEIRD_UI:
                bzero(gChaosContext.ui.heart_containers, sizeof(gChaosContext.ui.heart_containers));
            break;

            case CHAOS_CODE_RANDOM_SCALING:
                gChaosContext.link.limb_scales[gChaosContext.link.scaled_limb_index] = gChaosContext.link.temp_limb_scale;
            break;

            case CHAOS_CODE_HEART_SNAKE:
                bzero(gChaosContext.ui.heart_containers, sizeof(gChaosContext.ui.heart_containers));
                if(gChaosContext.ui.snake_state != CHAOS_SNAKE_GAME_STATE_DIED || 
                    gChaosContext.ui.snake_state != CHAOS_SNAKE_GAME_STATE_WIN)
                {
                    gSaveContext.save.saveInfo.playerData.healthCapacity = LIFEMETER_FULL_HEART_HEALTH * gChaosContext.ui.heart_count;

                    if(gSaveContext.save.saveInfo.playerData.health > gSaveContext.save.saveInfo.playerData.healthCapacity)
                    {
                        gSaveContext.save.saveInfo.playerData.health = gSaveContext.save.saveInfo.playerData.healthCapacity;
                    }
                }
            break;
            
            case CHAOS_CODE_BAD_CONNECTION:
                Chaos_NukeSnapshots();
            break;
        }

        if(index < gChaosContext.active_code_count)
        {
            gChaosContext.active_codes[index] = gChaosContext.active_codes[gChaosContext.active_code_count];
            code = gChaosContext.active_codes[index].code;
            gChaosContext.active_code_indices[code] = index;
        }
    }
}

void Chaos_DeactivateCode(u8 code)
{
    Chaos_DeactivateCodeAtIndex(gChaosContext.active_code_indices[code]);
}

void Chaos_DeactivateOneCode()
{
    if(gChaosContext.active_code_count > 0)
    {
        Chaos_DeactivateCodeAtIndex(0);
    }
}

u8 Chaos_IsCodeInActiveList(u8 code)
{
    u32 slot_index = gChaosContext.active_code_indices[code];
    return slot_index < gChaosContext.active_code_count;
}

u8 Chaos_IsCodeActive(u8 code)
{
    u16 restrictions = gChaosCodeDefs[code].restrictions & CHAOS_CODE_RESTRICTION_FLAG_AFFECT_MASK;
    return Chaos_IsCodeInActiveList(code) && ((restrictions & gChaosContext.effect_restrictions) == 0);
}

struct ChaosCode *Chaos_GetCode(u8 code)
{
    u32 slot_index = gChaosContext.active_code_indices[code];
    u16 restrictions = gChaosCodeDefs[code].restrictions & CHAOS_CODE_RESTRICTION_FLAG_AFFECT_MASK;
    if(slot_index < gChaosContext.active_code_count && (restrictions & gChaosContext.effect_restrictions) == 0)
    {
        return gChaosContext.active_codes + slot_index;
    }

    return NULL;
}

void Chaos_EnableCode(u8 code, f32 prob_scale)
{
    u32 same_as_previous_index = false;
    u16 effect_restrictions = gChaosCodeDefs[code].restrictions & CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_MASK;
    if(!Chaos_IsCodeEnabled(code) && !(effect_restrictions & gChaosContext.effect_restrictions))
    {
        u32 index = gChaosContext.enabled_code_count;
        gChaosContext.enabled_code_count++;

        gChaosContext.need_update_distribution |= gChaosContext.enabled_codes[index].code != code;
        gChaosContext.enabled_codes[index].code = code;
        gChaosContext.enabled_codes[index].prob_scale = prob_scale;
        gChaosContext.enabled_code_indices[code] = index;
    }
}

void Chaos_DisableCode(u8 code)
{
    if(Chaos_IsCodeEnabled(code))
    {
        u32 index = gChaosContext.enabled_code_indices[code];
        gChaosContext.enabled_code_count--;
        gChaosContext.enabled_code_indices[code] = INVALID_CODE_INDEX;

        if(index < gChaosContext.enabled_code_count)
        {
            gChaosContext.enabled_codes[index] = gChaosContext.enabled_codes[gChaosContext.enabled_code_count];
            code = gChaosContext.enabled_codes[index].code;
            gChaosContext.enabled_code_indices[code] = index;
        }

        gChaosContext.need_update_distribution = true;
    }
}

u8 Chaos_IsCodeEnabled(u8 code)
{
    return gChaosContext.enabled_code_indices[code] < INVALID_CODE_INDEX;
}

void Chaos_ClearEnabledCodes(void)
{
    u32 index;

    for(index = 0; index < CHAOS_CODE_LAST; index++)
    {
        gChaosContext.enabled_code_indices[index] = INVALID_CODE_INDEX;
    }

    gChaosContext.enabled_code_count = 0;
}

void Chaos_UpdateCodeDistribution(void)
{
    u32 index;
    f32 probability_scale = 0.0f;
    u64 prev_end = 0;
    u64 range_fract = 0x00000000ffffffff;

    if(gChaosContext.enabled_code_count > 0)
    {
        for(index = 0; index < gChaosContext.enabled_code_count; index++)
        {
            probability_scale += gChaosCodeDefs[gChaosContext.enabled_codes[index].code].probability * 
                gChaosContext.enabled_codes[index].prob_scale;
        }

        probability_scale = 1.0f / probability_scale;
        range_fract *= probability_scale;

        for(index = 0; index < gChaosContext.enabled_code_count; index++)
        {
            struct ChaosCodeSlot *code_slot = gChaosContext.enabled_codes + index;
            struct ChaosCodeDef *code_def = gChaosCodeDefs + code_slot->code;
            // u64 next_range_end = prev_end + 0x00000000ffffffff * code_def->probability * probability_scale;
            u64 next_range_end = prev_end + range_fract * code_def->probability * code_slot->prob_scale;
            if(next_range_end > 0xffffffff)
            {
                next_range_end = 0xffffffff;
            }

            code_slot->range_start = prev_end;
            code_slot->range_end = (u32)next_range_end;
            prev_end = next_range_end;
        }

        gChaosContext.enabled_codes[gChaosContext.enabled_code_count - 1].range_end = 0xffffffff;
    }
}

u8 Chaos_CanUpdateChaos(struct PlayState *play)
{
    Player *player = GET_PLAYER(play);
    // u8 enable_update = gSaveContext.gameMode == GAMEMODE_NORMAL &&
    //                    CutsceneManager_GetCurrentCsId() == CS_ID_NONE &&
    //                    play->transitionMode == TRANS_MODE_OFF && 
    //                    play->pauseCtx.state == PAUSE_STATE_OFF &&
    //                    !(player->stateFlags1 & PLAYER_STATE1_80);

    u8 enable_update = (gSaveContext.gameMode == GAMEMODE_NORMAL ||
                        gSaveContext.gameMode == GAMEMODE_TITLE_SCREEN) &&
                        play->pauseCtx.state == PAUSE_STATE_OFF;

    if(enable_update && !gChaosContext.update_enabled)
    {
        gChaosContext.code_elapsed_usec = 0;
        gChaosContext.chaos_elapsed_usec = 0;
        gChaosContext.prev_update_counter = osGetTime();
    }

    gChaosContext.update_enabled = enable_update;
    gChaosContext.effect_restrictions = Chaos_EffectRestrictions(play);
    return gChaosContext.update_enabled;
}

void Chaos_StepUpInterruptionProbabilityScale(u8 seconds)
{
    if(Chaos_GetConfigFlag(CHAOS_CONFIG_USE_DISRUPTIVE_EFFECT_PROB))
    {
        if(gChaosContext.interruption_probability_scale < CHAOS_MAX_DISRUPTIVE_PROBABILITY_SCALE)
        {
            gChaosContext.interruption_probability_scale += (f32)seconds * 0.017f;

            if(gChaosContext.interruption_probability_scale > CHAOS_MAX_DISRUPTIVE_PROBABILITY_SCALE)
            {
                gChaosContext.interruption_probability_scale = CHAOS_MAX_DISRUPTIVE_PROBABILITY_SCALE;
            }
            
            gChaosContext.need_update_distribution = true;
        }
    }
}

void Chaos_StepDownInterruptionProbabilityScale(void)
{
    if(gChaosContext.interruption_probability_scale > 1.0f)
    {
        gChaosContext.interruption_probability_scale *= 0.9f;

        if(gChaosContext.interruption_probability_scale < 1.0f)
        {
            gChaosContext.interruption_probability_scale = 1.0f;
        }

        gChaosContext.need_update_distribution = true;
    }
}

void Chaos_UpdatePeriodicProbabilityScale(void)
{
    if(gChaosContext.periodic_probability_update_timer == 0)
    {
        if(gChaosContext.periodic_probability_scale_target > 1.0f)
        {
            gChaosContext.periodic_probability_scale_target = CHAOS_MIN_PERIODIC_PROBABILITY_SCALE + Chaos_ZeroOne() * 0.5f;
        }
        else
        {
            gChaosContext.periodic_probability_scale_target = 1.0f + Chaos_ZeroOne() * 2.0f;
        }

        gChaosContext.periodic_probability_update_timer = Chaos_RandS16Offset(60, 240);
    }

    gChaosContext.need_update_distribution |= !Math_StepToF(&gChaosContext.periodic_probability_scale, gChaosContext.periodic_probability_scale_target, 0.1f);
}

u16 Chaos_EffectRestrictions(struct PlayState *play)
{
    Player *player = GET_PLAYER(play);
    Camera *camera = Play_GetCamera(play, CAM_ID_MAIN);
    u16 restriction_flags = 0;

    if(CutsceneManager_GetCurrentCsId() != CS_ID_NONE && gSaveContext.gameMode != GAMEMODE_TITLE_SCREEN)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_CUTSCENE |
                             CHAOS_CODE_RESTRICTION_FLAG_AFFECT_CUTSCENE;
    }

    if(play->transitionMode != TRANS_MODE_OFF || play->transitionTrigger != TRANS_TRIGGER_OFF)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TRANSITION |
                             CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TRANSITION;
    }

    if((player->stateFlags1 & PLAYER_STATE1_DEAD) || player->csId == play->playerCsIds[PLAYER_CS_ID_DEATH])
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_DEAD |
                             CHAOS_CODE_RESTRICTION_FLAG_AFFECT_DEAD;
    }

    if(camera->setting == CAM_SET_BOAT_CRUISE)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_BOAT_RIDE | 
                             CHAOS_CODE_RESTRICTION_FLAG_AFFECT_BOAT_RIDE;
    }

    if(player->stateFlags1 & PLAYER_STATE1_TIME_STOPPED && gSaveContext.gameMode != GAMEMODE_TITLE_SCREEN)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TIME_STOPPED | 
                             CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TIME_STOPPED;
    }

    if(player->stateFlags2 & PLAYER_STATE2_GRABBED)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_GRABBED | 
                             CHAOS_CODE_RESTRICTION_FLAG_AFFECT_GRABBED;
    }

    if(player->stateFlags1 & PLAYER_STATE1_MOUNTED)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_EPONA_RIDE | 
                             CHAOS_CODE_RESTRICTION_FLAG_AFFECT_EPONA_RIDE;
    }

    if(play->pauseCtx.state != PAUSE_STATE_OFF)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_AFFECT_PAUSED;
    }

    if(gSaveContext.gameMode == GAMEMODE_TITLE_SCREEN)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TITLE_SCREEN;
    }

    return restriction_flags;
}

Actor *Chaos_SpawnActor(ActorContext *context, PlayState *play, s16 actor_id, f32 pos_x, f32 pos_y, f32 pos_z, s16 rot_x, s16 rot_y, s16 rot_z, s32 params)
{
    Actor *actor = NULL;

    if(gChaosContext.actors.spawned_actors < MAX_SPAWNED_ACTORS)
    {
        actor = Actor_Spawn(context, play, actor_id, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, params);

        if(actor != NULL)
        {
            struct ChaosActor *chaos_actor = gChaosContext.actors.slots + gChaosContext.actors.spawned_actors;
            chaos_actor->actor = actor;
            chaos_actor->timer = ACTOR_DESPAWN_TIMER;
            // actor->flags |= ACTOR_FLAG_CHAOS;

            Actor_ChangeCategory(play, &play->actorCtx, actor, ACTORCAT_CHAOS);

            if(actor_id == ACTOR_EN_ARWING)
            {
                chaos_actor->timer *= 10;
            }

            switch(actor_id)
            {
                case ACTOR_EN_SKB:
                    actor->destroy = Chaos_StalchildDestroyFunction;
                break;

                case ACTOR_EN_RR:
                    actor->destroy = Chaos_LikeLikeDestroyFunction;
                break;

                default:
                    actor->destroy = Chaos_DestroyFunction;
                break;
            }

            // if(actor_id == ACTOR_EN_SKB)
            // {
            //     actor->destroy = Chaos_StalchildDestroyFunction;
            // }
            // else
            // {
            //     actor->destroy = Chaos_DestroyFunction;
            // }

            gChaosContext.actors.spawned_actors++;
        }
    }

    return actor;
}

#define HALF_DANCE_FLOOR_LENGTH 35.0f

// These are all relative tot he player
Vec3f DancePositions_FourCorners[] = {
    {-HALF_DANCE_FLOOR_LENGTH, 0.0f, -HALF_DANCE_FLOOR_LENGTH},
    { HALF_DANCE_FLOOR_LENGTH, 0.0f, -HALF_DANCE_FLOOR_LENGTH},
    {-HALF_DANCE_FLOOR_LENGTH, 0.0f,  HALF_DANCE_FLOOR_LENGTH},
    { HALF_DANCE_FLOOR_LENGTH, 0.0f,  HALF_DANCE_FLOOR_LENGTH},
};

// Currently unused but could be fun!
Vec3f DancePositions_VFormation[] = {
    {-HALF_DANCE_FLOOR_LENGTH, 0.0f, 0.0f},
    {-0.5f*HALF_DANCE_FLOOR_LENGTH, 0.0f, -0.5f*HALF_DANCE_FLOOR_LENGTH},
    {0.0f, 0.0f, -HALF_DANCE_FLOOR_LENGTH},
    { 0.5f*HALF_DANCE_FLOOR_LENGTH, 0.0f, -0.5f*HALF_DANCE_FLOOR_LENGTH},
    { HALF_DANCE_FLOOR_LENGTH, 0.0f, 0.0f},
};
 
void Chaos_SpawnRedeadDanceParty(ActorContext *context, PlayState *play, Vec3f *player_pos)
{
    u32 dancerCount = 4;
    u32 dance_leader_index = Chaos_RandS16Offset(0, dancerCount - 1);
    u32 index;
    s16 yaw_to_player;

    // Determine if the dancers should be homegenous or all somewhat different.
    s16 allSameDance = Chaos_RandS16Offset(0, 2); 
    EnRdType redeadType = EN_RD_CHAOS_TYPE_HIT_THE_GRIDDY;
    if(allSameDance){
        redeadType = Chaos_RandS16Offset(EN_RD_CHAOS_TYPE_HIT_THE_GRIDDY, 
            EN_RD_CHAOS_TYPE_LASTDANCEENUM - EN_RD_CHAOS_TYPE_HIT_THE_GRIDDY - 1
        );
    }
    // Each dancer
    for(index = 0; index < dancerCount; index++){
        Vec3f spawnPos;
        Vec3f* dancerPositionOffsetSrc = &DancePositions_FourCorners[index];
        Math_Vec3f_Sum(player_pos, dancerPositionOffsetSrc, &spawnPos);
        if(!allSameDance){
            // Roll for each dude
            redeadType = Chaos_RandS16Offset(EN_RD_CHAOS_TYPE_HIT_THE_GRIDDY, 
                EN_RD_CHAOS_TYPE_LASTDANCEENUM - EN_RD_CHAOS_TYPE_HIT_THE_GRIDDY - 1
            );
        }

        yaw_to_player = Math_Vec3f_Yaw(&spawnPos, player_pos);
        
        // Spawn em
        Chaos_SpawnActor(context, play, ACTOR_EN_RD, spawnPos.x, spawnPos.y, spawnPos.z, 0, yaw_to_player, 0, 
            redeadType | ((index == dance_leader_index) ? (EN_RD_FLAG_DANCE_LEADER << 8) : 0)
        );
    }
}


Actor* Chaos_SpawnAsChild(ActorContext* context, Actor* parent, PlayState* play, s16 actor_id, f32 pos_x, f32 pos_y, f32 pos_z, s16 rot_x, s16 rot_y, s16 rot_z, s32 params)
{
    Actor *actor = NULL;

    if(gChaosContext.actors.spawned_actors < MAX_SPAWNED_ACTORS)
    {
        actor = Actor_SpawnAsChild(context, parent, play, actor_id, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, params);

        if(actor != NULL)
        {
            struct ChaosActor *chaos_actor = gChaosContext.actors.slots + gChaosContext.actors.spawned_actors;
            chaos_actor->actor = actor;
            chaos_actor->timer = ACTOR_DESPAWN_TIMER;
            // actor->flags |= ACTOR_FLAG_CHAOS;
            Actor_ChangeCategory(play, &play->actorCtx, actor, ACTORCAT_CHAOS);

            // if(actor_id == ACTOR_EN_SKB)
            // {
            //     actor->destroy = Chaos_StalchildDestroyFunction;
            // }
            // else
            // {
            //     actor->destroy = Chaos_DestroyFunction;
            // }

            switch(actor_id)
            {
                case ACTOR_EN_SKB:
                    actor->destroy = Chaos_StalchildDestroyFunction;
                break;

                case ACTOR_EN_RR:
                    actor->destroy = Chaos_LikeLikeDestroyFunction;
                break;

                default:
                    actor->destroy = Chaos_DestroyFunction;
                break;
            }

            gChaosContext.actors.spawned_actors++;
        }
    }

    return actor;
}

void Chaos_DestroyFunction(struct Actor *actor, struct PlayState *play)
{
    u32 index;

    if(actor != NULL)
    {
        for(index = 0; index < gChaosContext.actors.spawned_actors; index++)
        {
            if(gChaosContext.actors.slots[index].actor == actor)
            {
                struct ChaosActor *chaos_actor = gChaosContext.actors.slots + index;
                ActorProfile *init = Actor_GetActorInit(&play->actorCtx, actor->id);
                Chaos_DropActorAtIndex(index);
                init->destroy(actor, play);
                return;
            }
        }    
    }
}

void Chaos_StalchildDestroyFunction(struct Actor *actor, struct PlayState *play)
{
    if(gChaosContext.env.stalchild_count > 0)
    {
        gChaosContext.env.stalchild_count--;
    }

    Chaos_DestroyFunction(actor, play);
}

void Chaos_KillActorAtIndex(u32 index)
{
    if(index < gChaosContext.actors.spawned_actors)
    {
        struct ChaosActor *slot = gChaosContext.actors.slots + index;

        if(slot->actor != NULL)
        {
            Actor_Kill(slot->actor);
        }

        Chaos_DropActorAtIndex(index);
    }
}

void Chaos_DropActorAtIndex(u32 index)
{
    if(index < gChaosContext.actors.spawned_actors)
    {
        struct ChaosActor *slot = gChaosContext.actors.slots + index;

        if(slot->actor != NULL)
        {
            slot->actor = NULL;
            slot->timer = 0;

            gChaosContext.actors.spawned_actors--;

            if(index < gChaosContext.actors.spawned_actors)
            {
                gChaosContext.actors.slots[index] = gChaosContext.actors.slots[gChaosContext.actors.spawned_actors];
            }
        }
    }
}

// void Chaos_DropActor(Actor *actor)
// {
//     u32 index;

//     if(actor != NULL)
//     {
//         for(index = 0; index < gChaosContext.actors.spawned_actors; index++)
//         {
//             if(gChaosContext.actors.slots[index].actor == actor)
//             {
//                 Chaos_DropActorAtIndex(index);
//                 break;
//             }
//         }    
//     }

//     return;
// }

void Chaos_ClearActors(void)
{
    gChaosContext.actors.spawned_actors = 0;
    gChaosContext.chicken.cucco.attackNiwCount = 0;
    gChaosContext.env.stalchild_count = 0;
}

u16 Chaos_RandomEntrance(PlayState *play)
{
    // u16 *entrances;
    Player *player = GET_PLAYER(play);
    u16 scene_index;
    u16 entrance_index;
    u8 test_index;
    u32 boss_entrance_rando = 0;
    struct RandomSceneEntrances *entrances;
    // SceneEntranceTableEntry *scene_entry;

    while(true)
    {
        scene_index = gChaosContext.entrance.enabled_scenes[Rand_Next() % gChaosContext.entrance.enabled_scene_count];
        entrances = gEntrances + scene_index;

        if(entrances->entrance_count > 0)
        {
            entrance_index = entrances->entrances[Rand_Next() % entrances->entrance_count];

            switch(scene_index)
            {
                case ENTR_SCENE_ODOLWAS_LAIR:
                    if(!boss_entrance_rando)
                    {
                        continue;
                    }
                break;

                case ENTR_SCENE_GYORGS_LAIR:
                    if(!boss_entrance_rando || gSaveContext.save.saveInfo.inventory.items[SLOT_MASK_ZORA] != ITEM_MASK_ZORA)
                    {
                        /* not going through boss entrance or player doesn't have zora mask */
                        continue;
                    }
                break;

                case ENTR_SCENE_GOHTS_LAIR:
                    if(!boss_entrance_rando || (gSaveContext.save.saveInfo.inventory.items[SLOT_MASK_GORON] != ITEM_MASK_GORON &&
                                                gSaveContext.save.saveInfo.inventory.items[SLOT_BOW] != ITEM_BOW))
                    {
                        continue;
                    }
                break;

                case ENTR_SCENE_TWINMOLDS_LAIR:
                    if(!boss_entrance_rando || gSaveContext.save.saveInfo.inventory.items[SLOT_BOW] != ITEM_BOW)
                    {
                        continue;
                    }
                break;

                case ENTR_SCENE_PIRATES_FORTRESS_INTERIOR:
                    if(player->transformation != PLAYER_FORM_ZORA && entrance_index == 9)
                    {
                        /* entrance is underwater and player is not zora */
                        continue;
                    }
                break;

                case ENTR_SCENE_PINNACLE_ROCK:
                    if(player->transformation == PLAYER_FORM_DEKU || player->transformation == PLAYER_FORM_GORON)
                    {
                        /* entrance is in the middle of the ocean and player can't swim */
                        continue;
                    }
                break;

                case ENTR_SCENE_FAIRY_FOUNTAIN:
                {
                    if(entrances->entrances[entrance_index] > 1)
                    {
                        u32 enabled_scene_index;
                        u32 search_index;
                        switch(entrances->entrances[entrance_index])
                        {
                            case 2:
                                /* snowhead */
                                enabled_scene_index = ENTR_SCENE_SNOWHEAD;
                            break;

                            case 3:
                                /* zora hall */
                                enabled_scene_index = ENTR_SCENE_ZORA_HALL;
                            break;

                            case 4:
                                /* ikana canyon */
                                enabled_scene_index = ENTR_SCENE_IKANA_CANYON;
                            break;
                        }  

                        for(search_index = 0; search_index < gChaosContext.entrance.enabled_scene_count; search_index++)
                        {
                            if(gChaosContext.entrance.enabled_scenes[search_index] == enabled_scene_index)
                            {
                                break;
                            }
                        }

                        if(search_index == gChaosContext.entrance.enabled_scene_count)
                        {
                            /* scene on the other side of the entrance is not enabled */
                            continue;
                        }
                    }
                }
                break;

                case ENTR_SCENE_WOODFALL:
                    if(!CHECK_WEEKEVENTREG(WEEKEVENTREG_WOODFALL_TEMPLE_RISEN) && (entrance_index == 1 || entrance_index == 3))
                    {
                        /* player hasn't raised woodfall temple yet, so pick another entrance */
                        continue;
                    }
                break;
            }

            return Entrance_Create(scene_index, entrance_index, 0);
        }
    }

    return play->nextEntrance;
}

#define ENABLE_ENTRANCE(scene_entrance) (gChaosContext.entrance.enabled_scenes[gChaosContext.entrance.enabled_scene_count++] = scene_entrance)

void Chaos_UpdateEntrances(PlayState *play)
{
    gChaosContext.entrance.enabled_scene_count = 0;
    ENABLE_ENTRANCE(ENTR_SCENE_MAYORS_RESIDENCE);
    ENABLE_ENTRANCE(ENTR_SCENE_HONEY_AND_DARLINGS_SHOP);
    ENABLE_ENTRANCE(ENTR_SCENE_CURIOSITY_SHOP);
    ENABLE_ENTRANCE(ENTR_SCENE_MILK_BAR);
    ENABLE_ENTRANCE(ENTR_SCENE_TREASURE_CHEST_SHOP);
    ENABLE_ENTRANCE(ENTR_SCENE_TOWN_SHOOTING_GALLERY);
    ENABLE_ENTRANCE(ENTR_SCENE_FAIRY_FOUNTAIN);
    ENABLE_ENTRANCE(ENTR_SCENE_ASTRAL_OBSERVATORY);
    ENABLE_ENTRANCE(ENTR_SCENE_POST_OFFICE);
    ENABLE_ENTRANCE(ENTR_SCENE_TRADING_POST);
    ENABLE_ENTRANCE(ENTR_SCENE_LOTTERY_SHOP);
    ENABLE_ENTRANCE(ENTR_SCENE_SWORDMANS_SCHOOL);
    ENABLE_ENTRANCE(ENTR_SCENE_STOCK_POT_INN);
    ENABLE_ENTRANCE(ENTR_SCENE_CLOCK_TOWER_INTERIOR);
    ENABLE_ENTRANCE(ENTR_SCENE_BOMB_SHOP);
    ENABLE_ENTRANCE(ENTR_SCENE_EAST_CLOCK_TOWN);
    ENABLE_ENTRANCE(ENTR_SCENE_NORTH_CLOCK_TOWN);
    ENABLE_ENTRANCE(ENTR_SCENE_WEST_CLOCK_TOWN);
    ENABLE_ENTRANCE(ENTR_SCENE_SOUTH_CLOCK_TOWN);
    ENABLE_ENTRANCE(ENTR_SCENE_LAUNDRY_POOL);

    if(gSaveContext.save.saveInfo.inventory.items[SLOT_OCARINA] == ITEM_OCARINA_OF_TIME && 
       gSaveContext.save.saveInfo.inventory.items[SLOT_MASK_DEKU] == ITEM_MASK_DEKU)
    {
        /* only warp the player outside clock town after completing the first cycle */
        u32 owl_warps = gSaveContext.save.saveInfo.playerData.owlActivationFlags & OWL_WARP_ALL_MASK;
        u32 has_bow = gSaveContext.save.saveInfo.inventory.items[SLOT_BOW] == ITEM_BOW;
        u32 has_hookshot = gSaveContext.save.saveInfo.inventory.items[SLOT_HOOKSHOT] == ITEM_HOOKSHOT;
        u32 has_bomb = gSaveContext.save.saveInfo.inventory.items[SLOT_BOMB] == ITEM_BOMB_BAG_20 ||
                       gSaveContext.save.saveInfo.inventory.items[SLOT_BOMB] == ITEM_BOMB_BAG_30 ||
                       gSaveContext.save.saveInfo.inventory.items[SLOT_BOMB] == ITEM_BOMB_BAG_40;
        
        u32 has_powder_keg = gSaveContext.save.saveInfo.inventory.items[SLOT_POWDER_KEG] == ITEM_POWDER_KEG;

        u32 has_goron_mask = gSaveContext.save.saveInfo.inventory.items[SLOT_MASK_GORON] == ITEM_MASK_GORON;
        u32 has_zora_mask = gSaveContext.save.saveInfo.inventory.items[SLOT_MASK_ZORA] == ITEM_MASK_ZORA;
        u32 has_epona_song = gSaveContext.save.saveInfo.inventory.questItems & (1 << QUEST_SONG_EPONA);
        u32 has_mirror_shield = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) == (ITEM_SHIELD_MIRROR - ITEM_SHIELD_HERO) + EQUIP_VALUE_SHIELD_HERO;
        u32 has_light_arrow = gSaveContext.save.saveInfo.inventory.items[SLOT_ARROW_LIGHT] == ITEM_ARROW_LIGHT;

        ENABLE_ENTRANCE(ENTR_SCENE_ROAD_TO_SOUTHERN_SWAMP);
        ENABLE_ENTRANCE(ENTR_SCENE_SWAMP_SHOOTING_GALLERY);
        ENABLE_ENTRANCE(ENTR_SCENE_TOURIST_INFORMATION);
        ENABLE_ENTRANCE(ENTR_SCENE_TERMINA_FIELD);
        ENABLE_ENTRANCE(ENTR_SCENE_GORMAN_TRACK);

        if(!(gSaveContext.save.saveInfo.inventory.questItems & (1 << QUEST_SONG_SOARING)))
        {
            /* player doesn't have song of soaring, so owl warping is out */
            owl_warps = 0;
        }

        if(owl_warps || has_powder_keg)
        {
            /* need either powder keg or warp to get out of romani ranch area */
            ENABLE_ENTRANCE(ENTR_SCENE_RANCH_HOUSE);
            ENABLE_ENTRANCE(ENTR_SCENE_ROMANI_RANCH);
            ENABLE_ENTRANCE(ENTR_SCENE_CUCCO_SHACK);
            ENABLE_ENTRANCE(ENTR_SCENE_DOGGY_RACETRACK);
        }
        
        if((owl_warps & ~((1 << OWL_WARP_SOUTHERN_SWAMP) | (1 << OWL_WARP_WOODFALL))) || has_bow || has_hookshot)
        {
            /* to exit the southern swamp the player needs to either have an owl that's not southern swamp's nor woodfall's, 
            the bow or the hookshot (so it can kill the big octo)*/
            ENABLE_ENTRANCE(ENTR_SCENE_MAGIC_HAGS_POTION_SHOP);
            ENABLE_ENTRANCE(ENTR_SCENE_WOODFALL);
            ENABLE_ENTRANCE(ENTR_SCENE_DEKU_PALACE);
            ENABLE_ENTRANCE(ENTR_SCENE_DEKU_KINGS_CHAMBER);
            ENABLE_ENTRANCE(ENTR_SCENE_SWAMP_SPIDER_HOUSE);
            ENABLE_ENTRANCE(ENTR_SCENE_DEKU_SHRINE);
            ENABLE_ENTRANCE(ENTR_SCENE_WOODS_OF_MYSTERY);

            if(CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE))
            {
                ENABLE_ENTRANCE(ENTR_SCENE_SOUTHERN_SWAMP_CLEARED);
            }
            else
            {
                ENABLE_ENTRANCE(ENTR_SCENE_SOUTHERN_SWAMP_POISONED);
            }

            if(CHECK_WEEKEVENTREG(WEEKEVENTREG_WOODFALL_TEMPLE_RISEN))
            {
                /* only warp the player to inside the woodfall temple if it's been already
                risen. Otherwise, walking out of it can lead to softlocks (if player is in
                deku form) */
                ENABLE_ENTRANCE(ENTR_SCENE_WOODFALL_TEMPLE);
            }
        }

        if((owl_warps & ~((1 << OWL_WARP_MOUNTAIN_VILLAGE) | (1 << OWL_WARP_SNOWHEAD))) || (has_bow && has_bomb))
        {
            /* path to mountain village is blocked by ice traps, which require the bow to be cleard. 
            To get back from it, it'll be necessary to either break them or warp out */

            ENABLE_ENTRANCE(ENTR_SCENE_PATH_TO_MOUNTAIN_VILLAGE);
            ENABLE_ENTRANCE(ENTR_SCENE_MOUNTAIN_SMITHY);
            ENABLE_ENTRANCE(ENTR_SCENE_GORON_GRAVERYARD);
            ENABLE_ENTRANCE(ENTR_SCENE_GORON_RACETRACK);
            ENABLE_ENTRANCE(ENTR_SCENE_GORON_SHRINE);
            ENABLE_ENTRANCE(ENTR_SCENE_GORON_SHOP);
            ENABLE_ENTRANCE(ENTR_SCENE_PATH_TO_SNOWHEAD);

            if(CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE))
            {
                ENABLE_ENTRANCE(ENTR_SCENE_PATH_TO_GORON_VILLAGE_SPRING);
                ENABLE_ENTRANCE(ENTR_SCENE_GORON_VILLAGE_SPRING);
            }
            else
            {
                ENABLE_ENTRANCE(ENTR_SCENE_PATH_TO_GORON_VILLAGE_WINTER);
                ENABLE_ENTRANCE(ENTR_SCENE_GORON_VILLAGE_WINTER);
            }

            if(has_goron_mask)
            {
                /* it's technically possible to get out of snowhead temple without
                goron mask, but it's not possible to complete it without, so just warp
                the player there if they have the goron mask */
                ENABLE_ENTRANCE(ENTR_SCENE_SNOWHEAD_TEMPLE);
                ENABLE_ENTRANCE(ENTR_SCENE_SNOWHEAD);
            }
        }

        if((owl_warps & ~((1 << OWL_WARP_GREAT_BAY_COAST) | (1 << OWL_WARP_ZORA_CAPE))) || 
            (has_goron_mask && has_bomb) || has_epona_song)
        {
            /* reaching great bay coast requires clearing a fence. To get back from it
            it'll be necessary to either have epona, jump over it using bombs and goron
            mask, or warp out */

            ENABLE_ENTRANCE(ENTR_SCENE_GREAT_BAY_COAST);
            ENABLE_ENTRANCE(ENTR_SCENE_ZORA_CAPE);
            ENABLE_ENTRANCE(ENTR_SCENE_WATERFALL_RAPIDS);
            ENABLE_ENTRANCE(ENTR_SCENE_OCEANSIDE_SPIDER_HOUSE);
            ENABLE_ENTRANCE(ENTR_SCENE_FISHERMANS_HUT);
            ENABLE_ENTRANCE(ENTR_SCENE_MARINE_RESEARCH_LAB);

            if(has_zora_mask)
            {
                /* it's technically possible to leave the pirate fortress without 
                the zora mask, but... */
                ENABLE_ENTRANCE(ENTR_SCENE_PIRATES_FORTRESS);
                ENABLE_ENTRANCE(ENTR_SCENE_PIRATES_FORTRESS_EXTERIOR);
                ENABLE_ENTRANCE(ENTR_SCENE_PIRATES_FORTRESS_INTERIOR);
                ENABLE_ENTRANCE(ENTR_SCENE_GREAT_BAY_TEMPLE);
                ENABLE_ENTRANCE(ENTR_SCENE_PINNACLE_ROCK);
                ENABLE_ENTRANCE(ENTR_SCENE_ZORA_HALL);
                ENABLE_ENTRANCE(ENTR_SCENE_ZORA_HALL_ROOMS);
            }
        }

        if((owl_warps & ~((1 << OWL_WARP_IKANA_CANYON) | (1 << OWL_WARP_STONE_TOWER))) || 
            (has_goron_mask && has_bomb) || has_epona_song)
        {
            /* road to ikana is fenced off, so only warp the player there if they can use
            goron + bombs or epona to clear it */
            ENABLE_ENTRANCE(ENTR_SCENE_ROAD_TO_IKANA);
            ENABLE_ENTRANCE(ENTR_SCENE_IKANA_CANYON);
            ENABLE_ENTRANCE(ENTR_SCENE_IKANA_GRAVEYARD);
            ENABLE_ENTRANCE(ENTR_SCENE_SAKONS_HIDEOUT);
            ENABLE_ENTRANCE(ENTR_SCENE_GHOST_HUT);
            ENABLE_ENTRANCE(ENTR_SCENE_SECRET_SHRINE);
            
            if(has_mirror_shield || has_light_arrow)
            {
                /* ikana castle has a bunch of sun stones, so don't warp
                the player in there unless they can get rid of them */
                ENABLE_ENTRANCE(ENTR_SCENE_IKANA_CASTLE);
            }

            if(has_hookshot)
            {
                /* not sure if this is necessary. Stone tower might be exitable
                without hookshot, but having it makes things a bit easier */
                ENABLE_ENTRANCE(ENTR_SCENE_STONE_TOWER);
                ENABLE_ENTRANCE(ENTR_SCENE_STONE_TOWER_TEMPLE);
            }
        }
    }
}

void Chaos_UpdateEnabledChaosEffectsAndEntrances(PlayState *this)
{
    u32 index;
    u32 enable_low_grav = true;
    u32 scene_index = gSaveContext.save.entrance >> 9;
    u32 room_index;
    f32 change_rupee_prob = 1.0f;
    Player *player = GET_PLAYER(this);
    Camera *camera = Play_GetCamera(this, CAM_ID_MAIN);
    u32 has_ocarina = gSaveContext.save.saveInfo.inventory.items[SLOT_OCARINA] == ITEM_OCARINA_OF_TIME;
    u32 is_lunatic = scene_index == ENTR_SCENE_THE_MOON || scene_index == ENTR_SCENE_MOON_GORON_TRIAL ||
                    scene_index == ENTR_SCENE_MOON_LINK_TRIAL || scene_index == ENTR_SCENE_MOON_DEKU_TRIAL ||
                    scene_index == ENTR_SCENE_MOON_ZORA_TRIAL || scene_index == ENTR_SCENE_MAJORAS_LAIR;

    f32 disruptive_prob_scale = gChaosContext.interruption_probability_scale * gChaosContext.periodic_probability_scale;    
    Chaos_ClearEnabledCodes();
    Chaos_EnableCode(CHAOS_CODE_YEET, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_MOON_DANCE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_ONE_HIT_KO, gChaosContext.periodic_probability_scale);
    Chaos_EnableCode(CHAOS_CODE_TIMER_UP, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_EARTHQUAKE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_WEIRD_UI, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_INVINCIBLE, gChaosContext.periodic_probability_scale);
    Chaos_EnableCode(CHAOS_CODE_TRAP_FLAP, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_SLIPPERY_FLOORS, gChaosContext.periodic_probability_scale);
    Chaos_EnableCode(CHAOS_CODE_SLOW_DOWN, gChaosContext.periodic_probability_scale);
    Chaos_EnableCode(CHAOS_CODE_TERRIBLE_MUSIC, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_INCREDIBLE_KNOCKBACK, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_RANDOM_SCALING, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_BIG_BROTHER, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_WEIRD_SKYBOX, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_SWAP_HEAL_AND_HURT, gChaosContext.periodic_probability_scale);
    Chaos_EnableCode(CHAOS_CODE_CHANGE_HEALTH, gChaosContext.periodic_probability_scale);
    Chaos_EnableCode(CHAOS_CODE_SCALE_RANDOM_LIMB, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_HEART_SNAKE, gChaosContext.periodic_probability_scale);
    Chaos_EnableCode(CHAOS_CODE_SIMON_SAYS, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_SPEEDBOOST, gChaosContext.periodic_probability_scale);
    Chaos_EnableCode(CHAOS_CODE_BILLBOARD_ACTORS, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_SIGNPOST, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_LENGTH_CONTRACTION, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_RANDOM_HEALTH_UP, 1.0f / gChaosContext.periodic_probability_scale);
    Chaos_EnableCode(CHAOS_CODE_RANDOM_HEALTH_DOWN, gChaosContext.periodic_probability_scale);
    Chaos_EnableCode(CHAOS_CODE_FISH, gChaosContext.periodic_probability_scale);

    Chaos_EnableCode(CHAOS_CODE_CHANGE_RUPEE, gChaosContext.periodic_probability_scale);

    if(gSaveContext.save.saveInfo.playerData.isMagicAcquired)
    {
        Chaos_EnableCode(CHAOS_CODE_CHANGE_MAGIC, gChaosContext.periodic_probability_scale);
    }

    if(gSaveContext.save.saveInfo.inventory.items[SLOT_BOW] == ITEM_BOW)
    {
        f32 useful_arrow_probability = 1.0f;

        if(Chaos_GetConfigFlag(CHAOS_CONFIG_USE_PERIODIC_EFFECT_PROB))
        {
            useful_arrow_probability += 0.5f / gChaosContext.periodic_probability_scale;
        }

        Chaos_EnableCode(CHAOS_CODE_BOMB_ARROWS, useful_arrow_probability);
        Chaos_EnableCode(CHAOS_CODE_BUCKSHOT_ARROWS, useful_arrow_probability);
        Chaos_EnableCode(CHAOS_CODE_WEIRD_ARROWS, CLAMP_MAX(gChaosContext.periodic_probability_scale, 1.5f));
    }

    if(gSaveContext.save.saveInfo.inventory.items[SLOT_BOMB] == ITEM_BOMB ||
       gSaveContext.save.saveInfo.inventory.items[SLOT_BOMBCHU] == ITEM_BOMBCHU)
    {
        Chaos_EnableCode(CHAOS_CODE_RANDOM_BOMB_TIMER, gChaosContext.periodic_probability_scale);
    }
 
    Chaos_EnableCode(CHAOS_CODE_POKE, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_RANDOM_KNOCKBACK, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_ICE_TRAP, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_SHOCK, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_SYKE, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_DIE, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_ACTOR_CHASE, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_LOVELESS_MARRIAGE, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_CHICKEN_ARISE, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_STARFOX, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_WALLMASTER, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_REDEADASS_GROOVE, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_LIFTOFF, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_JUNK_ITEM, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_UNSTABLE_ROOMS, disruptive_prob_scale);
    Chaos_EnableCode(CHAOS_CODE_BAD_CONNECTION, disruptive_prob_scale);


    if(!Chaos_IsCodeInActiveList(CHAOS_CODE_IMAGINARY_FRIENDS))
    {
        Chaos_EnableCode(CHAOS_CODE_OUT_OF_SHAPE, disruptive_prob_scale);
    }

    if(!Chaos_IsCodeInActiveList(CHAOS_CODE_OUT_OF_SHAPE))
    {
        Chaos_EnableCode(CHAOS_CODE_IMAGINARY_FRIENDS, disruptive_prob_scale);
    }

    if(Chaos_GetConfigFlag(CHAOS_CONFIG_ALLOW_BEER_GOGGLES_AND_SILENT_FIELD) || 
        !Chaos_IsCodeInActiveList(CHAOS_CODE_BEER_GOGGLES))
    {
        Chaos_EnableCode(CHAOS_CODE_SILENT_FIELD, disruptive_prob_scale);
    }

    if(Chaos_GetConfigFlag(CHAOS_CONFIG_ALLOW_BEER_GOGGLES_AND_SILENT_FIELD) || 
        !Chaos_IsCodeInActiveList(CHAOS_CODE_SILENT_FIELD))
    {
        Chaos_EnableCode(CHAOS_CODE_BEER_GOGGLES, disruptive_prob_scale);
    }

    if(CHECK_WEEKEVENTREG(WEEKEVENTREG_ENTERED_SOUTH_CLOCK_TOWN))
    {
        if(!is_lunatic)
        {
            if(has_ocarina)
            {
                if(TIME_UNTIL_MOON_CRASH > CLOCK_TIME(6, 0))
                {
                    /* start random moon crashes only if we're not close to the real one */
                    Chaos_EnableCode(CHAOS_CODE_MOON_CRASH, disruptive_prob_scale);
                }

                if(Chaos_TimeUntilMoonCrash() > CLOCK_TIME(4, 0) && player->transformation != PLAYER_FORM_FIERCE_DEITY)
                {
                    /* activate fast time only if there's enough time for the player to react and the player
                    can actually play the ocarina */
                    Chaos_EnableCode(CHAOS_CODE_FAST_TIME, gChaosContext.periodic_probability_scale);
                }
            }
        }
        else
        {
            gChaosContext.moon.moon_crash_timer = 0;
            gChaosContext.moon.moon_crash_time_offset = 0;
            gSaveContext.save.timeSpeedOffset = 0;
        }
    }

    /* some bosses/mini-bosses can fall out of bounds if they jump when low-grav is active,
    so check if we're not in those rooms */
    for(index = 0; index < ARRAY_COUNT(gNoLowGravRooms); index++)
    {
        if(scene_index == gNoLowGravRooms[index].scene_index)
        {
            u32 room_index;
            for(room_index = 0; room_index < gNoLowGravRooms[index].room_count; room_index++)
            {
                if(gNoLowGravRooms[index].room_indices[room_index] == this->roomCtx.curRoom.num)
                {
                    index = ARRAY_COUNT(gNoLowGravRooms);
                    enable_low_grav = false;
                    break;
                }
            }
        }
    }

    if(enable_low_grav)
    {
        Chaos_EnableCode(CHAOS_CODE_LOW_GRAVITY, 1.0f);
    }
    else
    {
        Chaos_DeactivateCode(CHAOS_CODE_LOW_GRAVITY);
    }

    if(has_ocarina)
    {
        Chaos_EnableCode(CHAOS_CODE_PLAY_OCARINA, disruptive_prob_scale);

        if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_OBTAINED_FIERCE_DEITY_MASK)) 
        {
            Chaos_EnableCode(CHAOS_CODE_RANDOM_FIERCE_DEITY, 1.0f);
        }

        if(!is_lunatic)
        {
            if(!Map_IsInBossScene(this) && CHECK_WEEKEVENTREG(WEEKEVENTREG_ENTERED_SOUTH_CLOCK_TOWN))
            {
                /* don't enable entrance rando until the player enters south clock town.
                Also don't enable it in boss rooms as it could interact badly with the warp 
                out */
                Chaos_EnableCode(CHAOS_CODE_ENTRANCE_RANDO, gChaosContext.periodic_probability_scale);
                Chaos_UpdateEntrances(this);
            }
            else
            {
                Chaos_DeactivateCode(CHAOS_CODE_ENTRANCE_RANDO);
            }
        }
    }
}

static u8 gOppositeMoveDirs[] = {
    /* [CHAOS_SNAKE_MOVE_DIR_RIGHT] =  */ CHAOS_SNAKE_MOVE_DIR_LEFT,
    /* [CHAOS_SNAKE_MOVE_DIR_LEFT]  =  */ CHAOS_SNAKE_MOVE_DIR_RIGHT,
    /* [CHAOS_SNAKE_MOVE_DIR_UP]    =  */ CHAOS_SNAKE_MOVE_DIR_DOWN,
    /* [CHAOS_SNAKE_MOVE_DIR_DOWN]  =  */ CHAOS_SNAKE_MOVE_DIR_UP,
};

void Chaos_SpawnHeartContainer(PlayState *play)
{
    u32 container_index = gChaosContext.ui.heart_count;
    gChaosContext.ui.heart_containers[container_index].pos_x = Chaos_RandS16Offset(0, CHAOS_MAX_SNAKE_X) * LIFEMETER_HEART_CONTAINER_SIZE;
    gChaosContext.ui.heart_containers[container_index].pos_y = Chaos_RandS16Offset(CHAOS_MIN_SNAKE_HEART_SPAWN_Y, 
        CHAOS_MAX_SNAKE_Y - CHAOS_MIN_SNAKE_HEART_SPAWN_Y - 1) * LIFEMETER_HEART_CONTAINER_SIZE;

    gSaveContext.save.saveInfo.playerData.healthCapacity += LIFEMETER_FULL_HEART_HEALTH;
}

u32 Chaos_UpdateSnakeGame(PlayState *play, Input *input)
{
    u32 container_index = 0;
    GfxPrint gfx_print;

    if(gChaosContext.ui.snake_state == CHAOS_SNAKE_GAME_STATE_INIT)
    {
        u32 container_index;
        gChaosContext.ui.blink_timer = 100;
        gChaosContext.ui.snake_state = CHAOS_SNAKE_GAME_STATE_PLAY;        
        gChaosContext.ui.orig_heart_count = gSaveContext.save.saveInfo.playerData.healthCapacity / LIFEMETER_FULL_HEART_HEALTH;
        gChaosContext.ui.heart_count = 3;
        gChaosContext.ui.move_timer = 0;
        gChaosContext.ui.next_move_dir = CHAOS_SNAKE_MOVE_DIR_RIGHT;

        gChaosContext.ui.heart_containers[0].pos_x = CHAOS_MIN_SNAKE_X * LIFEMETER_HEART_CONTAINER_SIZE;
        gChaosContext.ui.heart_containers[0].pos_y = CHAOS_SNAKE_START_Y * LIFEMETER_HEART_CONTAINER_SIZE;
        gChaosContext.ui.heart_containers[1].pos_x = (CHAOS_MIN_SNAKE_X + 1) * LIFEMETER_HEART_CONTAINER_SIZE;
        gChaosContext.ui.heart_containers[1].pos_y = CHAOS_SNAKE_START_Y * LIFEMETER_HEART_CONTAINER_SIZE;
        gChaosContext.ui.heart_containers[2].pos_x = (CHAOS_MIN_SNAKE_X + 2) * LIFEMETER_HEART_CONTAINER_SIZE;
        gChaosContext.ui.heart_containers[2].pos_y = CHAOS_SNAKE_START_Y * LIFEMETER_HEART_CONTAINER_SIZE;

        for(container_index = 3; container_index < gChaosContext.ui.orig_heart_count; container_index++)
        {
            gChaosContext.ui.heart_containers[container_index].pos_x = Chaos_RandS16Offset(CHAOS_MIN_SNAKE_X, CHAOS_MAX_SNAKE_X - 1) * 
                LIFEMETER_HEART_CONTAINER_SIZE;
            gChaosContext.ui.heart_containers[container_index].pos_y = Chaos_RandS16Offset(CHAOS_MIN_SNAKE_HEART_SPAWN_Y, 
                CHAOS_MAX_SNAKE_Y - CHAOS_MIN_SNAKE_HEART_SPAWN_Y - 1) * LIFEMETER_HEART_CONTAINER_SIZE;
        }

        gSaveContext.save.saveInfo.playerData.healthCapacity = (gChaosContext.ui.heart_count + 1) * LIFEMETER_FULL_HEART_HEALTH;
    }

    if(gChaosContext.ui.blink_timer > 0)
    {
        u8 remainder = gChaosContext.ui.blink_timer % 10;
        if(remainder == 5)
        {
            gChaosContext.ui.flags |= CHAOS_SNAKE_GAME_FLAG_BLINK;
        }
        else if(remainder == 0)
        {
            gChaosContext.ui.flags &= ~CHAOS_SNAKE_GAME_FLAG_BLINK;
        }

        gChaosContext.ui.blink_timer--;
    }
    else
    {
        gChaosContext.ui.flags &= ~CHAOS_SNAKE_GAME_FLAG_BLINK;
    }

    if(gChaosContext.ui.snake_state == CHAOS_SNAKE_GAME_STATE_PLAY)
    {
        struct HeartContainerPos next_head_pos;
        struct HeartContainerPos *collect_heart_pos;
        u8 next_move_dir = CHAOS_SNAKE_MOVE_DIR_NONE;
        gChaosContext.ui.stick_x = input->cur.stick_x;
        gChaosContext.ui.stick_y = input->cur.stick_y;

        if(ABS(gChaosContext.ui.stick_x) > ABS(gChaosContext.ui.stick_y))
        {
            if(gChaosContext.ui.stick_x > 30)
            {
                next_move_dir = CHAOS_SNAKE_MOVE_DIR_RIGHT;
            }
            else if(gChaosContext.ui.stick_x < -30)
            {
                next_move_dir = CHAOS_SNAKE_MOVE_DIR_LEFT;
            }
        }
        else
        {
            if(gChaosContext.ui.stick_y < -30)
            {
                next_move_dir = CHAOS_SNAKE_MOVE_DIR_UP;
            }
            else if(gChaosContext.ui.stick_y > 30)
            {
                next_move_dir = CHAOS_SNAKE_MOVE_DIR_DOWN;
            }
        }

        if(next_move_dir != CHAOS_SNAKE_MOVE_DIR_NONE)
        {
            gChaosContext.ui.next_move_dir = next_move_dir;
        }

        if(gChaosContext.ui.move_timer == 0)
        {
            gChaosContext.ui.flags &= ~CHAOS_SNAKE_GAME_FLAG_MOVE_FAST;
            if(gOppositeMoveDirs[gChaosContext.ui.next_move_dir] != gChaosContext.ui.move_dir)
            {
                gChaosContext.ui.move_dir = gChaosContext.ui.next_move_dir;
            }

            if(next_move_dir == gChaosContext.ui.next_move_dir)
            {
                gChaosContext.ui.flags |= CHAOS_SNAKE_GAME_FLAG_MOVE_FAST;
            }

            next_head_pos = gChaosContext.ui.heart_containers[gChaosContext.ui.heart_count - 1];

            switch(gChaosContext.ui.move_dir)
            {
                case CHAOS_SNAKE_MOVE_DIR_RIGHT:
                    next_head_pos.pos_x += LIFEMETER_HEART_CONTAINER_SIZE;
                break;

                case CHAOS_SNAKE_MOVE_DIR_LEFT:
                    next_head_pos.pos_x -= LIFEMETER_HEART_CONTAINER_SIZE;
                break;

                case CHAOS_SNAKE_MOVE_DIR_UP:
                    next_head_pos.pos_y += LIFEMETER_HEART_CONTAINER_SIZE;
                break;

                case CHAOS_SNAKE_MOVE_DIR_DOWN:
                    next_head_pos.pos_y -= LIFEMETER_HEART_CONTAINER_SIZE;
                break;
            }

            collect_heart_pos = gChaosContext.ui.heart_containers + gChaosContext.ui.heart_count;

            if(collect_heart_pos->pos_x == next_head_pos.pos_x && collect_heart_pos->pos_y == next_head_pos.pos_y)
            {
                Audio_PlaySfx(NA_SE_SY_HP_RECOVER);
                gChaosContext.ui.heart_count++;
                if(gChaosContext.ui.heart_count < gChaosContext.ui.orig_heart_count)
                {
                    gSaveContext.save.saveInfo.playerData.healthCapacity += LIFEMETER_FULL_HEART_HEALTH;
                }
            }
            else
            {  
                u32 self_collision = false;

                for(container_index = 0; container_index < gChaosContext.ui.heart_count - 1; container_index++)
                {
                    gChaosContext.ui.heart_containers[container_index] = gChaosContext.ui.heart_containers[container_index + 1];
                }
 
                gChaosContext.ui.heart_containers[container_index] = next_head_pos;

                for(container_index = 0; container_index < gChaosContext.ui.heart_count - 1; container_index++)
                {
                    if(next_head_pos.pos_x == gChaosContext.ui.heart_containers[container_index].pos_x && 
                       next_head_pos.pos_y == gChaosContext.ui.heart_containers[container_index].pos_y)
                    {
                        self_collision = true;
                        break;
                    }
                }

                if(next_head_pos.pos_y > CHAOS_MAX_SNAKE_Y * LIFEMETER_HEART_CONTAINER_SIZE || 
                   next_head_pos.pos_x > CHAOS_MAX_SNAKE_X * LIFEMETER_HEART_CONTAINER_SIZE ||
                   next_head_pos.pos_y <= CHAOS_MIN_SNAKE_Y || next_head_pos.pos_x <= CHAOS_MIN_SNAKE_X || self_collision)
                {
                    Audio_PlaySfx(NA_SE_SY_ERROR);
                    gChaosContext.ui.snake_state = CHAOS_SNAKE_GAME_STATE_DIED;
                    gChaosContext.ui.blink_timer = 100;
                }

                
            }
   
            if(gChaosContext.ui.heart_count == gChaosContext.ui.orig_heart_count)
            {
                Audio_PlayFanfare(NA_BGM_CLEAR_EVENT);
                gChaosContext.ui.snake_state = CHAOS_SNAKE_GAME_STATE_WIN;
                gChaosContext.ui.blink_timer = 100;
            }

            if(gChaosContext.ui.flags & CHAOS_SNAKE_GAME_FLAG_MOVE_FAST)
            {
                gChaosContext.ui.move_timer = 5;
            }
            else
            {
                gChaosContext.ui.move_timer = 10;
            }
        }

        gChaosContext.ui.move_timer--;
    }

    if(gChaosContext.ui.snake_state == CHAOS_SNAKE_GAME_STATE_DIED || gChaosContext.ui.snake_state == CHAOS_SNAKE_GAME_STATE_WIN)
    {
        gSaveContext.save.saveInfo.playerData.healthCapacity = LIFEMETER_FULL_HEART_HEALTH * gChaosContext.ui.heart_count;

        if(gSaveContext.save.saveInfo.playerData.health > gSaveContext.save.saveInfo.playerData.healthCapacity)
        {
            gSaveContext.save.saveInfo.playerData.health = gSaveContext.save.saveInfo.playerData.healthCapacity;
        }

        if(gChaosContext.ui.blink_timer == 0)
        {
            gChaosContext.ui.snake_state = CHAOS_SNAKE_GAME_STATE_NONE;
            return false;
        }
    }

    return true;
}

void Chaos_UpdateSimonSays(PlayState *play, Input *input)
{
    if(gChaosContext.link.simon_says_state == CHAOS_SIMON_SAYS_STATE_START)
    {
        gChaosContext.link.simon_says_config = Chaos_RandS16Offset(0, CHAOS_SIMON_SAYS_CONFIG_LAST);
        gChaosContext.link.simon_says_timer = 100;
        gChaosContext.link.simon_says_state = CHAOS_SIMON_SAYS_STATE_WAIT_INPUT;
        gChaosContext.link.simon_says_keys[0] = Chaos_RandS16Offset(0, CHAOS_SIMON_SAYS_KEY_LAST);
    }
    else if(gChaosContext.link.simon_says_state == CHAOS_SIMON_SAYS_STATE_WAIT_INPUT)
    {
        struct SimonSaysConfig *config = gSimonSaysConfigs + gChaosContext.link.simon_says_config;
        u32 key_match = CHECK_BTN_ANY(input->cur.button, gSimonSaysKeyMap[gChaosContext.link.simon_says_keys[0]]);

        if(key_match)
        {
            if(config->match_to_live)
            {
                Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
                gChaosContext.link.simon_says_state = CHAOS_SIMON_SAYS_STATE_IDLE;
            }
            else
            {
                gSaveContext.healthAccumulator = 0;
                gSaveContext.save.saveInfo.playerData.health = 0;
                gChaosContext.link.simon_says_state = CHAOS_SIMON_SAYS_STATE_WAIT_DEATH;
                Audio_PlaySfx(NA_SE_SY_ERROR);
            }


            return;
        }

        if(gChaosContext.link.simon_says_timer > 0)
        {
            gChaosContext.link.simon_says_timer--;
        }

        if(gChaosContext.link.simon_says_timer == 0)
        {
            if(config->match_to_live)
            {
                gSaveContext.healthAccumulator = 0;
                gSaveContext.save.saveInfo.playerData.health = 0;
                gChaosContext.link.simon_says_state = CHAOS_SIMON_SAYS_STATE_WAIT_DEATH;
                Audio_PlaySfx(NA_SE_SY_ERROR);
            }
            else
            {
                Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
                gChaosContext.link.simon_says_state = CHAOS_SIMON_SAYS_STATE_IDLE;
            }

            return;
        }
    }
}

void Chaos_PrintSnakeGameStuff(PlayState *play)
{
    if(gChaosContext.ui.snake_state == CHAOS_SNAKE_GAME_STATE_PLAY && gChaosContext.ui.blink_timer > 0)
    {
        Gfx *polyOpa;
        Gfx *gfx;
        GfxPrint gfx_print;
        u32 y_pos = 3 << 3;
        OPEN_DISPS(play->state.gfxCtx);
        f32 alpha_scale = 1.0f;

        if(gChaosContext.ui.blink_timer < 10)
        {
            alpha_scale = (f32)gChaosContext.ui.blink_timer / 10.0f;
        }

        gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, 150 * alpha_scale);
        gDPFillRectangle(OVERLAY_DISP++, 0, y_pos - 4, SCREEN_WIDTH,  y_pos + (5 << 3));
        gDPPipeSync(OVERLAY_DISP++);

        polyOpa = POLY_OPA_DISP;
        gfx = Gfx_Open(polyOpa);
        // gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        // gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);

        gSPDisplayList(OVERLAY_DISP++, gfx);

        GfxPrint_Init(&gfx_print);
        GfxPrint_Open(&gfx_print, gfx);
        gfx_print.flags |= GFXP_FLAG_BLEND;
        // gDPSetRenderMode(gfx_print.dList++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        // gDPSetCombineMode(gfx_print.dList++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPSetOtherMode(gfx_print.dList++, G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_BILERP | 
            G_TT_IA16 | G_TL_TILE | G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_RM_XLU_SURF | G_RM_XLU_SURF2);

        GfxPrint_SetColor(&gfx_print, 255, 255, 255, 255 * alpha_scale);

        GfxPrint_SetPos(&gfx_print, 6, 4);
        GfxPrint_Printf(&gfx_print, "Use the stick to move the snake.");
        GfxPrint_SetPos(&gfx_print, 6, 5);
        GfxPrint_Printf(&gfx_print, "Collect all containers...");
        GfxPrint_SetPos(&gfx_print, 6, 6);
        GfxPrint_Printf(&gfx_print, "or lose them ");
        GfxPrint_SetColor(&gfx_print, 255, 5, 5, 255 * alpha_scale);
        GfxPrint_Printf(&gfx_print, "FOREVER!");

        gfx = GfxPrint_Close(&gfx_print);
        GfxPrint_Destroy(&gfx_print);
        // gSPEndDisplayList(gfx++);
        Gfx_Close(polyOpa, gfx);
        POLY_OPA_DISP = gfx;
        CLOSE_DISPS(gfxCtx);   
    }
}

void Chaos_PrintSimonSaysStuff(PlayState *play)
{
    if(gChaosContext.link.simon_says_state == CHAOS_SIMON_SAYS_STATE_WAIT_INPUT)
    {

        Gfx *polyOpa;
        Gfx *gfx;
        GfxPrint gfx_print;
        u32 y_pos = 3 << 3;
        OPEN_DISPS(play->state.gfxCtx);
        f32 alpha_scale = 1.0f;
        struct SimonSaysConfig *config = gSimonSaysConfigs + gChaosContext.link.simon_says_config;

        gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, 190);
        gDPFillRectangle(OVERLAY_DISP++, 0, y_pos - 4, SCREEN_WIDTH,  y_pos + (5 << 3));
        gDPPipeSync(OVERLAY_DISP++);

        polyOpa = POLY_OPA_DISP;
        gfx = Gfx_Open(polyOpa);
        // gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        // gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);

        gSPDisplayList(OVERLAY_DISP++, gfx);

        GfxPrint_Init(&gfx_print);
        GfxPrint_Open(&gfx_print, gfx);
        gfx_print.flags |= GFXP_FLAG_BLEND;
        // gDPSetRenderMode(gfx_print.dList++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        // gDPSetCombineMode(gfx_print.dList++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPSetOtherMode(gfx_print.dList++, G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_BILERP | 
            G_TT_IA16 | G_TL_TILE | G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_RM_XLU_SURF | G_RM_XLU_SURF2);

        
        GfxPrint_SetColor(&gfx_print, 255, 255, 255, 255);
        GfxPrint_SetPos(&gfx_print, config->text_x_offset, 4);
        GfxPrint_Printf(&gfx_print, config->str, gSimonSaysKeyStrs[gChaosContext.link.simon_says_keys[0]]);
        // GfxPrint_SetPos(&gfx_print, 6, 5);
        // GfxPrint_Printf(&gfx_print, "Collect all containers...");
        // GfxPrint_SetPos(&gfx_print, 6, 6);
        // GfxPrint_Printf(&gfx_print, "or lose them ");
        // GfxPrint_SetColor(&gfx_print, 255, 5, 5, 255 * alpha_scale);
        // GfxPrint_Printf(&gfx_print, "FOREVER!");

        gfx = GfxPrint_Close(&gfx_print);
        GfxPrint_Destroy(&gfx_print);
        // gSPEndDisplayList(gfx++);
        Gfx_Close(polyOpa, gfx);
        POLY_OPA_DISP = gfx;
        CLOSE_DISPS(gfxCtx);   
    }
}

void Chaos_SetConfigFlag(u32 config, u32 value)
{
    u32 dword_index = config / 32;
    u32 bit_index = config % 32;
    if(value)
    {
        gSaveContext.save.chaos.config[dword_index] |= 1 << bit_index;   
    }
    else
    {
        gSaveContext.save.chaos.config[dword_index] &= ~(1 << bit_index);   
    }
}

u32 Chaos_GetConfigFlag(u32 config)
{
    u32 dword_index = config / 32;
    u32 bit_index = config % 32;
    return gSaveContext.save.chaos.config[dword_index] & (1 << bit_index);   
}

s32 Chaos_TimeUntilMoonCrash(void)
{
    return TIME_UNTIL_MOON_CRASH - gChaosContext.moon.moon_crash_time_offset;
}

void Chaos_StartMoonCrash(void)
{
    u32 index;
    u32 has_ocarina = gSaveContext.save.saveInfo.inventory.items[SLOT_OCARINA] == ITEM_OCARINA_OF_TIME;
    u32 is_real_mooncrash = (gSaveContext.save.chaos.moon_crash_count >= 3) && has_ocarina;

    if(gSaveContext.save.chaos.moon_crash_count < 3)
    {
        gSaveContext.save.chaos.moon_crash_count++;
    }

    gChaosContext.moon.moon_crash_timer = 0;
    is_real_mooncrash &= ((Chaos_RandNext() & 0x1f) == 0x1f);

    do
    {
        u32 hours_until_crash = Chaos_RandS16Offset(0, 5);
        u32 minutes_until_crash = Chaos_RandS16Offset(0, 59);
        s32 time_until_moon_crash;
        u32 remaining_frames;
        
        if(hours_until_crash == 0 && minutes_until_crash == 0)
        {
            minutes_until_crash = Chaos_RandS16Offset(20, 39);
        }

        time_until_moon_crash = CLOCK_TIME(hours_until_crash, minutes_until_crash);
        gChaosContext.moon.moon_crash_time_offset = TIME_UNTIL_MOON_CRASH - time_until_moon_crash;
        remaining_frames = time_until_moon_crash / (R_TIME_SPEED + gSaveContext.save.timeSpeedOffset);

        if(is_real_mooncrash)
        {
            gChaosContext.moon.moon_crash_timer = 0xffffffff;
        }
        else if(remaining_frames >= 20)
        {
            gChaosContext.moon.moon_crash_timer = Chaos_RandS16Offset(20, remaining_frames - 20);
        }
    }
    while(gChaosContext.moon.moon_crash_timer == 0);

    gChaosContext.moon.need_update_bell_time = true;
}

void Chaos_ClearMoonCrash(void)
{
    gChaosContext.moon.moon_crash_time_offset = 0;
    gChaosContext.moon.moon_crash_timer = 0;
    gChaosContext.moon.need_update_bell_time = true;
}

void Chaos_NukeSnapshots()
{
    gChaosContext.link.snapshot_timer = 2;
    gChaosContext.link.bad_connection_timer = 2;
    bzero(&gChaosContext.link.player_snapshot, sizeof(gChaosContext.link.player_snapshot));
    bzero(&gChaosContext.link.child_snapshot, sizeof(gChaosContext.link.child_snapshot));
    bzero(&gChaosContext.link.arrow_snapshot, sizeof(gChaosContext.link.arrow_snapshot));
    bzero(&gChaosContext.link.parent_snapshot, sizeof(gChaosContext.link.parent_snapshot));
    bzero(&gChaosContext.link.door_snapshot, sizeof(gChaosContext.link.door_snapshot));
}

void Chaos_SnapshotChild(PlayState *play, Actor *child)
{
    if(Chaos_IsCodeActive(CHAOS_CODE_BAD_CONNECTION) && 
        gChaosContext.link.bad_connection_mode == CHAOS_BAD_CONNECTION_ROLLBACK)
    {
        ActorProfile *init_info;

        if(gChaosContext.link.child_snapshot.instance != NULL)
        {
            if(gChaosContext.link.child_snapshot.instance != child ||
               gChaosContext.link.child_snapshot.instance->id != child->id)
            {
                /* unsnapshot the previous actor */
                init_info = Actor_GetActorInit(&play->actorCtx, gChaosContext.link.child_snapshot.actor.id);
                gChaosContext.link.child_snapshot.instance->destroy = init_info->destroy;
            }
        }
        init_info = Actor_GetActorInit(&play->actorCtx, child->id);
        child->destroy = Chaos_SnapshotChildActorDestroy;
        Lib_MemCpy(&gChaosContext.link.child_snapshot.actor, child, init_info->instanceSize);
        gChaosContext.link.child_snapshot.instance = child;
    }
}

void Chaos_UnsnapshotChild(PlayState *play, Actor *child)
{
    if(gChaosContext.link.child_snapshot.instance != NULL)
    {
        ActorProfile *init_info = Actor_GetActorInit(&play->actorCtx, gChaosContext.link.child_snapshot.actor.id);
        gChaosContext.link.child_snapshot.instance->destroy = init_info->destroy;
    }

    bzero(&gChaosContext.link.child_snapshot, sizeof(gChaosContext.link.child_snapshot));
}

void Chaos_SnapshotParent(PlayState *play, Actor *parent)
{
    ActorProfile *init_info;

    if(Chaos_IsCodeActive(CHAOS_CODE_BAD_CONNECTION) && 
        gChaosContext.link.bad_connection_mode == CHAOS_BAD_CONNECTION_ROLLBACK)
    {
        if(gChaosContext.link.parent_snapshot.instance != NULL)
        {
            if(gChaosContext.link.parent_snapshot.instance != parent ||
               gChaosContext.link.parent_snapshot.instance->id != parent->id)
            {
                /* unsnapshot the previous actor */
                init_info = Actor_GetActorInit(&play->actorCtx, gChaosContext.link.parent_snapshot.actor.id);
                gChaosContext.link.parent_snapshot.instance->destroy = init_info->destroy;
            }
        }

        init_info = Actor_GetActorInit(&play->actorCtx, parent->id);
        parent->destroy = Chaos_SnapshotParentActorDestroy;
        Lib_MemCpy(&gChaosContext.link.parent_snapshot.actor, parent, init_info->instanceSize);
        gChaosContext.link.parent_snapshot.instance = parent;
    }
}

void Chaos_UnsnapshotParent(PlayState *play, Actor *parent)
{
    if(gChaosContext.link.parent_snapshot.instance != NULL)
    {
        ActorProfile *init_info = Actor_GetActorInit(&play->actorCtx, gChaosContext.link.parent_snapshot.actor.id);
        gChaosContext.link.parent_snapshot.instance->destroy = init_info->destroy;
    }

    bzero(&gChaosContext.link.parent_snapshot, sizeof(gChaosContext.link.parent_snapshot));
}

void Chaos_SnapshotMagicArrow(PlayState *play, Actor *magic_arrow)
{
    ActorProfile *init_info;

    if(Chaos_IsCodeActive(CHAOS_CODE_BAD_CONNECTION) && 
        gChaosContext.link.bad_connection_mode == CHAOS_BAD_CONNECTION_ROLLBACK)
    {
        if(gChaosContext.link.arrow_snapshot.instance != NULL)
        {
            if(gChaosContext.link.arrow_snapshot.instance != magic_arrow ||
               gChaosContext.link.arrow_snapshot.instance->id != magic_arrow->id)
            {
                /* unsnapshot the previous actor */
                init_info = Actor_GetActorInit(&play->actorCtx, gChaosContext.link.arrow_snapshot.actor.id);
                gChaosContext.link.arrow_snapshot.instance->destroy = init_info->destroy;
            }
        }
        
        init_info = Actor_GetActorInit(&play->actorCtx, magic_arrow->id);
        magic_arrow->destroy = Chaos_SnapshotMagicArrowDestroy;
        Lib_MemCpy(&gChaosContext.link.arrow_snapshot.actor, magic_arrow, init_info->instanceSize);
        gChaosContext.link.arrow_snapshot.instance = magic_arrow;
    }
}

void Chaos_UnsnapshotMagicArrow(PlayState *play, Actor *magic_arrow)
{
    if(gChaosContext.link.arrow_snapshot.instance != NULL)
    {
        ActorProfile *init_info = Actor_GetActorInit(&play->actorCtx, gChaosContext.link.arrow_snapshot.actor.id);
        gChaosContext.link.arrow_snapshot.instance->destroy = init_info->destroy;
    }

    bzero(&gChaosContext.link.arrow_snapshot, sizeof(gChaosContext.link.arrow_snapshot));
}

void Chaos_SnapshotDoor(PlayState *play, Actor *door)
{
    ActorProfile *init_info;

    if(Chaos_IsCodeActive(CHAOS_CODE_BAD_CONNECTION) && 
        gChaosContext.link.bad_connection_mode == CHAOS_BAD_CONNECTION_ROLLBACK)
    {
        if(gChaosContext.link.door_snapshot.instance != NULL)
        {
            if(gChaosContext.link.door_snapshot.instance != door ||
               gChaosContext.link.door_snapshot.instance->id != door->id)
            {
                /* unsnapshot the previous actor */
                init_info = Actor_GetActorInit(&play->actorCtx, gChaosContext.link.door_snapshot.actor.id);
                gChaosContext.link.door_snapshot.instance->destroy = init_info->destroy;
            }
        }

        init_info = Actor_GetActorInit(&play->actorCtx, door->id);
        door->destroy = Chaos_SnapshotDoorDestroy;
        Lib_MemCpy(&gChaosContext.link.door_snapshot.actor, door, init_info->instanceSize);
        gChaosContext.link.door_snapshot.instance = door;
    }
}

void Chaos_UnsnapshotDoor(PlayState *play, Actor *door)
{
    bzero(&gChaosContext.link.door_snapshot, sizeof(gChaosContext.link.door_snapshot));
}

void Chaos_SnapshotChildActorDestroy(Actor *actor, PlayState *play)
{
    ActorProfile *init = Actor_GetActorInit(&play->actorCtx, actor->id);
    gChaosContext.link.child_snapshot.instance = NULL;
    actor->destroy = init->destroy;
    actor->destroy(actor, play);
}

void Chaos_SnapshotParentActorDestroy(Actor *actor, PlayState *play)
{
    ActorProfile *init = Actor_GetActorInit(&play->actorCtx, actor->id);
    gChaosContext.link.parent_snapshot.instance = NULL;
    actor->destroy = init->destroy;
    actor->destroy(actor, play);
}

void Chaos_SnapshotMagicArrowDestroy(Actor *actor, PlayState *play)
{
    ActorProfile *init = Actor_GetActorInit(&play->actorCtx, actor->id);
    gChaosContext.link.arrow_snapshot.instance = NULL;
    actor->destroy = init->destroy;
    actor->destroy(actor, play);
}

void Chaos_SnapshotDoorDestroy(Actor *actor, PlayState *play)
{
    ActorProfile *init = Actor_GetActorInit(&play->actorCtx, actor->id);
    gChaosContext.link.door_snapshot.instance = NULL;
    actor->destroy = init->destroy;
    actor->destroy(actor, play);
}

void Chaos_LikeLikeDestroyFunction(Actor *actor, PlayState *play)
{
    Player *player = GET_PLAYER(play);
    if(player->actor.parent == actor)
    {
        /* in the ultra rare case where a like-like gets destroyed by a room transition
        after grabbing the player */
        gChaosContext.EnRr_SpitPlayer((EnRr *)actor, play);
    }

    Chaos_DestroyFunction(actor, play);
}

void Chaos_SetV044ConfigDefaults(void)
{
    Chaos_SetConfigFlag(CHAOS_CONFIG_BEER_GOGGLES_BLUR, true);
    Chaos_SetConfigFlag(CHAOS_CONFIG_IKANA_CLIMB_TREE_ACTOR_CHASE, false);
    Chaos_SetConfigFlag(CHAOS_CONFIG_STONE_TOWER_CLIMB_ACTOR_CHASE, false);
    Chaos_SetConfigFlag(CHAOS_CONFIG_DETERMINISTIC_EFFECT_RNG, false);
    Chaos_SetConfigFlag(CHAOS_CONFIG_USE_DISRUPTIVE_EFFECT_PROB, true);
}

void Chaos_SetV045ConfigDefaults(void)
{
    Chaos_SetConfigFlag(CHAOS_CONFIG_ALLOW_BEER_GOGGLES_AND_SILENT_FIELD, true);
    Chaos_SetConfigFlag(CHAOS_CONFIG_ALLOW_ENEMY_INFIGHTING, true);
}

void Chaos_SetV046ConfigDefaults(void)
{
    Chaos_SetConfigFlag(CHAOS_CONFIG_DPAD_DOWN_TO_DIE, false);
}

void Chaos_SetV050ConfigDefaults(void)
{
    Chaos_SetConfigFlag(CHAOS_CONFIG_USE_PERIODIC_EFFECT_PROB, false);
    Chaos_SetConfigFlag(CHAOS_CONFIG_RANDOM_MOUNTAIN_VILLAGE_CLIMB, false);
    Chaos_SetConfigFlag(CHAOS_CONFIG_GIVE_FIERCE_DEITY_MASK, true);
}

void Chaos_SetV053ConfigDefaults(void)
{
    Chaos_SetConfigFlag(CHAOS_CONFIG_ALLOW_UNDERWATER_OCARINA, true);
    Chaos_SetConfigFlag(CHAOS_CONFIG_RANDOM_MOUNTAIN_VILLAGE_CLIMB, true);
    Chaos_SetConfigFlag(CHAOS_CONFIG_DPAD_DOWN_TO_DIE, true);
}

void Chaos_SetConfigDefaults(void)
{
    u32 index;
    u32 current_version = gSaveContext.save.chaos.major;
    current_version <<= 8;
    current_version |= gSaveContext.save.chaos.minor;
    current_version <<= 8;
    current_version |= gSaveContext.save.chaos.patch;
    for(index = 0; index < ARRAY_COUNT(gChaosConfigSetups); index++)
    {
        struct ChaosConfigSetup *setup = gChaosConfigSetups + index;

        u32 setup_version = setup->major;
        setup_version <<= 8;
        setup_version |= setup->minor;
        setup_version <<= 8;
        setup_version |= setup->patch;

        // if(gSaveContext.save.chaos.major < setup->major || 
        //    gSaveContext.save.chaos.minor < setup->minor ||
        //    gSaveContext.save.chaos.patch < setup->patch)
        if(current_version < setup_version)
        {
            setup->config();
        }
    }
}

// CollisionPoly test_collision_poly[] = {
//     {0x0003, 0x0015 + 0x206, 0x0016 + 0x206, 0x0017 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0015 + 0x206, 0x0017 + 0x206, 0x0018 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0019 + 0x206, 0x001A + 0x206, 0x001B + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0019 + 0x206, 0x001B + 0x206, 0x001C + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x001D + 0x206, 0x001E + 0x206, 0x001F + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x001E + 0x206, 0x0020 + 0x206, 0x0021 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x001E + 0x206, 0x0021 + 0x206, 0x001F + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0021 + 0x206, 0x0022 + 0x206, 0x0023 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0021 + 0x206, 0x0023 + 0x206, 0x001F + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0020 + 0x206, 0x0024 + 0x206, 0x0021 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x001D + 0x206, 0x0025 + 0x206, 0x0026 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x001D + 0x206, 0x0026 + 0x206, 0x001E + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0020 + 0x206, 0x0027 + 0x206, 0x0028 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0020 + 0x206, 0x0028 + 0x206, 0x0024 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0029 + 0x206, 0x002A + 0x206, 0x002B + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x002A + 0x206, 0x002C + 0x206, 0x002B + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x002C + 0x206, 0x002D + 0x206, 0x002B + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x002C + 0x206, 0x002E + 0x206, 0x002D + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x002D + 0x206, 0x002F + 0x206, 0x0030 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x002D + 0x206, 0x0030 + 0x206, 0x002B + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0029 + 0x206, 0x0031 + 0x206, 0x0032 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0029 + 0x206, 0x0032 + 0x206, 0x002A + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x002C + 0x206, 0x0033 + 0x206, 0x0034 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x002C + 0x206, 0x0034 + 0x206, 0x002E + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0035 + 0x206, 0x0036 + 0x206, 0x0037 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0035 + 0x206, 0x0037 + 0x206, 0x0038 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0025 + 0x206, 0x0035 + 0x206, 0x0038 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0026 + 0x206, 0x0025 + 0x206, 0x0038 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0026 + 0x206, 0x0038 + 0x206, 0x0030 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x002F + 0x206, 0x0026 + 0x206, 0x0030 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0019 + 0x206, 0x001C + 0x206, 0x0028 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0018 + 0x206, 0x0019 + 0x206, 0x0028 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0018 + 0x206, 0x0028 + 0x206, 0x0027 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
//     {0x0003, 0x0015 + 0x206, 0x0018 + 0x206, 0x0027 + 0x206, 0x0000, 0x0000, 0x7FFF, 0x044B}, // mountain climb ladder
// };

// Vec3s test_verts[] = {
//     {    395,    768,  -1099 }, // ladder verts
//     {    395,   1088,  -1099 }, // ladder verts
//     {    315,   1088,  -1099 }, // ladder verts
//     {    315,    848,  -1099 }, // ladder verts
//     {    195,    848,  -1099 }, // ladder verts
//     {    195,   1168,  -1099 }, // ladder verts
//     {    115,   1168,  -1099 }, // ladder verts
//     {    115,    768,  -1099 }, // ladder verts
//     {    635,    368,  -1099 }, // ladder verts
//     {    555,    448,  -1099 }, // ladder verts
//     {    475,    368,  -1099 }, // ladder verts
//     {    315,    448,  -1099 }, // ladder verts
//     {    395,    368,  -1099 }, // ladder verts
//     {    395,     28,  -1099 }, // ladder verts
//     {    475,     28,  -1099 }, // ladder verts
//     {    235,    368,  -1099 }, // ladder verts
//     {    635,    768,  -1099 }, // ladder verts
//     {    555,    768,  -1099 }, // ladder verts
//     {    315,    768,  -1099 }, // ladder verts
//     {    235,    768,  -1099 }, // ladder verts
//     {    755,   1168,  -1099 }, // ladder verts
//     {    675,   1248,  -1099 }, // ladder verts
//     {    555,   1168,  -1099 }, // ladder verts
//     {    395,   1248,  -1099 }, // ladder verts
//     {    475,   1168,  -1099 }, // ladder verts
//     {    315,   1168,  -1099 }, // ladder verts
//     {    475,    768,  -1099 }, // ladder verts
//     {    555,    848,  -1099 }, // ladder verts
//     {    755,   1408,  -1099 }, // ladder verts
//     {    675,   1408,  -1099 }, // ladder verts
//     {    395,   1288,  -1099 }, // ladder verts
//     {    315,   1288,  -1099 }, // ladder verts
//     {    795,    768,  -1099 }, // ladder verts
//     {    795,    968,  -1099 }, // ladder verts
//     {    715,    968,  -1099 }, // ladder verts
//     {    715,    848,  -1099 }, // ladder verts
// };
 
void Chaos_AppendCollisionQuad(u32 type, f32 max_x, f32 min_x, f32 max_y, f32 min_y, CollisionContext *context)
{
    Vec3s *collision_vert = context->colHeader->vtxList + context->colHeader->numVertices;
    CollisionPoly *collision_poly = context->colHeader->polyList + context->colHeader->numPolygons;

    collision_vert->x = min_x + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_X_OFFSET;
    collision_vert->y = max_y + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_Y_OFFSET;
    collision_vert->z = -12   + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_Z_OFFSET;
    collision_vert++;

    collision_vert->x = min_x + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_X_OFFSET;
    collision_vert->y = min_y + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_Y_OFFSET;
    collision_vert->z = -12   + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_Z_OFFSET;
    collision_vert++;

    collision_vert->x = max_x + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_X_OFFSET;
    collision_vert->y = min_y + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_Y_OFFSET;
    collision_vert->z = -12   + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_Z_OFFSET;
    collision_vert++;

    collision_vert->x = max_x + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_X_OFFSET;
    collision_vert->y = max_y + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_Y_OFFSET;
    collision_vert->z = -12   + CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_Z_OFFSET;
    collision_vert++;

    collision_poly->type = type;
    collision_poly->flags_vIA = context->colHeader->numVertices;
    collision_poly->flags_vIB = context->colHeader->numVertices + 1;
    collision_poly->vIC = context->colHeader->numVertices + 2;
    collision_poly->normal.x = 0;
    collision_poly->normal.y = 0;
    collision_poly->normal.z = 0x7fff;
    collision_poly->dist = 0x044B;
    collision_poly++;

    collision_poly->type = type;
    collision_poly->flags_vIA = context->colHeader->numVertices + 2;
    collision_poly->flags_vIB = context->colHeader->numVertices + 3;
    collision_poly->vIC = context->colHeader->numVertices;
    collision_poly->normal.x = 0;
    collision_poly->normal.y = 0;
    collision_poly->normal.z = 0x7fff;
    collision_poly->dist = 0x044B;
    collision_poly++;

    context->colHeader->numVertices += 4;
    context->colHeader->numPolygons += 2;
}

void Chaos_RandomizeMountainVillageClimb(struct PlayState *play)
{
    f32 min_y;
    f32 max_y = CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_Y;

    f32 max_x = CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH;
    f32 min_x = -CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH;
    f32 x_center = (max_x + min_x) / 2.0f;
    u32 segment_dir = 0;

    u32 collision_vertex_count = 539;
    u32 collision_poly_count = 762;

    Vtx *segment = gMountainVillageLadderSegments[0];
    Gfx *gfx = gMountainVillageLadderDL;

    u32 climbable_type;
    u32 wall_type;
    u32 scene = gSaveContext.save.entrance >> 9;

    if(scene == ENTR_SCENE_MOUNTAIN_VILLAGE_WINTER)
    {
        climbable_type = 3;
        wall_type = 2;
    }
    // else if(scene == ENTR_SCENE_MOUNTAIN_VILLAGE_SPRING)
    // {
    //     climbable_type = 0;
    //     wall_type = 14;
    // }
    else
    {
        return;
    }

    if(!Chaos_GetConfigFlag(CHAOS_CONFIG_RANDOM_MOUNTAIN_VILLAGE_CLIMB))
    {
        gSPDisplayList(gfx++, object_yukimura_obj_DL_000870);
        gSPDisplayList(gfx++, object_yukimura_obj_DL_000890);
        gSPEndDisplayList(gfx++);
        play->colCtx.colHeader->numVertices = 539;
        play->colCtx.colHeader->numPolygons = 833;
        return;
    }

    gfx += ARRAY_COUNT(gMountainVillageLadderSetupDL);
    bcopy(gMountainVillageLadderSetupDL, gMountainVillageLadderDL, sizeof(gMountainVillageLadderSetupDL));

    play->colCtx.colHeader->numVertices = collision_vertex_count;
    play->colCtx.colHeader->numPolygons = collision_poly_count;
    
    while(max_y < CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_Y)
    {
        u32 index;

        segment[0] = gMountainVillageLadderFragment[0];
        segment[1] = gMountainVillageLadderFragment[1];
        segment[2] = gMountainVillageLadderFragment[2];
        segment[3] = gMountainVillageLadderFragment[3];
  
        switch(segment_dir)
        {
            case 0:
            {
                f32 segment_length = 2 + Rand_S16Offset(0, 3);
                f32 y_step = segment_length * CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_HEIGHT * 2.0f;
                min_y = max_y;
                max_y = min_y + y_step;

                max_x = x_center + CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH;
                min_x = x_center - CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH;

                if(max_y > CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_Y)
                {
                    max_y = CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_Y;
                    y_step = max_y - min_y;
                    segment_length = y_step / (CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_HEIGHT * 2.0f);

                    /* last segment is always vertical, so add quads to both sides of the segment */
                    if(min_x > CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_X)
                    {
                        Chaos_AppendCollisionQuad(wall_type, min_x, CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_X, max_y, min_y, &play->colCtx);
                    }

                    if(max_x < CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_X)
                    {
                        Chaos_AppendCollisionQuad(wall_type, CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_X, max_x, max_y, min_y, &play->colCtx);
                    }
                }

                segment[0].v.tc[1] *= segment_length;       
                segment[3].v.tc[1] *= segment_length;

                Chaos_AppendCollisionQuad(climbable_type, max_x, min_x, max_y, min_y, &play->colCtx);
            }
            break;

            case 1:
            {
                f32 x_step = 0;
                f32 prev_min_y = min_y;
                f32 prev_min_x = min_x;
                f32 prev_max_x = max_x;

                while(x_step == 0)
                {
                    x_step = (f32)  Rand_S16Offset(-3, 6);

                    if(x_step > 0.0f)
                    {
                        if(max_x >= CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_X)
                        {
                            x_step = 0;
                            continue;
                        }

                        min_x = max_x;
                        max_x += x_step * CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH * 2;
                        max_x = CLAMP_MAX(max_x, CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_X);
                        x_center = max_x - CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH;
                    }
                    else
                    {
                        if(min_x <= CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_X)
                        {
                            x_step = 0;
                            continue;
                        }

                        max_x = min_x;
                        min_x += x_step * CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH * 2;
                        min_x = CLAMP_MIN(min_x, CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_X);
                        x_center = min_x + CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH;
                    }
                }
                min_y = max_y - CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_HEIGHT * 2.0f;

                /* quad under horizontal segment */
                Chaos_AppendCollisionQuad(wall_type, max_x, min_x, min_y, prev_min_y, &play->colCtx);

                if(x_step < 0.0f)
                {
                    /* quad to the left of the horizontal segment and to the right of the vertical segment */
                    if(min_x > CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_X)
                    {
                        Chaos_AppendCollisionQuad(wall_type, min_x, CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_X, max_y, prev_min_y, &play->colCtx);
                    }

                    if(prev_max_x < CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_X)
                    {
                        Chaos_AppendCollisionQuad(wall_type, CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_X, prev_max_x, max_y, prev_min_y, &play->colCtx);
                    }
                }
                else
                {
                    /* quad to the right of the horizontal segment and to the left of the vertical segment */
                    if(max_x < CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_X)
                    {
                        Chaos_AppendCollisionQuad(wall_type, CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_X, max_x, max_y, prev_min_y, &play->colCtx);
                    }

                    if(prev_min_x > CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_X)
                    {
                        Chaos_AppendCollisionQuad(wall_type, prev_min_x, CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_X, max_y, prev_min_y, &play->colCtx);
                    }
                }

                Chaos_AppendCollisionQuad(wall_type, max_x, min_x, max_y, min_y, &play->colCtx);

                x_step = fabsf(x_step);
                segment[2].v.tc[0] *= (s32)x_step;
                segment[3].v.tc[0] *= (s32)x_step;
            }
            break;
        }

        segment[0].v.ob[0] = min_x;
        segment[0].v.ob[1] = max_y;

        segment[1].v.ob[0] = min_x;
        segment[1].v.ob[1] = min_y;

        segment[2].v.ob[0] = max_x;
        segment[2].v.ob[1] = min_y;

        segment[3].v.ob[0] = max_x;
        segment[3].v.ob[1] = max_y;

        segment_dir = !segment_dir;

        gSPVertex(gfx++, segment, 4, 0);
        gSP2Triangles(gfx++, 0, 1, 2, 0, 0, 2, 3, 0);
        segment += 4;
    }

    gSPEndDisplayList(gfx++);
}