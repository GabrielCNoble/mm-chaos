#ifndef CHAOS_FUCKERY_H
#define CHAOS_FUCKERY_H

#include "ultra64.h"
#include "z64.h"
#include "libu64/pad.h"
#include "overlays/actors/ovl_En_Niw/z_en_niw.h"
#include "overlays/actors/ovl_En_Rr/z_en_rr.h"
#include "overlays/actors/ovl_En_Horse/z_en_horse.h"
#include "overlays/actors/ovl_En_Dg/z_en_dg.h"
#include "overlays/actors/ovl_En_Racedog/z_en_racedog.h"
#include "overlays/actors/ovl_En_Wallmas/z_en_wallmas.h"
#include "overlays/actors/ovl_En_Kusa/z_en_kusa.h"
#include "overlays/actors/ovl_Obj_Tsubo/z_obj_tsubo.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/actors/ovl_En_Bombf/z_en_bombf.h"
#include "overlays/actors/ovl_En_Arrow/z_en_arrow.h"
#include "overlays/actors/ovl_En_Ishi/z_en_ishi.h"
#include "overlays/actors/ovl_Boss_03/z_boss_03.h"
#include "overlays/actors/ovl_Boss_07/z_boss_07.h"
#include "overlays/actors/ovl_Arrow_Fire/z_arrow_fire.h"
#include "overlays/actors/ovl_Arrow_Light/z_arrow_light.h"
#include "overlays/actors/ovl_Arrow_Ice/z_arrow_ice.h"
#include "overlays/actors/ovl_Door_Shutter/z_door_shutter.h"
#include "overlays/actors/ovl_Bg_Open_Shutter/z_bg_open_shutter.h"
#include "overlays/actors/ovl_En_Door/z_en_door.h"
#include "overlays/actors/ovl_En_Door_Etc/z_en_door_etc.h"
#include "overlays/actors/ovl_En_Arwing/z_en_arwing.h"

#define CHAOS_MAJOR_VERSION 0
#define CHAOS_MINOR_VERSION 5
#define CHAOS_PATCH_VERSION 5

enum CHAOS_CODES
{
    CHAOS_CODE_NONE,
    CHAOS_CODE_FIRST,
    /* gravity is 5x lower */
    CHAOS_CODE_LOW_GRAVITY = CHAOS_CODE_FIRST,
    /* randomly changes health */
    CHAOS_CODE_CHANGE_HEALTH,
    /* set wallet to random value */
    CHAOS_CODE_CHANGE_RUPEE,
    /* self explanatory */
    CHAOS_CODE_ACTOR_CHASE,
    /* throw stuff far */
    CHAOS_CODE_YEET,
    /* poke */
    CHAOS_CODE_POKE,
    /* make the moon dance */
    CHAOS_CODE_MOON_DANCE,
    /* self explanatory */
    CHAOS_CODE_ONE_HIT_KO,
    /* player gets tossed randomly */
    CHAOS_CODE_RANDOM_KNOCKBACK,
    /* player gets ice-trapped */
    CHAOS_CODE_ICE_TRAP,
    /* chaos timer counts down twice as fast */
    CHAOS_CODE_TIMER_UP,
    /* shocks player */
    CHAOS_CODE_SHOCK,
    /* random quake */
    CHAOS_CODE_EARTHQUAKE,
    /* arrows explode on impact */
    CHAOS_CODE_BOMB_ARROWS,
    /* arrows fly erratically */
    CHAOS_CODE_WEIRD_ARROWS,
    /* bow fires several arrows at once */
    CHAOS_CODE_BUCKSHOT_ARROWS,
    /* gives bombs random timers */
    CHAOS_CODE_RANDOM_BOMB_TIMER,
    /* spawns like like above player, possibly swalling and voiding them out */
    CHAOS_CODE_LOVELESS_MARRIAGE,
    /* makes the ui behave unpredictably (shake, wave, bounce around) */
    CHAOS_CODE_WEIRD_UI,
    /* activate motion blur */
    CHAOS_CODE_BEER_GOGGLES,
    /* randomly changes magic */
    CHAOS_CODE_CHANGE_MAGIC,
    /* self explanatory */
    CHAOS_CODE_INVINCIBLE,
    /* fake insta-death */
    CHAOS_CODE_SYKE,
    /* real insta-death */
    CHAOS_CODE_DIE,
    /* player randomly screams */
    CHAOS_CODE_TRAP_FLAP,
    /* opens random textbox */
    // CHAOS_CODE_TEXTBOX,
    /* ice physics */
    CHAOS_CODE_SLIPPERY_FLOORS,
    /* link walks slowly */
    CHAOS_CODE_SLOW_DOWN,
    /* self explanatory */
    CHAOS_CODE_ENTRANCE_RANDO,
    /* varies bgm tempo and frequency randomly */
    CHAOS_CODE_TERRIBLE_MUSIC,
    /* received attacks have enormous knockback */
    CHAOS_CODE_INCREDIBLE_KNOCKBACK,
    /* randomly scales player model */
    CHAOS_CODE_RANDOM_SCALING,
    /* moon always faces the player */
    CHAOS_CODE_BIG_BROTHER,
    /* link randomly slows down and gasps for air */
    CHAOS_CODE_OUT_OF_SHAPE,
    /* sets tunic to random color */
    // CHAOS_CODE_TUNIC_COLOR,
    /* makes skybox rotate randomly */
    CHAOS_CODE_WEIRD_SKYBOX,
    /* deactivates a random owl after use */
    // CHAOS_CODE_SINGLE_ACTION_OWL,
    /* makes the player pull up the ocarina */
    CHAOS_CODE_PLAY_OCARINA,
    /* link randomly becomes fierce deity */
    CHAOS_CODE_RANDOM_FIERCE_DEITY,
    /* spawn chicken swarm */
    CHAOS_CODE_CHICKEN_ARISE,
    /* spawns oot arwing */
    CHAOS_CODE_STARFOX,
    /* self explanatory */
    CHAOS_CODE_SWAP_HEAL_AND_HURT,
    /* gives junk item */
    CHAOS_CODE_JUNK_ITEM,
    /* randomly adds heart container */
    CHAOS_CODE_RANDOM_HEALTH_UP,
    /* randomly removes heart container */
    CHAOS_CODE_RANDOM_HEALTH_DOWN,
    /* link stops and waves/bows at a random direction */
    CHAOS_CODE_IMAGINARY_FRIENDS,
    /* spawns either a real or a fake wallmaster */
    CHAOS_CODE_WALLMASTER,
    /* spawns dancing redeads around player */
    CHAOS_CODE_REDEADASS_GROOVE,
    /* scales up/down a random limb */
    CHAOS_CODE_SCALE_RANDOM_LIMB,
    /* player takes flight like a gossip stone */
    CHAOS_CODE_LIFTOFF,
    /* affects room geometry */
    CHAOS_CODE_UNSTABLE_ROOMS,
    /* snake game with the health meter */
    CHAOS_CODE_HEART_SNAKE,
    /* makes time flow a lot faster */
    CHAOS_CODE_FAST_TIME,
    /* fake moon crash, with a 1/64 chance of becoming a real moon crash */
    CHAOS_CODE_MOON_CRASH,
    /* makes link run 10x as fast */
    CHAOS_CODE_SPEEDBOOST,
    /* makes actors behave like billboards */
    CHAOS_CODE_BILLBOARD_ACTORS,
    /* draws link as a signpost */
    CHAOS_CODE_SIGNPOST,
    /* thick fog */
    CHAOS_CODE_SILENT_FIELD,
    /* makes link "buffer" and rollback randomly */
    CHAOS_CODE_BAD_CONNECTION,
    /*  
        A: makes it "dash" forward and spin faster
        B: makes it spin faster and jump. Jumping against a wall allows wall-jumping.
        (unfinished)
    */
    CHAOS_CODE_BEYBLADE,
    /* going throught a door leads to the same door in the same room (unfinished) */
    CHAOS_CODE_DIRECTILE_DYSFUNCTION,
    /* scale the world (except link) in a arbitrary direction */
    CHAOS_CODE_LENGTH_CONTRACTION,
    /* spawn giant fish at link's position */
    CHAOS_CODE_FISH,
    /* spawns friendly arwings that kill enemies (unfinished) */
    CHAOS_CODE_AIR_SUPPORT,
    /* simon says, player dies if they fail */
    CHAOS_CODE_SIMON_SAYS,
    /* player randomly auto-jumps */
    CHAOS_CODE_RANDOM_AUTO_JUMP,
    /* player faceplants instead of roll-landing (unfinished) */
    CHAOS_CODE_UNSTEADY_LEGS,
    /* intentionally triggers a crash and resumes the game thread a few seconds later 

        the exception type/text warning the progress is saved (if applicable) should
        be mispelled somewhere. 
    */
    CHAOS_CODE_FAKE_CRASH,
    /* spawns takkuri */
    // CHAOS_CODE_ASSHOLE_BIRD,
    /* a grotto entrance appears out of nowhere under link's feet */
    // CHAOS_CODE_RANDOM_GROTTO,  
    /* link stops, throws his hands to the sky and flops forwards/backwards */
    // CHAOS_CODE_TRUST_EXERCISE,
    /* gets hit by lighting strike (sets player on fire) */
    // CHAOS_CODE_LUCKY,
    /* link's hands shake */
    // CHAOS_CODE_TOO_MUCH_CAFFEINE,

