/*
 * File: z_bg_ingate.c
 * Overlay: ovl_Bg_Ingate
 * Description: Swamp Tour Boat
 */

#include "z_bg_ingate.h"
#include "assets/objects/object_sichitai_obj/object_sichitai_obj.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

void BgIngate_Init(Actor* thisx, PlayState* play2);
void BgIngate_Destroy(Actor* thisx, PlayState* play);
void BgIngate_Update(Actor* thisx, PlayState* play);
void BgIngate_Draw(Actor* thisx, PlayState* play);

Actor* BgIngate_FindActor(BgIngate* this, PlayState* play, u8 actorCategory, s16 actorId);
s32 func_80953BEC(BgIngate* this);
void func_80953B40(BgIngate* this);
void func_80953F8C(BgIngate* this, PlayState* play);
void BgIngate_BoatRide(BgIngate* this, PlayState* play);
void BgIngate_WaitForEmbark(BgIngate* this, PlayState* play);
void BgIngate_EndOfTrip(BgIngate* this, PlayState* play);
void func_80954340(BgIngate* this, PlayState* play);
void BgIngate_DekuCastleStop(BgIngate* this, PlayState* play);

ActorProfile Bg_Ingate_Profile = {
    /**/ ACTOR_BG_INGATE,
    /**/ ACTORCAT_BG,
    /**/ FLAGS,
    /**/ OBJECT_SICHITAI_OBJ,
    /**/ sizeof(BgIngate),
    /**/ BgIngate_Init,
    /**/ BgIngate_Destroy,
    /**/ BgIngate_Update,
    /**/ BgIngate_Draw,
};

/**
 * @brief Searches for an actor based on the parameters given to the function. Returns Actor* of actor found or NULL
 *
 * @param this
 * @param play
 * @param actorCategory - Category of Actor
 * @param actorId - ID of actor to search for
 * @return Actor*
 */
Actor* BgIngate_FindActor(BgIngate* this, PlayState* play, u8 actorCategory, s16 actorId) {
    Actor* actorIter = NULL;

    while (true) {
        actorIter = SubS_FindActor(play, actorIter, actorCategory, actorId);

        if (actorIter == NULL) {
            break;
        }

        if ((this != (BgIngate*)actorIter) && (actorIter->update != NULL)) {
            break;
        }

        if (actorIter->next == NULL) {
            actorIter = NULL;
            break;
        }
        actorIter = actorIter->next;
    }

    return actorIter;
}

void func_80953B40(BgIngate* this) {
    s32 temp;

    if (!CHECK_EVENTINF(EVENTINF_35)) {
        this->timePathTotalTime = 4 * 1000;
        this->timePathTimeSpeed = 4;
    } else {
        this->timePathTotalTime = 1 * 2000;
        this->timePathTimeSpeed = 1;
    }
    temp = this->timePath->count - (SUBS_TIME_PATHING_ORDER - 1);
    this->timePathWaypointTime = this->timePathTotalTime / temp;
    this->timePathWaypoint = SUBS_TIME_PATHING_ORDER - 1;
    this->timePathElapsedTime = 0;
    this->unk160 &= ~0x1;
    this->unk160 &= ~BGINGATE_REACHED_DEKU_CASTLE;
}

/* BgIngate_MoveBoatInPath */
s32 func_80953BEC(BgIngate* this) {
    f32 knots[265];
    Vec3f sp68;
    Vec3f sp5C;
    Vec3f timePathTargetPos;
    s16 yaw;

    SubS_TimePathing_FillKnots(knots, SUBS_TIME_PATHING_ORDER, this->timePath->count + SUBS_TIME_PATHING_ORDER);
    if (!(this->unk160 & 1)) {
        timePathTargetPos = gZeroVec3f;
        SubS_TimePathing_Update(this->timePath, &this->timePathProgress, &this->timePathElapsedTime,
                                this->timePathWaypointTime, this->timePathTotalTime, &this->timePathWaypoint, knots,
                                &timePathTargetPos, this->timePathTimeSpeed);
        this->unk160 |= 1;
    } else {
        timePathTargetPos = this->timePathTargetPos;
    }
    this->dyna.actor.world.pos.x = timePathTargetPos.x;
    this->dyna.actor.world.pos.z = timePathTargetPos.z;
    this->timePathTargetPos = gZeroVec3f;
    if (SubS_TimePathing_Update(this->timePath, &this->timePathProgress, &this->timePathElapsedTime,
                                this->timePathWaypointTime, this->timePathTotalTime, &this->timePathWaypoint, knots,
                                &this->timePathTargetPos, this->timePathTimeSpeed)) {
        this->unk160 |= BGINGATE_REACHED_DEKU_CASTLE;
    } else {
        sp68 = this->dyna.actor.world.pos;
        sp5C = this->timePathTargetPos;
        yaw = Math_Vec3f_Yaw(&sp68, &sp5C);
        this->dyna.actor.world.rot.y = yaw;
        this->dyna.actor.shape.rot.y = yaw;
    }

    return false;
}

