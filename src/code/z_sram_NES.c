#include "z64save.h"
#include "global.h"

#include "sys_flashrom.h"
#include "z64horse.h"
#include "overlays/gamestates/ovl_file_choose/z_file_select.h"
#include "chaos_fuckery.h"
#include "fault.h"

extern ChaosContext gChaosContext;

void Sram_SyncWriteToFlash(SramContext* sramCtx, s32 curPage, s32 numPages);
// void Sram_InvalidateOwlSave(SramContext* sramCtx, s32 fileNum);
void func_80147414(SramContext* sramCtx, s32 fileNum, s32 arg2);

#define CHECK_NEWF(newf)                                                                                 \
    ((newf)[0] != 'Z' || (newf)[1] != 'E' || (newf)[2] != 'L' || (newf)[3] != 'D' || (newf)[4] != 'A' || \
     (newf)[5] != '3')
#define CHECK_NEWF_VALID(newf) (!CHECK_NEWF(newf))

typedef struct PersistentCycleSceneFlags {
    /* 0x0 */ u32 switch0;
    /* 0x4 */ u32 switch1;
    /* 0x8 */ u32 chest;
    /* 0xC */ u32 collectible;
} PersistentCycleSceneFlags; // size = 0x10

#define PERSISTENT_CYCLE_FLAGS_SET(switch0, switch1, chest, collectible) { switch0, switch1, chest, collectible },
#define PERSISTENT_CYCLE_FLAGS_NONE PERSISTENT_CYCLE_FLAGS_SET(0, 0, 0, 0)

#define DEFINE_SCENE(_name, _enumValue, _textId, _drawConfig, _restrictionFlags, persistentCycleFlags) \
    persistentCycleFlags
#define DEFINE_SCENE_UNSET(_enumValue) PERSISTENT_CYCLE_FLAGS_NONE

/**
 * Array of bitwise flags which won't be turned off on a cycle reset (will persist between cycles)
 */
PersistentCycleSceneFlags sPersistentCycleSceneFlags[SCENE_MAX] = {
#include "tables/scene_table.h"
};

#undef DEFINE_SCENE
#undef DEFINE_SCENE_UNSET

// Each flag has 2 bits to store persistence over the three-day reset cycle
// Only 1 of these bits need to be set to persist (Values 1, 2, 3).
// Therefore, the final game does not distinguish between these two macros in use
#define PERSISTENT_WEEKEVENTREG(flag) (3 << (2 * BIT_FLAG_TO_SHIFT(flag)))
#define PERSISTENT_WEEKEVENTREG_ALT(flag) (2 << (2 * BIT_FLAG_TO_SHIFT(flag)))

// weekEventReg flags which will be not be cleared on a cycle reset
//! @note The index of the flag in this array must be the same to its index in the WeekeventReg array
//!       Only the mask is read from the `PERSISTENT_` macros.
u16 sPersistentCycleWeekEventRegs[ARRAY_COUNT(gSaveContext.save.saveInfo.weekEventReg)] = {
    /*  0 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_TERMINA_FIELD) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_IKANA_GRAVEYARD) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_ROMANI_RANCH) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_GORMAN_TRACK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_MOUNTAIN_VILLAGE_WINTER) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_GORON_SHRINE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_SNOWHEAD),
    /*  1 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_SOUTHERN_SWAMP_POISONED) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_WOODFALL) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_DEKU_PALACE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_GREAT_BAY_COAST) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_PIRATES_FORTRESS) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_ZORA_HALL) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_WATERFALL_RAPIDS) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_IKANA_CANYON),
    /*  2 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_IKANA_CASTLE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_STONE_TOWER) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_STONE_TOWER_INVERTED) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_EAST_CLOCK_TOWN) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_WEST_CLOCK_TOWN) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_NORTH_CLOCK_TOWN) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_WOODFALL_TEMPLE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_SNOWHEAD_TEMPLE),
    /*  3 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_PIRATES_FORTRESS_EXTERIOR) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_STONE_TOWER_TEMPLE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_STONE_TOWER_TEMPLE_INVERTED) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_THE_MOON) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_MOON_DEKU_TRIAL) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_MOON_GORON_TRIAL) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_MOON_ZORA_TRIAL) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_03_80),
    /*  4 */ 0,
    /*  5 */ 0,
    /*  6 */ 0,
    /*  7 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_ENTERED_WOODFALL_TEMPLE_PRISON),
    /*  8 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_DOGGY_RACETRACK_HEART_PIECE),
    /*  9 */ 0,
    /* 10 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_BANK_WALLET_UPGRADE),
    /* 11 */ 0,
    /* 12 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_KOTAKE_BOTTLE),
    /* 13 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_OCEANSIDE_WALLET_UPGRADE),
    /* 14 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_DEKU_PLAYGROUND_HEART_PIECE),
    /* 15 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_15_20),
    /* 16 */ 0,
    /* 17 */ 0,
    /* 18 */ 0,
    /* 19 */ 0,
    /* 20 */ 0,
    /* 21 */ 0,
    /* 22 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_ALIENS_BOTTLE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_HONEY_AND_DARLING_HEART_PIECE),
    /* 23 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_GREAT_SPIN_ATTACK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_BEAVER_RACE_BOTTLE),
    /* 24 */ PERSISTENT_WEEKEVENTREG_ALT(WEEKEVENTREG_24_02) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_24_80),
    /* 25 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_BEAVER_BROS_HEART_PIECE),
    /* 26 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_26_40),
    /* 27 */ 0,
    /* 28 */ 0,
    /* 29 */ 0,
    /* 30 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_30_10) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_30_20) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_30_40) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_30_80),
    /* 31 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_31_01) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_31_02) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_31_04) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_31_20),
    /* 32 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_SEAHORSE_HEART_PIECE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_SWAMP_SHOOTING_GALLERY_HEART_PIECE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_TOWN_SHOOTING_GALLERY_HEART_PIECE),
    /* 33 */ 0,
    /* 34 */ 0,
    /* 35 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_CLOCK_TOWN) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_WOODFALL) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_SNOWHEAD) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_ROMANI_RANCH) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_GREAT_BAY) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_STONE_TOWER) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_FROG_CHOIR_HEART_PIECE),
    /* 36 */ 0,
    /* 37 */ 0,
    /* 38 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_38_20),
    /* 39 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_EVAN_HEART_PIECE),
    /* 40 */ 0,
    /* 41 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_GORON_RACE_BOTTLE),
    /* 42 */ 0,
    /* 43 */ 0,
    /* 44 */ 0,
    /* 45 */ 0,
    /* 46 */ 0,
    /* 47 */ 0,
    /* 48 */ 0,
    /* 49 */ 0,
    /* 50 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_50_02) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_50_04),
    /* 51 */ PERSISTENT_WEEKEVENTREG_ALT(WEEKEVENTREG_51_04),
    /* 52 */ 0,
    /* 53 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_BUSINESS_SCRUB_HEART_PIECE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_53_40),
    /* 54 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_SPIRIT_HOUSE_HEART_PIECE),
    /* 55 */ 0,
    /* 56 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_MARINE_RESEARCH_LAB_FISH_HEART_PIECE),
    /* 57 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_57_08),
    /* 58 */ 0,
    /* 59 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_59_04) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_BANK_HEART_PIECE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_SWAMP_SHOOTING_GALLERY_QUIVER_UPGRADE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_TOWN_SHOOTING_GALLERY_QUIVER_UPGRADE),
    /* 60 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_MAYOR_HEART_PIECE),
    /* 61 */ 0,
    /* 62 */ 0,
    /* 63 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_SWORDSMANS_SCHOOL_HEART_PIECE),
    /* 64 */ 0,
    /* 65 */ 0,
    /* 66 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_ANJU) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_KAFEI) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_CURIOSITY_SHOP_MAN) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_BOMB_SHOP_LADY) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_ROMANI) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_CREMIA) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_MAYOR_DOTOUR) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_MADAME_AROMA),
    /* 67 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_TOTO) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_GORMAN) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_POSTMAN) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_ROSA_SISTERS) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_TOILET_HAND) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_ANJUS_GRANDMOTHER) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_KAMARO) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_GROG),
    /* 68 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_GORMAN_BROTHERS) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_SHIRO) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_GURU_GURU) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_BOMBERS) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ROOM_KEY) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_PROMISED_MIDNIGHT_MEETING) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_PROMISED_TO_MEET_KAFEI) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_LETTER_TO_KAFEI),
    /* 69 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_DEPOSITED_LETTER_TO_KAFEI) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_PENDANT_OF_MEMORIES) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_DELIVERED_PENDANT_OF_MEMORIES) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_ESCAPED_SAKONS_HIDEOUT) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_PROMISED_TO_HELP_WITH_ALIENS) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_DEFENDED_AGAINST_ALIENS) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ALIENS_BOTTLE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_ESCORTED_CREMIA),
    /* 70 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ROMANIS_MASK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_KEATON_MASK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_PRIORITY_MAIL) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_DELIVERED_PRIORITY_MAIL) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_LEARNED_SECRET_CODE) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BOMBERS_NOTEBOOK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_MAYOR_HP) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ROSA_SISTERS_HP),
    /* 71 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_TOILET_HAND_HP) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_GRANDMA_SHORT_STORY_HP) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_GRANDMA_LONG_STORY_HP) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_POSTMAN_HP) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_KAFEIS_MASK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ALL_NIGHT_MASK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BUNNY_HOOD) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_GAROS_MASK),
    /* 72 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_CIRCUS_LEADERS_MASK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_POSTMANS_HAT) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_COUPLES_MASK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BLAST_MASK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_KAMAROS_MASK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_STONE_MASK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BREMEN_MASK) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_72_80),
    /* 73 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_73_08),
    /* 74 */ 0,
    /* 75 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_ROSA_SISTERS_HEART_PIECE),
    /* 76 */ 0,
    /* 77 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_POSTMAN_COUNTING_GAME_HEART_PIECE),
    /* 78 */ 0,
    /* 79 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_KEATON_HEART_PIECE),
    /* 80 */ 0,
    /* 81 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_81_08),
    /* 82 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_FISHERMANS_JUMPING_GAME_HEART_PIECE),
    /* 83 */ 0,
    /* 84 */ 0,
    /* 85 */ 0,
    /* 86 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_86_80),
    /* 87 */
    PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_87_04) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_87_08) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_87_10) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_87_20) |
        PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_87_40) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_87_80),
    /* 88 */ 0,
    /* 89 */ 0,
    /* 90 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_GOSSIP_STONE_GROTTO_HEART_PIECE),
    /* 91 */ 0,
    /* 92 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_92_80),
    /* 93 */ PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_93_04) | PERSISTENT_WEEKEVENTREG(WEEKEVENTREG_93_08),
    /* 94 */ 0,
    /* 95 */ 0,
    /* 96 */ 0,
    /* 97 */ 0,
    /* 98 */ 0,
    /* 99 */ 0,
};

// Unused remnant values from OoT. Not the correct sizes in MM.
#define OOT_SAVECONTEXT_SIZE 0x1428
#define OOT_SLOT_SIZE (OOT_SAVECONTEXT_SIZE + 0x28)
#define OOT_SRAM_HEADER_SIZE 0x10
#define OOT_SLOT_OFFSET(index) (OOT_SRAM_HEADER_SIZE + 0x10 + (index * OOT_SLOT_SIZE))

// used in other files
u32 gSramSlotOffsets[] = {
    OOT_SLOT_OFFSET(0),
    OOT_SLOT_OFFSET(1),
    OOT_SLOT_OFFSET(2),
    // the latter three saves are backup saves for the former saves (in OoT)
    OOT_SLOT_OFFSET(3),
    OOT_SLOT_OFFSET(4),
    OOT_SLOT_OFFSET(5),
};

u8 gAmmoItems[ITEM_NUM_SLOTS] = {
    ITEM_NONE,           // SLOT_OCARINA
    ITEM_BOW,            // SLOT_BOW
    ITEM_NONE,           // SLOT_ARROW_FIRE
    ITEM_NONE,           // SLOT_ARROW_ICE
    ITEM_NONE,           // SLOT_ARROW_LIGHT
    ITEM_NONE,           // SLOT_TRADE_DEED
    ITEM_BOMB,           // SLOT_BOMB
    ITEM_BOMBCHU,        // SLOT_BOMBCHU
    ITEM_DEKU_STICK,     // SLOT_DEKU_STICK
    ITEM_DEKU_NUT,       // SLOT_DEKU_NUT
    ITEM_MAGIC_BEANS,    // SLOT_MAGIC_BEANS
    ITEM_NONE,           // SLOT_TRADE_KEY_MAMA
    ITEM_POWDER_KEG,     // SLOT_POWDER_KEG
    ITEM_PICTOGRAPH_BOX, // SLOT_PICTOGRAPH_BOX
    ITEM_NONE,           // SLOT_LENS_OF_TRUTH
    ITEM_NONE,           // SLOT_HOOKSHOT
    ITEM_NONE,           // SLOT_SWORD_GREAT_FAIRY
    ITEM_NONE,           // SLOT_TRADE_COUPLE
    ITEM_NONE,           // SLOT_BOTTLE_1
    ITEM_NONE,           // SLOT_BOTTLE_2
    ITEM_NONE,           // SLOT_BOTTLE_3
    ITEM_NONE,           // SLOT_BOTTLE_4
    ITEM_NONE,           // SLOT_BOTTLE_5
    ITEM_NONE,           // SLOT_BOTTLE_6
};

// Stores flash start page number
s32 gFlashSaveStartPages[] = {
    0,     // File 1 New Cycle Save
    0x40,  // File 1 New Cycle Save Backup
    0x80,  // File 2 New Cycle Save
    0xC0,  // File 2 New Cycle Save Backup
    0x100, // File 1 Owl Save
    0x180, // File 1 Owl Save Backup
    0x200, // File 2 Owl Save
    0x280, // File 2 Owl Save Backup
    0x300, // Sram Header (SaveOptions)
    0x380, // Sram Header Backup (SaveOptions)
};

// Flash rom number of pages
s32 gFlashSaveNumPages[] = {
    0x40, // File 1 New Cycle Save
    0x40, // File 1 New Cycle Save Backup
    0x40, // File 2 New Cycle Save
    0x40, // File 2 New Cycle Save Backup
    0x80, // File 1 Owl Save
    0x80, // File 1 Owl Save Backup
    0x80, // File 2 Owl Save
    0x80, // File 2 Owl Save Backup
    1,    // Sram Header (SaveOptions)
    1,    // Sram Header Backup (SaveOptions)
};

