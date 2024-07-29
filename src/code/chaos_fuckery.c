#include "chaos_fuckery.h"
#include "functions.h"
#include "regs.h"
#include "gfxprint.h"
#include "variables.h"
#include "z64.h"
#include "z64cutscene.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

ChaosContext gChaosContext; 

struct ChaosCodeDef gChaosCodeDefs[] = {
    /* [CHAOS_CODE_NONE]                = */ {0, 0, 0                                       },
    /* [CHAOS_CODE_LOW_GRAVITY]         = */ {/* .min_time = */ 10,  /* .max_time = */ 18, /* .always_update = */ 1},
    /* [CHAOS_CODE_CHANGE_HEALTH]       = */ {/* .min_time = */ 1,   /* .max_time = */ 10, /* .always_update = */ 1},
    /* [CHAOS_CODE_CHANGE_RUPEE]        = */ {/* .min_time = */ 0,   /* .max_time = */ 0,  /* .always_update = */ 1},
    /* [CHAOS_CODE_ACTOR_CHASE]         = */ {/* .min_time = */ 6,   /* .max_time = */ 15, /* .always_update = */ 0},
    /* [CHAOS_CODE_YEET]                = */ {/* .min_time = */ 8,   /* .max_time = */ 17, /* .always_update = */ 1},
    /* [CHAOS_CODE_POKE]                = */ {/* .min_time = */ 0,   /* .max_time = */ 0,  /* .always_update = */ 0},
    /* [CHAOS_CODE_MOON_DANCE]          = */ {/* .min_time = */ 5,   /* .max_time = */ 30, /* .always_update = */ 1},
    /* [CHAOS_CODE_ONE_HIT_KO]          = */ {/* .min_time = */ 8,   /* .max_time = */ 23, /* .always_update = */ 0},
    /* [CHAOS_CODE_RANDOM_KNOCKBACK]    = */ {/* .min_time = */ 8,   /* .max_time = */ 15, /* .always_update = */ 0},
    /* [CHAOS_CODE_ICE_TRAP]            = */ {/* .min_time = */ 0,   /* .max_time = */ 0,  /* .always_update = */ 0},
    /* [CHAOS_CODE_TIMER_UP]            = */ {/* .min_time = */ 10,  /* .max_time = */ 20, /* .always_update = */ 0},
    /* [CHAOS_CODE_SHOCK]               = */ {/* .min_time = */ 0,   /* .max_time = */ 0,  /* .always_update = */ 0},
    /* [CHAOS_CODE_EARTHQUAKE]          = */ {/* .min_time = */ 0,   /* .max_time = */ 0,  /* .always_update = */ 1},
    // /* [CHAOS_CODE_TUNIC_COLOR]         = */ {/* .min_time = */ 0,   /* .max_time = */ 0,  /* .always_update = */ 1},
    /* [CHAOS_CODE_BOMB_ARROWS]         = */ {/* .min_time = */ 15,  /* .max_time = */ 25, /* .always_update = */ 0},
    /* [CHAOS_CODE_WEIRD_ARROWS]        = */ {/* .min_time = */ 15,  /* .max_time = */ 25, /* .always_update = */ 0},
    /* [CHAOS_CODE_BUCKSHOT_ARROWS]     = */ {/* .min_time = */ 15,  /* .max_time = */ 25, /* .always_update = */ 0},
    /* [CHAOS_CODE_RANDOM_BOMB_TIMER]   = */ {/* .min_time = */ 10,  /* .max_time = */ 15, /* .always_update = */ 0},
    /* [CHAOS_CODE_LOVELESS_MARRIAGE]   = */ {/* .min_time = */ 0,   /* .max_time = */ 0,  /* .always_update = */ 0},
    /* [CHAOS_CODE_WEIRD_UI]            = */ {/* .min_time = */ 8,   /* .max_time = */ 15, /* .always_update = */ 0},
    // /* [CHAOS_CODE_VILETILE_ENEMIES]    = */ {/* .min_time = */ 10,  /* .max_time = */ 20, /* .always_update = */ 0},
    /* [CHAOS_CODE_BEER_GOGGLES]        = */ {/* .min_time = */ 15,   /* .max_time = */ 30, /* .always_update = */ 1},
    /* [CHAOS_CODE_CHANGE_MAGIC]        = */ {/* .min_time = */ 1,   /* .max_time = */ 10, /* .always_update = */ 1},
    /* [CHAOS_CODE_INVINCIBLE]          = */ {/* .min_time = */ 8,   /* .max_time = */ 23, /* .always_update = */ 0},
};

