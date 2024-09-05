#include "chaos_fuckery.h"
#include "functions.h"
#include "regs.h"
#include "gfxprint.h"
#include "gfxalloc.h"
#include "variables.h"
#include "z64.h"
#include "z64cutscene.h"
#include "z64scene.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

ChaosContext    gChaosContext; 
u64             gChaosRngState = 1;
u32             gDisplayEffectInfo = 0;
u32             gChaosEffectPageIndex = 0;
u32             gAcceptPageChange = 0;
u32             gPlayerAction;
u32             gRngInitialized = 0;
extern u32      gSceneIndex;
extern u32      gEntranceIndex;

PlayerAnimationHeader *gImaginaryFriendAnimations[] = {
    &gPlayerAnim_cl_tewofuru,
    &gPlayerAnim_link_kei_wait,
    &gPlayerAnim_link_keirei,
};

// struct ChaosCodeDef gChaosCodeDefs[] = {
//     /* [CHAOS_CODE_NONE]                    = */ CHAOS_CODE_DEF(0,  0,  false, 0),
//     /* [CHAOS_CODE_LOW_GRAVITY]             = */ CHAOS_CODE_DEF(10, 18, true,  0.020f),
//     /* [CHAOS_CODE_CHANGE_HEALTH]           = */ CHAOS_CODE_DEF(1,  10, false, 0.04f),
//     /* [CHAOS_CODE_CHANGE_RUPEE]            = */ CHAOS_CODE_DEF(0,  0,  true,  0.04f),
//     /* [CHAOS_CODE_ACTOR_CHASE]             = */ CHAOS_CODE_DEF(6,  15, false, 0.01f),
//     /* [CHAOS_CODE_YEET]                    = */ CHAOS_CODE_DEF(8,  17, true,  0.01f),
//     /* [CHAOS_CODE_POKE]                    = */ CHAOS_CODE_DEF(0,  0,  false, 0.017f),
//     /* [CHAOS_CODE_MOON_DANCE]              = */ CHAOS_CODE_DEF(5,  30, true,  0.006f),
//     /* [CHAOS_CODE_ONE_HIT_KO]              = */ CHAOS_CODE_DEF(8,  23, false, 0.006f),
//     /* [CHAOS_CODE_RANDOM_KNOCKBACK]        = */ CHAOS_CODE_DEF(8,  15, false, 0.007f),
//     /* [CHAOS_CODE_ICE_TRAP]                = */ CHAOS_CODE_DEF(0,  0,  false, 0.01f),
//     /* [CHAOS_CODE_TIMER_UP]                = */ CHAOS_CODE_DEF(10, 20, false, 0.011f),
//     /* [CHAOS_CODE_SHOCK]                   = */ CHAOS_CODE_DEF(0,  0,  false, 0.015f),
//     /* [CHAOS_CODE_EARTHQUAKE]              = */ CHAOS_CODE_DEF(0,  0,  true,  0.06f),
//     /* [CHAOS_CODE_BOMB_ARROWS]             = */ CHAOS_CODE_DEF(15, 25, true,  0.02f),
//     /* [CHAOS_CODE_WEIRD_ARROWS]            = */ CHAOS_CODE_DEF(15, 25, true,  0.02f),
//     /* [CHAOS_CODE_BUCKSHOT_ARROWS]         = */ CHAOS_CODE_DEF(15, 25, true,  0.02f),
//     /* [CHAOS_CODE_RANDOM_BOMB_TIMER]       = */ CHAOS_CODE_DEF(10, 15, false, 0.02f),
//     /* [CHAOS_CODE_LOVELESS_MARRIAGE]       = */ CHAOS_CODE_DEF(0,  0,  false, 0.0075f),
//     /* [CHAOS_CODE_WEIRD_UI]                = */ CHAOS_CODE_DEF(8,  15, true,  0.04f),
//     /* [CHAOS_CODE_BEER_GOGGLES]            = */ CHAOS_CODE_DEF(15, 30, true,  0.014f),
//     /* [CHAOS_CODE_CHANGE_MAGIC]            = */ CHAOS_CODE_DEF(1,  10, true,  0.04f),
//     /* [CHAOS_CODE_INVINCIBLE]              = */ CHAOS_CODE_DEF(8,  23, false, 0.005f),
//     /* [CHAOS_CODE_SYKE]                    = */ CHAOS_CODE_DEF(0,  0,  false, 0.00007f),
//     /* [CHAOS_CODE_DIE]                     = */ CHAOS_CODE_DEF(0,  0,  false, 0.000005f),
//     /* [CHAOS_CODE_TRAP_FLAP]               = */ CHAOS_CODE_DEF(10, 20, true,  0.02f),
//     /* [CHAOS_CODE_TEXTBOX]                 = */ CHAOS_CODE_DEF(0,   0, false, 0.008f),
//     /* [CHAOS_CODE_SLIPPERY_FLOORS]         = */ CHAOS_CODE_DEF(10, 20, false, 0.012f),
//     /* [CHAOS_CODE_SLOW_DOWN]               = */ CHAOS_CODE_DEF(5,  12, false, 0.006f),
//     /* [CHAOS_CODE_ENTRANCE_RANDO]          = */ CHAOS_CODE_DEF(5,  15, false, 0.001f),
//     /* [CHAOS_CODE_TERRIBLE_MUSIC]          = */ CHAOS_CODE_DEF(15, 35, true,  0.01f),
//     /* [CHAOS_CODE_INCREDIBLE_KNOCKBACK]    = */ CHAOS_CODE_DEF(10, 21, true,  0.008f),
//     /* [CHAOS_CODE_RANDOM_SCALING]          = */ CHAOS_CODE_DEF(10, 21, true,  0.007f),
//     /* [CHAOS_CODE_BIG_BROTHER]             = */ CHAOS_CODE_DEF(25, 70, true,  0.002f),
//     /* [CHAOS_CODE_OUT_OF_SHAPE]            = */ CHAOS_CODE_DEF(5, 12,  true,  0.008f),
//     /* [CHAOS_CODE_TUNIC_COLOR]             = */ CHAOS_CODE_DEF(0, 0,   false, 0.006f),
//     /* [CHAOS_CODE_WEIRD_SKYBOX]            = */ CHAOS_CODE_DEF(10, 15, true,  0.006f),
//     /* [CHAOS_CODE_SINGLE_ACTION_OWL]       = */ CHAOS_CODE_DEF(5,  15, false, 0.0005f),
//     /* [CHAOS_CODE_PLAY_OCARINA]            = */ CHAOS_CODE_DEF(0,  0,  false, 0.004f),
//     /* [CHAOS_CODE_SNEEZE]                  = */ CHAOS_CODE_DEF(5, 15,  false, 0.006f),
//     /* [CHAOS_CODE_RANDO_FIERCE_DEITY]      = */ CHAOS_CODE_DEF(25, 75, true,  0.00095f),
//     /* [CHAOS_CODE_CHICKEN_ARISE]           = */ CHAOS_CODE_DEF(25, 45, false, 0.0035f),
//     /* [CHAOS_CODE_STARFOX]                 = */ CHAOS_CODE_DEF(0, 0,   true,  0.0055f),
//     /* [CHAOS_CODE_SWAP_HEAL_AND_HURT]      = */ CHAOS_CODE_DEF(5, 25,  false, 0.003f),
//     /* [CHAOS_CODE_JUNK_ITEM]               = */ CHAOS_CODE_DEF(0, 0,   false, 0.003f),
//     /* [CHAOS_CODE_RANDOM_HEALTH_UP]        = */ CHAOS_CODE_DEF(0, 0,   false, 0.0005f),
//     /* [CHAOS_CODE_RANDOM_HEALTH_DOWN]      = */ CHAOS_CODE_DEF(0, 0,   false, 0.0005f),
//     /* [CHAOS_CODE_IMAGINARY_FRIENDS]       = */ CHAOS_CODE_DEF(5, 12,  false, 0.008f),
// };