// Flash rom number of pages on very first time Player enters South Clock Town from the Clock Tower
s32 gFlashSpecialSaveNumPages[] = {
    0x80, // File 1 New Cycle Save
    0x80, // File 1 New Cycle Save Backup
    0x80, // File 2 New Cycle Save
    0x80, // File 2 New Cycle Save Backup
    0x80, // File 1 Owl Save
    0x80, // File 1 Owl Save Backup
    0x80, // File 2 Owl Save
    0x80, // File 2 Owl Save Backup
    1,    // Sram Header (SaveOptions)
    1,    // Sram Header Backup (SaveOptions)
};

// Owl Save flash rom start page number
s32 gFlashOwlSaveStartPages[] = {
    0x100, // File 1 Owl Save
    0x180, // File 1 Owl Save Backup
    0x200, // File 2 Owl Save
    0x280, // File 2 Owl Save Backup
};

// Owl Save flash rom number of pages
s32 gFlashOwlSaveNumPages[] = {
    0x80, // File 1 Owl Save
    0x80, // File 1 Owl Save Backup
    0x80, // File 2 Owl Save
    0x80, // File 2 Owl Save Backup
};

// Save Options Sram Header flash rom start page number
s32 gFlashOptionsSaveStartPages[] = {
    0x300, // Sram Header (SaveOptions)
    0x380, // Sram Header Backup (SaveOptions)
};

// Save Options Sram Header flash rom number of pages
s32 gFlashOptionsSaveNumPages[] = {
    1, // Sram Header (SaveOptions)
    1, // Sram Header Backup (SaveOptions)
};

// Flash rom actual size needed
s32 gFlashSaveSizes[] = {
    sizeof(Save),                   // size = 0x100C - File 1 New Cycle Save
    sizeof(Save),                   // size = 0x100C - File 1 New Cycle Save Backup
    sizeof(Save),                   // size = 0x100C - File 2 New Cycle Save
    sizeof(Save),                   // size = 0x100C - File 2 New Cycle Save Backup
    offsetof(SaveContext, fileNum), // size = 0x3CA0 - File 1 Owl Save
    offsetof(SaveContext, fileNum), // size = 0x3CA0 - File 1 Owl Save Backup
    offsetof(SaveContext, fileNum), // size = 0x3CA0 - File 2 Owl Save
    offsetof(SaveContext, fileNum), // size = 0x3CA0 - File 2 Owl Save Backup
};

struct SaveFileInfo
{
    u32 save_size;
    u32 main_start_page;
    u32 main_page_count;
    u32 backup_start_page;
    u32 backup_page_count;

} gSaveFileInfo[] = {
    {sizeof(Save),                   0x00,  0x40, 0x40,  0x40},
    {sizeof(Save),                   0x80,  0x40, 0xC0,  0x40},
    {offsetof(SaveContext, fileNum), 0x100, 0x80, 0x180, 0x80},
    {offsetof(SaveContext, fileNum), 0x200, 0x80, 0x280, 0x80},
    {sizeof(SaveOptions),            0x300, 0x01, 0x380, 0x01}
};

// Bit Flag array in which sBitFlags8[n] is (1 << n)
u8 sBitFlags8[] = {
    (1 << 0), (1 << 1), (1 << 2), (1 << 3), (1 << 4), (1 << 5), (1 << 6), (1 << 7),
};

u16 D_801F6AF0;
u8 D_801F6AF2;

void Sram_ActivateOwl(u8 owlWarpId) {
    SET_OWL_STATUE_ACTIVATED(owlWarpId);

    if (gSaveContext.save.saveInfo.playerData.owlWarpId == OWL_WARP_NONE) {
        gSaveContext.save.saveInfo.playerData.owlWarpId = owlWarpId;
    }
}

void Sram_ClearHighscores(void) {
    HS_SET_BOAT_ARCHERY_HIGH_SCORE(19);
    HS_SET_HIGH_SCORE_3_LOWER(10);
    HS_SET_HORSE_BACK_BALLOON_TIME(SECONDS_TO_TIMER(60));
    HS_SET_TOWN_SHOOTING_GALLERY_HIGH_SCORE(39);
    HS_SET_SWAMP_SHOOTING_GALLERY_HIGH_SCORE(10);

    gSaveContext.save.saveInfo.dekuPlaygroundHighScores[0] = SECONDS_TO_TIMER(75);
    gSaveContext.save.saveInfo.dekuPlaygroundHighScores[1] = SECONDS_TO_TIMER(75);
    gSaveContext.save.saveInfo.dekuPlaygroundHighScores[2] = SECONDS_TO_TIMER(76);
}

/**
 * Clears specific weekEventReg flags. Used by the "Dawn of the First Day" message
 */
void Sram_ClearFlagsAtDawnOfTheFirstDay(void) {
    CLEAR_WEEKEVENTREG(WEEKEVENTREG_55_02);
    CLEAR_WEEKEVENTREG(WEEKEVENTREG_90_01);
    CLEAR_WEEKEVENTREG(WEEKEVENTREG_89_40);
    CLEAR_WEEKEVENTREG(WEEKEVENTREG_89_08);
    CLEAR_WEEKEVENTREG(WEEKEVENTREG_85_80);
}

/**
 * Used by Song of Time (when clicking "Yes") and (indirectly) by the "Dawn of the New Day" cutscene
 */
void Sram_SaveEndOfCycle(PlayState* play) {
    s16 sceneId;
    s32 j;
    s32 i;
    u8 slot;
    u8 item;

    gSaveContext.save.timeSpeedOffset = 0;
    gSaveContext.save.eventDayCount = 0;
    gSaveContext.save.day = 0;
    gSaveContext.save.time = CLOCK_TIME(6, 0) - 1;
    // gSaveContext.save.is_crash_save = false;

    gSaveContext.save.saveInfo.playerData.threeDayResetCount++;
    if (gSaveContext.save.saveInfo.playerData.threeDayResetCount > 999) {
        gSaveContext.save.saveInfo.playerData.threeDayResetCount = 999;
    }

    sceneId = Play_GetOriginalSceneId(play->sceneId);
    Play_SaveCycleSceneFlags(play);

    play->actorCtx.sceneFlags.chest &= sPersistentCycleSceneFlags[sceneId].chest;
    play->actorCtx.sceneFlags.switches[0] &= sPersistentCycleSceneFlags[sceneId].switch0;
    play->actorCtx.sceneFlags.switches[1] &= sPersistentCycleSceneFlags[sceneId].switch1;
    play->actorCtx.sceneFlags.collectible[0] &= sPersistentCycleSceneFlags[sceneId].collectible;
    play->actorCtx.sceneFlags.clearedRoom = 0;

    for (i = 0; i < SCENE_MAX; i++) {
        gSaveContext.cycleSceneFlags[i].switch0 =
            gSaveContext.cycleSceneFlags[i].switch0 & sPersistentCycleSceneFlags[i].switch0;
        gSaveContext.cycleSceneFlags[i].switch1 =
            gSaveContext.cycleSceneFlags[i].switch1 & sPersistentCycleSceneFlags[i].switch1;
        gSaveContext.cycleSceneFlags[i].chest =
            gSaveContext.cycleSceneFlags[i].chest & sPersistentCycleSceneFlags[i].chest;
        gSaveContext.cycleSceneFlags[i].collectible =
            gSaveContext.cycleSceneFlags[i].collectible & sPersistentCycleSceneFlags[i].collectible;
        gSaveContext.cycleSceneFlags[i].clearedRoom = 0;
        gSaveContext.save.saveInfo.permanentSceneFlags[i].unk_14 = 0;
        gSaveContext.save.saveInfo.permanentSceneFlags[i].rooms = 0;
    }

    for (; i < ARRAY_COUNT(gSaveContext.cycleSceneFlags); i++) {
        gSaveContext.cycleSceneFlags[i].chest = 0;
        gSaveContext.cycleSceneFlags[i].switch0 = 0;
        gSaveContext.cycleSceneFlags[i].switch1 = 0;
        gSaveContext.cycleSceneFlags[i].clearedRoom = 0;
        gSaveContext.cycleSceneFlags[i].collectible = 0;
    }

    for (i = 0; i < ARRAY_COUNT(gSaveContext.masksGivenOnMoon); i++) {
        gSaveContext.masksGivenOnMoon[i] = 0;
    }

    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_84_20)) {
        Inventory_DeleteItem(ITEM_MASK_FIERCE_DEITY, SLOT(ITEM_MASK_FIERCE_DEITY));
    }

    for (i = 0; i < ARRAY_COUNT(sPersistentCycleWeekEventRegs); i++) {
        u16 isPersistentBits = sPersistentCycleWeekEventRegs[i];

        for (j = 0; j < ARRAY_COUNT(sBitFlags8); j++) {
            if (!(isPersistentBits & 3)) {
                gSaveContext.save.saveInfo.weekEventReg[i] =
                    ((void)0, gSaveContext.save.saveInfo.weekEventReg[i]) & (0xFF ^ sBitFlags8[j]);
            }
            isPersistentBits >>= 2;
        }
    }

    for (i = 0; i < ARRAY_COUNT(gSaveContext.eventInf); i++) {
        gSaveContext.eventInf[i] = 0;
    }

    CLEAR_EVENTINF(EVENTINF_THREEDAYRESET_LOST_RUPEES);
    CLEAR_EVENTINF(EVENTINF_THREEDAYRESET_LOST_BOMB_AMMO);
    CLEAR_EVENTINF(EVENTINF_THREEDAYRESET_LOST_NUT_AMMO);
    CLEAR_EVENTINF(EVENTINF_THREEDAYRESET_LOST_STICK_AMMO);
    CLEAR_EVENTINF(EVENTINF_THREEDAYRESET_LOST_ARROW_AMMO);

    if (gSaveContext.save.saveInfo.playerData.rupees != 0) {
        SET_EVENTINF(EVENTINF_THREEDAYRESET_LOST_RUPEES);
    }

    if (INV_CONTENT(ITEM_BOMB) == ITEM_BOMB) {
        item = INV_CONTENT(ITEM_BOMB);
        if (AMMO(item) != 0) {
            SET_EVENTINF(EVENTINF_THREEDAYRESET_LOST_BOMB_AMMO);
        }
    }
    if (INV_CONTENT(ITEM_DEKU_NUT) == ITEM_DEKU_NUT) {
        item = INV_CONTENT(ITEM_DEKU_NUT);
        if (AMMO(item) != 0) {
            SET_EVENTINF(EVENTINF_THREEDAYRESET_LOST_NUT_AMMO);
        }
    }
    if (INV_CONTENT(ITEM_DEKU_STICK) == ITEM_DEKU_STICK) {
        item = INV_CONTENT(ITEM_DEKU_STICK);
        if (AMMO(item) != 0) {
            SET_EVENTINF(EVENTINF_THREEDAYRESET_LOST_STICK_AMMO);
        }
    }
    if (INV_CONTENT(ITEM_BOW) == ITEM_BOW) {
        item = INV_CONTENT(ITEM_BOW);
        if (AMMO(item) != 0) {
            SET_EVENTINF(EVENTINF_THREEDAYRESET_LOST_ARROW_AMMO);
        }
    }

    for (i = 0; i < ITEM_NUM_SLOTS; i++) {
        if (gAmmoItems[i] != ITEM_NONE) {
            if ((gSaveContext.save.saveInfo.inventory.items[i] != ITEM_NONE) && (i != SLOT_PICTOGRAPH_BOX)) {
                item = gSaveContext.save.saveInfo.inventory.items[i];
                AMMO(item) = 0;
            }
        }
    }

    for (i = SLOT_BOTTLE_1; i <= SLOT_BOTTLE_6; i++) {
        // Check for all bottled items
        if (gSaveContext.save.saveInfo.inventory.items[i] >= ITEM_POTION_RED) {
            if (gSaveContext.save.saveInfo.inventory.items[i] <= ITEM_OBABA_DRINK) {
                for (j = EQUIP_SLOT_C_LEFT; j <= EQUIP_SLOT_C_RIGHT; j++) {
                    if (GET_CUR_FORM_BTN_ITEM(j) == gSaveContext.save.saveInfo.inventory.items[i]) {
                        SET_CUR_FORM_BTN_ITEM(j, ITEM_BOTTLE);
                        Interface_LoadItemIconImpl(play, j);
                    }
                }
                gSaveContext.save.saveInfo.inventory.items[i] = ITEM_BOTTLE;
            }
        }
    }

    REMOVE_QUEST_ITEM(QUEST_PICTOGRAPH);

    if (gSaveContext.save.saveInfo.playerData.health < 0x30) {
        gSaveContext.save.saveInfo.playerData.health = 0x30;
    }

    if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) <= EQUIP_VALUE_SWORD_RAZOR) {
        SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_KOKIRI);

        if (CUR_FORM == 0) {
            if ((STOLEN_ITEM_1 >= ITEM_SWORD_GILDED) || (STOLEN_ITEM_2 >= ITEM_SWORD_GILDED)) {
                CUR_FORM_EQUIP(EQUIP_SLOT_B) = ITEM_SWORD_GILDED;
                SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_GILDED);
            } else {
                CUR_FORM_EQUIP(EQUIP_SLOT_B) = ITEM_SWORD_KOKIRI;
            }
        } else {
            if ((STOLEN_ITEM_1 >= ITEM_SWORD_GILDED) || (STOLEN_ITEM_2 >= ITEM_SWORD_GILDED)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_SWORD_GILDED;
                SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_GILDED);
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_SWORD_KOKIRI;
            }
        }
    }

    if ((STOLEN_ITEM_1 == ITEM_SWORD_GREAT_FAIRY) || (STOLEN_ITEM_2 == ITEM_SWORD_GREAT_FAIRY)) {
        INV_CONTENT(ITEM_SWORD_GREAT_FAIRY) = ITEM_SWORD_GREAT_FAIRY;
    }

    if (STOLEN_ITEM_1 == ITEM_BOTTLE) {
        slot = SLOT(ITEM_BOTTLE);
        for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
            if (gSaveContext.save.saveInfo.inventory.items[slot + i] == ITEM_NONE) {
                gSaveContext.save.saveInfo.inventory.items[slot + i] = ITEM_BOTTLE;
                break;
            }
        }
    }

    if (STOLEN_ITEM_2 == ITEM_BOTTLE) {
        slot = SLOT(ITEM_BOTTLE);
        for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
            if (gSaveContext.save.saveInfo.inventory.items[slot + i] == ITEM_NONE) {
                gSaveContext.save.saveInfo.inventory.items[slot + i] = ITEM_BOTTLE;
                break;
            }
        }
    }

    SET_STOLEN_ITEM_1(STOLEN_ITEM_NONE);
    SET_STOLEN_ITEM_2(STOLEN_ITEM_NONE);

    Inventory_DeleteItem(ITEM_OCARINA_FAIRY, SLOT_TRADE_DEED);
    Inventory_DeleteItem(ITEM_SLINGSHOT, SLOT_TRADE_KEY_MAMA);
    Inventory_DeleteItem(ITEM_LONGSHOT, SLOT_TRADE_COUPLE);

    for (j = EQUIP_SLOT_C_LEFT; j <= EQUIP_SLOT_C_RIGHT; j++) {
        if ((GET_CUR_FORM_BTN_ITEM(j) >= ITEM_MOONS_TEAR) && (GET_CUR_FORM_BTN_ITEM(j) <= ITEM_PENDANT_OF_MEMORIES)) {
            SET_CUR_FORM_BTN_ITEM(j, ITEM_NONE);
            Interface_LoadItemIconImpl(play, j);
        }
    }

    gSaveContext.save.saveInfo.skullTokenCount &= ~0xFFFF0000;
    gSaveContext.save.saveInfo.skullTokenCount &= ~0x0000FFFF;
    gSaveContext.save.saveInfo.unk_EA0 = 0;

    gSaveContext.save.saveInfo.alienInfo[0] = 0;
    gSaveContext.save.saveInfo.alienInfo[1] = 0;
    gSaveContext.save.saveInfo.alienInfo[2] = 0;
    gSaveContext.save.saveInfo.alienInfo[3] = 0;
    gSaveContext.save.saveInfo.alienInfo[4] = 0;
    gSaveContext.save.saveInfo.alienInfo[5] = 0;
    gSaveContext.save.saveInfo.alienInfo[6] = 0;

    Sram_ClearHighscores();

    for (i = 0; i < 8; i++) {
        gSaveContext.save.saveInfo.inventory.dungeonItems[i] &= (u8)~1; // remove boss key
        DUNGEON_KEY_COUNT(i) = 0;
        gSaveContext.save.saveInfo.inventory.strayFairies[i] = 0;
    }

    gSaveContext.save.saveInfo.playerData.rupees = 0;
    gSaveContext.save.saveInfo.scarecrowSpawnSongSet = false;
    gSaveContext.powderKegTimer = 0;
    gSaveContext.unk_1014 = 0;
    gSaveContext.jinxTimer = 0;
    gSaveContext.rupeeAccumulator = 0;

    Horse_ResetHorseData(play);
    Chaos_ClearMoonCrash();

    // gSaveContext.save.is_crash_save = false;
}