    /* tatl randomly pipes up saying things like
        "The pit",
        "The tower",
        "The bridge",
        "The table",
        "The tile",
        "The corner",
        "The septum",
        "The diagonal",
        "The mug",
        "The shrapnel",
        "The parallelepiped. The fried one."
        "The moon",
        "The glass",
        "The boot",

        "So Lock up your mothers"
        "Lock up your wives"
        "Lock up your backdoor, run for your life"
        "This fairy is back in town"
        "Don't you mess me around"
        "He he he...",

        "Link, I think you might've stepped on shit."

        "Who are you talking to?"
        "Hey, are you busy right now?"

        "AAAAAAAHHHHHHHH!!!!",
        
     */
    // CHAOS_CODE_TATL, 
    /* link flips the moon with a very big hand (deku link's hand is the largest) */
    // CHAOS_CODE_FLIPPED,
    /* link's head inflate like a baloon and he floats */
    // CHAOS_CODE_AIRHEAD,
    /* hookshot pulls hookshot targets towards player */
    // CHAOS_CODE_BAD_FIXTURES,
    /* link can walk on any surface */
    // CHAOS_CODE_ALL_THE_TRACTION,
    /* gives link memory garbage, whose model is just random data from a random K0 address */
    // CHAOS_CODE_GIVE_MEMORY_GARBAGE,

    /* gives link an anvil, that causes massive damage when thrown */
    // CHAOS_CODE_DEATH_ANVIL,
    /* swaps link's bow with a bazooka and deku link's head with a m1 abrahams turret */
    // CHAOS_CODE_MODERN_WARFARE,
    
    /* fake random scene transition, with a small chance of being real */
    // CHAOS_CODE_RANDOM_SCENE_TRANSITION,
    /* randomly change environment settings */
    // CHAOS_CODE_WEIRD_ENVIRONMENT,
    /* sets far plane pretty close to link */
    // CHAOS_CODE_LOW_RENDER_DISTANCE,
    
    /* 
        player randomly loses grip, dropping items, falling from ledges/ladders
        TODO: change this one to include items from the inventory. The item should
        drop and become visible on the floor, so the player can grab it back. If 
        the player doesn't, it'll be returned to the player at a later time.
    */
    // CHAOS_CODE_BUTTERFINGERS,
    /* item randomly vanishes from inventory */
    // CHAOS_CODE_SHORT_TERM_MEMORY_LOSS,
    /* vanished item returns */
    // CHAOS_CODE_RECOVER_LOST_ITEM,
    /* randomly plays a little puzzle cutscene  */
    // CHAOS_CODE_PUZZLE_EVENT,
    