struct ChaosCodeDef gChaosCodeDefs[] = {
    /* [CHAOS_CODE_NONE]                    = */ CHAOS_CODE_DEF(0,  0,  false,  CHAOS_CODE_RESTRICTIONS(true,   true,   true,   true,   true,   true,   true),    0),
    /* [CHAOS_CODE_LOW_GRAVITY]             = */ CHAOS_CODE_DEF(10, 18, true,   CHAOS_CODE_RESTRICTIONS(false,  false,  false,  false,  false,  false,  false),   0.020f),
    /* [CHAOS_CODE_CHANGE_HEALTH]           = */ CHAOS_CODE_DEF(1,  10, false,  CHAOS_CODE_RESTRICTIONS(false,  false,  false,  false,  false,  true,  false),   0.04f),
    /* [CHAOS_CODE_CHANGE_RUPEE]            = */ CHAOS_CODE_DEF(0,  0,  false,  CHAOS_CODE_RESTRICTIONS(false,  false,  false,  false,  false,  false,  false),   0.04f),
    /* [CHAOS_CODE_ACTOR_CHASE]             = */ CHAOS_CODE_DEF(6,  15, false,  CHAOS_CODE_RESTRICTIONS(true,   true,   true,   true,   true,   true,   true),    0.01f),
    /* [CHAOS_CODE_YEET]                    = */ CHAOS_CODE_DEF(8,  17, true,   CHAOS_CODE_RESTRICTIONS(false,  false,  false,  false,  false,  false,  false),   0.01f),
    /* [CHAOS_CODE_POKE]                    = */ CHAOS_CODE_DEF(0,  0,  false,  CHAOS_CODE_RESTRICTIONS(true,   true,   true,   true,   true,   true,   true),    0.017f),
    /* [CHAOS_CODE_MOON_DANCE]              = */ CHAOS_CODE_DEF(5,  30, true,   CHAOS_CODE_RESTRICTIONS(false,  false,  false,  false,  false,  false,  false),   0.006f),
    /* [CHAOS_CODE_ONE_HIT_KO]              = */ CHAOS_CODE_DEF(8,  23, true,   CHAOS_CODE_RESTRICTIONS(false,  false,  false,  false,  false,  false,  false),   0.006f),
    /* [CHAOS_CODE_RANDOM_KNOCKBACK]        = */ CHAOS_CODE_DEF(8,  15, false,  CHAOS_CODE_RESTRICTIONS(true,   true,   true,   true,   true,   true,   true),    0.007f),
    /* [CHAOS_CODE_ICE_TRAP]                = */ CHAOS_CODE_DEF(0,  0,  false,  CHAOS_CODE_RESTRICTIONS(true,   true,   true,   true,   true,   true,   true),    0.01f),
    /* [CHAOS_CODE_TIMER_UP]                = */ CHAOS_CODE_DEF(10, 20, false,  CHAOS_CODE_RESTRICTIONS(false,  false,  false,  false,  false,  false,  false),   0.011f),
    /* [CHAOS_CODE_SHOCK]                   = */ CHAOS_CODE_DEF(0,  0,  false,  CHAOS_CODE_RESTRICTIONS(true,   true,   true,   true,   true,   true,   true),    0.015f),
    /* [CHAOS_CODE_EARTHQUAKE]              = */ CHAOS_CODE_DEF(0,  0,  false,  0,  0.06f),
    /* [CHAOS_CODE_BOMB_ARROWS]             = */ CHAOS_CODE_DEF(15, 25, false,  0,  0.02f),
    /* [CHAOS_CODE_WEIRD_ARROWS]            = */ CHAOS_CODE_DEF(15, 25, false,  0,  0.02f),
    /* [CHAOS_CODE_BUCKSHOT_ARROWS]         = */ CHAOS_CODE_DEF(15, 25, false,  0,  0.02f),
    /* [CHAOS_CODE_RANDOM_BOMB_TIMER]       = */ CHAOS_CODE_DEF(10, 15, false,  0,  0.02f),
    /* [CHAOS_CODE_LOVELESS_MARRIAGE]       = */ CHAOS_CODE_DEF(0,  0,  false,  CHAOS_CODE_RESTRICTIONS(false, false, true, true, false, true, false), 0.0075f),
    /* [CHAOS_CODE_WEIRD_UI]                = */ CHAOS_CODE_DEF(8,  15, true,   0,  0.04f),
    /* [CHAOS_CODE_BEER_GOGGLES]            = */ CHAOS_CODE_DEF(15, 30, true,   0,  0.014f),
    /* [CHAOS_CODE_CHANGE_MAGIC]            = */ CHAOS_CODE_DEF(1,  10, false,  0,  0.04f),
    /* [CHAOS_CODE_INVINCIBLE]              = */ CHAOS_CODE_DEF(8,  23, true,   0,  0.005f),
    /* [CHAOS_CODE_SYKE]                    = */ CHAOS_CODE_DEF(0,  0,  false,  CHAOS_CODE_RESTRICTIONS(true, true, true, true, true, true, true), 0.00007f),
    /* [CHAOS_CODE_DIE]                     = */ CHAOS_CODE_DEF(0,  0,  false,  CHAOS_CODE_RESTRICTIONS(true, true, true, true, true, true, true), 0.000005f),
    /* [CHAOS_CODE_TRAP_FLAP]               = */ CHAOS_CODE_DEF(10, 20, false,  0,  0.02f),
    /* [CHAOS_CODE_TEXTBOX]                 = */ CHAOS_CODE_DEF(0,   0, false,  CHAOS_CODE_RESTRICTIONS(true, true, true, true, true, true, true), 0.008f),
    /* [CHAOS_CODE_SLIPPERY_FLOORS]         = */ CHAOS_CODE_DEF(10, 20, false,  0,  0.012f),
    /* [CHAOS_CODE_SLOW_DOWN]               = */ CHAOS_CODE_DEF(5,  12, false,  0,  0.006f),
    /* [CHAOS_CODE_ENTRANCE_RANDO]          = */ CHAOS_CODE_DEF(5,  15, false,  0,  0.006f),
    /* [CHAOS_CODE_TERRIBLE_MUSIC]          = */ CHAOS_CODE_DEF(15, 35, true,   0,  0.01f),
    /* [CHAOS_CODE_INCREDIBLE_KNOCKBACK]    = */ CHAOS_CODE_DEF(10, 21, false,  0,  0.008f),
    /* [CHAOS_CODE_RANDOM_SCALING]          = */ CHAOS_CODE_DEF(10, 21, false,  0,  0.007f),
    /* [CHAOS_CODE_BIG_BROTHER]             = */ CHAOS_CODE_DEF(25, 70, true,   0,  0.002f),
    /* [CHAOS_CODE_OUT_OF_SHAPE]            = */ CHAOS_CODE_DEF(5, 12,  false,  CHAOS_CODE_RESTRICTIONS(true, true, true, true, true, true, true),  0.008f),
    /* [CHAOS_CODE_TUNIC_COLOR]             = */ CHAOS_CODE_DEF(0, 0,   false,  0, 0.006f),
    /* [CHAOS_CODE_WEIRD_SKYBOX]            = */ CHAOS_CODE_DEF(10, 15, true,   0, 0.006f),
    /* [CHAOS_CODE_SINGLE_ACTION_OWL]       = */ CHAOS_CODE_DEF(5,  15, false,  0, 0.0005f),
    /* [CHAOS_CODE_PLAY_OCARINA]            = */ CHAOS_CODE_DEF(0,  0,  false,  CHAOS_CODE_RESTRICTIONS(true, true, true, true, true, true, true), 0.004f),
    /* [CHAOS_CODE_SNEEZE]                  = */ CHAOS_CODE_DEF(5, 15,  false,  CHAOS_CODE_RESTRICTIONS(true, true, true, true, true, true, true), 0.006f),
    /* [CHAOS_CODE_RANDO_FIERCE_DEITY]      = */ CHAOS_CODE_DEF(25, 75, false,  CHAOS_CODE_RESTRICTIONS(false, false, true, true, false, false, false), 0.00095f),
    /* [CHAOS_CODE_CHICKEN_ARISE]           = */ CHAOS_CODE_DEF(25, 45, false,  CHAOS_CODE_RESTRICTIONS(false, false, true, true, false, true, false), 0.0055f),
    /* [CHAOS_CODE_STARFOX]                 = */ CHAOS_CODE_DEF(0, 0,   false,  CHAOS_CODE_RESTRICTIONS(false, false, true, true, false, true, false), 0.0055f),
    /* [CHAOS_CODE_SWAP_HEAL_AND_HURT]      = */ CHAOS_CODE_DEF(5, 25,  false,  0, 0.003f),
    /* [CHAOS_CODE_JUNK_ITEM]               = */ CHAOS_CODE_DEF(0, 0,   false,  CHAOS_CODE_RESTRICTIONS(true, true, true, true, true, true, true), 0.003f),
    /* [CHAOS_CODE_RANDOM_HEALTH_UP]        = */ CHAOS_CODE_DEF(0, 0,   false,  0, 0.0005f),
    /* [CHAOS_CODE_RANDOM_HEALTH_DOWN]      = */ CHAOS_CODE_DEF(0, 0,   false,  0, 0.0005f),
    /* [CHAOS_CODE_IMAGINARY_FRIENDS]       = */ CHAOS_CODE_DEF(5, 12,  false,  CHAOS_CODE_RESTRICTIONS(true, true, true, true, true, true, true), 0.008f),
};
 