void Sram_SaveAtGameCrashTime(PlayState *play)
{
    gSaveContext.save.is_crash_save = true;
    gSaveContext.save.isOwlSave = false;

    // if(!CHECK_QUEST_ITEM(QUEST_SONG_ELEGY))
    // {
    //     CLEAR_EVENTINF(EVENTINF_INTRO_CS_WATCHED_IGOS_DU_IKANA);
    // }

    Play_SaveCycleSceneFlags(play);
    func_8014546C(&play->sramCtx);

    Sram_SetFlashPagesOwlSave(&play->sramCtx, gFlashOwlSaveStartPages[gSaveContext.fileNum * 2],
                                gFlashOwlSaveNumPages[gSaveContext.fileNum * 2]);
    Sram_StartWriteToFlashOwlSave(&play->sramCtx);
}

void Sram_IncrementDay(void) {
    if (CURRENT_DAY <= 3) {
        gSaveContext.save.day++;
        gSaveContext.save.eventDayCount++;
    }

    gSaveContext.save.saveInfo.bombersCaughtNum = 0;
    gSaveContext.save.saveInfo.bombersCaughtOrder[0] = 0;
    gSaveContext.save.saveInfo.bombersCaughtOrder[1] = 0;
    gSaveContext.save.saveInfo.bombersCaughtOrder[2] = 0;
    gSaveContext.save.saveInfo.bombersCaughtOrder[3] = 0;
    gSaveContext.save.saveInfo.bombersCaughtOrder[4] = 0;

    CLEAR_WEEKEVENTREG(WEEKEVENTREG_73_10);
    CLEAR_WEEKEVENTREG(WEEKEVENTREG_85_02);
}

u16 Sram_CalcChecksum(void* data, size_t count) {
    u8* dataPtr = data;
    u16 chkSum = 0;

    while (count-- > 0) {
        chkSum += *dataPtr;
        dataPtr++;
    }
    return chkSum;
}

// Resets `Save` substruct
void Sram_ResetSave(void) {
    gSaveContext.save.entrance = ENTRANCE(CUTSCENE, 0);
    gSaveContext.save.equippedMask = 0;
    gSaveContext.save.isFirstCycle = false;
    gSaveContext.save.unk_06 = 0;
    gSaveContext.save.linkAge = 0;
    gSaveContext.save.isNight = false;
    gSaveContext.save.timeSpeedOffset = 0;
    gSaveContext.save.snowheadCleared = 0;
    gSaveContext.save.hasTatl = false;
    gSaveContext.save.isOwlSave = false;

    bzero(&gSaveContext.save.saveInfo, sizeof(SaveInfo));
}

/**
 * Initializes with random values the following fields:
 * - lotteryCodes
 * - spiderHouseMaskOrder
 * - bomberCode
 */
void Sram_GenerateRandomSaveFields(void) {
    s32 randBombers;
    s16 sp2A;
    s16 pad;
    s16 i;
    s16 j;
    s32 k;
    s16 randSpiderHouse;

    Sram_ClearHighscores();

    gSaveContext.save.saveInfo.lotteryCodes[0][0] = Rand_S16Offset(0, 10);
    gSaveContext.save.saveInfo.lotteryCodes[0][1] = Rand_S16Offset(0, 10);
    gSaveContext.save.saveInfo.lotteryCodes[0][2] = Rand_S16Offset(0, 10);
    gSaveContext.save.saveInfo.lotteryCodes[1][0] = Rand_S16Offset(0, 10);
    gSaveContext.save.saveInfo.lotteryCodes[1][1] = Rand_S16Offset(0, 10);
    gSaveContext.save.saveInfo.lotteryCodes[1][2] = Rand_S16Offset(0, 10);
    gSaveContext.save.saveInfo.lotteryCodes[2][0] = Rand_S16Offset(0, 10);
    gSaveContext.save.saveInfo.lotteryCodes[2][1] = Rand_S16Offset(0, 10);
    gSaveContext.save.saveInfo.lotteryCodes[2][2] = Rand_S16Offset(0, 10);

    // Needed to match...
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {}
    }

    i = 0;
    sp2A = Rand_S16Offset(0, 16) & 3;
    k = 6;
    while (i != k) {
        randSpiderHouse = Rand_S16Offset(0, 16) & 3;
        if (sp2A != randSpiderHouse) {
            gSaveContext.save.saveInfo.spiderHouseMaskOrder[i] = randSpiderHouse;
            i++;
            sp2A = randSpiderHouse;
        }
    }

    do {
        randBombers = Rand_S16Offset(0, 6);
    } while ((randBombers <= 0) || (randBombers >= 6));

    gSaveContext.save.saveInfo.bomberCode[0] = randBombers;

    i = 1;
    while (i != 5) {
        k = false;

        do {
            randBombers = Rand_S16Offset(0, 6);
        } while ((randBombers <= 0) || (randBombers >= 6));

        sp2A = 0;
        do {
            if (randBombers == gSaveContext.save.saveInfo.bomberCode[sp2A]) {
                k = true;
            }
            sp2A++;
        } while (sp2A < i);

        if (k == false) {
            gSaveContext.save.saveInfo.bomberCode[i] = randBombers;
            i++;
        }
    }
}

SavePlayerData sSaveDefaultPlayerData = {
    { '\0', '\0', '\0', '\0', '\0', '\0' },             // newf
    0,                                                  // threeDayResetCount
    { 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E }, // playerName "        "
    0x30,                                               // healthCapacity
    0x30,                                               // health
    0,                                                  // magicLevel
    MAGIC_NORMAL_METER,                                 // magic
    0,                                                  // rupees
    0,                                                  // swordHealth
    0,                                                  // tatlTimer
    false,                                              // isMagicAcquired
    false,                                              // isDoubleMagicAcquired
    0,                                                  // doubleDefense
    0,                                                  // unk_1F
    OWL_WARP_NONE,                                      // owlWarpId
    0,                                                  // owlActivationFlags
    0xFF,                                               // unk_24
    SCENE_SPOT00,                                       // savedSceneId
};

ItemEquips sSaveDefaultItemEquips = {
    {
        { ITEM_SWORD_KOKIRI, ITEM_NONE, ITEM_NONE, ITEM_NONE },
        { ITEM_SWORD_KOKIRI, ITEM_NONE, ITEM_NONE, ITEM_NONE },
        { ITEM_SWORD_KOKIRI, ITEM_NONE, ITEM_NONE, ITEM_NONE },
        { ITEM_FD, ITEM_NONE, ITEM_NONE, ITEM_NONE },
    },
    {
        { SLOT_OCARINA, SLOT_NONE, SLOT_NONE, SLOT_NONE },
        { SLOT_NONE, SLOT_NONE, SLOT_NONE, SLOT_NONE },
        { SLOT_NONE, SLOT_NONE, SLOT_NONE, SLOT_NONE },
        { SLOT_NONE, SLOT_NONE, SLOT_NONE, SLOT_NONE },
    },
    0x11,
};

Inventory sSaveDefaultInventory = {
    // items
    {
        ITEM_NONE, // SLOT_OCARINA
        ITEM_NONE, // SLOT_BOW
        ITEM_NONE, // SLOT_ARROW_FIRE
        ITEM_NONE, // SLOT_ARROW_ICE
        ITEM_NONE, // SLOT_ARROW_LIGHT
        ITEM_NONE, // SLOT_TRADE_DEED
        ITEM_NONE, // SLOT_BOMB
        ITEM_NONE, // SLOT_BOMBCHU
        ITEM_NONE, // SLOT_DEKU_STICK
        ITEM_NONE, // SLOT_DEKU_NUT
        ITEM_NONE, // SLOT_MAGIC_BEANS
        ITEM_NONE, // SLOT_TRADE_KEY_MAMA
        ITEM_NONE, // SLOT_POWDER_KEG
        ITEM_NONE, // SLOT_PICTOGRAPH_BOX
        ITEM_NONE, // SLOT_LENS_OF_TRUTH
        ITEM_NONE, // SLOT_HOOKSHOT
        ITEM_NONE, // SLOT_SWORD_GREAT_FAIRY
        ITEM_NONE, // SLOT_TRADE_COUPLE
        ITEM_NONE, // SLOT_BOTTLE_1
        ITEM_NONE, // SLOT_BOTTLE_2
        ITEM_NONE, // SLOT_BOTTLE_3
        ITEM_NONE, // SLOT_BOTTLE_4
        ITEM_NONE, // SLOT_BOTTLE_5
        ITEM_NONE, // SLOT_BOTTLE_6
        ITEM_NONE, // SLOT_MASK_POSTMAN
        ITEM_NONE, // SLOT_MASK_ALL_NIGHT
        ITEM_NONE, // SLOT_MASK_BLAST
        ITEM_NONE, // SLOT_MASK_STONE
        ITEM_NONE, // SLOT_MASK_GREAT_FAIRY
        ITEM_NONE, // SLOT_MASK_DEKU
        ITEM_NONE, // SLOT_MASK_KEATON
        ITEM_NONE, // SLOT_MASK_BREMEN
        ITEM_NONE, // SLOT_MASK_BUNNY
        ITEM_NONE, // SLOT_MASK_DON_GERO
        ITEM_NONE, // SLOT_MASK_SCENTS
        ITEM_NONE, // SLOT_MASK_GORON
        ITEM_NONE, // SLOT_MASK_ROMANI
        ITEM_NONE, // SLOT_MASK_CIRCUS_LEADER
        ITEM_NONE, // SLOT_MASK_KAFEIS_MASK
        ITEM_NONE, // SLOT_MASK_COUPLE
        ITEM_NONE, // SLOT_MASK_TRUTH
        ITEM_NONE, // SLOT_MASK_ZORA
        ITEM_NONE, // SLOT_MASK_KAMARO
        ITEM_NONE, // SLOT_MASK_GIBDO
        ITEM_NONE, // SLOT_MASK_GARO
        ITEM_NONE, // SLOT_MASK_CAPTAIN
        ITEM_NONE, // SLOT_MASK_GIANT
        ITEM_NONE, // SLOT_MASK_FIERCE_DEITY
    },
    // ammo
    {
        0, // SLOT_OCARINA
        0, // SLOT_BOW
        0, // SLOT_ARROW_FIRE
        0, // SLOT_ARROW_ICE
        0, // SLOT_ARROW_LIGHT
        0, // SLOT_TRADE_DEED
        0, // SLOT_BOMB
        0, // SLOT_BOMBCHU
        0, // SLOT_DEKU_STICK
        0, // SLOT_DEKU_NUT
        0, // SLOT_MAGIC_BEANS
        0, // SLOT_TRADE_KEY_MAMA
        0, // SLOT_POWDER_KEG
        0, // SLOT_PICTOGRAPH_BOX
        0, // SLOT_LENS_OF_TRUTH
        0, // SLOT_HOOKSHOT
        0, // SLOT_SWORD_GREAT_FAIRY
        0, // SLOT_TRADE_COUPLE
        0, // SLOT_BOTTLE_1
        0, // SLOT_BOTTLE_2
        0, // SLOT_BOTTLE_3
        0, // SLOT_BOTTLE_4
        0, // SLOT_BOTTLE_5
        0, // SLOT_BOTTLE_6
    },
    // upgrades
    (0 << 0) |      // UPG_QUIVER
        (0 << 3) |  // UPG_BOMB_BAG
        (0 << 6) |  // UPG_STRENGTH
        (0 << 9) |  // UPG_SCALE
        (0 << 12) | // UPG_WALLET
        (0 << 14) | // UPG_BULLET_BAG
        (1 << 17) | // UPG_DEKU_STICKS
        (1 << 20),  // UPG_DEKU_NUTS
    // questItems
    0,
    // dungeonItems
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // dungeonKeys
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
    // defenseHearts
    0,
    // strayFairies
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // dekuPlaygroundPlayerName
    {
        // "LINK    "
        { 0x15, 0x12, 0x17, 0x14, 0x3E, 0x3E, 0x3E, 0x3E },
        // "LINK    "
        { 0x15, 0x12, 0x17, 0x14, 0x3E, 0x3E, 0x3E, 0x3E },
        // "LINK    "
        { 0x15, 0x12, 0x17, 0x14, 0x3E, 0x3E, 0x3E, 0x3E },
    },
};