    // CHAOS_CODE_SLOWER_ANIMATIONS,
    // CHAOS_CODE_FASTER_ANIMATIONS,
    
    
    
    // CHAOS_CODE_RANDOM_SWORD_TRAILS,

    // CHAOS_CODE_UPSIDE_DOWN,
    
    // CHAOS_CODE_TAKE_SCREENSHOT,
    // CHAOS_CODE_SHOW_SCREENSHOT,
    
    // CHAOS_CODE_MUSIC_SWAP,
    
    
    /* spawn walls of fire around player */
    // CHAOS_CODE_ANTIVIRUS,
    // CHAOS_CODE_SONG_OF_STORMS,
    
    // CHAOS_CODE_ROTATE_SLOWLY,
    // CHAOS_CODE_ENVIRONMENT_SETTINGS,
    
    // CHAOS_CODE_AMBUSH,
    
    // CHAOS_CODE_RANDOM_PLAYER_SOUNDS,
    // CHAOS_CODE_RANDOM_C_BUTTONS,
    // CHAOS_CODE_HEAT_TIMER,
    // CHAOS_CODE_QUICKSAND,
    // CHAOS_CODE_SNAP_TO_FLOOR,
    // CHAOS_CODE_ACTIVATE_SWORD_COLLIDER,
    // CHAOS_CODE_NO_AUTOJUMP,
    // CHAOS_CODE_NO_CLIMB_UP,
    // CHAOS_CODE_NAVI,
    // CHAOS_CODE_EXPLOSIONS,
    // CHAOS_CODE_CARTIDGE_TILTING,
    // CHAOS_CODE_DIVE,
    // CHAOS_CODE_NO_COLLECTIBLES,
    // CHAOS_CODE_IGNORE_WATER,
    // CHAOS_CODE_ALL_SOUNDS_ARE_PLAYER,
    // CHAOS_CODE_INFINITE_HOVER_BOOTS,
    // CHAOS_CODE_MAGIC_ARMOR,

    // CHAOS_CODE_PUT_AWAY_ITEMS,
    // CHAOS_CODE_VOID_OUT,
    // CHAOS_CODE_CLIMB_EVERYTHING,
    // CHAOS_CODE_SUN_SONG,
    // CHAOS_CODE_USELESS_WEAPONS,
    
    /* walk over water */
    // CHAOS_CODE_HIM,
    
    /* enemies explode when killed */
    // CHAOS_CODE_VILETILE_ENEMIES,
    /* tatl frequently pipes up with random quest tips */
    // CHAOS_CODE_PARTICIPATION_AWARD,
    /* drop bombs around player */
    // CHAOS_CODE_AIR_STRIKE,
    /* spawns snow head wind effect */
    // CHAOS_CODE_BLIZZARD,
    /* spawns the 4 ghost sisters */
    // CHAOS_CODE_HEY_SOUL_SISTERS,
    /* spawns hostile redeads around player */
    // CHAOS_CODE_PAIN_IN_THE_REDEADASS,
    /* spawns majora's wrath */
    // CHAOS_CODE_MAJORAS_WRATH
    /* set all enemies on fire */
    // CHAOS_CODE_TORCH_ENEMIES,
    /* spawns a wasp nest, which when broken spawns three random enemies */
    // CHAOS_CODE_LOTTERY_NEST,

    /* camera sees from above (similar to gta) */
    // CHAOS_CODE_BIRDSEYE_VIEW,

    /* makes the player bonk on nothing while rolling */
    // CHAOS_CODE_INVISIBLE_WALL,
    /* spawns a random tree in front of the player */
    // CHAOS_CODE_TREE,
    
    
    CHAOS_CODE_LAST
};

#define CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TIME_STOPPED_SHIFT   0
#define CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TIME_STOPPED_SHIFT   1
#define CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_CUTSCENE_SHIFT       2
#define CHAOS_CODE_RESTRICTION_FLAG_AFFECT_CUTSCENE_SHIFT       3
#define CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_BOAT_RIDE_SHIFT      4
#define CHAOS_CODE_RESTRICTION_FLAG_AFFECT_BOAT_RIDE_SHIFT      5
#define CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_EPONA_RIDE_SHIFT     6
#define CHAOS_CODE_RESTRICTION_FLAG_AFFECT_EPONA_RIDE_SHIFT     7
#define CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_GRABBED_SHIFT        8
#define CHAOS_CODE_RESTRICTION_FLAG_AFFECT_GRABBED_SHIFT        9
#define CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_DEAD_SHIFT           10
#define CHAOS_CODE_RESTRICTION_FLAG_AFFECT_DEAD_SHIFT           11
#define CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TRANSITION_SHIFT     12
#define CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TRANSITION_SHIFT     13
#define CHAOS_CODE_RESTRICTION_FLAG_AFFECT_PAUSED_SHIFT         14
#define CHAOs_CODE_RESTRICTION_FLAG_TOGGLE_TITLE_SCREEN_SHIFT   15

enum CHAOS_CODE_RESTRICTION_FLAGS
{
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TIME_STOPPED   = 1 << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TIME_STOPPED_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TIME_STOPPED   = 1 << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TIME_STOPPED_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_CUTSCENE       = 1 << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_CUTSCENE_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_CUTSCENE       = 1 << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_CUTSCENE_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_BOAT_RIDE      = 1 << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_BOAT_RIDE_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_BOAT_RIDE      = 1 << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_BOAT_RIDE_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_EPONA_RIDE     = 1 << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_EPONA_RIDE_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_EPONA_RIDE     = 1 << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_EPONA_RIDE_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_GRABBED        = 1 << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_GRABBED_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_GRABBED        = 1 << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_GRABBED_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_DEAD           = 1 << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_DEAD_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_DEAD           = 1 << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_DEAD_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TRANSITION     = 1 << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TRANSITION_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TRANSITION     = 1 << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TRANSITION_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_PAUSED         = 1 << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TIME_STOPPED_SHIFT,
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TITLE_SCREEN   = 1 << CHAOs_CODE_RESTRICTION_FLAG_TOGGLE_TITLE_SCREEN_SHIFT,
};