const char *gChaosCodeNames[] = {
    /* [CHAOS_CODE_NONE]                    = */ "NONE",
    /* [CHAOS_CODE_LOW_GRAVITY]             = */ "Low gravity",
    /* [CHAOS_CODE_CHANGE_HEALTH]           = */ "Change health",
    /* [CHAOS_CODE_CHANGE_RUPEE]            = */ "Change rupee",
    /* [CHAOS_CODE_ACTOR_CHASE]             = */ "Actor chase",
    /* [CHAOS_CODE_YEET]                    = */ "YEET",
    /* [CHAOS_CODE_POKE]                    = */ "Poke",
    /* [CHAOS_CODE_MOON_DANCE]              = */ "Moon dance",
    /* [CHAOS_CODE_ONE_HIT_KO]              = */ "One-hit KO",
    /* [CHAOS_CODE_RANDOM_KNOCKBACK]        = */ "Random knockback",
    /* [CHAOS_CODE_ICE_TRAP]                = */ "Ice trap",
    /* [CHAOS_CODE_TIMER_UP]                = */ "Timer up",
    /* [CHAOS_CODE_SHOCK]                   = */ "Shock",
    /* [CHAOS_CODE_EARTHQUAKE]              = */ "Earthquake",
    /* [CHAOS_CODE_BOMB_ARROWS]             = */ "Bomb arrows",
    /* [CHAOS_CODE_WEIRD_ARROWS]            = */ "Weird arrows",
    /* [CHAOS_CODE_BUCKSHOT_ARROWS]         = */ "Buckshot arrows",
    /* [CHAOS_CODE_RANDOM_BOMB_TIMER]       = */ "Random bomb timer",
    /* [CHAOS_CODE_LOVELESS_MARRIAGE]       = */ "Loveless marriage",
    /* [CHAOS_CODE_WEIRD_UI]                = */ "Weird UI",
    /* [CHAOS_CODE_BEER_GOGGLES]            = */ "Beer goggles",
    /* [CHAOS_CODE_CHANGE_MAGIC]            = */ "Change magic",
    /* [CHAOS_CODE_INVINCIBLE]              = */ "Invincible",
    /* [CHAOS_CODE_SYKE]                    = */ "SYKE",
    /* [CHAOS_CODE_DIE]                     = */ "DIE",
    /* [CHAOS_CODE_TRAP_FLAP]               = */ "Trap flap",
    /* [CHAOS_CODE_TEXTBOX]                 = */ "Textbox",
    /* [CHAOS_CODE_SLIPPERY_FLOORS]         = */ "Slippery floors",
    /* [CHAOS_CODE_SLOW_DOWN]               = */ "Slow down",
    /* [CHAOS_CODE_ENTRANCE_RANDO]          = */ "Entrance rando",
    /* [CHAOS_CODE_TERRIBLE_MUSIC]          = */ "Terrible music",
    /* [CHAOS_CODE_INCREDIBLE_KNOCKBACK]    = */ "Incredible knockback",
    /* [CHAOS_CODE_RANDOM_SCALING]          = */ "Random scaling",
    /* [CHAOS_CODE_BIG_BROTHER]             = */ "Big brother",
    /* [CHAOS_CODE_OUT_OF_SHAPE]            = */ "Out of shape",
    /* [CHAOS_CODE_TUNIC_COLOR]             = */ "Tunic color",
    /* [CHAOS_CODE_WEIRD_SKYBOX]            = */ "Weird skybox",
    /* [CHAOS_CODE_SINGLE_ACTION_OWL]       = */ "Single action owl",
    /* [CHAOS_CODE_PLAY_OCARINA]            = */ "Play ocarina",
    /* [CHAOS_CODE_SNEEZE]                  = */ "Sneeze",
    /* [CHAOS_CODE_RANDO_FIERCE_DEITY]      = */ "Random fierce deity",
    /* [CHAOS_CODE_CHICKEN_ARISE]           = */ "Chicken arise",
    /* [CHAOS_CODE_STARFOX]                 = */ "Starfox",
    /* [CHAOS_CODE_SWAP_HEAL_AND_HURT]      = */ "Swap heal and hurt",
    /* [CHAOS_CODE_JUNK_ITEM]               = */ "Junk item",
    /* [CHAOS_CODE_RANDOM_HEALTH_UP]        = */ "Random health up",
    /* [CHAOS_CODE_RANDOM_HEALTH_DOWN]      = */ "Random health down",
    /* [CHAOS_CODE_IMAGINARY_FRIENDS]       = */ "Imaginary friends",
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

/* xorshift* */
u32 Chaos_Rand(void)
{
    gChaosRngState ^= gChaosRngState >> 12;
    gChaosRngState ^= gChaosRngState << 25;
    gChaosRngState ^= gChaosRngState >> 27;
    return gChaosRngState * 0x2545F4914F6CDD1DULL;
} 

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
    gChaosContext.chaos_timer = 7;
    gChaosContext.code_elapsed_usec = 0;
    gChaosContext.chaos_elapsed_usec = 0;
    gChaosContext.prev_update_counter = osGetTime();
    gChaosContext.update_enabled = 0;
    // gChaosContext.cur_spawn_actor_code = CHAOS_CODE_NONE;
    // gChaosContext.spawn_actor_code_was_activated = CHAOS_CODE_NONE;
    // gChaosContext.update_spawn_actor_code = false;
    gChaosContext.queued_spawn_actor_code = CHAOS_CODE_NONE;
    gChaosContext.loaded_object_id = 0;
    gChaosContext.need_update_distribution = false;
    gChaosContext.hide_actors = 0;
    gChaosContext.link.tunic_r = 30;
    gChaosContext.link.tunic_g = 105;
    gChaosContext.link.tunic_b = 27;
    gChaosContext.link.beer_alpha = 0;
    gChaosContext.link.syke = false; 
    gChaosContext.link.out_of_shape_speed_scale = 1.0f;
    gChaosContext.link.sneeze_speed_scale = 1.0f;
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
    gChaosContext.link.sneeze_state = CHAOS_SNEEZE_STATE_NONE;
    gChaosContext.link.fierce_deity_counter = 0;
    gChaosContext.link.imaginary_friends_anim_index = 0;

    gChaosContext.chicken.cucco.niwType = NIW_TYPE_CHAOS;
    gChaosContext.chicken.cucco.actor.update = EnNiw_Update;
    gChaosContext.chicken.cucco.actor.draw = EnNiw_Draw;
    
    for(index = 0; index < CHAOS_CODE_LAST; index++)
    {
        gChaosContext.active_code_indices[index] = INVALID_CODE_INDEX;
        gChaosContext.enabled_code_indices[index] = INVALID_CODE_INDEX;
    }
}