/* BgIngate_SetBoatRideCamera */
s32 BgIngate_SetBoatRideCamera(BgIngate* this, PlayState* play) {
    Camera* mainCam = Play_GetCamera(play, CAM_ID_MAIN);

    if (CHECK_EVENTINF(EVENTINF_35)) {
        Player_SetCsActionWithHaltedActors(play, &this->dyna.actor, PLAYER_CSACTION_WAIT);
    } else {
        SET_EVENTINF(EVENTINF_41);
    }
    Camera_ChangeSetting(mainCam, CAM_SET_BOAT_CRUISE);
    play->bButtonAmmoPlusOne = 99;

    return false;
}

/* BgIngate_SetNormalCamera */
void BgIngate_SetNormalCamera(PlayState* play) {
    Camera_ChangeSetting(Play_GetCamera(play, CAM_ID_MAIN), CAM_SET_NORMAL0);

    if (!CHECK_EVENTINF(EVENTINF_35)) {
        CLEAR_EVENTINF(EVENTINF_41);
    }

    play->bButtonAmmoPlusOne = -1;
}

void func_80953EA4(BgIngate* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    Player_SetCsActionWithHaltedActors(play, &this->dyna.actor, PLAYER_CSACTION_58);
    player->unk_3A0.x = this->dyna.actor.world.pos.x;
    player->unk_3A0.z = this->dyna.actor.world.pos.z;
    this->unk160 &= ~BGINGATE_PLAYER_DISEMBARKED;
    this->unk16A = 0x1E;
    this->actionFunc = func_80954340;
}

/* BgIngate_SetupBoatRide */
void func_80953F14(BgIngate* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    player->actor.shape.rot.y = this->dyna.actor.shape.rot.y;
    player->actor.world.rot.y = player->actor.shape.rot.y;
    player->yaw = player->actor.shape.rot.y;
    player->actor.focus.rot.y = player->actor.shape.rot.y;
    this->unk160 |= BGINGATE_RIDE_JUST_STARTED;
    BgIngate_SetBoatRideCamera(this, play);
    if (this->timePath != NULL) {
        func_80953B40(this);
    }
    this->csId = CS_ID_NONE;
    this->actionFunc = BgIngate_BoatRide;
}

void func_80953F8C(BgIngate* this, PlayState* play) {
}