u16 sSaveDefaultChecksum = 0;

/**
 *  Initialize new save.
 *  This save has an empty inventory with 3 hearts, sword and shield.
 */
void Sram_InitNewSave(void) {
    gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
    gSaveContext.save.eventDayCount = 0;
    gSaveContext.save.day = 0;
    gSaveContext.save.time = CLOCK_TIME(6, 0) - 1;
    Sram_ResetSave();

    Lib_MemCpy(&gSaveContext.save.saveInfo.playerData, &sSaveDefaultPlayerData, sizeof(SavePlayerData));
    Lib_MemCpy(&gSaveContext.save.saveInfo.equips, &sSaveDefaultItemEquips, sizeof(ItemEquips));
    Lib_MemCpy(&gSaveContext.save.saveInfo.inventory, &sSaveDefaultInventory, sizeof(Inventory));
    Lib_MemCpy(&gSaveContext.save.saveInfo.checksum, &sSaveDefaultChecksum,
               sizeof(gSaveContext.save.saveInfo.checksum));

    gSaveContext.save.saveInfo.horseData.sceneId = SCENE_F01;
    gSaveContext.save.saveInfo.horseData.pos.x = -1420;
    gSaveContext.save.saveInfo.horseData.pos.y = 257;
    gSaveContext.save.saveInfo.horseData.pos.z = -1285;
    gSaveContext.save.saveInfo.horseData.yaw = -0x7554;

    gSaveContext.nextCutsceneIndex = 0;
    gSaveContext.save.saveInfo.playerData.magicLevel = 0;
    
    // Chaos_SetConfigFlag(CHAOS_CONFIG_BEER_GOGGLES_BLUR, true);
    // Chaos_SetConfigFlag(CHAOS_CONFIG_IKANA_CLIMB_TREE_ACTOR_CHASE, false);
    // Chaos_SetConfigFlag(CHAOS_CONFIG_STONE_TOWER_CLIMB_ACTOR_CHASE, false);
    // Chaos_SetConfigFlag(CHAOS_CONFIG_DETERMINISTIC_EFFECT_RNG, false);
    // Chaos_SetConfigFlag(CHAOS_CONFIG_USE_DISRUPTIVE_EFFECT_PROB, true);

    gSaveContext.save.chaos.major = 0;
    gSaveContext.save.chaos.minor = 0;
    gSaveContext.save.chaos.patch = 0;

    Chaos_SetConfigDefaults();
    
    gSaveContext.save.chaos.major = CHAOS_MAJOR_VERSION;
    gSaveContext.save.chaos.minor = CHAOS_MINOR_VERSION;
    gSaveContext.save.chaos.patch = CHAOS_PATCH_VERSION;

    gSaveContext.save.chaos.moon_crash_count = 0;
    gSaveContext.save.chaos.evilness_probability_scale = 1.0f;

    Sram_GenerateRandomSaveFields();
}

SavePlayerData sSaveDebugPlayerData = {
    { 'Z', 'E', 'L', 'D', 'A', '3' },                   // newf
    0,                                                  // threeDayResetCount
    { 0x15, 0x12, 0x17, 0x14, 0x3E, 0x3E, 0x3E, 0x3E }, // playerName "LINK    "
    0x80,                                               // healthCapacity
    0x80,                                               // health
    0,                                                  // magicLevel
    MAGIC_NORMAL_METER,                                 // magic
    50,                                                 // rupees
    100,                                                // swordHealth
    0,                                                  // tatlTimer
    true,                                               // isMagicAcquired
    false,                                              // isDoubleMagicAcquired
    0,                                                  // doubleDefense
    0,                                                  // unk_1F
    OWL_WARP_NONE,                                      // owlWarpId
    0,                                                  // owlActivationFlags
    0xFF,                                               // unk_24
    SCENE_SPOT00,                                       // savedSceneId
};

ItemEquips sSaveDebugItemEquips = {
    {
        { ITEM_SWORD_KOKIRI, ITEM_BOW, ITEM_POTION_RED, ITEM_OCARINA_OF_TIME },
        { ITEM_SWORD_KOKIRI, ITEM_BOW, ITEM_MASK_GORON, ITEM_OCARINA_OF_TIME },
        { ITEM_SWORD_KOKIRI, ITEM_BOW, ITEM_MASK_ZORA, ITEM_OCARINA_OF_TIME },
        { ITEM_DEKU_NUT, ITEM_DEKU_NUT, ITEM_MASK_DEKU, ITEM_OCARINA_OF_TIME },
    },
    {
        { SLOT_OCARINA, SLOT_BOW, SLOT_BOTTLE_2, SLOT_OCARINA },
        { SLOT_OCARINA, SLOT_MAGIC_BEANS, SLOT_MASK_GORON, SLOT_BOMBCHU },
        { SLOT_OCARINA, SLOT_POWDER_KEG, SLOT_MASK_ZORA, SLOT_BOMBCHU },
        { SLOT_OCARINA, SLOT_BOW, SLOT_MASK_DEKU, SLOT_BOMBCHU },
    },
    0x11,
};

Inventory sSaveDebugInventory = {
    // items
    {
        ITEM_OCARINA_OF_TIME,    // SLOT_OCARINA
        ITEM_BOW,                // SLOT_BOW
        ITEM_ARROW_FIRE,         // SLOT_ARROW_FIRE
        ITEM_ARROW_ICE,          // SLOT_ARROW_ICE
        ITEM_ARROW_LIGHT,        // SLOT_ARROW_LIGHT
        ITEM_MOONS_TEAR,         // SLOT_TRADE_DEED
        ITEM_BOMB,               // SLOT_BOMB
        ITEM_BOMBCHU,            // SLOT_BOMBCHU
        ITEM_DEKU_STICK,         // SLOT_DEKU_STICK
        ITEM_DEKU_NUT,           // SLOT_DEKU_NUT
        ITEM_MAGIC_BEANS,        // SLOT_MAGIC_BEANS
        ITEM_ROOM_KEY,           // SLOT_TRADE_KEY_MAMA
        ITEM_POWDER_KEG,         // SLOT_POWDER_KEG
        ITEM_PICTOGRAPH_BOX,     // SLOT_PICTOGRAPH_BOX
        ITEM_LENS_OF_TRUTH,      // SLOT_LENS_OF_TRUTH
        ITEM_HOOKSHOT,           // SLOT_HOOKSHOT
        ITEM_SWORD_GREAT_FAIRY,  // SLOT_SWORD_GREAT_FAIRY
        ITEM_LETTER_TO_KAFEI,    // SLOT_TRADE_COUPLE
        ITEM_BOTTLE,             // SLOT_BOTTLE_1
        ITEM_POTION_RED,         // SLOT_BOTTLE_2
        ITEM_POTION_GREEN,       // SLOT_BOTTLE_3
        ITEM_POTION_BLUE,        // SLOT_BOTTLE_4
        ITEM_NONE,               // SLOT_BOTTLE_5
        ITEM_NONE,               // SLOT_BOTTLE_6
        ITEM_MASK_POSTMAN,       // SLOT_MASK_POSTMAN
        ITEM_MASK_ALL_NIGHT,     // SLOT_MASK_ALL_NIGHT
        ITEM_MASK_BLAST,         // SLOT_MASK_BLAST
        ITEM_MASK_STONE,         // SLOT_MASK_STONE
        ITEM_MASK_GREAT_FAIRY,   // SLOT_MASK_GREAT_FAIRY
        ITEM_MASK_DEKU,          // SLOT_MASK_DEKU
        ITEM_MASK_KEATON,        // SLOT_MASK_KEATON
        ITEM_MASK_BREMEN,        // SLOT_MASK_BREMEN
        ITEM_MASK_BUNNY,         // SLOT_MASK_BUNNY
        ITEM_MASK_DON_GERO,      // SLOT_MASK_DON_GERO
        ITEM_MASK_SCENTS,        // SLOT_MASK_SCENTS
        ITEM_MASK_GORON,         // SLOT_MASK_GORON
        ITEM_MASK_ROMANI,        // SLOT_MASK_ROMANI
        ITEM_MASK_CIRCUS_LEADER, // SLOT_MASK_CIRCUS_LEADER
        ITEM_MASK_KAFEIS_MASK,   // SLOT_MASK_KAFEIS_MASK
        ITEM_MASK_COUPLE,        // SLOT_MASK_COUPLE
        ITEM_MASK_TRUTH,         // SLOT_MASK_TRUTH
        ITEM_MASK_ZORA,          // SLOT_MASK_ZORA
        ITEM_MASK_KAMARO,        // SLOT_MASK_KAMARO
        ITEM_MASK_GIBDO,         // SLOT_MASK_GIBDO
        ITEM_MASK_GARO,          // SLOT_MASK_GARO
        ITEM_MASK_CAPTAIN,       // SLOT_MASK_CAPTAIN
        ITEM_MASK_GIANT,         // SLOT_MASK_GIANT
        ITEM_MASK_FIERCE_DEITY,  // SLOT_MASK_FIERCE_DEITY
    },
    // ammo
    {
        1,  // SLOT_OCARINA
        30, // SLOT_BOW
        1,  // SLOT_ARROW_FIRE
        1,  // SLOT_ARROW_ICE
        1,  // SLOT_ARROW_LIGHT
        1,  // SLOT_TRADE_DEED
        30, // SLOT_BOMB
        30, // SLOT_BOMBCHU
        30, // SLOT_DEKU_STICK
        30, // SLOT_DEKU_NUT
        1,  // SLOT_MAGIC_BEANS
        1,  // SLOT_TRADE_KEY_MAMA
        1,  // SLOT_POWDER_KEG
        1,  // SLOT_PICTOGRAPH_BOX
        30, // SLOT_LENS_OF_TRUTH
        1,  // SLOT_HOOKSHOT
        1,  // SLOT_SWORD_GREAT_FAIRY
        1,  // SLOT_TRADE_COUPLE
        1,  // SLOT_BOTTLE_1
        1,  // SLOT_BOTTLE_2
        1,  // SLOT_BOTTLE_3
        1,  // SLOT_BOTTLE_4
        0,  // SLOT_BOTTLE_5
        0,  // SLOT_BOTTLE_6
    },
    // upgrades
    (1 << 0) |      // UPG_QUIVER
        (1 << 3) |  // UPG_BOMB_BAG
        (0 << 6) |  // UPG_STRENGTH
        (0 << 9) |  // UPG_SCALE
        (0 << 12) | // UPG_WALLET
        (0 << 14) | // UPG_BULLET_BAG
        (1 << 17) | // UPG_DEKU_STICKS
        (1 << 20),  // UPG_DEKU_NUTS
    // questItems
    (1 << QUEST_SONG_SONATA) | (1 << QUEST_SONG_LULLABY) | (1 << QUEST_SONG_BOSSA_NOVA) | (1 << QUEST_SONG_ELEGY) |
        (1 << QUEST_SONG_OATH) | (1 << QUEST_SONG_TIME) | (1 << QUEST_SONG_HEALING) | (1 << QUEST_SONG_EPONA) |
        (1 << QUEST_SONG_SOARING) | (1 << QUEST_SONG_STORMS) | (1 << QUEST_BOMBERS_NOTEBOOK) |
        (1 << QUEST_SONG_LULLABY_INTRO),
    // dungeonItems
    {
        (1 << DUNGEON_BOSS_KEY) | (1 << DUNGEON_COMPASS) | (1 << DUNGEON_MAP),
        (1 << DUNGEON_BOSS_KEY) | (1 << DUNGEON_COMPASS) | (1 << DUNGEON_MAP),
        (1 << DUNGEON_BOSS_KEY) | (1 << DUNGEON_COMPASS) | (1 << DUNGEON_MAP),
        (1 << DUNGEON_BOSS_KEY) | (1 << DUNGEON_COMPASS) | (1 << DUNGEON_MAP),
        (1 << DUNGEON_BOSS_KEY) | (1 << DUNGEON_COMPASS) | (1 << DUNGEON_MAP),
        (1 << DUNGEON_BOSS_KEY) | (1 << DUNGEON_COMPASS) | (1 << DUNGEON_MAP),
        (1 << DUNGEON_BOSS_KEY) | (1 << DUNGEON_COMPASS) | (1 << DUNGEON_MAP),
        (1 << DUNGEON_BOSS_KEY) | (1 << DUNGEON_COMPASS) | (1 << DUNGEON_MAP),
        (1 << DUNGEON_BOSS_KEY) | (1 << DUNGEON_COMPASS) | (1 << DUNGEON_MAP),
        (1 << DUNGEON_BOSS_KEY) | (1 << DUNGEON_COMPASS) | (1 << DUNGEON_MAP),
    },
    // dungeonKeys
    { 8, 8, 8, 8, 8, 8, 8, 8, 8 },
    // defenseHearts
    0,
    // strayFairies
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // dekuPlaygroundPlayerName
    {
        // "LINK    "
        { 0x15, 0x12, 0x17, 0x14, 0x3E, 0x3E, 0x3E, 0x3E },
        // "LINK    "
        { 0x15, 0x12, 0x17, 0x14, 0x3E, 0x3E, 0x3E, 0x3E },
        // "LINK    "
        { 0x15, 0x12, 0x17, 0x14, 0x3E, 0x3E, 0x3E, 0x3E },
    },
};

u16 sSaveDebugChecksum = 0;

u8 D_801C6A48[PLAYER_FORM_MAX] = {
    ITEM_MASK_FIERCE_DEITY, // PLAYER_FORM_FIERCE_DEITY
    ITEM_MASK_GORON,        // PLAYER_FORM_GORON
    ITEM_MASK_ZORA,         // PLAYER_FORM_ZORA
    ITEM_MASK_DEKU,         // PLAYER_FORM_DEKU
    ITEM_MASK_FIERCE_DEITY, // PLAYER_FORM_HUMAN
};