const char *gChaosCodeNames[] = {
    /* [CHAOS_CODE_NONE]                = */ "NONE",
    /* [CHAOS_CODE_LOW_GRAVITY]         = */ "Low gravity",
    /* [CHAOS_CODE_CHANGE_HEALTH]       = */ "Change health",
    /* [CHAOS_CODE_CHANGE_RUPEE]        = */ "Change rupee",
    /* [CHAOS_CODE_ACTOR_CHASE]         = */ "Actor chase",
    /* [CHAOS_CODE_YEET]                = */ "YEET",
    /* [CHAOS_CODE_POKE]                = */ "Poke",
    /* [CHAOS_CODE_MOON_DANCE]          = */ "Moon dance",
    /* [CHAOS_CODE_ONE_HIT_KO]          = */ "One-hit KO",
    /* [CHAOS_CODE_RANDOM_KNOCKBACK]    = */ "Random knockback",
    /* [CHAOS_CODE_ICE_TRAP]            = */ "Ice trap",
    /* [CHAOS_CODE_TIMER_UP]            = */ "Timer up",
    /* [CHAOS_CODE_SHOCK]               = */ "Shock",
    /* [CHAOS_CODE_EARTHQUAKE]          = */ "Earthquake",
    // /* [CHAOS_CODE_TUNIC_COLOR]         = */ "Random tunic color",
    /* [CHAOS_CODE_BOMB_ARROWS]         = */ "Bomb arrows",
    /* [CHAOS_CODE_WEIRD_ARROWS]        = */ "Weird arrows",
    /* [CHAOS_CODE_BUCKSHOT_ARROWS]     = */ "Buckshot arrows",
    /* [CHAOS_CODE_RANDOM_BOMB_TIMER]   = */ "Random bomb timer",
    /* [CHAOS_CODE_LOVELESS_MARRIAGE]   = */ "Loveless marriage",
    /* [CHAOS_CODE_WEIRD_UI]            = */ "Weird UI",
    // /* [CHAOS_CODE_VILETILE_ENEMIES]    = */ "Viletile enemies",
    /* [CHAOS_CODE_BEER_GOGGLES]        = */ "Beer goggles",
    /* [CHAOS_CODE_CHANGE_MAGIC]        = */ "Change magic",
    /* [CHAOS_CODE_INVINCIBLE]          = */ "Invincible",
};