/* BgIngate_BoatRide */
void BgIngate_BoatRide(BgIngate* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Camera* mainCam = Play_GetCamera(play, CAM_ID_MAIN);

    if (!CHECK_EVENTINF(EVENTINF_40)) {

        if (!CHECK_EVENTINF(EVENTINF_35) && (this->unk160 & BGINGATE_RIDE_JUST_STARTED) && (this->unk16C == 0)) {
            this->dyna.actor.textId = MESSAGE_ID_WELCOME_TO_THE_BOAT;
            Message_StartTextbox(play, this->dyna.actor.textId, NULL);
            this->unk160 &= ~BGINGATE_RIDE_JUST_STARTED;
        }

        if (this->unk160 & BGINGATE_REACHED_DEKU_CASTLE) {

            if (this->timePath->additionalPathIndex != ADDITIONAL_PATH_INDEX_NONE) {
                this->unk16C += 1;

                if((this->unk160 & BGINGATE_PLAYER_ON_TOP) /* && player->transformation == PLAYER_FORM_HUMAN */) 
                {
                    /* link's logic expects link to be in boat mode here, and will
                    softlock otherwise, so force link into it in case he got pushed/damaged
                    during the ride */
                    Player_EnterBoatMode(play, player);

                    BgIngate_SetNormalCamera(play);
                    Player_SetCsActionWithHaltedActors(play, &this->dyna.actor, PLAYER_CSACTION_WAIT);
                    this->dyna.actor.textId = MESSAGE_ID_NOW_ARRIVING_AT_DEKU_PALACE_NORMAL;
                    Message_StartTextbox(play, this->dyna.actor.textId, NULL);
                    this->actionFunc = BgIngate_DekuCastleStop;
                }
                else
                {
                    this->actionFunc = BgIngate_WaitForEmbark;
                    this->unk160 &= ~BGINGATE_PLAYER_DISEMBARKED;
                }

                SET_WEEKEVENTREG(WEEKEVENTREG_BOAT_PARKED_AT_DEKU_PALACE);
            } else {
                if (!CHECK_EVENTINF(EVENTINF_35)) {
                    CLEAR_EVENTINF(EVENTINF_41);
                } else {
                    SET_EVENTINF(EVENTINF_40);
                }
                this->actionFunc = BgIngate_EndOfTrip;
            }
        } else if ((CutsceneManager_GetCurrentCsId() == CS_ID_NONE) && (this->timePath != NULL)) {
            Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_CRUISER - SFX_FLAG);
            func_80953BEC(this);
        }
    }
    if (CutsceneManager_GetCurrentCsId() != this->csId) 
    {
        if (CutsceneManager_GetCurrentCsId() != CS_ID_NONE) 
        {
            Camera_ChangeSetting(mainCam, CAM_SET_NORMAL0);
            player->stateFlags1 |= PLAYER_STATE1_20;
            play->actorCtx.flags &= ~ACTORCTX_FLAG_PICTO_BOX_ON;
        } 
        else 
        {
            /* return link to boat cruise camera mode after killing a bigocto */

            if(CHECK_EVENTINF(EVENTINF_RIDING_BOAT))
            {
                Camera_ChangeSetting(mainCam, CAM_SET_BOAT_CRUISE);
            }
            else
            {
                Camera_ChangeSetting(mainCam, CAM_SET_NORMAL0);
            }

            player->stateFlags1 &= ~PLAYER_STATE1_20;
        }
    }
    this->csId = CutsceneManager_GetCurrentCsId();
}