#define CHAOS_CODE_RESTRICTION_FLAG_MASK(toggle_time_stopped,                                   \
                                         affect_time_stopped,                                   \
                                         toggle_cutscene,                                       \
                                         affect_cutscene,                                       \
                                         toggle_boat_ride,                                      \
                                         affect_boat_ride,                                      \
                                         toggle_epona_ride,                                     \
                                         affect_epona_ride,                                     \
                                         toggle_grabbed,                                        \
                                         affect_grabbed,                                        \
                                         toggle_dead,                                           \
                                         affect_dead,                                           \
                                         toggle_transition,                                     \
                                         affect_transition,                                     \
                                         affect_paused,                                         \
                                         toggle_title_screen)                                   \
(                                                                                               \
    (((toggle_time_stopped) && 1)   << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TIME_STOPPED_SHIFT) | \
    (((affect_time_stopped) && 1)   << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TIME_STOPPED_SHIFT) | \
    (((toggle_cutscene) && 1)       << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_CUTSCENE_SHIFT) |     \
    (((affect_cutscene) && 1)       << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_CUTSCENE_SHIFT) |     \
    (((toggle_boat_ride) && 1)      << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_BOAT_RIDE_SHIFT) |    \
    (((affect_boat_ride) && 1)      << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_BOAT_RIDE_SHIFT) |    \
    (((toggle_epona_ride) && 1)     << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_EPONA_RIDE_SHIFT) |   \
    (((affect_epona_ride) && 1)     << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_EPONA_RIDE_SHIFT) |   \
    (((toggle_grabbed) && 1)        << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_GRABBED_SHIFT) |      \
    (((affect_grabbed) && 1)        << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_GRABBED_SHIFT) |      \
    (((toggle_dead) && 1)           << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_DEAD_SHIFT) |         \
    (((affect_dead) && 1)           << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_DEAD_SHIFT) |         \
    (((toggle_transition) && 1)     << CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TRANSITION_SHIFT) |   \
    (((affect_transition) && 1)     << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TRANSITION_SHIFT) |   \
    (((affect_paused) && 1)         << CHAOS_CODE_RESTRICTION_FLAG_AFFECT_PAUSED_SHIFT) |       \
    (((toggle_title_screen) && 1)   << CHAOs_CODE_RESTRICTION_FLAG_TOGGLE_TITLE_SCREEN_SHIFT)   \
)                                                                                               \

#define CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_MASK         \
(                                                       \
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TIME_STOPPED |   \
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_CUTSCENE |       \
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_BOAT_RIDE |      \
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_EPONA_RIDE |     \
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_GRABBED |        \
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_DEAD |           \
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TRANSITION |     \
    CHAOS_CODE_RESTRICTION_FLAG_TOGGLE_TITLE_SCREEN     \
)                                                       \

#define CHAOS_CODE_RESTRICTION_FLAG_AFFECT_MASK         \
(                                                       \
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TIME_STOPPED |   \
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_CUTSCENE |       \
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_BOAT_RIDE |      \
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_EPONA_RIDE |     \
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_GRABBED |        \
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_DEAD |           \
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_TRANSITION |     \
    CHAOS_CODE_RESTRICTION_FLAG_AFFECT_PAUSED           \
)                                                       \

struct ChaosCodeDef
{
    f32     probability;   
    u16     restrictions;
    u8      min_time;
    u8      max_time;
    // u8      always_active;
    // u8      update_restrictions;
    // u8      activate_restrictions;
};

struct ChaosSpawnActorCodeDef
{
    u16 object;
    u8  code;
};

struct ChaosCodeSlot
{
    u32 range_start;
    u32 range_end;
    f32 prob_scale;
    u8  code; 
};
 
#define CHAOS_CODE_DEF(min_time, max_time, restrictions, probability) {probability, restrictions, min_time, max_time}

struct ChaosCode
{
    // u32 data;
    // u16 timer;
    u8  timer;
    u8  code;
};

enum CHAOS_CONFIGS
{
    CHAOS_CONFIG_BEER_GOGGLES_BLUR = 0,
    CHAOS_CONFIG_IKANA_CLIMB_TREE_ACTOR_CHASE,
    CHAOS_CONFIG_STONE_TOWER_CLIMB_ACTOR_CHASE,
    CHAOS_CONFIG_DETERMINISTIC_EFFECT_RNG,
    CHAOS_CONFIG_USE_DISRUPTIVE_EFFECT_PROB,
    CHAOS_CONFIG_ALLOW_BEER_GOGGLES_AND_SILENT_FIELD,
    CHAOS_CONFIG_ALLOW_ENEMY_INFIGHTING,
    CHAOS_CONFIG_DPAD_DOWN_TO_DIE,
    CHAOS_CONFIG_USE_PERIODIC_EFFECT_PROB,
    CHAOS_CONFIG_RANDOM_MOUNTAIN_VILLAGE_CLIMB,
    CHAOS_CONFIG_GIVE_FIERCE_DEITY_MASK,
    CHAOS_CONFIG_ALLOW_UNDERWATER_OCARINA,
    CHAOS_CONFIG_SAVE_AT_GAME_CRASH,
    CHAOS_CONFIG_LAST,
};

struct ChaosConfig
{
    const char *    label;
    const char *    description;
    // u32             bit_index;
};

enum CHAOS_MOON_MOVES
{
    CHAOS_MOON_MOVE_SPEEN    = 1,
    CHAOS_MOON_MOVE_BOB      = 1 << 1,
    CHAOS_MOON_MOVE_SWAY     = 1 << 2,
    CHAOS_MOON_MOVE_BEEGER   = 1 << 3,
    CHAOS_MOON_MOVE_HYPE     = 1 << 4,
    CHAOS_MOON_MOVE_LAST     = CHAOS_MOON_MOVE_SPEEN + 
                               CHAOS_MOON_MOVE_BOB +
                               CHAOS_MOON_MOVE_SWAY +
                               CHAOS_MOON_MOVE_BEEGER +
                               CHAOS_MOON_MOVE_HYPE + 1
};