u8 D_801C6A50[PLAYER_FORM_MAX] = {
    SLOT_MASK_FIERCE_DEITY, // PLAYER_FORM_FIERCE_DEITY
    SLOT_MASK_GORON,        // PLAYER_FORM_GORON
    SLOT_MASK_ZORA,         // PLAYER_FORM_ZORA
    SLOT_MASK_DEKU,         // PLAYER_FORM_DEKU
    SLOT_MASK_FIERCE_DEITY, // PLAYER_FORM_HUMAN
};

/**
 *  Initialize debug save. This is also used on the Title Screen
 *  This save has a mostly full inventory, every mask and 10 hearts.
 *
 *  Some noteable flags that are set:
 *  TODO: Investigate the flags
 */
void Sram_InitDebugSave(void) {
    Sram_ResetSave();

    Lib_MemCpy(&gSaveContext.save.saveInfo.playerData, &sSaveDebugPlayerData, sizeof(SavePlayerData));
    Lib_MemCpy(&gSaveContext.save.saveInfo.equips, &sSaveDebugItemEquips, sizeof(ItemEquips));
    Lib_MemCpy(&gSaveContext.save.saveInfo.inventory, &sSaveDebugInventory, sizeof(Inventory));
    Lib_MemCpy(&gSaveContext.save.saveInfo.checksum, &sSaveDebugChecksum, sizeof(gSaveContext.save.saveInfo.checksum));

    if (GET_PLAYER_FORM != PLAYER_FORM_HUMAN) {
        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = D_801C6A48[GET_PLAYER_FORM];
        C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = D_801C6A50[GET_PLAYER_FORM];
    }

    gSaveContext.save.hasTatl = true;

    gSaveContext.save.saveInfo.horseData.sceneId = SCENE_F01;
    gSaveContext.save.saveInfo.horseData.pos.x = -1420;
    gSaveContext.save.saveInfo.horseData.pos.y = 257;
    gSaveContext.save.saveInfo.horseData.pos.z = -1285;
    gSaveContext.save.saveInfo.horseData.yaw = -0x7554;

    gSaveContext.save.entrance = ENTRANCE(CUTSCENE, 0);
    gSaveContext.save.isFirstCycle = true;

    SET_WEEKEVENTREG(WEEKEVENTREG_15_20);
    SET_WEEKEVENTREG(WEEKEVENTREG_59_04);
    SET_WEEKEVENTREG(WEEKEVENTREG_31_04);

    gSaveContext.cycleSceneFlags[SCENE_INSIDETOWER].switch0 = 1;
    gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_INSIDETOWER].switch0 = 1;
    gSaveContext.save.saveInfo.playerData.magicLevel = 0;

    Sram_GenerateRandomSaveFields();
}

void Sram_ResetSaveFromMoonCrash(SramContext* sramCtx) {
    s32 i;
    s32 cutsceneIndex = gSaveContext.save.cutsceneIndex;

    bzero(sramCtx->saveBuf, SAVE_BUFFER_SIZE);

    if (SysFlashrom_ReadData(sramCtx->saveBuf, gFlashSaveStartPages[gSaveContext.fileNum * 2],
                             gFlashSaveNumPages[gSaveContext.fileNum * 2]) != 0) {
        SysFlashrom_ReadData(sramCtx->saveBuf, gFlashSaveStartPages[gSaveContext.fileNum * 2 + 1],
                             gFlashSaveNumPages[gSaveContext.fileNum * 2 + 1]);
    }
    Lib_MemCpy(&gSaveContext.save, sramCtx->saveBuf, sizeof(Save));
    if (CHECK_NEWF(gSaveContext.save.saveInfo.playerData.newf)) {
        SysFlashrom_ReadData(sramCtx->saveBuf, gFlashSaveStartPages[gSaveContext.fileNum * 2 + 1],
                             gFlashSaveNumPages[gSaveContext.fileNum * 2 + 1]);
        Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, sizeof(Save));
    }
    gSaveContext.save.cutsceneIndex = cutsceneIndex;

    for (i = 0; i < ARRAY_COUNT(gSaveContext.eventInf); i++) {
        gSaveContext.eventInf[i] = 0;
    }

    for (i = 0; i < ARRAY_COUNT(gSaveContext.cycleSceneFlags); i++) {
        gSaveContext.cycleSceneFlags[i].chest = gSaveContext.save.saveInfo.permanentSceneFlags[i].chest;
        gSaveContext.cycleSceneFlags[i].switch0 = gSaveContext.save.saveInfo.permanentSceneFlags[i].switch0;
        gSaveContext.cycleSceneFlags[i].switch1 = gSaveContext.save.saveInfo.permanentSceneFlags[i].switch1;
        gSaveContext.cycleSceneFlags[i].clearedRoom = gSaveContext.save.saveInfo.permanentSceneFlags[i].clearedRoom;
        gSaveContext.cycleSceneFlags[i].collectible = gSaveContext.save.saveInfo.permanentSceneFlags[i].collectible;
    }

    for (i = 0; i < TIMER_ID_MAX; i++) {
        gSaveContext.timerStates[i] = TIMER_STATE_OFF;
        gSaveContext.timerCurTimes[i] = SECONDS_TO_TIMER(0);
        gSaveContext.timerTimeLimits[i] = SECONDS_TO_TIMER(0);
        gSaveContext.timerStartOsTimes[i] = 0;
        gSaveContext.timerStopTimes[i] = SECONDS_TO_TIMER(0);
        gSaveContext.timerPausedOsTimes[i] = 0;
    }

    D_801BDAA0 = true;
    gHorseIsMounted = false;
    gSaveContext.powderKegTimer = 0;
    gSaveContext.unk_1014 = 0;
    gSaveContext.jinxTimer = 0;
}

static u16 sOwlWarpEntrances[OWL_WARP_MAX - 1] = {
    ENTRANCE(GREAT_BAY_COAST, 11),         // OWL_WARP_GREAT_BAY_COAST
    ENTRANCE(ZORA_CAPE, 6),                // OWL_WARP_ZORA_CAPE
    ENTRANCE(SNOWHEAD, 3),                 // OWL_WARP_SNOWHEAD
    ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 8),  // OWL_WARP_MOUNTAIN_VILLAGE
    ENTRANCE(SOUTH_CLOCK_TOWN, 9),         // OWL_WARP_CLOCK_TOWN
    ENTRANCE(MILK_ROAD, 4),                // OWL_WARP_MILK_ROAD
    ENTRANCE(WOODFALL, 4),                 // OWL_WARP_WOODFALL
    ENTRANCE(SOUTHERN_SWAMP_POISONED, 10), // OWL_WARP_SOUTHERN_SWAMP
    ENTRANCE(IKANA_CANYON, 4),             // OWL_WARP_IKANA_CANYON
    ENTRANCE(STONE_TOWER, 3),              // OWL_WARP_STONE_TOWER
};

void Sram_OpenSave(FileSelectState* fileSelect, SramContext* sramCtx) {
    s32 i;
    s32 pad;
    s32 phi_t1;
    s32 pad1;
    // s32 fileNum;
    s32 file_index = gSaveContext.fileNum;

    struct save_info_t *owl_save_info = &fileSelect->save_info[file_index + 2];

    if (gSaveContext.flashSaveAvailable) {
        bzero(sramCtx->saveBuf, SAVE_BUFFER_SIZE);

        if (gSaveContext.fileNum == 0xFF) 
        {
            // SysFlashrom_ReadData(sramCtx->saveBuf, gFlashSaveStartPages[0], gFlashSaveNumPages[0]);
            file_index = 0;
        } 
        else if (owl_save_info->isOwlSave || owl_save_info->is_crash_save) 
        {
            file_index += 2;
        } 

        if (SysFlashrom_ReadData(sramCtx->saveBuf, gSaveFileInfo[file_index].main_start_page, gSaveFileInfo[file_index].main_page_count) != 0) 
        {
            SysFlashrom_ReadData(sramCtx->saveBuf, gSaveFileInfo[file_index].backup_start_page, gSaveFileInfo[file_index].backup_page_count);
        }

        Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gFlashSaveSizes[phi_t1]);

        if (!CHECK_NEWF_VALID(gSaveContext.save.saveInfo.playerData.newf)) 
        {
            SysFlashrom_ReadData(sramCtx->saveBuf, gSaveFileInfo[file_index].backup_start_page, gSaveFileInfo[file_index].backup_page_count);
            Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gSaveFileInfo[file_index].save_size);
        }
    }

    gSaveContext.save.saveInfo.playerData.magicLevel = 0;
    for (i = 0; i < ARRAY_COUNT(gSaveContext.cycleSceneFlags); i++) {
        gSaveContext.cycleSceneFlags[i].chest = gSaveContext.save.saveInfo.permanentSceneFlags[i].chest;
        gSaveContext.cycleSceneFlags[i].switch0 = gSaveContext.save.saveInfo.permanentSceneFlags[i].switch0;
        gSaveContext.cycleSceneFlags[i].switch1 = gSaveContext.save.saveInfo.permanentSceneFlags[i].switch1;
        gSaveContext.cycleSceneFlags[i].clearedRoom = gSaveContext.save.saveInfo.permanentSceneFlags[i].clearedRoom;
        gSaveContext.cycleSceneFlags[i].collectible = gSaveContext.save.saveInfo.permanentSceneFlags[i].collectible;
    }

    if (!gSaveContext.save.isOwlSave && !gSaveContext.save.is_crash_save) {
            for (i = 0; i < ARRAY_COUNT(gSaveContext.eventInf); i++) {
                gSaveContext.eventInf[i] = 0;
            }

            for (i = 0; i < TIMER_ID_MAX; i++) {
                gSaveContext.timerStates[i] = TIMER_STATE_OFF;
                gSaveContext.timerCurTimes[i] = SECONDS_TO_TIMER(0);
                gSaveContext.timerTimeLimits[i] = SECONDS_TO_TIMER(0);
                gSaveContext.timerStartOsTimes[i] = 0;
                gSaveContext.timerStopTimes[i] = SECONDS_TO_TIMER(0);
                gSaveContext.timerPausedOsTimes[i] = 0;
            }

            if (gSaveContext.save.isFirstCycle) {
                gSaveContext.save.entrance = ENTRANCE(SOUTH_CLOCK_TOWN, 0);
                gSaveContext.save.day = 0;
                gSaveContext.save.time = CLOCK_TIME(6, 0) - 1;
            } else {
                gSaveContext.save.entrance = ENTRANCE(CUTSCENE, 0);
                gSaveContext.nextCutsceneIndex = 0;
                gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
            }

    } else {

        // for (i = 0; i < ARRAY_COUNT(gSaveContext.cycleSceneFlags); i++) 
        // {
        //     gSaveContext.cycleSceneFlags[i].chest = gSaveContext.save.saveInfo.permanentSceneFlags[i].chest;
        //     gSaveContext.cycleSceneFlags[i].switch0 = gSaveContext.save.saveInfo.permanentSceneFlags[i].switch0;
        //     gSaveContext.cycleSceneFlags[i].switch1 = gSaveContext.save.saveInfo.permanentSceneFlags[i].switch1;
        //     gSaveContext.cycleSceneFlags[i].clearedRoom = gSaveContext.save.saveInfo.permanentSceneFlags[i].clearedRoom;
        //     gSaveContext.cycleSceneFlags[i].collectible = gSaveContext.save.saveInfo.permanentSceneFlags[i].collectible;
        // }

        if (gSaveContext.save.saveInfo.scarecrowSpawnSongSet) 
        {
            Lib_MemCpy(gScarecrowSpawnSongPtr, gSaveContext.save.saveInfo.scarecrowSpawnSong,
                    sizeof(gSaveContext.save.saveInfo.scarecrowSpawnSong));

            for (i = 0; i < ARRAY_COUNT(gSaveContext.save.saveInfo.scarecrowSpawnSong); i++) {}
        }

        if(!gSaveContext.save.is_crash_save)
        {
            gSaveContext.save.entrance = sOwlWarpEntrances[(void)0, gSaveContext.save.owlWarpId];
            if ((gSaveContext.save.entrance == ENTRANCE(SOUTHERN_SWAMP_POISONED, 10)) &&
                CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE)) {
                gSaveContext.save.entrance = ENTRANCE(SOUTHERN_SWAMP_CLEARED, 10);
            } else if ((gSaveContext.save.entrance == ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 8)) &&
                    CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE)) {
                gSaveContext.save.entrance = ENTRANCE(MOUNTAIN_VILLAGE_SPRING, 8);
            }

            /* only invalidate an owl save if it's not a crash save */
            Sram_InvalidateOwlSave(sramCtx, gSaveContext.fileNum);
        }
    }

    // gSaveCopy = gSaveContext.save;
}

// Similar to func_80145698, but accounts for owl saves?
void func_8014546C(SramContext* sramCtx) {
    s32 i;
    for (i = 0; i < ARRAY_COUNT(gSaveContext.cycleSceneFlags); i++) {
        gSaveContext.save.saveInfo.permanentSceneFlags[i].chest = gSaveContext.cycleSceneFlags[i].chest;
        gSaveContext.save.saveInfo.permanentSceneFlags[i].switch0 = gSaveContext.cycleSceneFlags[i].switch0;
        gSaveContext.save.saveInfo.permanentSceneFlags[i].switch1 = gSaveContext.cycleSceneFlags[i].switch1;
        gSaveContext.save.saveInfo.permanentSceneFlags[i].clearedRoom = gSaveContext.cycleSceneFlags[i].clearedRoom;
        gSaveContext.save.saveInfo.permanentSceneFlags[i].collectible = gSaveContext.cycleSceneFlags[i].collectible;
    }

    gSaveContext.save.saveInfo.checksum = 0;

    if (gSaveContext.save.isOwlSave || gSaveContext.save.is_crash_save) {
        gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext, offsetof(SaveContext, fileNum));
        Lib_MemCpy(sramCtx->saveBuf, &gSaveContext, offsetof(SaveContext, fileNum));
    } else {

        // for (i = 0; i < ARRAY_COUNT(gSaveContext.cycleSceneFlags); i++) {
        //     gSaveContext.save.saveInfo.permanentSceneFlags[i].chest = gSaveContext.cycleSceneFlags[i].chest;
        //     gSaveContext.save.saveInfo.permanentSceneFlags[i].switch0 = gSaveContext.cycleSceneFlags[i].switch0;
        //     gSaveContext.save.saveInfo.permanentSceneFlags[i].switch1 = gSaveContext.cycleSceneFlags[i].switch1;
        //     gSaveContext.save.saveInfo.permanentSceneFlags[i].clearedRoom = gSaveContext.cycleSceneFlags[i].clearedRoom;
        //     gSaveContext.save.saveInfo.permanentSceneFlags[i].collectible = gSaveContext.cycleSceneFlags[i].collectible;
        // }

        // gSaveContext.save.saveInfo.checksum = 0;
        gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext.save, sizeof(Save));

        if (gSaveContext.flashSaveAvailable) {
            Lib_MemCpy(sramCtx->saveBuf, &gSaveContext, sizeof(Save));
            Lib_MemCpy(&sramCtx->saveBuf[0x2000], &gSaveContext.save, sizeof(Save));
        }
    }
}