/* BgIngate_WaitForEmbark */
void BgIngate_WaitForEmbark(BgIngate* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (this->unk160 & BGINGATE_PLAYER_DISEMBARKED) {
        if ((player->transformation == PLAYER_FORM_HUMAN) && (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
            (this->dyna.actor.xzDistToPlayer < 40.0f)) {
            if (this->dyna.actor.playerHeightRel > 15.0f) {
                Player_SetCsActionWithHaltedActors(play, &this->dyna.actor, PLAYER_CSACTION_WAIT);
                // this->dyna.actor.textId = MESSAGE_ID_ARE_YOU_BOARDING_NORMAL;
                this->dyna.actor.textId = (Rand_Next() % 2) ? MESSAGE_ID_ARE_YOU_BOARDING_NORMAL : MESSAGE_ID_ARE_YOU_BOARDING_FLIPPED;
                Message_StartTextbox(play, this->dyna.actor.textId, NULL);
                this->actionFunc = BgIngate_DekuCastleStop;
            }
        }
    } 
    // else if (!DynaPolyActor_IsPlayerOnTop(&this->dyna)) {
    else if(!(this->unk160 & BGINGATE_PLAYER_ON_TOP)) {
        this->unk160 |= BGINGATE_PLAYER_DISEMBARKED;
    }
} 

/* BgIngate_EndOfTrip */
void BgIngate_EndOfTrip(BgIngate* this, PlayState* play) {
    if(this->unk160 & BGINGATE_PLAYER_ON_TOP)
    {
        if (CHECK_EVENTINF(EVENTINF_50)) {
            play->nextEntrance = ENTRANCE(TOURIST_INFORMATION, 2);
            CLEAR_EVENTINF(EVENTINF_50);
        } else {
            play->nextEntrance = ENTRANCE(TOURIST_INFORMATION, 1);
        }
        gSaveContext.nextCutsceneIndex = 0;
        play->transitionTrigger = TRANS_TRIGGER_START;
        play->transitionType = TRANS_TYPE_FADE_WHITE;
        gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;
    }
    
    CLEAR_WEEKEVENTREG(WEEKEVENTREG_BOAT_PARKED_AT_DEKU_PALACE);
    Environment_StartTime();
    this->actionFunc = func_80953F8C;
}

void func_80954340(BgIngate* this, PlayState* play) {
    if (DECR(this->unk16A) == 0) {
        if (this->timePath != NULL) {
            Player_SetCsActionWithHaltedActors(play, &this->dyna.actor, PLAYER_CSACTION_END);
            this->timePath = &play->setupPathList[this->timePath->additionalPathIndex];
            func_80953F14(this, play);
            Environment_StopTime();
        }
    }
}

/* BgIngate_DekuCastleStop */
void BgIngate_DekuCastleStop(BgIngate* this, PlayState* play) {
    u8 talkState = Message_GetState(&play->msgCtx);
    Player *player = GET_PLAYER(play);
    if (((talkState == TEXT_STATE_CHOICE) || (talkState == TEXT_STATE_EVENT)) && Message_ShouldAdvance(play)) {
        switch (this->dyna.actor.textId) {
            case MESSAGE_ID_NOW_ARRIVING_AT_DEKU_PALACE_NORMAL:
                // this->dyna.actor.textId = MESSAGE_ID_ARE_YOU_DISEMBARKING_NORMAL;
                this->dyna.actor.textId = (Rand_Next() % 2) ? MESSAGE_ID_ARE_YOU_DISEMBARKING_NORMAL : MESSAGE_ID_ARE_YOU_DISEMBARKING_FLIPPED;
                Message_ContinueTextbox(play, this->dyna.actor.textId);
                break;
            case MESSAGE_ID_ARE_YOU_DISEMBARKING_NORMAL:
            case MESSAGE_ID_ARE_YOU_DISEMBARKING_FLIPPED:
                if (play->msgCtx.choiceIndex == (this->dyna.actor.textId == MESSAGE_ID_ARE_YOU_DISEMBARKING_FLIPPED)) {
                    Player_SetCsActionWithHaltedActors(play, &this->dyna.actor, PLAYER_CSACTION_END);
                    this->unk160 &= ~BGINGATE_PLAYER_DISEMBARKED;
                    this->actionFunc = BgIngate_WaitForEmbark;
                    Environment_StartTime();
                    Audio_PlaySfx_MessageDecide();
                } else {
                    if (this->timePath != NULL) {
                        this->timePath = &play->setupPathList[this->timePath->additionalPathIndex];
                    }
                    func_80953F14(this, play);
                    CLEAR_WEEKEVENTREG(WEEKEVENTREG_BOAT_PARKED_AT_DEKU_PALACE);
                    Audio_PlaySfx_MessageCancel();
                }
                Message_CloseTextbox(play);
                break;
            case MESSAGE_ID_ARE_YOU_BOARDING_NORMAL:
            case MESSAGE_ID_ARE_YOU_BOARDING_FLIPPED:
                if (play->msgCtx.choiceIndex == (this->dyna.actor.textId == MESSAGE_ID_ARE_YOU_BOARDING_FLIPPED)) {
                    func_80953EA4(this, play);
                    CLEAR_WEEKEVENTREG(WEEKEVENTREG_BOAT_PARKED_AT_DEKU_PALACE);
                    Audio_PlaySfx_MessageDecide();
                } else {
                    Player_SetCsActionWithHaltedActors(play, &this->dyna.actor, PLAYER_CSACTION_END);
                    this->unk160 &= ~BGINGATE_PLAYER_DISEMBARKED;
                    this->actionFunc = BgIngate_WaitForEmbark;
                    Environment_StartTime();
                    Audio_PlaySfx_MessageCancel();
                }
                Message_CloseTextbox(play);
                break;
        }
    }
}

void BgIngate_Init(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    BgIngate* this = (BgIngate*)thisx;
    s32 phi_a2;
    Vec3s* sp38;
    Vec3f sp2C;
    Vec3f sp20;

    if (BgIngate_FindActor(this, play, ACTORCAT_BG, ACTOR_BG_INGATE) == NULL) {
        DynaPolyActor_Init(&this->dyna, DYNA_TRANSFORM_POS | DYNA_TRANSFORM_ROT_Y);
        DynaPolyActor_LoadMesh(play, &this->dyna, &gSichitaiBoatCol);
        this->unk160 = 0;
        this->unk160 |= 0x8;
        this->unk160 |= BGINGATE_RIDE_JUST_STARTED;
        Actor_SetScale(&this->dyna.actor, 1.0f);
        this->timePath = SubS_GetAdditionalPath(play, BGINGATE_GET_PATH_INDEX(&this->dyna.actor), 0);
        this->dyna.actor.room = -1;
        if (CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE)) {
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_BOAT_PARKED_AT_DEKU_PALACE);
        }
        if (!CHECK_EVENTINF(EVENTINF_35) && CHECK_WEEKEVENTREG(WEEKEVENTREG_BOAT_PARKED_AT_DEKU_PALACE)) {
            phi_a2 = 1;
            this->unk16C = 1;
            this->actionFunc = BgIngate_WaitForEmbark;
        } else {
            phi_a2 = 0;
            if (play->curSpawn == 6) {
                func_80953F14(this, play);
                if (CHECK_EVENTINF(EVENTINF_35)) {
                    Interface_InitMinigame(play);
                } else {
                    SET_EVENTINF(EVENTINF_41);
                }
            } else {
                this->actionFunc = func_80953F8C;
            }
        }
        this->timePath = SubS_GetAdditionalPath(play, BGINGATE_GET_PATH_INDEX(&this->dyna.actor), phi_a2);
        if (this->timePath != NULL) {
            sp38 = Lib_SegmentedToVirtual(this->timePath->points);
            Math_Vec3s_ToVec3f(&sp2C, &sp38[0]);
            Math_Vec3s_ToVec3f(&sp20, &sp38[1]);
            this->dyna.actor.world.rot.y = Math_Vec3f_Yaw(&sp2C, &sp20);
            this->dyna.actor.shape.rot.y = this->dyna.actor.world.rot.y;
            this->dyna.actor.world.pos.x = sp2C.x;
            this->dyna.actor.world.pos.y = -15.0f;
            this->dyna.actor.world.pos.z = sp2C.z;
        }
        this->timePath = SubS_GetAdditionalPath(play, BGINGATE_GET_PATH_INDEX(&this->dyna.actor), 0);
    } else {
        Actor_Kill(&this->dyna.actor);
    }
}

