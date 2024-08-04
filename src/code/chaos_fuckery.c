#include "chaos_fuckery.h"
#include "functions.h"
#include "regs.h"
#include "gfxprint.h"
#include "variables.h"
#include "z64.h"
#include "z64cutscene.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

ChaosContext    gChaosContext; 
u64             gChaosRngState = 1;
u32             gDisplayEffectInfo = 0;
u32             gChaosEffectPageIndex = 0;
u32             gAcceptPageChange = 0;

struct ChaosCodeDef gChaosCodeDefs[] = {
    /* [CHAOS_CODE_NONE]                    = */ CHAOS_CODE_DEF(0,  0,  false, 0),
    /* [CHAOS_CODE_LOW_GRAVITY]             = */ CHAOS_CODE_DEF(10, 18, true,  0.08f),
    /* [CHAOS_CODE_CHANGE_HEALTH]           = */ CHAOS_CODE_DEF(1,  10, false, 0.08f),
    /* [CHAOS_CODE_CHANGE_RUPEE]            = */ CHAOS_CODE_DEF(0,  0,  true,  0.08f),
    /* [CHAOS_CODE_ACTOR_CHASE]             = */ CHAOS_CODE_DEF(6,  15, false, 0.04f),
    /* [CHAOS_CODE_YEET]                    = */ CHAOS_CODE_DEF(8,  17, true,  0.03f),
    /* [CHAOS_CODE_POKE]                    = */ CHAOS_CODE_DEF(0,  0,  false, 0.025f),
    /* [CHAOS_CODE_MOON_DANCE]              = */ CHAOS_CODE_DEF(5,  30, true,  0.006f),
    /* [CHAOS_CODE_ONE_HIT_KO]              = */ CHAOS_CODE_DEF(8,  23, false, 0.005f),
    /* [CHAOS_CODE_RANDOM_KNOCKBACK]        = */ CHAOS_CODE_DEF(8,  15, false, 0.04f),
    /* [CHAOS_CODE_ICE_TRAP]                = */ CHAOS_CODE_DEF(0,  0,  false, 0.01f),
    /* [CHAOS_CODE_TIMER_UP]                = */ CHAOS_CODE_DEF(10, 20, false, 0.009f),
    /* [CHAOS_CODE_SHOCK]                   = */ CHAOS_CODE_DEF(0,  0,  false, 0.025f),
    /* [CHAOS_CODE_EARTHQUAKE]              = */ CHAOS_CODE_DEF(0,  0,  true,  0.06f),
    /* [CHAOS_CODE_BOMB_ARROWS]             = */ CHAOS_CODE_DEF(15, 25, false, 0.02f),
    /* [CHAOS_CODE_WEIRD_ARROWS]            = */ CHAOS_CODE_DEF(15, 25, false, 0.02f),
    /* [CHAOS_CODE_BUCKSHOT_ARROWS]         = */ CHAOS_CODE_DEF(15, 25, false, 0.02f),
    /* [CHAOS_CODE_RANDOM_BOMB_TIMER]       = */ CHAOS_CODE_DEF(10, 15, false, 0.01f),
    /* [CHAOS_CODE_LOVELESS_MARRIAGE]       = */ CHAOS_CODE_DEF(0,  0,  false, 0.01f),
    /* [CHAOS_CODE_WEIRD_UI]                = */ CHAOS_CODE_DEF(8,  15, false, 0.04f),
    /* [CHAOS_CODE_BEER_GOGGLES]            = */ CHAOS_CODE_DEF(15, 30, true,  0.03f),
    /* [CHAOS_CODE_CHANGE_MAGIC]            = */ CHAOS_CODE_DEF(1,  10, true,  0.08f),
    /* [CHAOS_CODE_INVINCIBLE]              = */ CHAOS_CODE_DEF(8,  23, false, 0.005f),
    /* [CHAOS_CODE_SYKE]                    = */ CHAOS_CODE_DEF(0,  0,  false, 0.00005f),
    /* [CHAOS_CODE_DIE]                     = */ CHAOS_CODE_DEF(0,  0,  false, 0.000005f),
    /* [CHAOS_CODE_TRAP_FLAP]               = */ CHAOS_CODE_DEF(10, 20, true,  0.04f),
    /* [CHAOS_CODE_TEXTBOX]                 = */ CHAOS_CODE_DEF(0,   0, false, 0.008f),
    /* [CHAOS_CODE_SLIPPERY_FLOORS]         = */ CHAOS_CODE_DEF(10, 20, false, 0.012f),
    /* [CHAOS_CODE_SLOW_DOWN]               = */ CHAOS_CODE_DEF(10, 20, false, 0.006f),
    /* [CHAOS_CODE_ENTRANCE_RANDO]          = */ CHAOS_CODE_DEF(5,  15, false, 0.0005f),
    /* [CHAOS_CODE_TERRIBLE_MUSIC]          = */ CHAOS_CODE_DEF(15, 35, true,  0.03f),
    /* [CHAOS_CODE_INCREDIBLE_KNOCKBACK]    = */ CHAOS_CODE_DEF(10, 21, true,  0.03f),
    /* [CHAOS_CODE_RANDOM_SCALING]          = */ CHAOS_CODE_DEF(10, 21, true,  0.025f),
    /* [CHAOS_CODE_BIG_BROTHER]             = */ CHAOS_CODE_DEF(60, 70, true,  0.006f),
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
    gChaosRngState = Rand_Next();
    gChaosContext.moon.pitch = 0.0f;
    gChaosContext.moon.yaw = 0.0f;
    gChaosContext.active_code_count = 0;
    gChaosContext.enabled_code_count = 0;
    gChaosContext.chaos_timer = 15;
    gChaosContext.code_elapsed_usec = 0;
    gChaosContext.chaos_elapsed_usec = 0;
    gChaosContext.prev_update_counter = osGetTime();
    gChaosContext.update_enabled = 0;
    gChaosContext.need_update_distribution = false;
    gChaosContext.hide_actors = 0;
    gChaosContext.link.tunic_r = 30;
    gChaosContext.link.tunic_g = 105;
    gChaosContext.link.tunic_b = 27;
    gChaosContext.link.beer_alpha = 0;
    gChaosContext.link.syke = false; 
    