/**
 * Save permanent scene flags, calculate checksum, copy save context to the save buffer
 */
void func_80145698(SramContext* sramCtx) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(gSaveContext.cycleSceneFlags); i++) {
        gSaveContext.save.saveInfo.permanentSceneFlags[i].chest = gSaveContext.cycleSceneFlags[i].chest;
        gSaveContext.save.saveInfo.permanentSceneFlags[i].switch0 = gSaveContext.cycleSceneFlags[i].switch0;
        gSaveContext.save.saveInfo.permanentSceneFlags[i].switch1 = gSaveContext.cycleSceneFlags[i].switch1;
        gSaveContext.save.saveInfo.permanentSceneFlags[i].clearedRoom = gSaveContext.cycleSceneFlags[i].clearedRoom;
        gSaveContext.save.saveInfo.permanentSceneFlags[i].collectible = gSaveContext.cycleSceneFlags[i].collectible;
    }

    gSaveContext.save.saveInfo.checksum = 0;
    gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext.save, sizeof(Save));
    if (gSaveContext.flashSaveAvailable) {
        Lib_MemCpy(sramCtx->saveBuf, &gSaveContext, sizeof(Save));
        Lib_MemCpy(&sramCtx->saveBuf[0x2000], &gSaveContext.save, sizeof(Save));
    }
}

// Verifies save and use backup if corrupted?
/* Sram_ValidateAndLoadFileSelectInfo */
void func_801457CC(GameState* gameState, SramContext* sramCtx) {
    FileSelectState* fileSelect = (FileSelectState*)gameState;
    u16 sp7A;
    // u16 oldCheckSum;
    // u16 sp76;
    u16 save_file_index;
    // u16 sp64;
    // u16 save_file_offset;
    // u16 phi_s2;
    u16 flash_load_failed = false;
    u16 pad;
    // u16 sp6E;
    u16 backup_file_loaded = false;
    // u16 newCheckSum;
    u16 maskCount;

    if (gSaveContext.flashSaveAvailable) {
        D_801F6AF0 = CURRENT_TIME;
        D_801F6AF2 = gSaveContext.flashSaveAvailable;
        // save_file_offset = 0;

        for (save_file_index = 0; save_file_index < 5; save_file_index++ /*, save_file_offset += 2 */) {
            struct save_info_t *save_info = &fileSelect->save_info[save_file_index];
            bzero(sramCtx->saveBuf, SAVE_BUFFER_SIZE);

            flash_load_failed = false;
            backup_file_loaded = false;

            if (SysFlashrom_ReadData(sramCtx->saveBuf, gSaveFileInfo[save_file_index].main_start_page, gSaveFileInfo[save_file_index].main_page_count) != 0) 
            {
                // main save didn't work
                // read backup save from flash
                backup_file_loaded = true;
                flash_load_failed = SysFlashrom_ReadData(sramCtx->saveBuf, gSaveFileInfo[save_file_index].backup_start_page, 
                    gSaveFileInfo[save_file_index].backup_page_count) != 0;
            }


            if (save_file_index < 4) 
            {
                // Non-owl save
                // sp76 = 0: main save
                // sp76 = 1: backup save

                u16 stored_file_checksum = 0;
                u16 computed_file_checksum = 0;
                u16 stored_backup_file_checksum = 0;
                u16 computed_backup_file_checksum = 0;

                save_info->isOwlSave = 0;
                save_info->is_crash_save = 0;
                bzero(save_info->newf, sizeof(save_info->newf));

                if (flash_load_failed) 
                {
                    // both main save and backup save failed
                    continue;
                } 
                else 
                {
                    // u16 stored_file_checksum = 0;
                    // u16 computed_file_checksum = 0;
                    // u16 stored_backup_file_checksum = 0;
                    // u16 computed_backup_file_checksum = 0;

                    // Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gFlashSaveSizes[save_file_offset]);
                    Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gSaveFileInfo[save_file_index].save_size);

                    // test checksum of main save
                    stored_file_checksum = gSaveContext.save.saveInfo.checksum;
                    gSaveContext.save.saveInfo.checksum = 0;
                    // newCheckSum = Sram_CalcChecksum(&gSaveContext, gFlashSaveSizes[save_file_offset]);
                    computed_file_checksum = Sram_CalcChecksum(&gSaveContext, gSaveFileInfo[save_file_index].save_size);
                    gSaveContext.save.saveInfo.checksum = computed_file_checksum;

                    if (!CHECK_NEWF_VALID(gSaveContext.save.saveInfo.playerData.newf) || (stored_file_checksum != computed_file_checksum)) 
                    {
                        /* save file is invalid or checksums don't match */
                        if(backup_file_loaded == 1)
                        {
                            /* backup got loaded from flash but is invalid, which means
                            both main and backup files failed in some capacity and are invalid */
                            continue;
                        }

                        /* main file was read from flash but is invalid, so try backup file */

                        // read backup save from flash
                        if (SysFlashrom_ReadData(sramCtx->saveBuf, gSaveFileInfo[save_file_index].backup_start_page, gSaveFileInfo[save_file_index].backup_page_count) != 0) 
                        {
                            continue;
                            // backup save didn't work
                        }

                        // Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gFlashSaveSizes[save_file_offset]);
                        Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gSaveFileInfo[save_file_index].save_size);

                        // test checksum of backup save
                        stored_backup_file_checksum = gSaveContext.save.saveInfo.checksum;
                        gSaveContext.save.saveInfo.checksum = 0;
                        computed_backup_file_checksum = Sram_CalcChecksum(&gSaveContext, gSaveFileInfo[save_file_index].save_size);

                        if (!CHECK_NEWF_VALID(gSaveContext.save.saveInfo.playerData.newf) || (stored_backup_file_checksum != computed_backup_file_checksum)) 
                        {
                            // backup save is invalid
                            continue;
                        }

                        /* overwrite main save file with backup */
                        Lib_MemCpy(&sramCtx->saveBuf, &gSaveContext.save, gSaveFileInfo[save_file_index].save_size);
                        Sram_SyncWriteToFlash(sramCtx, gSaveFileInfo[save_file_index].main_start_page, gSaveFileInfo[save_file_index].main_page_count);
                    }
                    else
                    {
                        /* main save worked, so test if backup file needs to be overwritten */
                        SaveContext *backup_save_context = (SaveContext *)sramCtx->saveBuf;
                        bzero(backup_save_context, gSaveFileInfo[save_file_index].save_size);

                        stored_file_checksum = gSaveContext.save.saveInfo.checksum;
                        stored_backup_file_checksum = 0;
                        computed_backup_file_checksum = 0;

                        if (SysFlashrom_ReadData(backup_save_context, gSaveFileInfo[save_file_index].backup_start_page, gSaveFileInfo[save_file_index].backup_page_count) == 0) 
                        {
                            stored_backup_file_checksum = backup_save_context->save.saveInfo.checksum;
                            backup_save_context->save.saveInfo.checksum = 0;
                            computed_backup_file_checksum = Sram_CalcChecksum(backup_save_context, gSaveFileInfo[save_file_index].save_size);
                        }

                        if (!CHECK_NEWF_VALID(backup_save_context->save.saveInfo.playerData.newf) || 
                            (stored_backup_file_checksum != computed_backup_file_checksum) ||
                            (stored_backup_file_checksum != stored_file_checksum)) 
                        {
                            /* backup save file is borked, so overwrite it with main save file */
                            Lib_MemCpy(sramCtx->saveBuf, &gSaveContext, gSaveFileInfo[save_file_index].save_size);

                            if(save_file_index < 2)
                            {
                                /* the backup file starts at a non-sector boundary, so to write it to flash it's necessary to also
                                write the main file */
                                Lib_MemCpy(&sramCtx->saveBuf[0x2000], &gSaveContext, gSaveFileInfo[save_file_index].save_size);
                                Sram_SyncWriteToFlash(sramCtx, gSaveFileInfo[save_file_index].main_start_page, gSaveFileInfo[save_file_index].main_page_count + 
                                                                                                               gSaveFileInfo[save_file_index].backup_page_count);
                            }
                            else
                            {
                                /* owl save files are sector aligned, so just overwrite the backup with the main save */
                                Sram_SyncWriteToFlash(sramCtx, gSaveFileInfo[save_file_index].backup_start_page, gSaveFileInfo[save_file_index].backup_page_count);
                            }
                        }
                    }
                }

                Lib_MemCpy(save_info->newf, gSaveContext.save.saveInfo.playerData.newf, sizeof(gSaveContext.save.saveInfo.playerData.newf));
                Lib_MemCpy(save_info->fileName, gSaveContext.save.saveInfo.playerData.playerName, sizeof(gSaveContext.save.saveInfo.playerData.playerName));

                // for (sp7A = 0; sp7A < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.newf); sp7A++) {
                //     save_info->newf[sp7A] = gSaveContext.save.saveInfo.playerData.newf[sp7A];
                // }

                // if (!CHECK_NEWF_VALID(save_info->newf)) 
                // {
                //     Fault_AddHangupPrintfAndCrash("Save file is NOT valid!");
                // }
                // {
                save_info->threeDayResetCount = gSaveContext.save.saveInfo.playerData.threeDayResetCount;
                save_info->healthCapacity = gSaveContext.save.saveInfo.playerData.healthCapacity;
                save_info->health = gSaveContext.save.saveInfo.playerData.health;
                save_info->defenseHearts = gSaveContext.save.saveInfo.inventory.defenseHearts;
                save_info->questItems = gSaveContext.save.saveInfo.inventory.questItems;
                save_info->time = CURRENT_TIME;
                save_info->day = gSaveContext.save.day;
                save_info->isOwlSave = gSaveContext.save.isOwlSave;
                save_info->is_crash_save = gSaveContext.save.is_crash_save;
                save_info->rupees = gSaveContext.save.saveInfo.playerData.rupees;
                save_info->walletUpgrades = CUR_UPG_VALUE(UPG_WALLET);

                for (sp7A = 0, maskCount = 0; sp7A < MASK_NUM_SLOTS; sp7A++) {
                    if (gSaveContext.save.saveInfo.inventory.items[sp7A + ITEM_NUM_SLOTS] == ITEM_NONE) {
                        continue;
                    }
                    maskCount++;
                }
                save_info->maskCount = maskCount;
                save_info->heartPieceCount = GET_QUEST_HEART_PIECE_COUNT;
                // }

                // if (loaded_file_index == 1) {
                //     // backup save used, so overwrite main save
                //     Lib_MemCpy(&sramCtx->saveBuf[0x2000], &gSaveContext.save, sizeof(Save));
                //     Sram_SyncWriteToFlash(sramCtx, gFlashSaveStartPages[save_file_offset], gFlashSpecialSaveNumPages[save_file_offset]);
                // } else if (!loaded_file_index) {
                //     // main save used
                //     oldCheckSum = gSaveContext.save.saveInfo.checksum;

                //     if (SysFlashrom_ReadData(sramCtx->saveBuf, gFlashSaveStartPages[save_file_offset + 1], gFlashSaveNumPages[save_file_offset + 1]) != 0) {
                //         flash_load_failed = true;
                //     } else {
                //         Lib_MemCpy(&gSaveContext.save, sramCtx->saveBuf, sizeof(Save));
                //         oldCheckSum = gSaveContext.save.saveInfo.checksum;
                //         gSaveContext.save.saveInfo.checksum = 0;
                //         sp7A = Sram_CalcChecksum(&gSaveContext.save, sizeof(Save));
                //     }

                //     if (!CHECK_NEWF_VALID(gSaveContext.save.saveInfo.playerData.newf) || (sp7A != phi_s2) ||
                //         (oldCheckSum != phi_s2)) {
                //         SysFlashrom_ReadData(sramCtx->saveBuf, gFlashSaveStartPages[save_file_offset], gFlashSaveNumPages[save_file_offset]);
                //         Lib_MemCpy(&gSaveContext.save, sramCtx->saveBuf, sizeof(Save));
                //         Lib_MemCpy(&sramCtx->saveBuf[0x2000], &gSaveContext.save, sizeof(Save));
                //         Sram_SyncWriteToFlash(sramCtx, gFlashSaveStartPages[save_file_offset], gFlashSpecialSaveNumPages[save_file_offset]);
                //     }
                // }
            } 
            // else if (save_file_index < 4) 
            // {
            //     // Owl Save:
            //     // sp76 = 3: main owl save
            //     // sp76 = 4: backup owl save
            //     // save_info->isOwlSave = 0;
            //     // save_info->is_crash_save = 0;

            //     if (CHECK_NEWF_VALID(save_info->newf)) {
            //         /* if the owl save is a valid save, meaning the player saved at an owl but didn't load from
            //         it yet */
            //         if (flash_load_failed) {
            //             // both main save and backup save failed
            //             bzero(sramCtx->saveBuf, SAVE_BUFFER_SIZE);
            //             Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gFlashSaveSizes[save_file_offset]);
            //         } else {
            //             Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gFlashSaveSizes[save_file_offset]);

            //             // test checksum of main save
            //             oldCheckSum = gSaveContext.save.saveInfo.checksum;
            //             gSaveContext.save.saveInfo.checksum = 0;
            //             newCheckSum = Sram_CalcChecksum(&gSaveContext, gFlashSaveSizes[save_file_offset]);
            //             gSaveContext.save.saveInfo.checksum = oldCheckSum;

            //             if (!CHECK_NEWF_VALID(gSaveContext.save.saveInfo.playerData.newf) || (newCheckSum != oldCheckSum)) {
            //                 // checksum didnt match, try backup save
            //                 loaded_file_index = 1;
            //                 flash_load_failed = false;
            //                 // read backup save from flash
            //                 if (SysFlashrom_ReadData(sramCtx->saveBuf, gFlashSaveStartPages[save_file_offset + 1],
            //                                          gFlashSaveNumPages[save_file_offset + 1]) != 0) {
            //                     // backup save didn't work
            //                     flash_load_failed = true;
            //                 }

            //                 Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gFlashSaveSizes[save_file_offset]);

            //                 // test checksum of backup save
            //                 oldCheckSum = gSaveContext.save.saveInfo.checksum;
            //                 gSaveContext.save.saveInfo.checksum = 0;

            //                 if (flash_load_failed || !CHECK_NEWF_VALID(gSaveContext.save.saveInfo.playerData.newf) ||
            //                     (oldCheckSum != Sram_CalcChecksum(&gSaveContext, gFlashSaveSizes[save_file_offset]))) {
            //                     bzero(sramCtx->saveBuf, SAVE_BUFFER_SIZE);
            //                     Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gFlashSaveSizes[save_file_offset]);
            //                     loaded_file_index = 999;
            //                 }
            //             }
            //         }

            //         gSaveContext.save.saveInfo.checksum = 0;
            //         //! FAKE: [sp64 + 0]?
            //         gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext, gFlashSaveSizes[save_file_offset + 0]);

            //         // for (sp7A = 0; sp7A < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.newf); sp7A++) {
            //         //     save_info->newf[sp7A] = gSaveContext.save.saveInfo.playerData.newf[sp7A];
            //         // }
            //         Lib_MemCpy(save_info->newf, gSaveContext.save.saveInfo.playerData.newf, sizeof(gSaveContext.save.saveInfo.playerData.newf));

            //         if (CHECK_NEWF_VALID(save_info->newf)) {
            //             save_info->threeDayResetCount = gSaveContext.save.saveInfo.playerData.threeDayResetCount;

            //             for (sp7A = 0; sp7A < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.playerName); sp7A++) {
            //                 save_info->fileName[sp7A] = (u32)gSaveContext.save.saveInfo.playerData.playerName[sp7A];
            //             }

            //             save_info->healthCapacity = gSaveContext.save.saveInfo.playerData.healthCapacity;
            //             save_info->health = gSaveContext.save.saveInfo.playerData.health;
            //             save_info->defenseHearts = gSaveContext.save.saveInfo.inventory.defenseHearts;
            //             save_info->questItems = gSaveContext.save.saveInfo.inventory.questItems;
            //             save_info->time = CURRENT_TIME;
            //             save_info->day = gSaveContext.save.day;
            //             save_info->isOwlSave = gSaveContext.save.isOwlSave;
            //             save_info->is_crash_save = gSaveContext.save.is_crash_save;
            //             save_info->rupees = gSaveContext.save.saveInfo.playerData.rupees;
            //             save_info->walletUpgrades = CUR_UPG_VALUE(UPG_WALLET);

            //             for (sp7A = 0, maskCount = 0; sp7A < MASK_NUM_SLOTS; sp7A++) {
            //                 if (gSaveContext.save.saveInfo.inventory.items[sp7A + ITEM_NUM_SLOTS] == ITEM_NONE) {
            //                     continue;
            //                 }
            //                 maskCount++;
            //             }
            //             save_info->maskCount = maskCount;
            //             save_info->heartPieceCount = GET_QUEST_HEART_PIECE_COUNT;
            //         }

            //         if (loaded_file_index == 1) {
            //             // backup save
            //             Sram_SyncWriteToFlash(sramCtx, gFlashSaveStartPages[save_file_offset], gFlashSaveNumPages[save_file_offset]);
            //             Sram_SyncWriteToFlash(sramCtx, gFlashSaveStartPages[save_file_offset + 1], gFlashSaveNumPages[save_file_offset + 1]);
            //         } else if (!loaded_file_index) {
            //             // main save
            //             oldCheckSum = gSaveContext.save.saveInfo.checksum;

            //             if (SysFlashrom_ReadData(sramCtx->saveBuf, gFlashSaveStartPages[save_file_offset + 1], gFlashSaveNumPages[save_file_offset + 1]) != 0) 
            //             {
            //                 flash_load_failed = true;
            //             } 
            //             else 
            //             {
            //                 Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gFlashSaveSizes[save_file_offset]);
            //                 phi_s2 = gSaveContext.save.saveInfo.checksum;
            //                 gSaveContext.save.saveInfo.checksum = 0;
            //                 newCheckSum = Sram_CalcChecksum(&gSaveContext, gFlashSaveSizes[save_file_offset]);
            //             }

            //             if (!CHECK_NEWF_VALID(gSaveContext.save.saveInfo.playerData.newf) || (sp7A != newCheckSum) ||
            //                 (oldCheckSum != phi_s2)) {
            //                 SysFlashrom_ReadData(sramCtx->saveBuf, gFlashSaveStartPages[save_file_offset], gFlashSaveNumPages[save_file_offset]);
            //                 Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gFlashSaveSizes[save_file_offset]);
            //                 Sram_SyncWriteToFlash(sramCtx, gFlashSaveStartPages[save_file_offset], gFlashSaveNumPages[save_file_offset]);
            //                 Sram_SyncWriteToFlash(sramCtx, gFlashSaveStartPages[save_file_offset + 1], gFlashSaveNumPages[save_file_offset + 1]);
            //             }
            //         }
            //     } else {
            //         bzero(sramCtx->saveBuf, SAVE_BUFFER_SIZE);
            //         Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, gFlashSaveSizes[save_file_offset]);
            //         Sram_SyncWriteToFlash(sramCtx, gFlashSaveStartPages[save_file_offset], gFlashSaveNumPages[save_file_offset]);
            //         Sram_SyncWriteToFlash(sramCtx, gFlashSaveStartPages[save_file_offset + 1], gFlashSaveNumPages[save_file_offset + 1]);
            //     }
            // } 
            else 
            {
                if (flash_load_failed) 
                {
                    gSaveContext.options.optionId = 0xA51D;
                    gSaveContext.options.language = LANGUAGE_ENG;
                    gSaveContext.options.audioSetting = SAVE_AUDIO_STEREO;
                    gSaveContext.options.languageSetting = 0;
                    gSaveContext.options.zTargetSetting = 0;
                } 
                else 
                {
                    Lib_MemCpy(&gSaveContext.options, sramCtx->saveBuf, sizeof(SaveOptions));

                    if (gSaveContext.options.optionId != 0xA51D) 
                    {
                        gSaveContext.options.optionId = 0xA51D;
                        gSaveContext.options.language = LANGUAGE_ENG;
                        gSaveContext.options.audioSetting = SAVE_AUDIO_STEREO;
                        gSaveContext.options.languageSetting = 0;
                        gSaveContext.options.zTargetSetting = 0;
                    }
                }
                Audio_SetFileSelectSettings(gSaveContext.options.audioSetting);
            }
        }

        gSaveContext.save.time = D_801F6AF0;
        gSaveContext.flashSaveAvailable = D_801F6AF2;
    }

    gSaveContext.options.language = LANGUAGE_ENG;
}