u8 Chaos_RandomCode(void)
{
    u32 rand_index = Chaos_Rand();
    s32 search_index = gChaosContext.enabled_code_count >> 1;
    s32 search_index_offset = search_index >> 1;

    if(search_index_offset <= 0)
    {
        search_index_offset = 1;
    }

    while(search_index >= 0 && search_index < gChaosContext.enabled_code_count)
    {
        struct ChaosCodeSlot *slot = gChaosContext.enabled_codes + search_index;
        if(rand_index < slot->range_start)
        {
            search_index -= search_index_offset;
        }
        else if(rand_index > slot->range_end)
        {
            search_index += search_index_offset;
        }
        else if(rand_index >= slot->range_start && 
                rand_index <= slot->range_end)
        {
            return slot->code;
        }

        if(search_index_offset > 1)
        {
            search_index_offset >>= 1;
        }
    }

    return 0;
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
    u8                  next_code; 
    u8                  next_code_timer;
    struct ChaosCode *  last_code = NULL;
    Player *            player = GET_PLAYER(playstate);
    Camera *            camera = Play_GetCamera(playstate, CAM_ID_MAIN);

    if(!gRngInitialized)
    {
        gChaosRngState = Rand_Next();
        gRngInitialized = true;
    }

    if(gChaosContext.need_update_distribution)
    {
        Chaos_UpdateCodeDistribution();
        gChaosContext.need_update_distribution = false;
    }

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

            if(!(chaos_actor->actor->flags & ACTOR_FLAG_40))
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

            if(code_elapsed_seconds >= code->timer)
            {
                Chaos_DeactivateCodeAtIndex(slot_index);
                continue;
            }

            code->timer -= code_elapsed_seconds;
            slot_index++;
        }
        gChaosContext.code_elapsed_usec -= code_elapsed_seconds * 1000000;

        if(chaos_elapsed_seconds > 0)
        {
            gChaosContext.chaos_elapsed_usec -= chaos_elapsed_seconds * 1000000;

            if(chaos_elapsed_seconds > gChaosContext.chaos_timer)
            {
                gChaosContext.chaos_timer = 0;
            }
            else
            {
                gChaosContext.chaos_timer -= chaos_elapsed_seconds;
            }  
        
            if(gChaosContext.chaos_timer == 0)  
            {
                u32 attempts = 0;
                struct ChaosCodeDef *code_def;
                code_add_result = CHAOS_ADD_RESULT_NO_SLOTS;
                gChaosContext.chaos_timer = MIN_CHAOS_TIMER + Rand_Next() % (MAX_CHAOS_TIMER - MIN_CHAOS_TIMER);
                next_rand = Rand_Next();
                do
                {
                    if(gChaosContext.queued_spawn_actor_code != CHAOS_CODE_NONE &&
                        Object_IsLoaded(&playstate->objectCtx, playstate->objectCtx.chaos_keep_slot))
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
                    next_code_timer = gChaosCodeDefs[next_code].min_time + next_rand % (gChaosCodeDefs[next_code].max_time - gChaosCodeDefs[next_code].min_time);
                    attempts++;

                    code_def = gChaosCodeDefs + next_code;

                    if(code_def->restrictions & gChaosContext.effect_restrictions)
                    {
                        continue;
                    }

                    /* TODO: create a effect exclusion list for each effect, to avoid a bunch of branching here */
                    switch(next_code)
                    {
                        case CHAOS_CODE_TERRIBLE_MUSIC:
                            if(Chaos_IsCodeActive(CHAOS_CODE_BEER_GOGGLES))
                            {
                                /* beer goggles also screws with sound tempo/frequency */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_BEER_GOGGLES:
                            if(Chaos_IsCodeActive(CHAOS_CODE_TERRIBLE_MUSIC))
                            {
                                /* terrible music also screws with sound tempo/frequency */
                                continue;
                            }
                        break;

                        // case CHAOS_CODE_POKE:
                        // case CHAOS_CODE_RANDOM_KNOCKBACK:
                        // case CHAOS_CODE_ICE_TRAP:
                        // case CHAOS_CODE_SHOCK:
                        // case CHAOS_CODE_SYKE:
                        // case CHAOS_CODE_DIE:
                        // case CHAOS_CODE_ACTOR_CHASE:
                        // case CHAOS_CODE_PLAY_OCARINA:
                        case CHAOS_CODE_OUT_OF_SHAPE:
                        
                            // if((player->stateFlags2 & PLAYER_STATE2_80) || (player->stateFlags1 & PLAYER_STATE1_800000) ||
                            //     (player->stateFlags1 & PLAYER_STATE1_20000000) || camera->setting == CAM_SET_BOAT_CRUISE)
                            // {
                            //     /* if the player has been grabbed, is mounted on epona, 
                            //     riding the boat or time is stopped, don't spawn any of those 
                            //     effects to avoid leaving the player in an inconsistent state */
                            //     gChaosContext.queued_spawn_actor_code = CHAOS_CODE_NONE;
                            //     continue;
                            // }
                            // else 
                            
                            if(next_code == CHAOS_CODE_OUT_OF_SHAPE)
                            {
                                if(Chaos_IsCodeActive(CHAOS_CODE_SNEEZE))
                                {
                                    continue;
                                }
                            }

                            break;

                        case CHAOS_CODE_LOVELESS_MARRIAGE:
                        case CHAOS_CODE_CHICKEN_ARISE:
                        case CHAOS_CODE_STARFOX:
                            if(gChaosContext.queued_spawn_actor_code == CHAOS_CODE_NONE &&
                                    gChaosContext.actors.spawned_actors == 0)
                            {
                                if(next_code == CHAOS_CODE_LOVELESS_MARRIAGE && 
                                    gChaosContext.loaded_object_id != OBJECT_RR)
                                {
                                    if(Chaos_IsCodeActive(CHAOS_CODE_CHICKEN_ARISE))
                                    {
                                        continue;
                                    }

                                    Object_RequestOverwrite(&playstate->objectCtx, playstate->objectCtx.chaos_keep_slot, OBJECT_RR);
                                    gChaosContext.loaded_object_id = OBJECT_RR;
                                    gChaosContext.queued_spawn_actor_code = CHAOS_CODE_LOVELESS_MARRIAGE;
                                    continue;
                                }
                                else if(next_code == CHAOS_CODE_CHICKEN_ARISE && 
                                        gChaosContext.loaded_object_id != OBJECT_NIW)
                                {
                                    Object_RequestOverwrite(&playstate->objectCtx, playstate->objectCtx.chaos_keep_slot, OBJECT_NIW);
                                    gChaosContext.loaded_object_id = OBJECT_NIW;
                                    gChaosContext.queued_spawn_actor_code = CHAOS_CODE_CHICKEN_ARISE;
                                    continue;
                                }
                                else if(next_code == CHAOS_CODE_STARFOX && 
                                        gChaosContext.loaded_object_id != OBJECT_ARWING)
                                {
                                    if(Chaos_IsCodeActive(CHAOS_CODE_CHICKEN_ARISE))
                                    {
                                        continue;
                                    }
                                    
                                    Object_RequestOverwrite(&playstate->objectCtx, playstate->objectCtx.chaos_keep_slot, OBJECT_ARWING);
                                    gChaosContext.loaded_object_id = OBJECT_ARWING;
                                    gChaosContext.queued_spawn_actor_code = CHAOS_CODE_STARFOX;
                                    continue;
                                }
                            }

                        break;

                        case CHAOS_CODE_SNEEZE:
                            if(Chaos_IsCodeActive(CHAOS_CODE_OUT_OF_SHAPE))
                            {
                                continue;
                            }
                        break;

                        case CHAOS_CODE_BUCKSHOT_ARROWS:
                            if(Chaos_IsCodeActive(CHAOS_CODE_BOMB_ARROWS))
                            {
                                /* bomb arrows and buckshot arrows don't mix very well */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_BOMB_ARROWS:
                            if(Chaos_IsCodeActive(CHAOS_CODE_BUCKSHOT_ARROWS))
                            {
                                /* bomb arrows and buckshot arrows don't mix very well */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_ONE_HIT_KO:
                            if(Chaos_IsCodeActive(CHAOS_CODE_CHANGE_HEALTH) || 
                               Chaos_IsCodeActive(CHAOS_CODE_INVINCIBLE))
                            {
                                /* changing health would one-hit the player or not have any 
                                effect at all, so don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_CHANGE_HEALTH:
                            if(Chaos_IsCodeActive(CHAOS_CODE_ONE_HIT_KO) || 
                               Chaos_IsCodeActive(CHAOS_CODE_INVINCIBLE) || 
                               Chaos_IsCodeActive(CHAOS_CODE_SWAP_HEAL_AND_HURT))
                            {
                                /* changing health would one-hit the player or not have any 
                                effect at all, so don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_INVINCIBLE:
                            if(Chaos_IsCodeActive(CHAOS_CODE_ONE_HIT_KO) || 
                               Chaos_IsCodeActive(CHAOS_CODE_CHANGE_HEALTH))
                            {
                                /* making the player invicible now would make both codes 
                                not have an effect, so don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_SWAP_HEAL_AND_HURT:
                            if(Chaos_IsCodeActive(CHAOS_CODE_CHANGE_HEALTH))
                            {
                                /* could potentially one-hit the player, so
                                don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_BIG_BROTHER:
                            if(Chaos_IsCodeActive(CHAOS_CODE_MOON_DANCE))
                            {
                                /* the moon won't be able to face the player if 
                                it starts dancing, so do it later */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_MOON_DANCE:
                            if(Chaos_IsCodeActive(CHAOS_CODE_BIG_BROTHER))
                            {
                                /* the moon won't be able to dance if it's
                                trying to face the player, so do it later */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_RANDOM_HEALTH_DOWN:
                            if(gSaveContext.save.saveInfo.playerData.healthCapacity == 16)
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
                                // last_code->data = Rand_Next() % CHAOS_MOON_MOVE_LAST;
                                gChaosContext.moon.moon_dance = Rand_Next() % CHAOS_MOON_MOVE_LAST;
                            }
                            while(gChaosContext.moon.moon_dance == 0);
                        break;

                        case CHAOS_CODE_RANDOM_KNOCKBACK:
                            /* deal knockback immediately */
                            gChaosContext.link.random_knockback_timer = 1;
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

                        case CHAOS_CODE_TUNIC_COLOR:
                        {
                            u32 color = Rand_Next() % 0xffffff;
                            gChaosContext.link.tunic_r = color;
                            gChaosContext.link.tunic_g = color >> 8;
                            gChaosContext.link.tunic_b = color >> 16;
                        }
                        break;

                        case CHAOS_CODE_CHICKEN_ARISE:
                            gChaosContext.chicken.cucco.attackNiwSpawnTimer = 0;
                            gChaosContext.chicken.cucco.attackNiwCount = 0;
                        break;
                    }
                }
            }
        }
    }
}

#define ENABLED_EFFECTS_PER_PAGE    25
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

        if(gChaosEffectPageIndex == 0)
        {
            GfxPrint_Printf(&gfx_print, "Chaos timer: %d", (u32)gChaosContext.chaos_timer);

            slot_index = 0;
            while(slot_index < gChaosContext.active_code_count)
            {
                struct ChaosCode *code = gChaosContext.active_codes + slot_index;

                // if(code->timer > 0)
                // {
                GfxPrint_SetPos(&gfx_print, 1, y_pos++);
                GfxPrint_Printf(&gfx_print, "%s: %d", gChaosCodeNames[code->code], (u32)code->timer);
                // }
                slot_index++;
            }

            // y_pos = 1;
            // GfxPrint_SetPos(&gfx_print, 20, y_pos++);
            // GfxPrint_Printf(&gfx_print, "%08x", player->stateFlags1);
            // GfxPrint_SetPos(&gfx_print, 20, y_pos++);
            // GfxPrint_Printf(&gfx_print, "%08x", player->stateFlags2);
            // GfxPrint_SetPos(&gfx_print, 20, y_pos++);
            // GfxPrint_Printf(&gfx_print, "%08x", player->stateFlags3);
            // GfxPrint_SetPos(&gfx_print, 20, y_pos++);
            // GfxPrint_Printf(&gfx_print, "%02x %02x", (s8)player->csId, (u8)player->csAction);
            // GfxPrint_SetPos(&gfx_print, 20, y_pos++);
            // GfxPrint_Printf(&gfx_print, "%04x", camera->setting);
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
            GfxPrint_Printf(&gfx_print, "Player state stuff");
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "stateFlags1: %08x", player->stateFlags1);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "stateFlags2: %08x", player->stateFlags2);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "stateFlags3: %08x", player->stateFlags3);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "csId: %02x csAction: %02x", player->csId, player->csAction);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "actionVar1: %d", player->av1.actionVar1);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "actionVar2: %d", player->av2.actionVar2);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "Action: %d", gPlayerAction);
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "Scene: %d, room: %d, hazard: %d", scene, playstate->roomCtx.curRoom.num, Player_GetEnvironmentalHazard(playstate));
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "effect restrictions: %x", gChaosContext.effect_restrictions);
            // GfxPrint_Printf(&gfx_print, "%x(%f) %x(%f) %x", player->skelAnime.animation, player->skelAnime.curFrame,
            //                                                 player->skelAnimeUpper.animation, player->skelAnimeUpper.curFrame,
            //                                                 gImaginaryFriendAnimations[gChaosContext.link.imaginary_friends_anim_index]);
            // GfxPrint_Printf(&gfx_print, "%x %x %x", gImaginaryFriendAnimations[0], gImaginaryFriendAnimations[1], gImaginaryFriendAnimations[2]);
            // GfxPrint_Printf(&gfx_print, "%04x %04x", camera->setting, ((s32)player->currentYaw) & 0xffff);
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
}