    for(index = 0; index < CHAOS_CODE_LAST; index++)
    {
        gChaosContext.active_code_indices[index] = INVALID_CODE_INDEX;
        gChaosContext.enabled_code_indices[index] = INVALID_CODE_INDEX;
    }

    // for(index = CHAOS_CODE_FIRST; index < CHAOS_CODE_LAST; index++)
    // {
    //     probability_scale += gChaosCodeDefs[index].probability;
    // }

    // probability_scale = 1.0f / probability_scale;
 
    // for(index = CHAOS_CODE_FIRST; index < CHAOS_CODE_LAST; index++)
    // {
    //     u64 next_range_end = prev_end + 0x00000000ffffffff * gChaosCodeDefs[index].probability * probability_scale;
    //     if(next_range_end > 0xffffffff)
    //     {
    //         next_range_end = 0xffffffff;
    //     }

    //     gChaosCodeDefs[index].range_start = prev_end;
    //     gChaosCodeDefs[index].range_end = (u32)next_range_end;
    //     prev_end = next_range_end;
    // }
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

    if(gChaosContext.need_update_distribution)
    {
        Chaos_UpdateCodeDistribution();
        gChaosContext.need_update_distribution = false;
    }

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
                gChaosContext.chaos_timer = MIN_CHAOS_TIMER + Rand_Next() % (MAX_CHAOS_TIMER - MIN_CHAOS_TIMER);
                next_rand = Rand_Next();
                do
                {
                    // next_code = CHAOS_CODE_FIRST + Rand_Next() % (CHAOS_CODE_LAST - CHAOS_CODE_FIRST);
                    // next_code = CHAOS_CODE_FIRST + Chaos_Rand() % (CHAOS_CODE_LAST - CHAOS_CODE_FIRST);
                    next_code = Chaos_RandomCode();
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
                                /* making the player invicible now would make both codes not have an effect, so don't activate it */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_BIG_BROTHER:
                            if(Chaos_IsCodeActive(CHAOS_CODE_MOON_DANCE))
                            {
                                /* the moon won't be able to face the player if it starts dancing, so do it later */
                                continue;
                            }
                        break;

                        case CHAOS_CODE_MOON_DANCE:
                            if(Chaos_IsCodeActive(CHAOS_CODE_BIG_BROTHER))
                            {
                                /* the moon won't be able to face the player if it starts dancing, so do it later */
                                continue;
                            }
                        break;
                    }

                    code_add_result = Chaos_ActivateCode(next_code, next_code_timer);
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

                        case CHAOS_CODE_TRAP_FLAP:
                            last_code->data = 1;
                        break;

                        case CHAOS_CODE_TERRIBLE_MUSIC:
                            /* start screwing up the bgm on the same frame the effect gets activated */
                            last_code->data = 1;
                        break;