void Sram_EraseSave(FileSelectState* fileSelect2, SramContext* sramCtx, s32 fileNum) {
    FileSelectState* fileSelect = fileSelect2;
    s32 pad;

    struct save_info_t *save_info = &fileSelect->save_info[fileNum + 2];

    if (gSaveContext.flashSaveAvailable) {
        if (save_info->isOwlSave || save_info->is_crash_save) {
            Sram_InvalidateOwlSave(sramCtx, fileNum);
            save_info->isOwlSave = false;
            save_info->is_crash_save = false;
        }
        bzero(sramCtx->saveBuf, SAVE_BUFFER_SIZE);
        Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, sizeof(Save));
    }

    gSaveContext.save.time = D_801F6AF0;
    gSaveContext.flashSaveAvailable = D_801F6AF2;
}

void Sram_CopySave(FileSelectState* fileSelect2, SramContext* sramCtx) {
    FileSelectState* fileSelect = fileSelect2;
    u16 i;
    s16 maskCount;
    struct save_info_t *src_save_info = &fileSelect->save_info[fileSelect->selectedFileIndex + 2];
    struct save_info_t *dst_save_info = &fileSelect->save_info[fileSelect->copyDestFileIndex + 2];

    if (gSaveContext.flashSaveAvailable) 
    {
        if (src_save_info->isOwlSave || src_save_info->is_crash_save) 
        {
            func_80147414(sramCtx, fileSelect->selectedFileIndex, fileSelect->copyDestFileIndex);
            dst_save_info->threeDayResetCount = gSaveContext.save.saveInfo.playerData.threeDayResetCount;

            for (i = 0; i < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.playerName); i++) 
            {
                dst_save_info->fileName[i] = gSaveContext.save.saveInfo.playerData.playerName[i];
            }

            dst_save_info->healthCapacity = gSaveContext.save.saveInfo.playerData.healthCapacity;
            dst_save_info->health = gSaveContext.save.saveInfo.playerData.health;
            dst_save_info->defenseHearts = gSaveContext.save.saveInfo.inventory.defenseHearts;
            dst_save_info->questItems = gSaveContext.save.saveInfo.inventory.questItems;
            dst_save_info->time = CURRENT_TIME;
            dst_save_info->day = gSaveContext.save.day;
            dst_save_info->isOwlSave = gSaveContext.save.isOwlSave;
            dst_save_info->is_crash_save = gSaveContext.save.is_crash_save;
            dst_save_info->rupees = gSaveContext.save.saveInfo.playerData.rupees;
            dst_save_info->walletUpgrades = CUR_UPG_VALUE(UPG_WALLET);

            for (i = 0, maskCount = 0; i < MASK_NUM_SLOTS; i++) {
                if (gSaveContext.save.saveInfo.inventory.items[i + ITEM_NUM_SLOTS] != ITEM_NONE) {
                    maskCount++;
                }
            }

            dst_save_info->maskCount = maskCount;
            dst_save_info->heartPieceCount = GET_QUEST_HEART_PIECE_COUNT;
        }

        // clear buffer
        bzero(sramCtx->saveBuf, SAVE_BUFFER_SIZE);
        // read to buffer
        if (SysFlashrom_ReadData(&sramCtx->saveBuf[0], gFlashSaveStartPages[fileSelect->selectedFileIndex * 2],
                                 gFlashSaveNumPages[fileSelect->selectedFileIndex * 2])) {}

        if (SysFlashrom_ReadData(&sramCtx->saveBuf[0x2000], gFlashSaveStartPages[fileSelect->selectedFileIndex * 2 + 1],
                                 gFlashSaveNumPages[fileSelect->selectedFileIndex * 2 + 1])) {}

        // copy buffer to save context
        Lib_MemCpy(&gSaveContext.save, sramCtx->saveBuf, sizeof(Save));

        dst_save_info->threeDayResetCount = gSaveContext.save.saveInfo.playerData.threeDayResetCount;

        for (i = 0; i < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.playerName); i++) {
            dst_save_info->fileName[i] = gSaveContext.save.saveInfo.playerData.playerName[i];
        }

        dst_save_info->healthCapacity = gSaveContext.save.saveInfo.playerData.healthCapacity;
        dst_save_info->health = gSaveContext.save.saveInfo.playerData.health;
        dst_save_info->defenseHearts = gSaveContext.save.saveInfo.inventory.defenseHearts;
        dst_save_info->questItems = gSaveContext.save.saveInfo.inventory.questItems;
        dst_save_info->time = CURRENT_TIME;
        dst_save_info->day = gSaveContext.save.day;
        dst_save_info->isOwlSave = gSaveContext.save.isOwlSave;
        dst_save_info->is_crash_save = gSaveContext.save.is_crash_save;
        dst_save_info->rupees = gSaveContext.save.saveInfo.playerData.rupees;
        dst_save_info->walletUpgrades = CUR_UPG_VALUE(UPG_WALLET);

        for (i = 0, maskCount = 0; i < MASK_NUM_SLOTS; i++) 
        {
            if (gSaveContext.save.saveInfo.inventory.items[i + ITEM_NUM_SLOTS] != ITEM_NONE) 
            {
                maskCount++;
            }
        }

        dst_save_info->maskCount = maskCount;
        dst_save_info->heartPieceCount = GET_QUEST_HEART_PIECE_COUNT;
    }

    gSaveContext.save.time = D_801F6AF0;
    gSaveContext.flashSaveAvailable = D_801F6AF2;
}

void Sram_InitSave(FileSelectState* fileSelect2, SramContext* sramCtx) {
    s32 phi_v0;
    u16 i;
    FileSelectState* fileSelect = fileSelect2;
    s16 maskCount;

    if (gSaveContext.flashSaveAvailable) {

        struct save_info_t *save_info = &fileSelect->save_info[fileSelect->buttonIndex];
        Sram_InitNewSave();
        if (fileSelect->buttonIndex == 0) {
            gSaveContext.save.cutsceneIndex = 0xFFF0;
        }

        for (phi_v0 = 0; phi_v0 < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.playerName); phi_v0++) {
            gSaveContext.save.saveInfo.playerData.playerName[phi_v0] =
                save_info->fileName[phi_v0];
        }

        gSaveContext.save.saveInfo.playerData.newf[0] = 'Z';
        gSaveContext.save.saveInfo.playerData.newf[1] = 'E';
        gSaveContext.save.saveInfo.playerData.newf[2] = 'L';
        gSaveContext.save.saveInfo.playerData.newf[3] = 'D';
        gSaveContext.save.saveInfo.playerData.newf[4] = 'A';
        gSaveContext.save.saveInfo.playerData.newf[5] = '3';

        gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext.save, sizeof(Save));

        Lib_MemCpy(sramCtx->saveBuf, &gSaveContext.save, sizeof(Save));
        Lib_MemCpy(&sramCtx->saveBuf[0x2000], &gSaveContext.save, sizeof(Save));

        for (i = 0; i < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.newf); i++) {
            save_info->newf[i] = gSaveContext.save.saveInfo.playerData.newf[i];
        }

        save_info->threeDayResetCount = gSaveContext.save.saveInfo.playerData.threeDayResetCount;

        for (i = 0; i < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.playerName); i++) {
            save_info->fileName[i] = gSaveContext.save.saveInfo.playerData.playerName[i];
        }

        save_info->healthCapacity = gSaveContext.save.saveInfo.playerData.healthCapacity;
        save_info->health = gSaveContext.save.saveInfo.playerData.health;
        save_info->defenseHearts = gSaveContext.save.saveInfo.inventory.defenseHearts;
        save_info->questItems = gSaveContext.save.saveInfo.inventory.questItems;
        save_info->time = CURRENT_TIME;
        save_info->day = gSaveContext.save.day;
        save_info->isOwlSave = gSaveContext.save.isOwlSave;
        save_info->is_crash_save = false;
        save_info->rupees = gSaveContext.save.saveInfo.playerData.rupees;
        save_info->walletUpgrades = CUR_UPG_VALUE(UPG_WALLET);

        for (i = 0, maskCount = 0; i < MASK_NUM_SLOTS; i++) {
            if (gSaveContext.save.saveInfo.inventory.items[i + ITEM_NUM_SLOTS] != ITEM_NONE) {
                maskCount++;
            }
        }

        save_info->maskCount = maskCount;
        save_info->heartPieceCount = GET_QUEST_HEART_PIECE_COUNT;
    }

    gSaveContext.save.time = D_801F6AF0;
    gSaveContext.flashSaveAvailable = D_801F6AF2;
}