enum CHAOS_BIG_BROTHER_STATES
{
    CHAOS_BIG_BROTHER_STATE_IDLE = 0,
    CHAOS_BIG_BROTHER_STATE_TRACKING,
    CHAOS_BIG_BROTHER_STATE_SLOW_LOCKED_ON,
    CHAOS_BIG_BROTHER_STATE_FAST_LOCKED_ON,
    CHAOS_BIG_BROTHER_STATE_LAST
};

enum CHAOS_ARROW_EFFECTS
{
    CHAOS_ARROW_EFFECT_WEIRD      = 1,
    CHAOS_ARROW_EFFECT_BUCKSHOT   = 1 << 1,
    CHAOS_ARROW_EFFECT_BOMB       = 1 << 2
};

enum CHAOS_WEIRD_UI_MODES
{
    CHAOS_WEIRD_UI_MODE_NONE,
    CHAOS_WEIRD_UI_MODE_WOBBLE,
    CHAOS_WEIRD_UI_MODE_SNAKE,
    CHAOS_WEIRD_UI_MODE_LAST
};

#define CHAOS_MIN_SNAKE_Y               3
#define CHAOS_MAX_SNAKE_Y               23
#define CHAOS_MIN_SNAKE_X               1
#define CHAOS_MAX_SNAKE_X               31
#define CHAOS_MIN_SNAKE_HEART_SPAWN_Y   6
#define CHAOS_SNAKE_START_Y             10

#define CHAOS_SNAKE_GAME_FLAG_BLINK         (1)
#define CHAOS_SNAKE_GAME_FLAG_MOVE_FAST     (1 << 1)

enum CHAOS_SNAKE_GAME_STATES
{
    CHAOS_SNAKE_GAME_STATE_NONE = 0,
    CHAOS_SNAKE_GAME_STATE_INIT,
    CHAOS_SNAKE_GAME_STATE_PLAY,
    CHAOS_SNAKE_GAME_STATE_DIED,
    CHAOS_SNAKE_GAME_STATE_WIN,
    CHAOS_SNAKE_GAME_STATE_LAST,
};

enum CHAOS_SNAKE_MOVE_DIRS
{
    CHAOS_SNAKE_MOVE_DIR_RIGHT,
    CHAOS_SNAKE_MOVE_DIR_LEFT,
    CHAOS_SNAKE_MOVE_DIR_UP,
    CHAOS_SNAKE_MOVE_DIR_DOWN, 
    CHAOS_SNAKE_MOVE_DIR_NONE,
}; 

enum CHAOS_BEER_GOGGLES_STATES
{
    CHAOS_BEER_GOGGLES_STATE_RAMPING_UP,
    CHAOS_BEER_GOGGLES_STATE_ACTIVE,
    CHAOS_BEER_GOGGLES_STATE_RAMPING_DOWN,
    CHAOS_BEER_GOGGLES_STATE_NONE
};

enum CHAOS_RANDOM_SCALING_MODES
{
    CHAOS_RANDOM_SCALING_MODE_ALL,
    CHAOS_RANDOM_SCALING_MODE_ROTATE,
    CHAOS_RANDOM_SCALING_MODE_LAST,
};

enum CHAOS_OUT_OF_SHAPE_STATES
{
    CHAOS_OUT_OF_SHAPE_STATE_SLOWING_DOWN,
    CHAOS_OUT_OF_SHAPE_STATE_GASPING,
    CHAOS_OUT_OF_SHAPE_STATE_NONE
};

enum CHAOS_SNEEZE_STATES
{
    CHAOS_SNEEZE_STATE_SLOWING_DOWN,
    CHAOS_SNEEZE_STATE_SNEEZE,
    CHAOS_SNEEZE_STATE_NONE,
};

enum CHAOS_RANDOM_FIERCE_DEITY_STATES
{
    CHAOS_RANDOM_FIERCE_DEITY_STATE_NONE,
    CHAOS_RANDOM_FIERCE_DEITY_STATE_SWITCH,
    CHAOS_RANDOM_FIERCE_DEITY_STATE_WAIT_FOR_FORM,
    CHAOS_RANDOM_FIERCE_DEITY_STATE_FIERCE_DEITY
};

enum CHAOS_IMAGINARY_FRIENDS_STATES
{
    CHAOS_IMAGINARY_FRIENDS_STATE_SLOWING_DOWN,
    CHAOS_IMAGINARY_FRIENDS_STATE_SCHIZO,
    CHAOS_IMAGINARY_FRIENDS_STATE_NONE,
};

enum CHAOS_LIFTOFF_STATES
{
    CHAOS_LIFTOFF_STATE_NONE,
    CHAOS_LIFTOFF_STATE_PREPARE,
    CHAOS_LIFTOFF_STATE_COUNTDOWN,
    CHAOS_LIFTOFF_STATE_BEGIN_LAUNCH,
    CHAOS_LIFTOFF_STATE_FLY
};

enum CHAOS_UNSTABLE_ROOMS_BEHAVIORS
{
    CHAOS_UNSTABLE_ROOMS_BEHAVIOR_WOBBLE           = 1,
    CHAOS_UNSTABLE_ROOMS_BEHAVIOR_SNAP_TO_PLAYER   = 1 << 1,
    CHAOS_UNSTABLE_ROOMS_BEHAVIOR_ROTATE           = 1 << 2,
    CHAOS_UNSTABLE_ROOMS_BEHAVIOR_LAST             = CHAOS_UNSTABLE_ROOMS_BEHAVIOR_WOBBLE + 
                                                     CHAOS_UNSTABLE_ROOMS_BEHAVIOR_SNAP_TO_PLAYER +
                                                     CHAOS_UNSTABLE_ROOMS_BEHAVIOR_ROTATE + 1
};