void BgIngate_Destroy(Actor* thisx, PlayState* play) {
    BgIngate* this = (BgIngate*)thisx;

    if (this->unk160 & 8) {
        DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
    }
}

void BgIngate_Update(Actor* thisx, PlayState* play) {
    BgIngate* this = (BgIngate*)thisx;
    Player *player = GET_PLAYER(play);
    Camera *camera = Play_GetCamera(play, CAM_ID_MAIN);
    this->actionFunc(this, play);

    if(DynaPolyActor_IsPlayerOnTop(&this->dyna))
    {
        this->unk160 |= BGINGATE_PLAYER_ON_TOP;
    }
    else
    {
        this->unk160 &= ~BGINGATE_PLAYER_ON_TOP;
    }

    if((((this->unk160 & BGINGATE_PLAYER_LANDED) && !(this->unk160 & BGINGATE_PLAYER_ON_TOP)) || 
        (player->stateFlags1 & PLAYER_STATE1_4000000) || camera->setting != CAM_SET_BOAT_CRUISE) 
        && CHECK_EVENTINF(EVENTINF_RIDING_BOAT) && !Player_IsInBoatMode(play, player))
    {
        /* player somehow got disturbed during the ride, so reenable B/C buttons */
        BgIngate_SetNormalCamera(play);
        play->bButtonAmmoPlusOne = 0;
    }
    else if(this->unk160 & BGINGATE_PLAYER_ON_TOP)
    {
        this->unk160 |= BGINGATE_PLAYER_LANDED;
    }
}

void BgIngate_Draw(Actor* thisx, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gSichitaiBoatDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