void Chaos_Init(void)
{
    u32 index;

    gChaosContext.moon.pitch = 0.0f;
    gChaosContext.moon.yaw = 0.0f;
    gChaosContext.active_code_count = 0;
    gChaosContext.chaos_timer = 15;
    gChaosContext.code_elapsed_usec = 0;
    gChaosContext.chaos_elapsed_usec = 0;
    gChaosContext.prev_update_counter = osGetTime();
    gChaosContext.update_enabled = 0;
    gChaosContext.link.tunic_r = 30;
    gChaosContext.link.tunic_g = 105;
    gChaosContext.link.tunic_b = 27;
    gChaosContext.link.beer_alpha = 0;

    for(index = 0; index < CHAOS_CODE_LAST; index++)
    {
        gChaosContext.active_code_indices[index] = INVALID_CODE_INDEX;
    }
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

    if(Chaos_CanUpdateChaos(playstate))
    {
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
            struct ChaosActor *actor = gChaosContext.actors.slots + slot_index;

            if(!(actor->actor->flags & ACTOR_FLAG_40))
            {
                if(code_elapsed_seconds >= actor->timer)
                {
                    Chaos_KillActorAtIndex(slot_index);
                    continue;
                }

                actor->timer -= code_elapsed_seconds;
            }
            else
            {
                actor->timer = ACTOR_DESPAWN_TIMER;
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
                Chaos_DropCodeAtIndex(slot_index);
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
                gChaosContext.chaos_timer = MIN_CHAOS_TIMER + Rand_Next() % (MAX_CHAOS_TIMER - MIN_CHAOS_TIMER);
                next_rand = Rand_Next();
                do
                {
                    next_code = CHAOS_CODE_FIRST + Rand_Next() % (CHAOS_CODE_LAST - CHAOS_CODE_FIRST);
                    next_code_timer = gChaosCodeDefs[next_code].min_time + next_rand % (gChaosCodeDefs[next_code].max_time - gChaosCodeDefs[next_code].min_time);

                    /* TODO: create a effect exclusion list for each effect, to avoid a bunch of branching here */
                    switch(next_code)
                    {
                        case CHAOS_CODE_POKE:
                        case CHAOS_CODE_RANDOM_KNOCKBACK:
                        case CHAOS_CODE_ICE_TRAP:
                        case CHAOS_CODE_SHOCK:
                            if(player->stateFlags2 & PLAYER_STATE2_80)
                            {
                                /* if the player has been grabbed, don't spawn any of those effects
                                to avoid leaving the player in an inconsistent state */
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
                            if(Chaos_IsCodeActive(CHAOS_CODE_CHANGE_HEALTH) || Chaos_IsCodeActive(CHAOS_CODE_INVINCIBLE))
                            {
                                /* changing health would one-hit the player or not have any effect at all, so don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_CHANGE_HEALTH:
                            if(Chaos_IsCodeActive(CHAOS_CODE_ONE_HIT_KO) || Chaos_IsCodeActive(CHAOS_CODE_INVINCIBLE))
                            {
                                /* changing health would one-hit the player or not have any effect at all, so don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_INVINCIBLE:
                            if(Chaos_IsCodeActive(CHAOS_CODE_ONE_HIT_KO) || Chaos_IsCodeActive(CHAOS_CODE_CHANGE_HEALTH))
                            {
                                continue;
                            }
                        break;
                    }

                    code_add_result = Chaos_AddCode(next_code, next_code_timer);
                }
                while(code_add_result == CHAOS_ADD_RESULT_ALREADY_ACTIVE);

                if(code_add_result == CHAOS_ADD_RESULT_OK)
                {
                    last_code = gChaosContext.active_codes + (gChaosContext.active_code_count - 1);

                    switch(last_code->code)
                    {
                        case CHAOS_CODE_MOON_DANCE:
                            do
                            {
                                /* pick a random combination of moon dance move flags */
                                last_code->data = Rand_Next() % CHAOS_MOON_MOVE_LAST;
                            }
                            while(last_code->data == 0);
                        break;

                        case CHAOS_CODE_RANDOM_KNOCKBACK:
                            /* deal first knockback on the same frame the effect gets activated */
                            last_code->data = 1;
                        break;

                        case CHAOS_CODE_BEER_GOGGLES:
                            last_code->data = Rand_S16Offset(200, 200);
                        break;

                        // case CHAOS_CODE_TUNIC_COLOR:
                        // {
                        //     u32 color = Rand_Next() % 0xffffff;
                        //     gChaosContext.link.tunic_r = color;
                        //     gChaosContext.link.tunic_g = color >> 8;
                        //     gChaosContext.link.tunic_b = color >> 16;
                        // }
                        // break;
                    }
                }
            }
        }
    }
}
 