enum CHAOS_FAST_TIME_STATES
{
    CHAOS_FAST_TIME_STATE_NONE,
    CHAOS_FAST_TIME_STATE_SPEEDING_UP,
};

enum CHAOS_BAD_CONNECTION_MODES
{
    CHAOS_BAD_CONNECTION_ROLLBACK,
    CHAOS_BAD_CONNECTION_BUFFER,
    CHAOS_BAD_CONNECTION_LAST
};

enum CHAOS_ROLLBACK_STATES
{
    CHAOS_ROLLBACK_STATE_CAPTURE_SNAPSHOT,
    CHAOS_ROLLBACK_STATE_APPLY_SNAPSHOT,
    CHAOS_ROLLBACK_STATE_LAST
};
 
#define INVALID_CODE_INDEX      0xff 
#define MAX_CHAOS_TIMER         8
#define MIN_CHAOS_TIMER         2
#define CHAOS_SECONDS_TO_FRAMES(seconds)    (((u16)(seconds)) * (20))
#define CHAOS_MAX_DISRUPTIVE_PROBABILITY_SCALE 7.0f
#define CHAOS_MAX_BEER_ALPHA 210
#define CHAOS_MIN_PERIODIC_PROBABILITY_SCALE 0.333333f
// #define CHAOS_LOW_PERIODIC_PROBABILITY_MAX_SCALE (CHAOS_MIN_PERIODIC_PROBABILITY_SCALE + 0.5f)
#define CHAOS_MAX_PERIODIC_PROBABILITY_SCALE 3.0f

#define MAX_SPAWNED_ACTORS      48
#define ACTOR_DESPAWN_TIMER     10
#define CHAOS_MAX_STALCHILDS    5

#define CHAOS_FAST_TIME_OFFSET 48

#define CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_WIDTH     60
#define CHAOS_MOUNTAIN_VILLAGE_LADDER_HALF_HEIGHT    40
#define CHAOS_MAX_MOUNTAIN_VILLAGE_LADDER_SEGS      128
#define CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_X        -400
#define CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_X         400
#define CHAOS_MOUNTAIN_VILLAGE_LADDER_MIN_Y         0
#define CHAOS_MOUNTAIN_VILLAGE_LADDER_MAX_Y         1380
#define CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_Y_OFFSET  28
#define CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_X_OFFSET  435
#define CHAOS_MOUNTAIN_VILLAGE_LADDER_COL_Z_OFFSET -1087

enum CHAOS_SIMON_SAYS_CONFIGS
{
    CHAOS_SIMON_SAYS_CONFIG_PRESS_KEY_OR_DIE,
    CHAOS_SIMON_SAYS_CONFIG_PRESS_KEY_TO_DIE,
    CHAOS_SIMON_SAYS_CONFIG_PRESS_KEY_AND_DIE,
    CHAOS_SIMON_SAYS_CONFIG_DO_NOT_PRESS_KEY_TO_NOT_NOT_DIE,
    CHAOS_SIMON_SAYS_CONFIG_DO_NOT_PRESS_KEY_OR_DIE,
    CHAOS_SIMON_SAYS_CONFIG_DO_NOT_PRESS_KEY_TO_NOT_DIE,
    CHAOS_SIMON_SAYS_CONFIG_DO_NOT_NOT_PRESS_KEY_TO_DIE,
    CHAOS_SIMON_SAYS_CONFIG_DO_NOT_NOT_PRESS_KEY_TO_NOT_DIE,
    CHAOS_SIMON_SAYS_CONFIG_DO_NOT_NOT_PRESS_KEY_OR_DIE,
    CHAOS_SIMON_SAYS_CONFIG_PRESS_KEY_NOT_TO_NOT_DIE,
    CHAOS_SIMON_SAYS_CONFIG_LAST
};

enum CHAOS_SIMON_SAYS_STATES
{
    CHAOS_SIMON_SAYS_STATE_IDLE,
    CHAOS_SIMON_SAYS_STATE_START,
    CHAOS_SIMON_SAYS_STATE_WAIT_INPUT,
    CHAOS_SIMON_SAYS_STATE_WAIT_DEATH,
};

enum CHAOS_SIMON_SAYS_KEYS
{
    CHAOS_SIMON_SAYS_KEY_DUP,
    CHAOS_SIMON_SAYS_KEY_DRIGHT,
    CHAOS_SIMON_SAYS_KEY_DDOWN,
    CHAOS_SIMON_SAYS_KEY_DLEFT,
    CHAOS_SIMON_SAYS_KEY_LAST
};

struct SimonSaysConfig
{
    const char *    str;
    u16             match_to_live;
    u16             text_x_offset;
    u16             timeout;
};

struct ChaosActor
{
    Actor *     actor;
    u16         timer;
};

#define CHAOS_MAX_HEART_CONTAINERS 64

struct HeartContainerPos
{
    s16 pos_x;
    s16 pos_y;
};

struct ChaosParentActorSnapshot
{
    union
    {
        Actor       actor;
        EnRr        like_like;
        EnWallmas   wallmaster;
        EnHorse     epona;
        Boss03      gyorg;
        Boss07      majora;
    };

    Actor *         instance;
};

struct ChaosMagicArrowSnapshot
{
    union
    {
        Actor       actor;
        ArrowFire   fire;
        ArrowLight  light;
        ArrowIce    ice;      
    };

    Actor *         instance;
};

struct ChaosChildActorSnapshot
{
    union
    {
        Actor       actor;
        EnDg        dog;
        EnRacedog   race_dog;
        EnKusa      bush;
        ObjTsubo    pot;
        EnIshi      rock;
        EnBom       bomb;
        EnBombf     bomb_flower;
        EnArrow     arrow;
    };

    Actor *         instance;
};

struct ChaosDoorActorSnapshot
{
    union
    {
        Actor           actor;
        DoorShutter     shutter;
        BgOpenShutter   opening_shutter;
        EnDoorEtc       door_etc;
        EnDoor          door;
    };

    Actor *             instance;
};

