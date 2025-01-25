#include "global.h"
#include "PR/gs2dex.h"
#include "libu64/debug.h"
#include "sys_ucode.h"
#include "chaos_fuckery.h"
#include "fault.h"

extern struct ChaosContext gChaosContext;

void Room_Noop(PlayState* play, Room* room, Input* input, s32 arg3) {
}

void Room_DrawNone(PlayState* play, Room* room, u32 flags) {
}

static Vec3f sZeroVec = { 0.0f, 0.0f, 0.0f };

// void Room_RotateRoom(PlayState *play, s16 x, s16 y, s16 z)
// {
//     OPEN_DISPS(play->state.gfxCtx);
//     Mtx *room_transform = GRAPH_ALLOC(play->state.gfxCtx, sizeof(Mtx) * 2);
//     Matrix_Push();
//     Matrix_RotateZS(z, MTXMODE_NEW);
//     Matrix_RotateYS(y, MTXMODE_APPLY);
//     Matrix_RotateXS(x, MTXMODE_APPLY);
//     Matrix_ToMtx(room_transform + 1);
//     Matrix_Pop();
//     gSPSegment(POLY_OPA_DISP++, 0x01, room_transform);
//     gSPMatrix(POLY_OPA_DISP++, OS_K0_TO_PHYSICAL(room_transform + 1), G_MTX_MODELVIEW | G_MTX_LOAD);
//     CLOSE_DISPS(play->state.gfxCtx);
// }

