#ifndef CHAOS_FUCKERY_H
#define CHAOS_FUCKERY_H

#include "ultra64.h"
#include "z64.h"
#include "padutils.h"
#include "overlays/actors/ovl_En_Niw/z_en_niw.h"

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
    /* fake insta-death */
    CHAOS_CODE_SYKE,
    /* real insta-death */
    CHAOS_CODE_DIE,
    /* player randomly screams */
    CHAOS_CODE_TRAP_FLAP,
    /* opens random textbox */
    CHAOS_CODE_TEXTBOX,
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
    CHAOS_CODE_TUNIC_COLOR,
    /* makes skybox rotate randomly */
    CHAOS_CODE_WEIRD_SKYBOX,
    /* deactivates a random owl after use */
    CHAOS_CODE_SINGLE_ACTION_OWL,
    /* makes the player pull up the ocarina */
    CHAOS_CODE_PLAY_OCARINA,
    /* link stops and sneezes */
    CHAOS_CODE_SNEEZE,
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
    
    // CHAOS_CODE_HIGH_PING,
    // CHAOS_CODE_UPSIDE_DOWN,
    
    // CHAOS_CODE_TAKE_SCREENSHOT,
    // CHAOS_CODE_SHOW_SCREENSHOT,
    
    // CHAOS_CODE_MUSIC_SWAP,
    
    
    /* spawn walls of fire around player */
    // CHAOS_CODE_ANTIVIRUS,
    // CHAOS_CODE_SONG_OF_STORMS,
    // CHAOS_CODE_SIGNPOST,
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
    // CHAOS_CODE_SPEEDBOOST,
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
    // CHAOS_CODE_THE_END_OF_THE_WORLD,
    // CHAOS_CODE_THE_WORLD_IS_SAVED,

    /* swap tatl model with the moon's */
    // CHAOS_CODE_EVIL_VOICES,
    
    /* walk over water */
    // CHAOS_CODE_HIM,
    
    /* enemies explode when killed */
    // CHAOS_CODE_VILETILE_ENEMIES,
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
    
    /* player takes flight like a gossip stone */
    // CHAOS_CODE_LIFTOFF,
    /* warps player to majora's lair, then warps them back out */
    // CHAOS_CODE_FAKE_MAJORAS_LAIR,

    /* camera sees from above (similar to gta) */
    // CHAOS_CODE_BIRDSEYE_VIEW,

    /* randomly spawns elegy statue behind player */
    // CHAOS_CODE_BEN,
    /* when in goron shape, goron curls up and stays like that */
    // CHAOS_CODE_SHY_GORON,
    /* makes the player bonk on nothing while rolling */
    // CHAOS_CODE_INVISIBLE_WALL,
    
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

enum CHAOS_MOON_MOVES
{
    CHAOS_MOON_MOVE_SPEEN    = 1,
    CHAOS_MOON_MOVE_BOB      = 1 << 1,
    CHAOS_MOON_MOVE_SWAY     = 1 << 2,
    CHAOS_MOON_MOVE_BEEGER   = 1 << 3,
    CHAOS_MOON_MOVE_HYPE     = 1 << 4,
    CHAOS_MOON_MOVE_LAST
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

enum CHAOS_UI_EFFECTS
{
    CHAOS_UI_EFFECT_SHAKE   = 1,
    CHAOS_UI_EFFECT_BOUNCE  = 1 << 1,
    CHAOS_UI_EFFECT_SPEEN   = 1 << 2
};

enum CHAOS_BEER_GOGGLES_STATES
{
    // CHAOS_BEER_GOGGLES_STATE_JUST_STARTED_RAMPING_UP,
    CHAOS_BEER_GOGGLES_STATE_RAMPING_UP,
    CHAOS_BEER_GOGGLES_STATE_ACTIVE,
    // CHAOS_BEER_GOGGLES_STATE_JUST_STARTED_RAMPING_DOWN,
    CHAOS_BEER_GOGGLES_STATE_RAMPING_DOWN,
    CHAOS_BEER_GOGGLES_STATE_NONE
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
    CHAOS_RANDOM_FIERCE_DEITY_STATE_FIERCE_DEITY
};

enum CHAOS_IMAGINARY_FRIENDS_STATES
{
    CHAOS_IMAGINARY_FRIENDS_STATE_SLOWING_DOWN,
    CHAOS_IMAGINARY_FRIENDS_STATE_SCHIZO,
    CHAOS_IMAGINARY_FRIENDS_STATE_NONE,
};

#define INVALID_CODE_INDEX      0xff 
#define MAX_CHAOS_TIMER         8
#define MIN_CHAOS_TIMER         2
#define CHAOS_SECONDS_TO_FRAMES(seconds)    (((u16)(seconds)) * (20))

#define MAX_SPAWNED_ACTORS  12
#define ACTOR_DESPAWN_TIMER 10

struct ChaosActor
{
    Actor *     actor;
    u16         timer;
};

#define MAX_ACTIVE_CODES 8
typedef struct ChaosContext 
{
    OSTime                  prev_update_counter; 
    u32                     chaos_elapsed_usec; 
    u32                     code_elapsed_usec; 
    u16                     chaos_timer;
    u16                     effect_restrictions;
    u16                     chaos_keep_size;
    u16                     chaos_keep_largest_object;
    u8                      active_code_count;
    u8                      update_enabled;
    // u8                      effect_restrictions;
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
        
    } moon;