void Chaos_PrintCodes(PlayState *playstate)
{
    Gfx* gfx;
    Gfx* polyOpa;
    GfxPrint gfx_print;
    u32 slot_index;
    u32 y_pos = 1;
    if(gSaveContext.gameMode == GAMEMODE_NORMAL)
    {
        OPEN_DISPS(playstate->state.gfxCtx);
        polyOpa = POLY_OPA_DISP;
        gfx = Graph_GfxPlusOne(polyOpa);
        gSPDisplayList(OVERLAY_DISP++, gfx);

        GfxPrint_Init(&gfx_print);
        GfxPrint_Open(&gfx_print, gfx);
        GfxPrint_SetColor(&gfx_print, 255, 255, 255, 255);
        GfxPrint_SetPos(&gfx_print, 5, y_pos);

        GfxPrint_SetPos(&gfx_print, 1, y_pos++);
        GfxPrint_Printf(&gfx_print, "chaos timer: %d", (u32)gChaosContext.chaos_timer);

        slot_index = 0;
        while(slot_index < gChaosContext.active_code_count)
        {
            struct ChaosCode *code = gChaosContext.active_codes + slot_index;

            // if(code->code > CHAOS_CODE_LAST)
            // {
            //     u32 *blah = (u32 *)code->code;
            //     *blah = 5;
            // }

            if(code->timer > 0)
            {
                GfxPrint_SetPos(&gfx_print, 1, y_pos++);
                GfxPrint_Printf(&gfx_print, "%s: %d", gChaosCodeNames[code->code], (u32)code->timer);
                // GfxPrint_Printf(&gfx_print, "%s: %d", gChaosCodeNames[0], (u32)code->timer);
            }
            slot_index++;
        }

        gfx = GfxPrint_Close(&gfx_print);
        GfxPrint_Destroy(&gfx_print);
        gSPEndDisplayList(gfx++);
        Graph_BranchDlist(polyOpa, gfx);
        POLY_OPA_DISP = gfx;
        CLOSE_DISPS(gfxCtx);
    }
}

u8 Chaos_AddCode(u8 code, u8 seconds)
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

u8 Chaos_DropCodeAtIndex(u8 index)
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

u8 Chaos_IsCodeActive(u8 code)
{
    // u32 slot_index;
    // for(slot_index = 0; slot_index < gChaosContext.active_code_count; slot_index++)
    // {
    //     if(gChaosContext.active_codes[slot_index].code == code)
    //     {
    //         return gChaosCodeDefs[code].always_update || gChaosContext.update_enabled;
    //     }
    // }
    u32 slot_index = gChaosContext.active_code_indices[code];
    return slot_index < gChaosContext.active_code_count && (gChaosCodeDefs[code].always_update || gChaosContext.update_enabled);
}

struct ChaosCode *Chaos_GetCode(u8 code)
{
    u32 slot_index = gChaosContext.active_code_indices[code];

    if(slot_index < gChaosContext.active_code_count && (gChaosCodeDefs[code].always_update || gChaosContext.update_enabled))
    {
        return gChaosContext.active_codes + slot_index;
    }
    // for(slot_index = 0; slot_index < gChaosContext.active_code_count; slot_index++)
    // {
    //     if(gChaosContext.active_codes[slot_index].code == code)
    //     {
    //         if(gChaosCodeDefs[code].always_update || gChaosContext.update_enabled)
    //         {
    //             return gChaosContext.active_codes + slot_index;
    //         }

    //         return NULL;
    //     }
    // }

    return NULL;
}

u8 Chaos_CanUpdateChaos(struct PlayState *play)
{
    u8 enable_update = gSaveContext.gameMode == GAMEMODE_NORMAL &&
                       CutsceneManager_GetCurrentCsId() == CS_ID_NONE &&
                       play->transitionMode == TRANS_MODE_OFF && 
                       play->pauseCtx.state == PAUSE_STATE_OFF;

    if(enable_update && !gChaosContext.update_enabled)
    {
        gChaosContext.code_elapsed_usec = 0;
        gChaosContext.chaos_elapsed_usec = 0;
        gChaosContext.prev_update_counter = osGetTime();
    }

    gChaosContext.update_enabled = enable_update;
    return gChaosContext.update_enabled;
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
            gChaosContext.actors.spawned_actors++;
        }
    }

    return actor;
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

        Chaos_DropCodeAtIndex(index);

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
}