/**
 *  Write the SaveOptions of SaveContext to the save buffer
 */
void Sram_WriteSaveOptionsToBuffer(SramContext* sramCtx) {
    if (gSaveContext.flashSaveAvailable) {
        gSaveContext.options.language = LANGUAGE_ENG;
        Lib_MemCpy(sramCtx->saveBuf, &gSaveContext.options, sizeof(SaveOptions));
    }
}

void Sram_InitSram(GameState* gameState, SramContext* sramCtx) {
    if (gSaveContext.save.entrance) {} // Required to match

    Audio_SetFileSelectSettings(gSaveContext.options.audioSetting);
}

void Sram_Alloc(GameState* gameState, SramContext* sramCtx) {
    if (gSaveContext.flashSaveAvailable) {
        sramCtx->saveBuf = THA_AllocTailAlign16(&gameState->tha, SAVE_BUFFER_SIZE);
        sramCtx->status = 0;
    }
}

/**
 * Synchronous flash write
 */
void Sram_SyncWriteToFlash(SramContext* sramCtx, s32 curPage, s32 numPages) {
    sramCtx->curPage = curPage;
    sramCtx->numPages = numPages;
    SysFlashrom_WriteDataSync(sramCtx->saveBuf, curPage, numPages);
}

/**
 * Saves the game on the very first time Player enters South Clock Town from the Clock Tower
 */
void Sram_SaveSpecialEnterClockTown(PlayState* play) {
    s32 pad[2];
    SramContext* sramCtx = &play->sramCtx;

    gSaveContext.save.isFirstCycle = true;
    gSaveContext.save.isOwlSave = false;
    func_80145698(sramCtx);
    SysFlashrom_WriteDataSync(sramCtx->saveBuf, gFlashSaveStartPages[gSaveContext.fileNum * 2],
                              gFlashSpecialSaveNumPages[gSaveContext.fileNum * 2]);
}

/**
 * Saves when beating the game, after showing the "Dawn of the New Day" message
 */
void Sram_SaveSpecialNewDay(PlayState* play) {
    s32 cutsceneIndex = gSaveContext.save.cutsceneIndex;
    s32 day;
    u16 time = CURRENT_TIME;

    day = gSaveContext.save.day;

    CLEAR_WEEKEVENTREG(WEEKEVENTREG_84_20);

    Sram_SaveEndOfCycle(play);
    func_8014546C(&play->sramCtx);

    gSaveContext.save.day = day;
    gSaveContext.save.time = time;
    gSaveContext.save.cutsceneIndex = cutsceneIndex;
    SysFlashrom_WriteDataSync(play->sramCtx.saveBuf, gFlashSaveStartPages[gSaveContext.fileNum * 2],
                              gFlashSaveNumPages[gSaveContext.fileNum * 2]);
}

void Sram_SetFlashPagesDefault(SramContext* sramCtx, u32 curPage, u32 numPages) {
    sramCtx->curPage = curPage;
    sramCtx->numPages = numPages;
    sramCtx->status = 1;
}

void Sram_StartWriteToFlashDefault(SramContext* sramCtx) {
    // async flash write
    SysFlashrom_WriteDataAsync(sramCtx->saveBuf, sramCtx->curPage, sramCtx->numPages);

    sramCtx->startWriteOsTime = osGetTime();
    sramCtx->status = 2;
}

void Sram_UpdateWriteToFlashDefault(SramContext* sramCtx) {
    if (sramCtx->status == 2) {
        if (SysFlashrom_IsBusy() != 0) {          // if task running
            if (SysFlashrom_AwaitResult() == 0) { // wait for task done
                // task success
                sramCtx->status = 4;
            } else {
                // task failure
                sramCtx->status = 4;
            }
        }
    } else if (OSTIME_TO_TIMER(osGetTime() - sramCtx->startWriteOsTime) >= SECONDS_TO_TIMER(2)) {
        // Finished status is hardcoded to 2 seconds instead of when the task finishes
        sramCtx->status = 0;
    }
}

void Sram_SetFlashPagesOwlSave(SramContext* sramCtx, s32 curPage, s32 numPages) {
    sramCtx->curPage = curPage;
    sramCtx->numPages = numPages;
    sramCtx->status = 6;
}

void Sram_StartWriteToFlashOwlSave(SramContext* sramCtx) {
    SysFlashrom_WriteDataAsync(sramCtx->saveBuf, sramCtx->curPage, sramCtx->numPages);

    sramCtx->startWriteOsTime = osGetTime();
    sramCtx->status = 7;
}

void Sram_UpdateWriteToFlashOwlSave(SramContext* sramCtx) {
    if (sramCtx->status == 7) {
        if (SysFlashrom_IsBusy() != 0) {          // Is task running
            if (SysFlashrom_AwaitResult() == 0) { // Wait for task done
                SysFlashrom_WriteDataAsync(sramCtx->saveBuf, sramCtx->curPage + 0x80, sramCtx->numPages);
                sramCtx->status = 8;
            } else {
                SysFlashrom_WriteDataAsync(sramCtx->saveBuf, sramCtx->curPage + 0x80, sramCtx->numPages);
                sramCtx->status = 8;
            }
        }
    } else if (sramCtx->status == 8) {
        if (SysFlashrom_IsBusy() != 0) {          // Is task running
            if (SysFlashrom_AwaitResult() == 0) { // Wait for task done
                sramCtx->status = 4;
            } else {
                sramCtx->status = 4;
            }
        }
    } else if (OSTIME_TO_TIMER(osGetTime() - sramCtx->startWriteOsTime) >= SECONDS_TO_TIMER(2)) {
        // Finished status is hardcoded to 2 seconds instead of when the task finishes
        sramCtx->status = 0;
        bzero(sramCtx->saveBuf, SAVE_BUFFER_SIZE);
        gSaveContext.save.isOwlSave = false;
        gSaveContext.save.saveInfo.checksum = 0;
        // flash read to buffer then copy to save context
        SysFlashrom_ReadData(sramCtx->saveBuf, sramCtx->curPage, sramCtx->numPages);
        Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, offsetof(SaveContext, fileNum));
    }
}
 
void Sram_LoadChaosConfig(SramContext *sram_ctx, u8 file_index)
{
    Save *save = (Save *)sram_ctx->saveBuf;
    SysFlashrom_ReadData(save, gFlashSaveStartPages[file_index << 1], gFlashSaveNumPages[file_index << 1]);
    gSaveContext.save.chaos = save->chaos;
    
    Chaos_SetConfigDefaults();

    gSaveContext.save.chaos.major = CHAOS_MAJOR_VERSION;
    gSaveContext.save.chaos.minor = CHAOS_MINOR_VERSION;
    gSaveContext.save.chaos.patch = CHAOS_PATCH_VERSION;
}

void Sram_SaveChaosConfig(SramContext *sram_ctx, u8 file_index)
{
    // Save *save = (Save *)sram_ctx->saveBuf;
    // SysFlashrom_ReadData(save, gFlashSaveStartPages[file_index << 1], gFlashSaveNumPages[file_index << 1]);
    SaveContext *save_context = (SaveContext *)sram_ctx->saveBuf;
    u32 backup_file = false;
    SysFlashrom_ReadData(sram_ctx->saveBuf, gSaveFileInfo[file_index].main_start_page, gSaveFileInfo[file_index].main_page_count);

    if(!CHECK_NEWF_VALID(save_context->save.saveInfo.playerData.newf))
    {
        SysFlashrom_ReadData(sram_ctx->saveBuf, gSaveFileInfo[file_index].backup_start_page, gSaveFileInfo[file_index].backup_page_count);
        backup_file = true;
    }

    save_context->save.chaos = gSaveContext.save.chaos;
    save_context->save.saveInfo.checksum = 0;
    save_context->save.saveInfo.checksum = Sram_CalcChecksum(save_context, gSaveFileInfo[file_index].save_size);

    if(backup_file)
    {
        if(file_index >= 2)
        {
            Lib_MemCpy(&sram_ctx->saveBuf[0x2000], sram_ctx->saveBuf, gSaveFileInfo[file_index].save_size);
            SysFlashrom_ReadData(sram_ctx->saveBuf, gSaveFileInfo[file_index].main_start_page, gSaveFileInfo[file_index].main_page_count);
            Sram_SyncWriteToFlash(sram_ctx, gSaveFileInfo[file_index].main_start_page, gSaveFileInfo[file_index].main_page_count + 
                                                                                       gSaveFileInfo[file_index].backup_page_count);
        }
        else
        {
            /* owl save */
            Sram_SyncWriteToFlash(sram_ctx, gSaveFileInfo[file_index].main_start_page, gSaveFileInfo[file_index].main_page_count);
        }
    }
    else
    {
        Sram_SyncWriteToFlash(sram_ctx, gSaveFileInfo[file_index].main_start_page, gSaveFileInfo[file_index].main_page_count);
    }

    // if(!backup_file)
    // {
    // }
    // else
    // {
    //     Sram_SyncWriteToFlash(sram_ctx, gSaveFileInfo[file_index].backup_start_page, gSaveFileInfo[file_index].backup_page_count);   
    // }

    // Sram_SetFlashPagesDefault(sram_ctx, gFlashSaveStartPages[file_index * 2], gFlashSpecialSaveNumPages[file_index * 2]);
    // Sram_StartWriteToFlashDefault(sram_ctx);
}



/* Sram_InvalidateOwlSave? */
void Sram_InvalidateOwlSave(SramContext* sramCtx, s32 fileNum) {
    s32 pad;

    // SaveContext *owl_save_context = (SaveContext *)sramCtx->saveBuf;
    // Lib_MemCpy(owl_save_context, &gSaveContext, offsetof(SaveContext, fileNum));

    // owl_save_context->save.isOwlSave = false;
    /* clear the newf array and write the contents back to flash, so this owl save
    becomes invalid and non-loadable after resetting */
    // owl_save_context->save.saveInfo.playerData.newf[0] = '\0';
    // owl_save_context->save.saveInfo.playerData.newf[1] = '\0';
    // owl_save_context->save.saveInfo.playerData.newf[2] = '\0';
    // owl_save_context->save.saveInfo.playerData.newf[3] = '\0';
    // owl_save_context->save.saveInfo.playerData.newf[4] = '\0';
    // owl_save_context->save.saveInfo.playerData.newf[5] = '\0';

    bzero(sramCtx->saveBuf, offsetof(SaveContext, fileNum));

    // owl_save_context->save.saveInfo.checksum = 0;
    // owl_save_context->save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext, offsetof(SaveContext, fileNum));

    // Sram_SyncWriteToFlash(sramCtx, gFlashOwlSaveStartPages[fileNum * 2], gFlashOwlSaveNumPages[fileNum * 2]);
    // Sram_SyncWriteToFlash(sramCtx, gFlashOwlSaveStartPages[fileNum * 2 + 1], gFlashOwlSaveNumPages[fileNum * 2 + 1]);

    Sram_SyncWriteToFlash(sramCtx, gSaveFileInfo[fileNum + 2].main_start_page, gSaveFileInfo[fileNum + 2].main_page_count);
    Sram_SyncWriteToFlash(sramCtx, gSaveFileInfo[fileNum + 2].backup_start_page, gSaveFileInfo[fileNum + 2].backup_page_count);
    // Sram_SyncWriteToFlash(sramCtx, gFlashOwlSaveStartPages[fileNum * 2 + 1], gFlashOwlSaveNumPages[fileNum * 2 + 1]);
    gSaveContext.save.isOwlSave = true;

    // gSaveContext.save.saveInfo.playerData.newf[0] = 'Z';
    // gSaveContext.save.saveInfo.playerData.newf[1] = 'E';
    // gSaveContext.save.saveInfo.playerData.newf[2] = 'L';
    // gSaveContext.save.saveInfo.playerData.newf[3] = 'D';
    // gSaveContext.save.saveInfo.playerData.newf[4] = 'A';
    // gSaveContext.save.saveInfo.playerData.newf[5] = '3';
}

// Used by `Sram_CopySave` with `isOwlSave` set
void func_80147414(SramContext* sramCtx, s32 fileNum, s32 arg2) {
    s32 pad;

    // Clear save buffer
    bzero(sramCtx->saveBuf, SAVE_BUFFER_SIZE);

    // Read save file
    if (SysFlashrom_ReadData(sramCtx->saveBuf, gFlashOwlSaveStartPages[fileNum * 2],
                             gFlashOwlSaveNumPages[fileNum * 2]) != 0) {
        // If failed, read backup save file
        SysFlashrom_ReadData(sramCtx->saveBuf, gFlashOwlSaveStartPages[fileNum * 2 + 1],
                             gFlashOwlSaveNumPages[fileNum * 2 + 1]);
    }

    // Copy buffer to save context
    Lib_MemCpy(&gSaveContext, sramCtx->saveBuf, offsetof(SaveContext, fileNum));

    Sram_SyncWriteToFlash(sramCtx, gFlashOwlSaveStartPages[arg2 * 2], gFlashOwlSaveNumPages[arg2 * 2]);
    Sram_SyncWriteToFlash(sramCtx, gFlashOwlSaveStartPages[arg2 * 2 + 1], gFlashOwlSaveNumPages[arg2 * 2]);
}

void Sram_nop8014750C(UNK_TYPE4 arg0) {
}