    struct
    {
        PlayerAnimationHeader * cur_animation;
        // PlayerAnimationHeader * imaginary_friends_animation;
        f32                     cur_animation_frame;
        f32                     cur_animation_play_speed;
        u32                     cur_animation_mode;

        f32                     out_of_shape_speed_scale;
        f32                     sneeze_speed_scale;
        f32                     imaginary_friends_speed_scale;
        f32                     beer_x_offset;
        f32                     beer_y_offset;
        f32                     beer_pitch;
        f32                     beer_yaw;
        f32                     beer_roll;
        Vec3f                   beer_sway;
        Vec2f                   ear_scales[2];
        u8                      beer_alpha;
        u8                      tunic_r;
        u8                      tunic_g;
        u8                      tunic_b;
        u8                      out_of_shape_state;
        u8                      beer_goggles_state;
        u8                      fierce_deity_state;
        u8                      imaginary_friends_state;
        u8                      imaginary_friends_anim_index;
        u8                      sneeze_state;
        u8                      fierce_deity_counter;
        u8                      prev_link_form;
        u16                     syke_health;
        s16                     imaginary_friends_target_yaw;
        u8                      syke;
        u8                      random_knockback_timer;
        u8                      magic_gauge_sfx_timer;
        u8                      trap_flap_timer;
    } link;

    struct
    {
        struct ChaosActor   slots[MAX_SPAWNED_ACTORS];
        u8                  spawned_actors;
    } actors;

    struct
    {
        u8 enabled_scenes[ENTR_SCENE_MAX];
        u8 enabled_scene_count;
    } entrance;

    struct
    {
        Vec3f               talk_translation;
        Vec3f               talk_scale;
        Vec3s               talk_rotation;
    } npc;

    struct
    {
        EnNiw               cucco;
    } chicken;

    struct 
    {
        u8                  change_timer;
    } bgm;

    // struct
    // {
    //     s8 life_meter_offset[20][2];
    //     s8 magic_container_offset[2][2];
    //     s8 rupee_counter_offest[2][3];
    //     s8 rupee_icon_offset[2];
    //     s8 cbutton_offset[8][2];
    //     s8 bbutton_offset[2][2];
    //     s8 abutton_offset[2][2];
    //     s8 start_button_offset[2][2];
    //     s8 clock_offset[18][2];
    // } ui;
    
} ChaosContext;

#define CHAOS_ADD_RESULT_OK             0
#define CHAOS_ADD_RESULT_ALREADY_ACTIVE 1
#define CHAOS_ADD_RESULT_NO_SLOTS       2

/* forward declaration */
struct PlayState;

void Chaos_Init(void);

void Chaos_UpdateChaos(PlayState *playstate);

void Chaos_PrintCodes(PlayState *playstate, Input *input);

u8 Chaos_ActivateCode(u8 code, u8 seconds);

void Chaos_DeactivateCodeAtIndex(u8 index);

void Chaos_DeactivateCode(u8 code);

u8 Chaos_IsCodeInActiveList(u8 code);

u8 Chaos_IsCodeActive(u8 code);

struct ChaosCode *Chaos_GetCode(u8 code);

void Chaos_EnableCode(u8 code, f32 prob_scale);

void Chaos_DisableCode(u8 code);

u8 Chaos_IsCodeEnabled(u8 code);

void Chaos_ClearEnabledCodes(void);

void Chaos_UpdateCodeDistribution(void);

u8 Chaos_CanUpdateChaos(struct PlayState *play);

u16 Chaos_EffectRestrictions(struct PlayState *play);

Actor *Chaos_SpawnActor(ActorContext *context, PlayState *play, s16 actor_id, f32 pos_x, f32 pos_y, f32 pos_z, s16 rot_x, s16 rot_y, s16 rot_z, s32 params);

Actor* Chaos_SpawnAsChild(ActorContext* context, Actor* parent, PlayState* play, s16 actor_id, f32 pos_x, f32 pos_y, f32 pos_z, s16 rot_x, s16 rot_y, s16 rot_z, s32 params);

void Chaos_DestroyFunction(struct Actor *actor, struct PlayState *play);

void Chaos_KillActorAtIndex(u32 index);

void Chaos_DropActorAtIndex(u32 index);

void Chaos_DropActor(Actor *actor);

void Chaos_ClearActors(void);

u16 Chaos_RandomEntrance(PlayState *play);

void Chaos_UpdateEntrances(PlayState *play);

void Chaos_UpdateEnabledChaosEffectsAndEntrances(PlayState *this);

#endif