                        case CHAOS_CODE_BIG_BROTHER:
                            gChaosContext.moon.eye_glow = 0.0f;
                            last_code->data = CHAOS_BIG_BROTHER_STATE_TRACKING; 
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

#define ENABLED_EFFECTS_PER_PAGE 18
void Chaos_PrintCodes(PlayState *playstate, Input *input)
{
    Gfx* gfx;
    Gfx* polyOpa;
    GfxPrint gfx_print;
    u32 slot_index;
    u32 y_pos = 1;
    u32 enabled_effects_page_count = gChaosContext.enabled_code_count / ENABLED_EFFECTS_PER_PAGE;

    if(gChaosContext.enabled_code_count % ENABLED_EFFECTS_PER_PAGE)
    {
        enabled_effects_page_count++;
    }

    if(CHECK_BTN_ANY(input->press.button, BTN_DUP))
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
        gfx = Graph_GfxPlusOne(polyOpa);
        gSPDisplayList(OVERLAY_DISP++, gfx);

        GfxPrint_Init(&gfx_print);
        GfxPrint_Open(&gfx_print, gfx);
        GfxPrint_SetColor(&gfx_print, 255, 255, 255, 255);
        GfxPrint_SetPos(&gfx_print, 1, y_pos++);

        // if(!CHECK_BTN_ANY(input->press.button, BTN_R | BTN_L))
        // {
        //     gAcceptPageChange = 1;
        // }

        // if(gAcceptPageChange)
        // {
        //     if(CHECK_BTN_ANY(input->press.button, BTN_L))
        //     {
        //         gChaosEffectPageIndex = (gChaosEffectPageIndex - 1) % -(enabled_effects_page_count + 1);
        //         gAcceptPageChange = 0;
        //     }

        //     if(CHECK_BTN_ANY(input->press.button, BTN_R))
        //     {
        //         gChaosEffectPageIndex = (gChaosEffectPageIndex + 1) % (enabled_effects_page_count + 1);
        //         gAcceptPageChange = 0;
        //     }
        // }

        // if(gChaosEffectPageIndex == 0)
        // {
        GfxPrint_Printf(&gfx_print, "Chaos timer: %d", (u32)gChaosContext.chaos_timer);

        slot_index = 0;
        while(slot_index < gChaosContext.active_code_count)
        {
            struct ChaosCode *code = gChaosContext.active_codes + slot_index;

            if(code->timer > 0)
            {
                GfxPrint_SetPos(&gfx_print, 1, y_pos++);
                GfxPrint_Printf(&gfx_print, "%s: %d", gChaosCodeNames[code->code], (u32)code->timer);
            }
            slot_index++;
        }
        // }
        // else
        // {
        //     struct ChaosCodeSlot *first_slot;
        //     u32 display_effect_count;
        //     u32 first_effect_index = (gChaosEffectPageIndex - 1) * ENABLED_EFFECTS_PER_PAGE;
        //     first_effect_index = CLAMP_MAX(first_effect_index, gChaosContext.enabled_code_count);
        //     first_slot = gChaosContext.enabled_codes + first_effect_index;
        //     display_effect_count = gChaosContext.enabled_code_count - first_effect_index;
        //     display_effect_count = CLAMP_MAX(display_effect_count, ENABLED_EFFECTS_PER_PAGE);
            
        //     GfxPrint_Printf(&gfx_print, "Enabled effects: (%d/%d)", gChaosEffectPageIndex, enabled_effects_page_count);            

        //     slot_index = 0;
        //     while(slot_index < display_effect_count)
        //     {
        //         struct ChaosCodeSlot *slot = first_slot + slot_index;
        //         GfxPrint_SetPos(&gfx_print, 1, y_pos++);
        //         GfxPrint_Printf(&gfx_print, "%s", gChaosCodeNames[slot->code]);
        //         slot_index++;
        //     }
        // }        

        gfx = GfxPrint_Close(&gfx_print);
        GfxPrint_Destroy(&gfx_print);
        gSPEndDisplayList(gfx++);
        Graph_BranchDlist(polyOpa, gfx);
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
    return slot_index < gChaosContext.active_code_count && (gChaosCodeDefs[code].always_update || gChaosContext.update_enabled);
}

struct ChaosCode *Chaos_GetCode(u8 code)
{
    u32 slot_index = gChaosContext.active_code_indices[code];

    if(slot_index < gChaosContext.active_code_count && (gChaosCodeDefs[code].always_update || gChaosContext.update_enabled))
    {
        return gChaosContext.active_codes + slot_index;
    }

    return NULL;
}

void Chaos_EnableCode(u8 code)
{
    if(!Chaos_IsCodeEnabled(code))
    {
        u32 index = gChaosContext.enabled_code_count;
        gChaosContext.enabled_code_count++;
        gChaosContext.enabled_codes[index].code = code;
        gChaosContext.enabled_code_indices[code] = index;
        gChaosContext.need_update_distribution = true;
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

    if(gChaosContext.enabled_code_count > 0)
    {
        for(index = 0; index < gChaosContext.enabled_code_count; index++)
        {
            probability_scale += gChaosCodeDefs[gChaosContext.enabled_codes[index].code].probability;
        }

        probability_scale = 1.0f / probability_scale;

        for(index = 0; index < gChaosContext.enabled_code_count; index++)
        {
            struct ChaosCodeSlot *code_slot = gChaosContext.enabled_codes + index;
            struct ChaosCodeDef *code_def = gChaosCodeDefs + code_slot->code;
            u64 next_range_end = prev_end + 0x00000000ffffffff * code_def->probability * probability_scale;
            if(next_range_end > 0xffffffff)
            {
                next_range_end = 0xffffffff;
            }

            code_slot->range_start = prev_end;
            code_slot->range_end = (u32)next_range_end;
            prev_end = next_range_end;
        }
    }
}

u8 Chaos_CanUpdateChaos(struct PlayState *play)
{
    Player *player = GET_PLAYER(play);
    u8 enable_update = gSaveContext.gameMode == GAMEMODE_NORMAL &&
                       CutsceneManager_GetCurrentCsId() == CS_ID_NONE &&
                       play->transitionMode == TRANS_MODE_OFF && 
                       play->pauseCtx.state == PAUSE_STATE_OFF &&
                       !(player->stateFlags1 & PLAYER_STATE1_80);

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
}