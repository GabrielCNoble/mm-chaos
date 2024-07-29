#ifndef CHAOS_FUCKERY_H
#define CHAOS_FUCKERY_H

#include "ultra64.h"
#include "z64.h"

/* 
    random-knockback/poke/shock/ice-trap should not be active when there's a 
    like-like around. It gets the player stuck midair, in a falling pose
*/

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
    /* sets tunic to random color */
    // CHAOS_CODE_TUNIC_COLOR,
    /* arrows explode on impact */
    CHAOS_CODE_BOMB_ARROWS,
    /* arrows fly erratically */
    CHAOS_CODE_WEIRD_ARROWS,
    /* bow fires several arrows at once */
    CHAOS_CODE_BUCKSHOT_ARROWS,
    /* gives bombs random timers */
    CHAOS_CODE_RANDOM_BOMB_TIMER,
    /* spawns like like above player 
        
       TODO: make like-like steal more items and teleport them to the curiosity shop
    */
    CHAOS_CODE_LOVELESS_MARRIAGE,
    /* makes the ui behave unpredictably (shake, wave, bounce around) */
    CHAOS_CODE_WEIRD_UI,
    /* activate motion blur */
    CHAOS_CODE_BEER_GOGGLES,
    /* randomly changes magic */
    CHAOS_CODE_CHANGE_MAGIC,
    /* self explanatory */
    CHAOS_CODE_INVINCIBLE,
    /* enemies explode when killed */
    // CHAOS_CODE_VILETILE_ENEMIES,

    // CHAOS_CODE_ENTRANCE_RANDO,
    // CHAOS_CODE_SLOW_ANIMATION,
    // CHAOS_CODE_RANDOM_ITEM,
    // CHAOS_CODE_BUTTERFINGERS,
    // CHAOS_CODE_INCREDIBLE_KNOCKBACK,
    // CHAOS_CODE_RANDOM_SWORD_TRAILS,
    // CHAOS_CODE_NO_Z_TARGETING,
    
    // CHAOS_CODE_HIGH_PING,
    // CHAOS_CODE_UPSIDE_DOWN,
    
    // CHAOS_CODE_TAKE_SCREENSHOT,
    // CHAOS_CODE_MOVE_BACKWARDS,
    
    // CHAOS_CODE_MUSIC_SWAP,
    // CHAOS_CODE_RANDOM_SCALING,
    // CHAOS_CODE_SWAP_HEAL_AND_HURT,
    // CHAOS_CODE_SHOW_SCREENSHOT,
    /* spawn walls of fire around player */
    // CHAOS_CODE_ANTIVIRUS,
    // CHAOS_CODE_RANDOM_HEALTH_DOWN,
    // CHAOS_CODE_FASTER_ANIMATIONS,
    // CHAOS_CODE_SONG_OF_STORMS,
    // CHAOS_CODE_SIGNPOST,
    // CHAOS_CODE_SLIPPERY_FLOORS,
    // CHAOS_CODE_ROTATE_SLOWLY,
    // CHAOS_CODE_ENVIRONMENT_SETTINGS,
    
    // CHAOS_CODE_AMBUSH,
    // CHAOS_CODE_SHORT_TERM_MEMORY_LOSS,
    
    // CHAOS_CODE_RANDOM_PLAYER_SOUNDS,
    // CHAOS_CODE_SLOW_DOWN,
    // CHAOS_CODE_JUNK_ITEM,
    // CHAOS_CODE_RANDOM_C_BUTTONS,
    // CHAOS_CODE_HEAT_TIMER,
    // CHAOS_CODE_QUICKSAND,
    // CHAOS_CODE_STARFOX,
    // CHAOS_CODE_SNAP_TO_FLOOR,
    // CHAOS_CODE_TERRIBLE_MUSIC,
    // CHAOS_CODE_ACTIVATE_SWORD_COLLIDER,
    // CHAOS_CODE_NO_AUTOJUMP,
    // CHAOS_CODE_NAVI,
    // CHAOS_CODE_SPEEDBOOST,
    // CHAOS_CODE_EXPLOSIONS,
    // CHAOS_CODE_CARTIDGE_TILTING,
    // CHAOS_CODE_DIVE,
    // CHAOS_CODE_NO_COLLECTIBLES,
    // CHAOS_CODE_IGNORE_WATER,
    // CHAOS_CODE_ALL_SOUNDS_ARE_PLAYER,
    // CHAOS_CODE_CHANGE_MAGIC,
    // CHAOS_CODE_INFINITE_HOVER_BOOTS,
    // CHAOS_CODE_BUTTON_SWAP,
    // CHAOS_CODE_MAGIC_ARMOR,

    // CHAOS_CODE_PUZZLE_EVENT,

    // CHAOS_CODE_PUT_AWAY_ITEMS,
    // CHAOS_CODE_TEXTBOX,
    // CHAOS_CODE_EARTHQUAKE,
    // CHAOS_CODE_CAMERA_SHAKE,
    // CHAOS_CODE_VOID_OUT,
    // CHAOS_CODE_CLIMB_EVERYTHING,
    // CHAOS_CODE_SUN_SONG,
    // CHAOS_CODE_USE_OCARINA,


    /* slow goron roll */
    // CHAOS_CODE_GOT_A_FLAT,
    /* swap tatl model with the moon's */
    // CHAOS_CODE_EVIL_VOICES,
    /* moon always faces the player */
    // CHAOS_CODE_BIG_BROTHER,
    /* walk over water */
    // CHAOS_CODE_HIM,
    /* fake insta-death */
    // CHAOS_CODE_SYKE,
    /* all sounds are the same player sound */
    // CHAOS_CODE_UNORIGINAL,
    /* tatl frequently pipes up with random quest tips */
    // CHAOS_CODE_PARTICIPATION_AWARD,
    /* drop bombs around player */
    // CHAOS_CODE_AIR_STRIKE,
    /* spawns snow head wind effect */
    // CHAOS_CODE_BLIZZARD,
    /* spawns the 4 ghost sisters */
    // CHAOS_CODE_HEY_SOUL_SISTERS,
    /* spawns dancing redeads around player */
    // CHAOS_CODE_REDEADASS_GROOVE,
    /* spawns hostile redeads around player */
    // CHAOS_CODE_PAIN_IN_THE_REDEADASS,
    /* spawns majora's wrath */
    // CHAOS_CODE_MAJORAS_WRATH
    /* set all enemies on fire */
    // CHAOS_CODE_TORCH_ENEMIES,
    /* spawns a wasp nest, which when broken spawns three random enemies */
    // CHAOS_CODE_LOTTERY_NEST,
    /* player randomly screams */
    // CHAOS_CODE_TOURETTE,
    /* player takes flight like a gossip stone */
    // CHAOS_CODE_LIFTOFF,
    
    
    
    CHAOS_CODE_LAST
};