u8 Chaos_ActivateCode(u8 code, u8 seconds)
{
    if(Chaos_IsCodeActive(code))
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

u8 Chaos_IsCodeActive(u8 code)
{
    u32 slot_index = gChaosContext.active_code_indices[code];
    return slot_index < gChaosContext.active_code_count && (gChaosCodeDefs[code].always_active || 
        (gChaosContext.effect_restrictions & gChaosCodeDefs[code].restrictions) == 0);
}

struct ChaosCode *Chaos_GetCode(u8 code)
{
    u32 slot_index = gChaosContext.active_code_indices[code];

    if(slot_index < gChaosContext.active_code_count && (gChaosCodeDefs[code].always_active || 
        (gChaosContext.effect_restrictions & gChaosCodeDefs[code].restrictions) == 0))
    {
        return gChaosContext.active_codes + slot_index;
    }

    return NULL;
}

void Chaos_EnableCode(u8 code, f32 prob_scale)
{
    u32 same_as_previous_index = false;
    if(!Chaos_IsCodeEnabled(code) && !(gChaosCodeDefs[code].restrictions & gChaosContext.effect_restrictions))
    {
        u32 index = gChaosContext.enabled_code_count;
        gChaosContext.enabled_code_count++;

        gChaosContext.need_update_distribution |= gChaosContext.enabled_codes[index].code != code;
        gChaosContext.enabled_codes[index].code = code;
        gChaosContext.enabled_codes[index].prob_scale = prob_scale;
        gChaosContext.enabled_code_indices[code] = index;
        // gChaosContext.need_update_distribution = true;
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

    u8 enable_update = gSaveContext.gameMode == GAMEMODE_NORMAL &&
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

u8 Chaos_EffectRestrictions(struct PlayState *play)
{
    Player *player = GET_PLAYER(play);
    Camera *camera = Play_GetCamera(play, CAM_ID_MAIN);
    u8 restriction_flags = 0;

    // if(gSaveContext.gameMode != GAMEMODE_NORMAL)
    // {
    //     restriction_flags = CHAOS_CODE_RESTRICTION_FLAG_ALL;
    // }
    // else
    // {
    if(CutsceneManager_GetCurrentCsId() != CS_ID_NONE)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_CUTSCENE;
    }

    if(play->transitionMode != TRANS_MODE_OFF)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_TRANSITION;
    }

    // if(play->pauseCtx.state != PAUSE_STATE_OFF)
    // {
    //     restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_PAUSED;
    // }

    if(player->stateFlags1 & PLAYER_STATE1_DEAD)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_DEAD;
    }

    if(camera->setting == CAM_SET_BOAT_CRUISE)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_BOAT_RIDE;
    }

    if(player->stateFlags1 & PLAYER_STATE1_TIME_STOPPED)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_TIME_STOPPED;
    }

    if(player->stateFlags2 & PLAYER_STATE2_GRABBED)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_GRABBED;
    }

    if(player->stateFlags1 & PLAYER_STATE1_MOUNTED)
    {
        restriction_flags |= CHAOS_CODE_RESTRICTION_FLAG_EPONA_RIDE;
    }
    // }

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

            Actor_ChangeCategory(play, &play->actorCtx, actor, ACTORCAT_CHAOS);

            if(actor_id == ACTOR_EN_ARWING)
            {
                chaos_actor->timer *= 10;
            }
            // chaos_actor->destroy = actor->destroy;
            // actor->destroy = Chaos_DestroyFunction;
            gChaosContext.actors.spawned_actors++;
        }
    }

    return actor;
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
            Actor_ChangeCategory(play, &play->actorCtx, actor, ACTORCAT_CHAOS);
            // chaos_actor->destroy = actor->destroy;
            // actor->destroy = Chaos_DestroyFunction;
            gChaosContext.actors.spawned_actors++;
        }
    }

    return actor;
}