#define MAX_ACTIVE_CODES        8
#define MAX_AIR_SUPPORT_ARWINGS 3

typedef struct ChaosContext 
{
    OSTime                  prev_update_counter; 
    size_t                  chaos_keep_size;
    u32                     chaos_elapsed_usec; 
    u32                     code_elapsed_usec; 
    f32                     interruption_probability_scale;
    f32                     periodic_probability_scale;
    f32                     periodic_probability_scale_target;
    u16                     periodic_probability_update_timer;
    u16                     chaos_timer;
    u16                     effect_restrictions;
    u16                     chaos_keep_largest_object;
    s16                     input_mash_accumulator;
    u8                      active_code_count;
    u8                      update_enabled;

    struct ChaosCode        active_codes[MAX_ACTIVE_CODES];
    u8                      active_code_indices[CHAOS_CODE_LAST];

    u8                      enabled_code_count;
    struct ChaosCodeSlot    enabled_codes[CHAOS_CODE_LAST];
    u8                      enabled_code_indices[CHAOS_CODE_LAST];
    u8                      need_update_distribution;
    u8                      hide_actors; 

    u8                      queued_spawn_actor_code;
    u8                      loaded_object_id;
    u8                      chaos_keep_slot;
    u8                      fake_crash;
    u8 *                    fake_crash_pointer;

    void                  (*EnRr_SpitPlayer)(EnRr* this, PlayState* play);

    struct 
    {
        f32                 pitch;
        f32                 yaw;
        f32                 bob;
        f32                 sway;

        union
        {
            f32             scale;
            f32             eye_glow;
        };

        u8                  big_brother_state;
        u8                  moon_dance;
        u8                  need_update_bell_time;
        u32                 moon_crash_timer;
        s32                 moon_crash_time_offset;

    } moon;

    struct
    {
        f32                     beer_x_offset;
        f32                     beer_y_offset;
        f32                     beer_pitch;
        f32                     beer_yaw;
        f32                     beer_roll;
        f32                     beer_time;
        Vec3f                   beer_sway;

        f32                     small_c_scale;
        Vec3f                   prev_camera_pos;
        Vec3f                   camera_velocity;
        Vec3f                   prev_camera_velocity[2];
        // Vec3f                   prev_camera_velocity;
    }view;

    struct
    {
        PlayerAnimationHeader *         cur_animation;

        f32                             cur_animation_frame;
        f32                             cur_animation_play_speed;
        u32                             cur_animation_mode;

        f32                             out_of_shape_speed_scale;
        f32                             speed_boost_speed_scale;
        f32                             imaginary_friends_speed_scale;
        Vec2f                           ear_scales[2];
        u8                              beer_alpha;
        u8                              tunic_r;
        u8                              tunic_g;
        u8                              tunic_b;
        u8                              out_of_shape_state;
        u8                              beer_goggles_state;
        u8                              fierce_deity_state;
        u8                              imaginary_friends_state;
        u8                              imaginary_friends_anim_index;
        u8                              speed_boost_state;
        u8                              fierce_deity_counter;
        u8                              prev_link_form;
        u8                              liftoff_timer;
        u8                              liftoff_state;
        u16                             syke_health;
        s16                             imaginary_friends_target_yaw;
        u8                              syke;
        u8                              random_knockback_timer;
        u8                              magic_gauge_sfx_timer;
        u8                              trap_flap_timer;
        f32                             limb_scales[PLAYER_LIMB_MAX];
        f32                             temp_limb_scale;
        u8                              random_scaling_mode;
        u8                              scaled_limb_index;
        u8                              dpad_down_timer;
        u8                              magic_state;   
        s16                             magic_available;                           
        Player                          player_snapshot;
        s16                             health;
        s8                              ammo[24];

        struct ChaosChildActorSnapshot  child_snapshot;
        struct ChaosMagicArrowSnapshot  arrow_snapshot;
        struct ChaosParentActorSnapshot parent_snapshot;
        struct ChaosDoorActorSnapshot   door_snapshot;

        u8                              random_autojump_timer;

        u8                              bad_connection_mode;
        u8                              bad_connection_timer;
        union
        {
            u8                          snapshot_timer; 
            u8                          input_frames;
        };

        u8                              simon_says_keys[2];
        u8                              simon_says_config;
        // u8                              simon_says_death_queued;
        u8                              simon_says_timer;
        u8                              simon_says_state;
    } link;

    struct
    {
        struct ChaosActor       slots[MAX_SPAWNED_ACTORS];
        u8                      spawned_actors;
    } actors;

    struct
    {
        u8                      enabled_scenes[ENTR_SCENE_MAX];
        u8                      enabled_scene_count;
    } entrance;

    struct
    {
        Vec3f                   talk_translation;
        Vec3f                   talk_scale;
        Vec3s                   talk_rotation;
    } npc;

    struct
    {
        EnNiw                   cucco;
    } chicken;

    struct 
    {
        u8                      change_timer;
    } bgm;

    struct
    {
        struct HeartContainerPos    heart_containers[CHAOS_MAX_HEART_CONTAINERS];
        u8                          orig_heart_count;
        u8                          heart_count;
        u8                          snake_state;
        u8                          blink_timer;
        u8                          move_timer;
        u8                          next_move_dir;
        u8                          move_dir;
        s8                          stick_x;
        s8                          stick_y;
        u8                          flags;             

        // s8 life_meter_offset[20][2];
        // s8 magic_container_offset[2][2];
        // s8 rupee_counter_offest[2][3];
        // s8 rupee_icon_offset[2];
        // s8 cbutton_offset[8][2];
        // s8 bbutton_offset[2][2];
        // s8 abutton_offset[2][2];
        // s8 start_button_offset[2][2];
        // s8 clock_offset[18][2];
    } ui;

    struct
    {
        Vec3f                   transition_actor_pos;
        s16                     transition_actor_yaw;
        u8                      transition_actor_side;
        u8                      pivot_room;
        RoomVertListList *      vert_list_list[2];
        u8                      weirdness_behavior;
        u8                      snap_to_player_timer;
        u8                      room_rotation_timer;
        Vec3s                   room_rotation;

    } room;

    struct 
    {
        u8                      fast_time_state;
    } time;

    struct
    {
        f32                     fog_lerp;
        f32                     length_contraction_scale;
        Vec3f                   length_contraction_axis;
        Mtx                     length_contraction_matrix;
        u8                      stalchild_spawn_timer;
        u8                      stalchild_count;
        u8                      spawn_stalchilds;
    } env;

    struct
    {
        EnArwing *              arwings[MAX_AIR_SUPPORT_ARWINGS];
        Actor *                 enemies[MAX_AIR_SUPPORT_ARWINGS];
        u8                      arwing_spawn_timer;
        u8                      arwing_count;
        u8                      enemy_count;
    } air_support;
    
} ChaosContext;