struct ChaosCodeDef
{
    u8 min_time;
    u8 max_time;
    u8 always_update;
};

struct ChaosCode
{
    u32 data;
    u16 timer;
    u8  code;
};

enum CHAOS_MOON_MOVES
{
    CHAOS_MOON_MOVE_SPEEN    = 1,
    CHAOS_MOON_MOVE_BOB      = 1 << 1,
    CHAOS_MOON_MOVE_SWAY     = 1 << 2,
    CHAOS_MOON_MOVE_BEEGER   = 1 << 3,
    CHAOS_MOON_MOVE_HYPE     = 1 << 4,
    CHAOS_MOON_MOVE_LAST
};

enum CHAOS_ARROW_EFFECTS
{
    CHAOS_ARROW_EFFECT_WEIRD      = 1,
    CHAOS_ARROW_EFFECT_BUCKSHOT   = 1 << 1,
    CHAOS_ARROW_EFFECT_BOMB       = 1 << 2
};

enum CHAOS_UI_EFFECTS
{
    CHAOS_UI_EFFECT_SHAKE   = 1,
    CHAOS_UI_EFFECT_BOUNCE  = 1 << 1,
    CHAOS_UI_EFFECT_SPEEN   = 1 << 2
};

#define INVALID_CODE_INDEX      0xff 
#define MAX_CHAOS_TIMER         8
#define MIN_CHAOS_TIMER         2
#define CHAOS_SECONDS_TO_FRAMES(seconds)    (((u16)(seconds)) * (20))

#define MAX_SPAWNED_ACTORS  32
#define ACTOR_DESPAWN_TIMER 10

struct ChaosActor
{
    Actor *     actor;
    u16         timer;
};

// struct ChaosLikeLikeItems
// {

// };

#define MAX_ACTIVE_CODES 8
typedef struct ChaosContext 
{
    OSTime                  prev_update_counter; 
    u32                     chaos_elapsed_usec; 
    u32                     code_elapsed_usec; 
    u16                     chaos_timer;
    u8                      active_code_count;
    u8                      update_enabled;
    struct ChaosCode        active_codes[MAX_ACTIVE_CODES];
    u8                      active_code_indices[CHAOS_CODE_LAST];

    struct 
    {
        f32                 pitch;
        f32                 yaw;
        f32                 bob;
        f32                 sway;
        f32                 scale;
    } moon;

    struct
    {
        f32                 beer_x_offset;
        f32                 beer_y_offset;
        f32                 beer_pitch;
        f32                 beer_yaw;
        f32                 beer_roll;
        Vec3f               beer_sway;
        // Vec3f               beer_pitch_yaw;
        // Vec3f               beer_forward_vec;
        // Vec3f               beer_right_vec;
        u8                  beer_alpha;
        u8                  tunic_r;
        u8                  tunic_g;
        u8                  tunic_b;
    } link;

    struct
    {
        struct ChaosActor   slots[MAX_SPAWNED_ACTORS];
        u8                  spawned_actors;
    } actors;
    
} ChaosContext;

#define CHAOS_ADD_RESULT_OK             0
#define CHAOS_ADD_RESULT_ALREADY_ACTIVE 1
#define CHAOS_ADD_RESULT_NO_SLOTS       2

/* forward declaration */
struct PlayState;

void Chaos_Init(void);

void Chaos_UpdateChaos(PlayState *playstate);

void Chaos_PrintCodes(PlayState *playstate);

u8 Chaos_AddCode(u8 code, u8 seconds);

u8 Chaos_DropCodeAtIndex(u8 index);

u8 Chaos_IsCodeActive(u8 code);

struct ChaosCode *Chaos_GetCode(u8 code);

u8 Chaos_CanUpdateChaos(struct PlayState *play);

Actor *Chaos_SpawnActor(ActorContext *context, PlayState *play, s16 actor_id, f32 pos_x, f32 pos_y, f32 pos_z, s16 rot_x, s16 rot_y, s16 rot_z, s32 params);

void Chaos_KillActorAtIndex(u32 index);

void Chaos_DropActorAtIndex(u32 index);

void Chaos_DropActor(Actor *actor);

void Chaos_ClearActors(void);

#endif