void Room_PivotRoom(PlayState *play, Vec3f *pivot_point, s16 x, s16 y, s16 z)
{
    OPEN_DISPS(play->state.gfxCtx);
    Mtx *room_transform = GRAPH_ALLOC(play->state.gfxCtx, sizeof(Mtx) * 2);
    Matrix_Push();
    Matrix_Translate(pivot_point->x, pivot_point->y, pivot_point->z, MTXMODE_NEW);
    Matrix_RotateZS(z, MTXMODE_APPLY);
    Matrix_RotateYS(y, MTXMODE_APPLY);
    Matrix_RotateXS(x, MTXMODE_APPLY);
    Matrix_Translate(-pivot_point->x, -pivot_point->y, -pivot_point->z, MTXMODE_APPLY);
    Matrix_ToMtx(room_transform + 1);
    Matrix_Pop();
    gSPSegment(POLY_OPA_DISP++, 0x01, room_transform);
    gSPMatrix(POLY_OPA_DISP++, OS_K0_TO_PHYSICAL(room_transform + 1), G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPSegment(POLY_XLU_DISP++, 0x01, room_transform);
    gSPMatrix(POLY_XLU_DISP++, OS_K0_TO_PHYSICAL(room_transform + 1), G_MTX_MODELVIEW | G_MTX_NOPUSH);
    CLOSE_DISPS(play->state.gfxCtx);
}

void Room_ScaleRoomInDirection(PlayState *play, Vec3f *pivot_point, Vec3f *scale_axis, f32 scale_factor)
{
    f32 scale_minus_one;
    MtxF scale_transform;

    scale_minus_one = scale_factor - 1.0f;

    scale_transform.mf[0][0] = 1.0f + scale_minus_one * scale_axis->x * scale_axis->x;
    scale_transform.mf[1][0] = scale_minus_one * scale_axis->x * scale_axis->y;
    scale_transform.mf[2][0] = scale_minus_one * scale_axis->x * scale_axis->z;
    scale_transform.mf[3][0] = 0.0f;

    scale_transform.mf[0][1] = scale_minus_one * scale_axis->x * scale_axis->y;
    scale_transform.mf[1][1] = 1.0f + scale_minus_one * scale_axis->y * scale_axis->y;
    scale_transform.mf[2][1] = scale_minus_one * scale_axis->y * scale_axis->z;
    scale_transform.mf[3][1] = 0.0f;

    scale_transform.mf[0][2] = scale_minus_one * scale_axis->x * scale_axis->z;
    scale_transform.mf[1][2] = scale_minus_one * scale_axis->y * scale_axis->z;
    scale_transform.mf[2][2] = 1.0f + scale_minus_one * scale_axis->z * scale_axis->z;
    scale_transform.mf[3][2] = 0.0f;
 
    scale_transform.mf[0][3] = 0;
    scale_transform.mf[1][3] = 0;
    scale_transform.mf[2][3] = 0;
    scale_transform.mf[3][3] = 1;

    OPEN_DISPS(play->state.gfxCtx);
    Mtx *room_transform = GRAPH_ALLOC(play->state.gfxCtx, sizeof(Mtx) * 2);
    Matrix_Push();
    Matrix_Translate(pivot_point->x, pivot_point->y, pivot_point->z, MTXMODE_NEW);
    Matrix_Mult(&scale_transform, MTXMODE_APPLY);
    Matrix_Translate(-pivot_point->x, -pivot_point->y, -pivot_point->z, MTXMODE_APPLY);
    Matrix_ToMtx(room_transform + 1);
    Matrix_Pop();
    gSPSegment(POLY_OPA_DISP++, 0x01, room_transform);
    gSPMatrix(POLY_OPA_DISP++, OS_K0_TO_PHYSICAL(room_transform + 1), G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPSegment(POLY_XLU_DISP++, 0x01, room_transform);
    gSPMatrix(POLY_XLU_DISP++, OS_K0_TO_PHYSICAL(room_transform + 1), G_MTX_MODELVIEW | G_MTX_NOPUSH);
    CLOSE_DISPS(play->state.gfxCtx);
}

void Room_DrawNormal(PlayState* play, Room* room, u32 flags) {
    RoomShapeNormal* roomShape;
    RoomShapeDListsEntry* entry;
    Player *player = GET_PLAYER(play);
    s32 i;

    OPEN_DISPS(play->state.gfxCtx);

    if (flags & ROOM_DRAW_OPA) {
        func_800BCBF4(&sZeroVec, play);
        gSPSegment(POLY_OPA_DISP++, 0x03, room->segment);
        func_8012C268(&play->state);
        gSPMatrix(POLY_OPA_DISP++, &gIdentityMtx, G_MTX_MODELVIEW | G_MTX_LOAD);
    }

    if (flags & ROOM_DRAW_XLU) {
        func_800BCC68(&sZeroVec, play);
        gSPSegment(POLY_XLU_DISP++, 0x03, room->segment);
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        gSPMatrix(POLY_XLU_DISP++, &gIdentityMtx, G_MTX_MODELVIEW | G_MTX_LOAD);
    }

    if(Chaos_IsCodeActive(CHAOS_CODE_UNSTABLE_ROOMS) && (gChaosContext.room.weirdness_behavior & CHAOS_UNSTABLE_ROOMS_BEHAVIOR_ROTATE))
    {
        Room_PivotRoom(play, &player->actor.world.pos, gChaosContext.room.room_rotation.x, gChaosContext.room.room_rotation.y, gChaosContext.room.room_rotation.z);
    }

    if(flags & ROOM_DRAW_ROTATED)
    {
        Room_PivotRoom(play, &gChaosContext.room.transition_actor_pos, 0, 0x8000, 0);
    }

    roomShape = &room->roomShape->normal;
    entry = Lib_SegmentedToVirtual(roomShape->entries);
    for (i = 0; i < roomShape->numEntries; i++) {
        if ((flags & ROOM_DRAW_OPA) && (entry->opa != NULL)) {
            gSPDisplayList(POLY_OPA_DISP++, entry->opa);
        }

        if ((flags & ROOM_DRAW_XLU) && (entry->xlu != NULL)) {
            gSPDisplayList(POLY_XLU_DISP++, entry->xlu);
        }

        entry++;
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

typedef enum {
    /* 0 */ ROOM_CULL_DEBUG_MODE_OFF,
    /* 1 */ ROOM_CULL_DEBUG_MODE_UP_TO_TARGET,
    /* 2 */ ROOM_CULL_DEBUG_MODE_ONLY_TARGET
} RoomCullableDebugMode;

typedef struct RoomShapeCullableEntryLinked {
    /* 0x0 */ RoomShapeCullableEntry* entry;
    /* 0x4 */ f32 boundsNearZ;
    /* 0x8 */ struct RoomShapeCullableEntryLinked* prev;
    /* 0xC */ struct RoomShapeCullableEntryLinked* next;
} RoomShapeCullableEntryLinked; // size = 0x10

#define ROOM_SHAPE_CULLABLE_MAX_ENTRIES 128

void Room_DrawCullable(PlayState* play, Room* room, u32 flags) {
    RoomShapeCullable* roomShape;
    RoomShapeCullableEntry* roomShapeCullableEntry;
    RoomShapeCullableEntryLinked linkedEntriesBuffer[ROOM_SHAPE_CULLABLE_MAX_ENTRIES];
    RoomShapeCullableEntryLinked* head = NULL;
    RoomShapeCullableEntryLinked* tail = NULL;
    RoomShapeCullableEntryLinked* iter;
    Gfx* dList;
    RoomShapeCullableEntryLinked* insert;
    f32 entryBoundsNearZ;
    Player *player = GET_PLAYER(play);
    s32 i;
    Vec3f pos;
    Vec3f projectedPos;
    Vec3f scale_axis = {0, 0, 1};
    RoomShapeCullableEntry* roomShapeCullableEntries;
    Camera *camera = GET_ACTIVE_CAM(play);
    f32 camera_speed;
    f32 scale_factor;
    f32 c_speed = 20.0f;
    static s16 angle = 0; 
    static Vec3f prev_cam_pos = {0, 0, 0};

    OPEN_DISPS(play->state.gfxCtx);

    if (flags & ROOM_DRAW_OPA) {
        func_800BCBF4(&sZeroVec, play);

        gSPSegment(POLY_OPA_DISP++, 0x03, room->segment);
        if (play->roomCtx.unk74 != NULL) {
            gSPSegment(POLY_OPA_DISP++, 0x06, play->roomCtx.unk74);
        }
        func_8012C268(&play->state);
        gSPMatrix(POLY_OPA_DISP++, &gIdentityMtx, G_MTX_MODELVIEW | G_MTX_LOAD);
    }

    if (flags & ROOM_DRAW_XLU) {
        func_800BCC68(&sZeroVec, play);
        gSPSegment(POLY_XLU_DISP++, 0x03, room->segment);
        if (play->roomCtx.unk74 != NULL) {
            gSPSegment(POLY_XLU_DISP++, 0x06, play->roomCtx.unk74);
        }
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        gSPMatrix(POLY_XLU_DISP++, &gIdentityMtx, G_MTX_MODELVIEW | G_MTX_LOAD);
    }

    if(Chaos_IsCodeActive(CHAOS_CODE_UNSTABLE_ROOMS) && (gChaosContext.room.weirdness_behavior & CHAOS_UNSTABLE_ROOMS_BEHAVIOR_ROTATE))
    {
        Room_PivotRoom(play, &player->actor.world.pos, gChaosContext.room.room_rotation.x, gChaosContext.room.room_rotation.y, gChaosContext.room.room_rotation.z);
    }

    if(flags & ROOM_DRAW_ROTATED)
    {
        Room_PivotRoom(play, &gChaosContext.room.transition_actor_pos, 0, 0x8000, 0);
    }

    // Math_Vec3f_Diff(&camera->eye, &prev_cam_pos, &scale_axis);
    // camera_speed = Math3D_Normalize(&scale_axis);

    // scale_factor = sqrtf(1.0f - (camera_speed * camera_speed) / (c_speed * c_speed));


    // Room_ScaleRoomInDirection(play, &player->actor.world.pos, &scale_axis, scale_factor); 

    // prev_cam_pos = camera->eye;

    if (room->enablePosLights || (MREG(93) != 0)) {
        gSPSetGeometryMode(POLY_OPA_DISP++, G_LIGHTING_POSITIONAL);
        gSPSetGeometryMode(POLY_XLU_DISP++, G_LIGHTING_POSITIONAL);
    }

    roomShape = &room->roomShape->cullable;
    roomShapeCullableEntry = Lib_SegmentedToVirtual(roomShape->entries);
    insert = linkedEntriesBuffer;

    roomShapeCullableEntries = roomShapeCullableEntry;

    if (play->roomCtx.unk78 < 0) {
        for (i = 0; i < roomShape->numEntries; i++, roomShapeCullableEntry++) {
            if (R_ROOM_CULL_DEBUG_MODE != 0) {
                if (((R_ROOM_CULL_DEBUG_MODE == ROOM_CULL_DEBUG_MODE_UP_TO_TARGET) &&
                     (i <= R_ROOM_CULL_DEBUG_TARGET)) ||
                    ((R_ROOM_CULL_DEBUG_MODE == ROOM_CULL_DEBUG_MODE_ONLY_TARGET) && (i == R_ROOM_CULL_DEBUG_TARGET))) {
                    if (flags & ROOM_DRAW_OPA) {
                        dList = roomShapeCullableEntry->opa;
                        if (dList != NULL) {
                            gSPDisplayList(POLY_OPA_DISP++, dList);
                        }
                    }

                    if (flags & ROOM_DRAW_XLU) {
                        dList = roomShapeCullableEntry->xlu;
                        if (dList != NULL) {
                            gSPDisplayList(POLY_XLU_DISP++, dList);
                        }
                    }
                }
            } else {
                if (flags & ROOM_DRAW_OPA) {
                    dList = roomShapeCullableEntry->opa;
                    if (dList != NULL) {
                        gSPDisplayList(POLY_OPA_DISP++, dList);
                    }
                }

                if (flags & ROOM_DRAW_XLU) {
                    dList = roomShapeCullableEntry->xlu;
                    if (dList != NULL) {
                        gSPDisplayList(POLY_XLU_DISP++, dList);
                    }
                }
            }
        }
    } else {
        f32 var_fa1 = 1.0f / play->projectionMtxFDiagonal.z;
        f32 var_fv1;
        s32 var_a1;

        // Pick and sort entries by depth
        for (i = 0; i < roomShape->numEntries; i++, roomShapeCullableEntry++) {

            // Project the entry position, to get the depth it is at.
            pos.x = roomShapeCullableEntry->boundsSphereCenter.x;
            pos.y = roomShapeCullableEntry->boundsSphereCenter.y;
            pos.z = roomShapeCullableEntry->boundsSphereCenter.z;
            SkinMatrix_Vec3fMtxFMultXYZ(&play->viewProjectionMtxF, &pos, &projectedPos);

            projectedPos.z *= var_fa1;

            var_fv1 = ABS_ALT(roomShapeCullableEntry->boundsSphereRadius);

            // If the entry bounding sphere isn't fully before the rendered depth range
            if (-var_fv1 < projectedPos.z) {

                // Compute the depth of the nearest point in the entry's bounding sphere
                entryBoundsNearZ = projectedPos.z - var_fv1;

                // If the entry bounding sphere isn't fully beyond the rendered depth range
                if (entryBoundsNearZ < play->lightCtx.zFar) {

                    // This entry will be rendered
                    insert->entry = roomShapeCullableEntry;

                    if (roomShapeCullableEntry->boundsSphereRadius < 0) {
                        insert->boundsNearZ = FLT_MAX;
                    } else {
                        insert->boundsNearZ = entryBoundsNearZ;
                    }

                    // Insert into the linked list, ordered by ascending depth of the nearest point in the bounding
                    // sphere
                    iter = head;
                    if (iter == NULL) {
                        head = tail = insert;
                        insert->prev = insert->next = NULL;
                    } else {
                        do {
                            if (insert->boundsNearZ < iter->boundsNearZ) {
                                break;
                            }
                            iter = iter->next;
                        } while (iter != NULL);

                        if (iter == NULL) {
                            insert->prev = tail;
                            insert->next = NULL;
                            tail->next = insert;
                            tail = insert;
                        } else {
                            insert->prev = iter->prev;
                            if (insert->prev == NULL) {
                                head = insert;
                            } else {
                                insert->prev->next = insert;
                            }
                            iter->prev = insert;
                            insert->next = iter;
                        }
                    }

                    insert++;
                }
            }
        }

        //! FAKE: Similar trick used in OoT
        R_ROOM_CULL_NUM_ENTRIES = roomShape->numEntries & 0xFFFF & 0xFFFF & 0xFFFF;

        // Draw entries, from nearest to furthest
        i = 1;

        if (flags & ROOM_DRAW_OPA) {
            for (; head != NULL; head = head->next, i++) {
                s32 pad;

                roomShapeCullableEntry = head->entry;

                if (R_ROOM_CULL_DEBUG_MODE != ROOM_CULL_DEBUG_MODE_OFF) {
                    // Debug mode drawing

                    if (((R_ROOM_CULL_DEBUG_MODE == ROOM_CULL_DEBUG_MODE_UP_TO_TARGET) &&
                         (i <= R_ROOM_CULL_DEBUG_TARGET)) ||
                        ((R_ROOM_CULL_DEBUG_MODE == ROOM_CULL_DEBUG_MODE_ONLY_TARGET) &&
                         (i == R_ROOM_CULL_DEBUG_TARGET))) {

                        dList = roomShapeCullableEntry->opa;
                        if (dList != NULL) {
                            gSPDisplayList(POLY_OPA_DISP++, dList);
                        }
                    }
                } else {
                    dList = roomShapeCullableEntry->opa;
                    if (dList != NULL) {
                        gSPDisplayList(POLY_OPA_DISP++, dList);
                    }
                }
            }
        }

        if (flags & ROOM_DRAW_XLU) {
            for (; tail != NULL; tail = tail->prev) {
                f32 temp_fv0;
                f32 temp_fv1;

                roomShapeCullableEntry = tail->entry;
                dList = roomShapeCullableEntry->xlu;

                if (dList != NULL) {
                    if (roomShapeCullableEntry->boundsSphereRadius & 1) {

                        temp_fv0 = tail->boundsNearZ - (f32)(iREG(93) + 0xBB8);
                        temp_fv1 = iREG(94) + 0x7D0;

                        if (temp_fv0 < temp_fv1) {
                            if (temp_fv0 < 0.0f) {
                                var_a1 = 255;
                            } else {
                                var_a1 = 255 - (s32)((temp_fv0 / temp_fv1) * 255.0f);
                            }
                            gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 255, var_a1);
                            gSPDisplayList(POLY_XLU_DISP++, dList);
                        }
                    } else {
                        gSPDisplayList(POLY_XLU_DISP++, dList);
                    }
                }
            }
        }

        R_ROOM_CULL_USED_ENTRIES = i - 1;
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

#define ROOM_IMAGE_NODRAW_BACKGROUND (1 << 0)
#define ROOM_IMAGE_NODRAW_OPA (1 << 1)
#define ROOM_IMAGE_NODRAW_XLU (1 << 2)

void Room_DrawImageSingle(PlayState* play, Room* room, u32 flags) {
    Camera* activeCam;
    Gfx* gfx;
    RoomShapeImageSingle* roomShape;
    RoomShapeDListsEntry* entry;
    Player *player = GET_PLAYER(play);
    u32 isFixedCamera;
    u32 drawBackground;
    u32 drawOpa;
    u32 drawXlu;

    OPEN_DISPS(play->state.gfxCtx);

    activeCam = GET_ACTIVE_CAM(play);
    isFixedCamera = false; // Condition is inferred from OoT
    roomShape = &room->roomShape->image.single;
    entry = Lib_SegmentedToVirtual(roomShape->base.entry);
    drawBackground = (flags & ROOM_DRAW_OPA) && isFixedCamera && (roomShape->source != NULL) &&
                     !(R_ROOM_IMAGE_NODRAW_FLAGS & ROOM_IMAGE_NODRAW_BACKGROUND);
    drawOpa = (flags & ROOM_DRAW_OPA) && (entry->opa != NULL) && !(R_ROOM_IMAGE_NODRAW_FLAGS & ROOM_IMAGE_NODRAW_OPA);
    drawXlu = (flags & ROOM_DRAW_XLU) && (entry->xlu != NULL) && !(R_ROOM_IMAGE_NODRAW_FLAGS & ROOM_IMAGE_NODRAW_XLU);

    if (drawOpa || drawBackground) {
        gSPSegment(POLY_OPA_DISP++, 0x03, room->segment);

        if (drawOpa) {
            Gfx_SetupDL25_Opa(play->state.gfxCtx);
            gSPMatrix(POLY_OPA_DISP++, &gIdentityMtx, G_MTX_MODELVIEW | G_MTX_LOAD);

            if(Chaos_IsCodeActive(CHAOS_CODE_UNSTABLE_ROOMS) && (gChaosContext.room.weirdness_behavior & CHAOS_UNSTABLE_ROOMS_BEHAVIOR_ROTATE))
            {
                Room_PivotRoom(play, &player->actor.world.pos, gChaosContext.room.room_rotation.x, gChaosContext.room.room_rotation.y, gChaosContext.room.room_rotation.z);
            }
            gSPDisplayList(POLY_OPA_DISP++, entry->opa);
        }

        if (drawBackground) {
            gSPLoadUcodeL(POLY_OPA_DISP++, gspS2DEX2_fifo);

            gfx = POLY_OPA_DISP;

            {
                Vec3f quakeOffset;

                quakeOffset = Camera_GetQuakeOffset(activeCam);

                Prerender_DrawBackground2D(
                    &gfx, roomShape->source, roomShape->tlut, roomShape->width, roomShape->height, roomShape->fmt,
                    roomShape->siz, roomShape->tlutMode, roomShape->tlutCount,
                    (quakeOffset.x + quakeOffset.z) * 1.2f + quakeOffset.y * 0.6f,
                    quakeOffset.y * 2.4f + (quakeOffset.x + quakeOffset.z) * 0.3f, 1.0f, 1.0f, 0);
            }

            POLY_OPA_DISP = gfx;

            gSPLoadUcode(POLY_OPA_DISP++, SysUcode_GetUCode(), SysUcode_GetUCodeData());
        }
    }

    if (drawXlu) {
        gSPSegment(POLY_XLU_DISP++, 0x03, room->segment);
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        gSPMatrix(POLY_XLU_DISP++, &gIdentityMtx, G_MTX_MODELVIEW | G_MTX_LOAD);
        if(Chaos_IsCodeActive(CHAOS_CODE_UNSTABLE_ROOMS) && (gChaosContext.room.weirdness_behavior & CHAOS_UNSTABLE_ROOMS_BEHAVIOR_ROTATE))
        {
            Room_PivotRoom(play, &player->actor.world.pos, gChaosContext.room.room_rotation.x, gChaosContext.room.room_rotation.y, gChaosContext.room.room_rotation.z);
        }
        gSPDisplayList(POLY_XLU_DISP++, entry->xlu);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

RoomShapeImageMultiBgEntry* Room_GetImageMultiBgEntry(RoomShapeImageMulti* roomShapeImageMulti, PlayState* play) {
    Camera* activeCam = GET_ACTIVE_CAM(play);
    s32 bgCamIndex = activeCam->bgCamIndex;
    s16 overrideBgCamIndex;
    Player* player;
    RoomShapeImageMultiBgEntry* bgEntry;
    s32 i;

    overrideBgCamIndex = ((BgCamFuncData*)BgCheck_GetBgCamFuncDataImpl(&play->colCtx, bgCamIndex, BGCHECK_SCENE))
                             ->roomImageOverrideBgCamIndex;
    if (overrideBgCamIndex >= 0) {
        bgCamIndex = overrideBgCamIndex;
    }

    player = GET_PLAYER(play);
    player->actor.params = (player->actor.params & 0xFF00) | bgCamIndex;

    bgEntry = Lib_SegmentedToVirtual(roomShapeImageMulti->backgrounds);
    for (i = 0; i < roomShapeImageMulti->numBackgrounds; i++) {
        if (bgEntry->bgCamIndex == bgCamIndex) {
            return bgEntry;
        }
        bgEntry++;
    }

    _dbg_hungup("../z_room.c", 849);

    return NULL;
}

void Room_DrawImageMulti(PlayState* play, Room* room, u32 flags) {
    Camera* activeCam;
    Gfx* gfx;
    RoomShapeImageMulti* roomShape;
    RoomShapeImageMultiBgEntry* bgEntry;
    RoomShapeDListsEntry* dListsEntry;
    u32 isFixedCamera;
    u32 drawBackground;
    u32 drawOpa;
    u32 drawXlu;

    OPEN_DISPS(play->state.gfxCtx);

    activeCam = GET_ACTIVE_CAM(play);
    isFixedCamera = false; // Condition is inferred from OoT
    roomShape = &room->roomShape->image.multi;
    dListsEntry = Lib_SegmentedToVirtual(roomShape->base.entry);

    bgEntry = Room_GetImageMultiBgEntry(roomShape, play);

    drawBackground = (flags & ROOM_DRAW_OPA) && isFixedCamera && (bgEntry->source != NULL) &&
                     !(R_ROOM_IMAGE_NODRAW_FLAGS & ROOM_IMAGE_NODRAW_BACKGROUND);
    drawOpa =
        (flags & ROOM_DRAW_OPA) && (dListsEntry->opa != NULL) && !(R_ROOM_IMAGE_NODRAW_FLAGS & ROOM_IMAGE_NODRAW_OPA);
    drawXlu =
        (flags & ROOM_DRAW_XLU) && (dListsEntry->xlu != NULL) && !(R_ROOM_IMAGE_NODRAW_FLAGS & ROOM_IMAGE_NODRAW_XLU);

    if (drawOpa || drawBackground) {
        gSPSegment(POLY_OPA_DISP++, 0x03, room->segment);

        if (drawOpa) {
            Gfx_SetupDL25_Opa(play->state.gfxCtx);
            gSPMatrix(POLY_OPA_DISP++, &gIdentityMtx, G_MTX_MODELVIEW | G_MTX_LOAD);
            gSPDisplayList(POLY_OPA_DISP++, dListsEntry->opa);
        }

        if (drawBackground) {
            gSPLoadUcodeL(POLY_OPA_DISP++, gspS2DEX2_fifo);

            gfx = POLY_OPA_DISP;

            {
                Vec3f quakeOffset;

                quakeOffset = Camera_GetQuakeOffset(activeCam);

                Prerender_DrawBackground2D(&gfx, bgEntry->source, bgEntry->tlut, bgEntry->width, bgEntry->height,
                                           bgEntry->fmt, bgEntry->siz, bgEntry->tlutMode, bgEntry->tlutCount,
                                           (quakeOffset.x + quakeOffset.z) * 1.2f + quakeOffset.y * 0.6f,
                                           quakeOffset.y * 2.4f + (quakeOffset.x + quakeOffset.z) * 0.3f, 1.0f, 1.0f,
                                           0);
            }

            POLY_OPA_DISP = gfx;

            gSPLoadUcode(POLY_OPA_DISP++, SysUcode_GetUCode(), SysUcode_GetUCodeData());
        }
    }

    if (drawXlu) {
        gSPSegment(POLY_XLU_DISP++, 0x03, room->segment);
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        gSPMatrix(POLY_XLU_DISP++, &gIdentityMtx, G_MTX_MODELVIEW | G_MTX_LOAD);
        gSPDisplayList(POLY_XLU_DISP++, dListsEntry->xlu);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void Room_DrawImage(PlayState* play, Room* room, u32 flags) {
    RoomShapeImageBase* roomShape = &room->roomShape->image.base;

    if (roomShape->amountType == ROOM_SHAPE_IMAGE_AMOUNT_SINGLE) {
        Room_DrawImageSingle(play, room, flags);
    } else if (roomShape->amountType == ROOM_SHAPE_IMAGE_AMOUNT_MULTI) {
        Room_DrawImageMulti(play, room, flags);
    } else {
        _dbg_hungup("../z_room.c", 965);
    }
}

void Room_Init(PlayState* play, RoomContext* roomCtx) {
    s32 i;

    roomCtx->curRoom.num = -1;
    roomCtx->curRoom.segment = NULL;
    roomCtx->unk78 = 1;
    roomCtx->unk79 = 0;
    for (i = 0; i < ARRAY_COUNT(roomCtx->unk7A); i++) {
        roomCtx->unk7A[i] = 0;
    }
}

/**
 * Allocates memory for rooms and fetches the first room that the player will spawn into.
 *
 * @return u32 size of the buffer reserved for room data
 */
size_t Room_SetupFirstRoom(PlayState* play, RoomContext* roomCtx) {
    size_t roomBufferSize = 0;
    size_t roomSize;
    s32 i;
    s32 j;
    s32 frontRoom;
    s32 backRoom;
    size_t frontRoomSize;
    size_t backRoomSize;
    size_t cumulRoomSize;
    s32 pad[2];

    // Set roomBufferSize to the largest room
    {
        RomFile* roomList = play->roomList.romFiles;

        for (i = 0; i < play->roomList.count; i++) {
            roomSize = roomList[i].vromEnd - roomList[i].vromStart;
            roomBufferSize = MAX(roomSize, roomBufferSize);
        }
    }

    // If there any rooms are connected, find their combined size and update roomBufferSize if larger
    if ((u32)play->transitionActors.count != 0) {
        RomFile* roomList = play->roomList.romFiles;
        TransitionActorEntry* transitionActor = &play->transitionActors.list[0];

        for (j = 0; j < play->transitionActors.count; j++) {
            frontRoom = transitionActor->sides[0].room;
            backRoom = transitionActor->sides[1].room;
            frontRoomSize = (frontRoom < 0) ? 0 : roomList[frontRoom].vromEnd - roomList[frontRoom].vromStart;
            backRoomSize = (backRoom < 0) ? 0 : roomList[backRoom].vromEnd - roomList[backRoom].vromStart;
            cumulRoomSize = (frontRoom != backRoom) ? frontRoomSize + backRoomSize : frontRoomSize;

            roomBufferSize = MAX(cumulRoomSize, roomBufferSize);
            transitionActor++;
        }
    }

    roomCtx->bufPtrs[0] = THA_AllocTailAlign16(&play->state.tha, roomBufferSize);
    if (roomCtx->bufPtrs[0] == NULL) {
        _dbg_hungup("../z_room.c", 1078);
    }
    roomCtx->bufPtrs[1] = (void*)((uintptr_t)roomCtx->bufPtrs[0] + roomBufferSize);
    roomCtx->activeBufPage = 0;
    roomCtx->status = 0;

    if ((gSaveContext.respawnFlag != 0) && (gSaveContext.respawnFlag != -2) && (gSaveContext.respawnFlag != -7)) {
        s32 respawnMode;

        if ((gSaveContext.respawnFlag == -8) || (gSaveContext.respawnFlag == -5) || (gSaveContext.respawnFlag == -4) ||
            ((gSaveContext.respawnFlag < 0) && (gSaveContext.respawnFlag != -1) && (gSaveContext.respawnFlag != -6))) {
            respawnMode = RESPAWN_MODE_DOWN;
        } else if (gSaveContext.respawnFlag < 0) {
            respawnMode = RESPAWN_MODE_TOP;
        } else {
            respawnMode = gSaveContext.respawnFlag - 1;
        }
        frontRoom = gSaveContext.respawn[respawnMode].roomIndex;
    } else {
        frontRoom = play->setupEntranceList[play->curSpawn].room;
    }

    // Load into a room for the first time.
    // Since curRoom was initialized to `room` = -1 and `segment` = NULL in Play_InitScene, the previous room
    // will also be initialized to the nulled state when this function completes.
    Room_RequestNewRoom(play, roomCtx, frontRoom);

    return roomBufferSize;
}

/**
 * Tries to create an asynchronous request to transfer room data into memory.
 * If successful, the requested room will be loaded into memory and becomes the new current room; the room that was
 * current before becomes the previous room.
 *
 * Room_RequestNewRoom will be blocked from loading new rooms until Room_ProcessRoomRequest completes room
 * initialization.
 *
 * Calling Room_RequestNewRoom outside of Room_SetupFirstRoom will allow for two rooms being initialized simultaneously.
 * This allows an actor like ACTOR_EN_HOLL to seamlessly swap the two rooms as the player moves between them. Calling
 * Room_FinishRoomChange afterward will finalize the room swap.
 *
 * @param roomNum is the id of the room to load. roomNum must NOT be the same id as curRoom.num, since this will create
 * duplicate actor instances that cannot be cleaned up by calling Room_FinishRoomChange
 * @returns bool false if the request could not be created.
 */
s32 Room_RequestNewRoom(PlayState* play, RoomContext* roomCtx, s32 index) {
    if (roomCtx->status == 0) {
        size_t size;

        roomCtx->prevRoom = roomCtx->curRoom;
        roomCtx->curRoom.num = index;
        roomCtx->curRoom.segment = NULL;
        roomCtx->status = 1;

        size = play->roomList.romFiles[index].vromEnd - play->roomList.romFiles[index].vromStart;
        roomCtx->roomRequestAddr = (void*)(ALIGN16((uintptr_t)roomCtx->bufPtrs[roomCtx->activeBufPage] -
                                                   (size + 8) * roomCtx->activeBufPage - 7));

        osCreateMesgQueue(&roomCtx->loadQueue, roomCtx->loadMsg, ARRAY_COUNT(roomCtx->loadMsg));
        DmaMgr_RequestAsync(&roomCtx->dmaRequest, roomCtx->roomRequestAddr, play->roomList.romFiles[index].vromStart,
                            size, 0, &roomCtx->loadQueue, NULL);
        roomCtx->activeBufPage ^= 1;

        gChaosContext.room.vert_list_list[roomCtx->activeBufPage] = NULL;

        return true;
    }

    return false;
}

/**
 * Completes room initialization for the room requested by a call to Room_RequestNewRoom.
 * This function does not block the thread if the room data is still being transferred.
 *
 * @returns bool false if a dma transfer is in progress.
 */
s32 Room_ProcessRoomRequest(PlayState* play, RoomContext* roomCtx) {
    if (roomCtx->status == 1) {
        if (osRecvMesg(&roomCtx->loadQueue, NULL, OS_MESG_NOBLOCK) == 0) {
            roomCtx->status = 0;
            roomCtx->curRoom.segment = roomCtx->roomRequestAddr;
            gSegments[0x03] = OS_K0_TO_PHYSICAL(roomCtx->curRoom.segment);

            Scene_ExecuteCommands(play, roomCtx->curRoom.segment);
            func_80123140(play, GET_PLAYER(play));
            Actor_SpawnTransitionActors(play, &play->actorCtx);

            if (((play->sceneId != SCENE_IKANA) || (roomCtx->curRoom.num != 1)) && (play->sceneId != SCENE_IKNINSIDE)) {
                play->envCtx.lightSettingOverride = LIGHT_SETTING_OVERRIDE_NONE;
                play->envCtx.lightBlendOverride = LIGHT_BLEND_OVERRIDE_NONE;
            }
            func_800FEAB0();
            if (Environment_GetStormState(play) == STORM_STATE_OFF) {
                Environment_StopStormNatureAmbience(play);
            }
        } else {
            return false;
        }
    }

    return true;
}

RoomDrawHandler sRoomDrawHandlers[] = {
    Room_DrawNormal,   // ROOM_SHAPE_TYPE_NORMAL
    Room_DrawImage,    // ROOM_SHAPE_TYPE_IMAGE
    Room_DrawCullable, // ROOM_SHAPE_TYPE_CULLABLE
    Room_DrawNone,     // ROOM_SHAPE_TYPE_NONE
};

void Room_Draw(PlayState* play, Room* room, u32 flags) {
    if (room->segment != NULL) {
        gSegments[0x03] = OS_K0_TO_PHYSICAL(room->segment);
        sRoomDrawHandlers[room->roomShape->base.type](play, room, flags);
    }
    return;
}

/**
 * Finalizes a swap between two rooms.
 *
 * When a new room is created with Room_RequestNewRoom, the previous room and its actors remain in memory. This allows
 * an actor like ACTOR_EN_HOLL to seamlessly swap the two rooms as the player moves between them.
 */
void Room_FinishRoomChange(PlayState* play, RoomContext* roomCtx) {
    // Delete the previous room
    roomCtx->prevRoom.num = -1;
    roomCtx->prevRoom.segment = NULL;

    func_800BA798(play, &play->actorCtx);
    Actor_SpawnTransitionActors(play, &play->actorCtx);
    if (roomCtx->curRoom.num > -1) {
        Map_InitRoomData(play, roomCtx->curRoom.num);
        Map_SetAreaEntrypoint(play);
    }
    Audio_SetEnvReverb(play->roomCtx.curRoom.echo);
}