struct ChaosConfigSetup
{
    u8  major;
    u8  minor;
    u8  patch;
    void (*config)(void);
};

#define CHAOS_ADD_RESULT_OK             0
#define CHAOS_ADD_RESULT_ALREADY_ACTIVE 1
#define CHAOS_ADD_RESULT_NO_SLOTS       2

/* forward declaration */
struct PlayState;

u32 Chaos_Rand(void);

f32 Chaos_ZeroOne(void);

s16 Chaos_RandS16Offset(s16 base, s16 range);

u32 Chaos_RandNext(void);

void Chaos_InitRng(void);

void Chaos_Init(void);

void Chaos_UpdateChaos(PlayState *playstate);

void Chaos_PrintCodes(PlayState *playstate, Input *input);

void Chaos_AppendActionChange(PlayState *play, u32 action);

u8 Chaos_ActivateCode(u8 code, u8 seconds);

void Chaos_DeactivateCodeAtIndex(u8 index);

void Chaos_DeactivateCode(u8 code);

void Chaos_DeactivateOneCode();

u8 Chaos_IsCodeInActiveList(u8 code);

u8 Chaos_IsCodeActive(u8 code);

struct ChaosCode *Chaos_GetCode(u8 code);

void Chaos_EnableCode(u8 code, f32 prob_scale);

void Chaos_DisableCode(u8 code);

u8 Chaos_IsCodeEnabled(u8 code);

void Chaos_ClearEnabledCodes(void);

void Chaos_UpdateCodeDistribution(void);

u8 Chaos_CanUpdateChaos(struct PlayState *play);

void Chaos_StepUpInterruptionProbabilityScale(u8 seconds);

void Chaos_StepDownInterruptionProbabilityScale(void);

void Chaos_UpdatePeriodicProbabilityScale(void);

u16 Chaos_EffectRestrictions(struct PlayState *play);

Actor *Chaos_SpawnActor(ActorContext *context, PlayState *play, s16 actor_id, f32 pos_x, f32 pos_y, f32 pos_z, s16 rot_x, s16 rot_y, s16 rot_z, s32 params);

void Chaos_SpawnRedeadDanceParty(ActorContext *context, PlayState *play, Vec3f *player_pos);

Actor* Chaos_SpawnAsChild(ActorContext* context, Actor* parent, PlayState* play, s16 actor_id, f32 pos_x, f32 pos_y, f32 pos_z, s16 rot_x, s16 rot_y, s16 rot_z, s32 params);

void Chaos_DestroyFunction(struct Actor *actor, struct PlayState *play);

void Chaos_StalchildDestroyFunction(struct Actor *actor, struct PlayState *play);

void Chaos_KillActorAtIndex(u32 index);

void Chaos_DropActorAtIndex(u32 index);

void Chaos_DropActor(Actor *actor);

void Chaos_ClearActors(void);

u16 Chaos_RandomEntrance(PlayState *play);

void Chaos_UpdateEntrances(PlayState *play);

void Chaos_UpdateEnabledChaosEffectsAndEntrances(PlayState *this);

u32 Chaos_UpdateSnakeGame(PlayState *play, Input *input);

void Chaos_UpdateSimonSays(PlayState *play, Input *input);

void Chaos_PrintSnakeGameStuff(PlayState *play);

void Chaos_PrintSimonSaysStuff(PlayState *play);

void Chaos_SetConfigFlag(u32 config, u32 value);

u32 Chaos_GetConfigFlag(u32 config);

s32 Chaos_TimeUntilMoonCrash(void);

void Chaos_StartMoonCrash(void);

void Chaos_ClearMoonCrash(void);

void Chaos_NukeSnapshots(void);

void Chaos_SnapshotChild(PlayState *play, Actor *child);

void Chaos_UnsnapshotChild(PlayState *play, Actor *child);

void Chaos_SnapshotParent(PlayState *play, Actor *parent);

void Chaos_UnsnapshotParent(PlayState *play, Actor *parent);

void Chaos_SnapshotMagicArrow(PlayState *play, Actor *magic_arrow);

void Chaos_UnsnapshotMagicArrow(PlayState *play, Actor *magic_arrow);

void Chaos_SnapshotDoor(PlayState *play, Actor *door);

void Chaos_UnsnapshotDoor(PlayState *play, Actor *door);

void Chaos_SnapshotChildActorDestroy(Actor *actor, PlayState *play);

void Chaos_SnapshotParentActorDestroy(Actor *actor, PlayState *play);

void Chaos_SnapshotMagicArrowDestroy(Actor *actor, PlayState *play);

void Chaos_SnapshotDoorDestroy(Actor *actor, PlayState *play);

void Chaos_LikeLikeDestroyFunction(Actor *actor, PlayState *play);

void Chaos_SetV044ConfigDefaults(void);

void Chaos_SetV045ConfigDefaults(void);

void Chaos_SetV046ConfigDefaults(void);

void Chaos_SetV050ConfigDefaults(void);

void Chaos_SetV053ConfigDefaults(void);

void Chaos_SetV055ConfigDefaults(void);

void Chaos_SetConfigDefaults(void);

void Chaos_RandomizeMountainVillageClimb(struct PlayState *play);

#endif