void Chaos_DestroyFunction(struct Actor *actor, struct PlayState *play)
{
    // u32 index;

    // if(actor != NULL)
    // {
    //     for(index = 0; index < gChaosContext.actors.spawned_actors; index++)
    //     {
    //         if(gChaosContext.actors.slots[index].actor == actor)
    //         {
    //             struct ChaosActor *chaos_actor = gChaosContext.actors.slots + index;
    //             actor->destroy = chaos_actor->destroy;
    //             actor->destroy(actor, play);
    //             return;
    //         }
    //     }    
    // }
}

void Chaos_KillActorAtIndex(u32 index)
{
    if(index < gChaosContext.actors.spawned_actors)
    {
        struct ChaosActor *slot = gChaosContext.actors.slots + index;

        if(slot->actor != NULL)
        {
            Actor_Kill(slot->actor);
            Actor_Destroy(slot->actor);
        }

        Chaos_DropActorAtIndex(index);

        // slot->actor = NULL;
        // slot->timer = 0;

        // gChaosContext.actors.spawned_actors--;

        // if(index < gChaosContext.actors.spawned_actors)
        // {
        //     gChaosContext.actors.slots[index] = gChaosContext.actors.slots[gChaosContext.actors.spawned_actors];
        // }
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

void Chaos_DropActor(Actor *actor)
{
    u32 index;

    if(actor != NULL)
    {
        for(index = 0; index < gChaosContext.actors.spawned_actors; index++)
        {
            if(gChaosContext.actors.slots[index].actor == actor)
            {
                Chaos_DropActorAtIndex(index);
                break;
            }
        }    
    }

    return;
}

void Chaos_ClearActors(void)
{
    gChaosContext.actors.spawned_actors = 0;
    gChaosContext.chicken.cucco.attackNiwCount = 0;
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
                    if(!CHECK_WEEKEVENTREG(WEEKEVENTREG_WOODFALL_TEMPLE_RISEN) && entrance_index == 1)
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
    u32 scene_index;
    u32 room_index;
    Player *player = GET_PLAYER(this);
    Camera *camera = Play_GetCamera(this, CAM_ID_MAIN);
    u32 has_ocarina = gSaveContext.save.saveInfo.inventory.items[SLOT_OCARINA] == ITEM_OCARINA_OF_TIME;
    u32 can_be_pushed_around = !(player->stateFlags2 & PLAYER_STATE2_80) && 
                        !(player->stateFlags1 & (PLAYER_STATE1_800000 | PLAYER_STATE1_20000000)) &&
                        camera->setting != CAM_SET_BOAT_CRUISE;
    
    Chaos_ClearEnabledCodes();
    Chaos_EnableCode(CHAOS_CODE_YEET, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_MOON_DANCE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_ONE_HIT_KO, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_TIMER_UP, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_EARTHQUAKE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_WEIRD_UI, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_BEER_GOGGLES, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_INVINCIBLE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_TRAP_FLAP, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_SLIPPERY_FLOORS, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_SLOW_DOWN, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_TERRIBLE_MUSIC, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_INCREDIBLE_KNOCKBACK, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_RANDOM_SCALING, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_BIG_BROTHER, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_WEIRD_SKYBOX, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_CHANGE_HEALTH, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_CHANGE_RUPEE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_SWAP_HEAL_AND_HURT, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_RANDOM_FIERCE_DEITY, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_JUNK_ITEM, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_RANDOM_HEALTH_UP, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_RANDOM_HEALTH_DOWN, 1.0f);

    if(gSaveContext.save.saveInfo.playerData.isMagicAcquired)
    {
        Chaos_EnableCode(CHAOS_CODE_CHANGE_MAGIC, 1.0f);
    }

    if(gSaveContext.save.saveInfo.inventory.items[SLOT_BOW] == ITEM_BOW)
    {
        Chaos_EnableCode(CHAOS_CODE_BOMB_ARROWS, 1.0f);
        Chaos_EnableCode(CHAOS_CODE_BUCKSHOT_ARROWS, 1.0f);
        Chaos_EnableCode(CHAOS_CODE_WEIRD_ARROWS, 1.0f);
    }

    if(gSaveContext.save.saveInfo.inventory.items[SLOT_BOMB] == ITEM_BOMB ||
       gSaveContext.save.saveInfo.inventory.items[SLOT_BOMBCHU] == ITEM_BOMBCHU)
    {
        Chaos_EnableCode(CHAOS_CODE_RANDOM_BOMB_TIMER, 1.0f);
    }
 
    // if(can_be_pushed_around)
    // {
    // u32 index;
    /* if the player has been grabbed, is mounted on epona, 
    riding the boat or time is stopped, don't spawn any of those 
    effects to avoid leaving the player in an inconsistent state */
    Chaos_EnableCode(CHAOS_CODE_POKE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_RANDOM_KNOCKBACK, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_ICE_TRAP, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_SHOCK, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_SYKE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_DIE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_ACTOR_CHASE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_OUT_OF_SHAPE, 1.0f);

    Chaos_EnableCode(CHAOS_CODE_LOVELESS_MARRIAGE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_CHICKEN_ARISE, 1.0f);
    Chaos_EnableCode(CHAOS_CODE_STARFOX, 1.0f);

    if(has_ocarina)
    {
        Chaos_EnableCode(CHAOS_CODE_PLAY_OCARINA, 1.0f);
    }
    // }

    scene_index = gSaveContext.save.entrance >> 9;
    if(scene_index != ENTR_SCENE_ODOLWAS_LAIR && scene_index != ENTR_SCENE_IGOS_DU_IKANAS_LAIR &&
       scene_index != ENTR_SCENE_PIRATES_FORTRESS_INTERIOR || (scene_index == ENTR_SCENE_PIRATES_FORTRESS_INTERIOR &&
       this->roomCtx.curRoom.num != 0 && this->roomCtx.curRoom.num != 1 && this->roomCtx.curRoom.num != 2))
    {
        /* some bosses jump around and may fall out of bounds of low gravity is active */
        Chaos_EnableCode(CHAOS_CODE_LOW_GRAVITY, 1.0f);
    }
    else
    {
        Chaos_DeactivateCode(CHAOS_CODE_LOW_GRAVITY);
    }

    // if(scene_index != ENTR_SCENE_PIRATES_FORTRESS_INTERIOR || (this->roomCtx.curRoom.num != 14 && this->roomCtx.curRoom.num != 12))
    // {
    //     /* random fierce deity disallows changing to zora, so only enable it if the player isn't in an underwater section */
    //     Chaos_EnableCode(CHAOS_CODE_RANDOM_FIERCE_DEITY);
    // }
    // else
    // {
    //     Chaos_DeactivateCode(CHAOS_CODE_RANDOM_FIERCE_DEITY);
    // }

    if(has_ocarina)
    {
        if(!Map_IsInBossScene(this) && CHECK_WEEKEVENTREG(WEEKEVENTREG_ENTERED_SOUTH_CLOCK_TOWN))
        {
            /* don't enable entrance rando until the player enters south clock town.
            Also don't enable it in boss rooms as it could interact badly with the warp 
            out */
            Chaos_EnableCode(CHAOS_CODE_ENTRANCE_RANDO, 1.0f);
            // Chaos_EnableCode(CHAOS_CODE_SINGLE_ACTION_OWL);
            Chaos_UpdateEntrances(this);
        }
        else
        {
            Chaos_DeactivateCode(CHAOS_CODE_ENTRANCE_RANDO);
        }
    }
}