#include "chaos_fuckery.h"
#include "functions.h"
#include "regs.h"
#include "gfxprint.h"

ChaosContext gChaosContext; 

struct ChaosCodeDef gChaosCodeDefs[] = {
    /* [CHAOS_CODE_NONE]                = */ {0, 0                                       },
    /* [CHAOS_CODE_LOW_GRAVITY]         = */ {/* .min_time = */ 10,  /* .max_time = */ 18},
    /* [CHAOS_CODE_CHANGE_HEALTH]       = */ {/* .min_time = */ 1,   /* .max_time = */ 10},
    /* [CHAOS_CODE_CHANGE_RUPEE]        = */ {/* .min_time = */ 0,   /* .max_time = */ 0},
    /* [CHAOS_CODE_ACTOR_CHASE]         = */ {/* .min_time = */ 6,   /* .max_time = */ 15},
    /* [CHAOS_CODE_YEET]                = */ {/* .min_time = */ 8,   /* .max_time = */ 17},
    /* [CHAOS_CODE_POKE]                = */ {/* .min_time = */ 0,   /* .max_time = */ 0},
    /* [CHAOS_CODE_MOON_DANCE]          = */ {/* .min_time = */ 5,   /* .max_time = */ 30},
    /* [CHAOS_CODE_ONE_HIT_KO]          = */ {/* .min_time = */ 8,   /* .max_time = */ 23},
    /* [CHAOS_CODE_RANDOM_KNOCKBACK]    = */ {/* .min_time = */ 8,   /* .max_time = */ 15},
    /* [CHAOS_CODE_ICE_TRAP]            = */ {/* .min_time = */ 0,   /* .max_time = */ 0},
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
};

void Chaos_Init()
{
    gChaosContext.moon.pitch = 0.0f;
    gChaosContext.moon.yaw = 0.0f;
    gChaosContext.active_code_count = 0;
    // gChaosContext.chaos_timer = CHAOS_SECONDS_TO_FRAMES(5);
    gChaosContext.chaos_timer = 15;
    gChaosContext.elapsed_usec = 0;
    gChaosContext.prev_update_counter = osGetTime();
}
 
void Chaos_UpdateChaos(PlayState *playstate)
{
    OSTime              update_counter = osGetTime();
    u32                 elapsed_seconds;
    u32                 slot_index;
    u32                 next_rand;
    u32                 code_add_result;
    u8                  next_code; 
    u8                  next_code_timer;
    struct ChaosCode *  last_code = NULL;

    if(update_counter < gChaosContext.prev_update_counter)
    {
        /* PARANOID: cpu counter overflow */
        gChaosContext.prev_update_counter = update_counter;
    }

    gChaosContext.elapsed_usec += (OS_CYCLES_TO_USEC(update_counter) - OS_CYCLES_TO_USEC(gChaosContext.prev_update_counter));
    gChaosContext.prev_update_counter = update_counter;

    elapsed_seconds = gChaosContext.elapsed_usec / 1000000;

    slot_index = 0;
    while(slot_index < gChaosContext.active_code_count)
    {
        struct ChaosCode *code = gChaosContext.active_codes + slot_index;

        if(elapsed_seconds >= code->timer)
        {
            Chaos_DropCodeAtIndex(slot_index);
            continue;
        }

        code->timer -= elapsed_seconds;
        slot_index++;
    }

    if(elapsed_seconds > 0)
    {
        gChaosContext.elapsed_usec -= elapsed_seconds * 1000000;

        if(elapsed_seconds > gChaosContext.chaos_timer)
        {
            gChaosContext.chaos_timer = 0;
        }
        else
        {
            gChaosContext.chaos_timer -= elapsed_seconds;
        }  
    
        if(gChaosContext.chaos_timer == 0)  
        {
            gChaosContext.chaos_timer = MIN_CHAOS_TIMER + Rand_Next() % (MAX_CHAOS_TIMER - MIN_CHAOS_TIMER);
            next_rand = Rand_Next();
            do
            {
                next_code = CHAOS_CODE_FIRST + Rand_Next() % (CHAOS_CODE_LAST - CHAOS_CODE_FIRST);
                next_code_timer = gChaosCodeDefs[next_code].min_time + next_rand % (gChaosCodeDefs[next_code].max_time - gChaosCodeDefs[next_code].min_time);
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
                            last_code->data = Rand_Next() % MOON_MOVE_LAST;
                        }
                        while(last_code->data == 0);
                    break;

                    case CHAOS_CODE_RANDOM_KNOCKBACK:
                        /* deal first knockback on the same frame this got activated */
                        last_code->data = 1;
                    break;
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
        if(code->timer > 0)
        {
            GfxPrint_SetPos(&gfx_print, 1, y_pos++);
            GfxPrint_Printf(&gfx_print, "%s: %d", gChaosCodeNames[code->code], (u32)code->timer);
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

u8 Chaos_AddCode(u8 code, u8 seconds)
{
    if(Chaos_IsCodeActive(code))
    {
        return CHAOS_ADD_RESULT_ALREADY_ACTIVE;
    }

    if(gChaosContext.active_code_count < MAX_ACTIVE_CODES)
    {
        struct ChaosCode *slot = gChaosContext.active_codes + gChaosContext.active_code_count;
        gChaosContext.active_code_count++;
        slot->code = code;
        slot->timer = seconds;
        // slot->timer = CHAOS_SECONDS_TO_FRAMES(seconds);
        return CHAOS_ADD_RESULT_OK;
    }

    return CHAOS_ADD_RESULT_NO_SLOTS;
}

u8 Chaos_DropCodeAtIndex(u8 index)
{
    if(index < gChaosContext.active_code_count)
    {
        if(index < gChaosContext.active_code_count - 1)
        {
            gChaosContext.active_codes[index] = gChaosContext.active_codes[gChaosContext.active_code_count - 1];
        }

        gChaosContext.active_code_count--;
    }
}

u8 Chaos_IsCodeActive(u8 code)
{
    u32 slot_index;
    for(slot_index = 0; slot_index < gChaosContext.active_code_count; slot_index++)
    {
        if(gChaosContext.active_codes[slot_index].code == code)
        {
            return 1;
        }
    }

    return 0;
}

struct ChaosCode *Chaos_GetCode(u8 code)
{
    u32 slot_index;
    for(slot_index = 0; slot_index < gChaosContext.active_code_count; slot_index++)
    {
        if(gChaosContext.active_codes[slot_index].code == code)
        {
            return gChaosContext.active_codes + slot_index;
        }
    }

    return NULL;
}
