#include "global.h"
#include "chaos_fuckery.h"
#include "fault.h"
#include "z64object.h"

extern struct ChaosContext gChaosContext;
extern const char *gObjectNames[];

static u8 gLoadedObjects[(OBJECT_ID_MAX / 8) + 1];

extern FaultClient gLoadedObjectsClient;

void Object_LoadedObjectsClientCallback(void *obj_ctx, void *arg1)
{
    ObjectContext *ctx = obj_ctx;
    u32 index;

    FaultDrawer_SetCursor(32, 32);
    FaultDrawer_Printf("Loaded objects (%d/%d)\n", ctx->numEntries, ARRAY_COUNT(ctx->slots));
    FaultDrawer_Printf("___________________________________\n");

    for(index = 0; index < ctx->numEntries; index++)
    {
        ObjectEntry *slot = ctx->slots + index;
        if(slot->id > 0)
        {
            FaultDrawer_Printf("%03d - %s (%03d)\n", index, gObjectNames[slot->id], slot->id);
        }
    }
}

void Object_InitObjectTableFaultClient(ObjectContext *obj_ctx)
{
    Fault_AddClient(&gLoadedObjectsClient, Object_LoadedObjectsClientCallback, obj_ctx, NULL);
}

void Object_CleanupObjectTableFaultClient(void)
{
    Fault_RemoveClient(&gLoadedObjectsClient);
}

s32 Object_AllocatePersistent(ObjectContext* objectCtx, s16 id) 
{
    size_t size = gObjectTable[id].vromEnd - gObjectTable[id].vromStart;
    // objectCtx->slots[objectCtx->numEntries].id = id;
    // size = gObjectTable[id].vromEnd - gObjectTable[id].vromStart;

    if (objectCtx->numEntries < ARRAY_COUNT(objectCtx->slots) - 1) {
        objectCtx->slots[objectCtx->numEntries + 1].segment =
            (void*)ALIGN16((uintptr_t)objectCtx->slots[objectCtx->numEntries].segment + size);
    }

    objectCtx->numEntries++;
    objectCtx->numPersistentEntries = objectCtx->numEntries;

    return objectCtx->numEntries - 1;
}

/**
 * Spawn an object file of a specified ID that will persist through room changes.
 *
 * This waits for the file to be fully loaded, the data is available when the function returns.
 *
 * @return The new object slot corresponding to the requested object ID.
 *
 * @note This function is not meant to be called externally to spawn object files on the fly.
 * When an object is spawned with this function, all objects that come before it in the entry list will be treated as
 * persistent, which will likely cause either the amount of free slots or object space memory to run out.
 * This function is only meant to be called internally on scene load, before the object list from any room is processed.
 */

s32 Object_SpawnPersistent(ObjectContext* objectCtx, s16 id) {
    size_t size = gObjectTable[id].vromEnd - gObjectTable[id].vromStart;
    objectCtx->slots[objectCtx->numEntries].id = id;
    // size = gObjectTable[id].vromEnd - gObjectTable[id].vromStart;

    //! FAKE:
    if (1) {}

    if (size != 0) {
        DmaMgr_RequestSync(objectCtx->slots[objectCtx->numEntries].segment, gObjectTable[id].vromStart, size);
    }

    if (objectCtx->numEntries < ARRAY_COUNT(objectCtx->slots) - 1) {
        objectCtx->slots[objectCtx->numEntries + 1].segment =
            (void*)ALIGN16((uintptr_t)objectCtx->slots[objectCtx->numEntries].segment + size);
    }

    objectCtx->numEntries++;
    objectCtx->numPersistentEntries = objectCtx->numEntries;
    Object_MarkObjectAsLoaded(objectCtx, id);

    return objectCtx->numEntries - 1;
}

void Object_InitContext(GameState* gameState, ObjectContext* objectCtx) {
    PlayState* play = (PlayState*)gameState;
    s32 pad;
    u32 spaceSize;
    s32 index;
    size_t largest_size = 0;
    u32 largest_id = 0;
    u32 *p;

    if (play->sceneId == SCENE_CLOCKTOWER || play->sceneId == SCENE_TOWN || play->sceneId == SCENE_BACKTOWN ||
        play->sceneId == SCENE_ICHIBA) {
        spaceSize = 1530 * 1024;
    } else if (play->sceneId == SCENE_MILK_BAR) {
        spaceSize = 1580 * 1024;
    } else if (play->sceneId == SCENE_00KEIKOKU) {
        spaceSize = 1470 * 1024;
    } else {
        spaceSize = 1380 * 1024;
    }

    objectCtx->numEntries = 0;
    objectCtx->numPersistentEntries = 0;
    objectCtx->mainKeepSlot = 0;
    objectCtx->subKeepSlot = 0;

    #ifdef USE_NEW_METHOD
    objectCtx->pending_request_count = 0;
    objectCtx->next_available_request = 0;
    #endif

    for (index = 0; index < ARRAY_COUNT(objectCtx->slots); index++)
    { 
        objectCtx->slots[index].id = 0; 
        #ifdef USE_NEW_METHOD
        objectCtx->slots[index].load_pending = false;
        #endif
    }

    bzero(gLoadedObjects, sizeof(gLoadedObjects));

    spaceSize += ALIGN16(gObjectTable[OBJECT_RR].vromEnd - gObjectTable[OBJECT_RR].vromStart);
    spaceSize += ALIGN16(gObjectTable[OBJECT_SKB].vromEnd - gObjectTable[OBJECT_SKB].vromStart);
    spaceSize += ALIGN16(gObjectTable[OBJECT_GI_SHIELD_2].vromEnd - gObjectTable[OBJECT_GI_SHIELD_2].vromStart);
    // spaceSize += ALIGN16(gObjectTable[OBJECT_BEYBLADE].vromEnd - gObjectTable[OBJECT_BEYBLADE].vromStart);
    spaceSize += ALIGN16(gObjectTable[OBJECT_ARWING].vromEnd - gObjectTable[OBJECT_ARWING].vromStart);
    // spaceSize += ALIGN16(gObjectTable[OBJECT_DARK_LINK].vromEnd - gObjectTable[OBJECT_DARK_LINK].vromStart);
    spaceSize += ALIGN16(gObjectTable[OBJECT_DAI].vromEnd - gObjectTable[OBJECT_DAI].vromStart);
    // spaceSize += ALIGN16(gObjectTable[OBJECT_FALL_CAKE].vromEnd - gObjectTable[OBJECT_FALL_CAKE].vromStart);
    spaceSize += gChaosContext.chaos_keep_size;

    objectCtx->spaceStart = objectCtx->slots[0].segment = THA_AllocTailAlign16(&gameState->tha, spaceSize);
    objectCtx->spaceEnd = (void*)((u32)objectCtx->spaceStart + spaceSize);
    objectCtx->mainKeepSlot = Object_SpawnPersistent(objectCtx, GAMEPLAY_KEEP);
    Object_SpawnPersistent(objectCtx, OBJECT_RR);
    Object_SpawnPersistent(objectCtx, OBJECT_SKB);
    Object_SpawnPersistent(objectCtx, OBJECT_GI_SHIELD_2);
    // Object_SpawnPersistent(objectCtx, OBJECT_BEYBLADE);
    Object_SpawnPersistent(objectCtx, OBJECT_ARWING);
    // Object_SpawnPersistent(objectCtx, OBJECT_DARK_LINK);
    Object_SpawnPersistent(objectCtx, OBJECT_DAI);
    // Object_SpawnPersistent(objectCtx, OBJECT_FALL_CAKE);


    /* allocate enough space for the largest object */
    gChaosContext.chaos_keep_slot = Object_AllocatePersistent(objectCtx, gChaosContext.chaos_keep_largest_object);

    if(gChaosContext.loaded_object_id > 0)
    {
        /* some effect might be using this object, so reload it */
        Object_RequestOverwrite(objectCtx, gChaosContext.chaos_keep_slot, gChaosContext.loaded_object_id);
    }

    gSegments[0x04] = OS_K0_TO_PHYSICAL(objectCtx->slots[objectCtx->mainKeepSlot].segment);
}

void Object_UpdateEntries(ObjectContext* objectCtx) {
    s32 entry_index;
    s32 request_index;
    ObjectEntry* entry = &objectCtx->slots[0];
    RomFile* objectFile;
    size_t size;

    #ifdef USE_NEW_METHOD
    request_index = objectCtx->next_available_request;
    if(objectCtx->next_available_request < objectCtx->pending_request_count)
    {
        request_index += MAX_OBJECT_REQUESTS;
    }
    request_index -= objectCtx->pending_request_count;

    while(objectCtx->pending_request_count > 0)
    {
        struct ObjectLoadRequest *request = objectCtx->load_requests + request_index;
        ObjectEntry *slot = objectCtx->slots + request->slot_index;

        request_index = (request_index + 1) % MAX_OBJECT_REQUESTS;

        if(osRecvMesg(&request->load_queue, NULL, OS_MESG_NOBLOCK))
        {
            break;
        }

        slot->id = -slot->id;
        objectCtx->pending_request_count--;
        Object_MarkObjectAsLoaded(objectCtx, slot->id);
    }

    for (entry_index = 0; entry_index < objectCtx->numEntries; entry_index++) 
    {
        ObjectEntry *entry = objectCtx->slots + entry_index;

        if (entry->id < 0) 
        {
            if(!entry->load_pending)
            {
                objectFile = &gObjectTable[-entry->id];
                size = objectFile->vromEnd - objectFile->vromStart;

                if (size == 0) 
                {
                    entry->id = 0;
                } 
                else 
                {
                    struct ObjectLoadRequest *request = NULL;

                    if(objectCtx->pending_request_count >= MAX_OBJECT_REQUESTS)
                    {
                        break;
                    }

                    request = objectCtx->load_requests + objectCtx->next_available_request;
                    objectCtx->next_available_request = (objectCtx->next_available_request + 1) % MAX_OBJECT_REQUESTS;
                    objectCtx->pending_request_count++;
                    request->slot_index = entry_index;
                    osCreateMesgQueue(&request->load_queue, &request->load_msg, 1);
                    DmaMgr_RequestAsync(&request->dma_req, entry->segment, objectFile->vromStart, size, 0, &request->load_queue, NULL);
                    entry->load_pending = true;
                }
            }
        }
    }
    #else
    for (entry_index = 0; entry_index < objectCtx->numEntries; entry_index++) {
        if (entry->id < 0) {
            s32 id = -entry->id;

            if (entry->dmaReq.vromAddr == 0) {
                objectFile = &gObjectTable[id];
                size = objectFile->vromEnd - objectFile->vromStart;

                if (size == 0) {
                    Object_MarkObjectAsUnloaded(objectCtx, id);
                    entry->id = 0;
                } else {
                    osCreateMesgQueue(&entry->loadQueue, &entry->loadMsg, 1);
                    DmaMgr_RequestAsync(&entry->dmaReq, entry->segment, objectFile->vromStart, size, 0,
                                        &entry->loadQueue, NULL);
                }
            } else if (!osRecvMesg(&entry->loadQueue, NULL, OS_MESG_NOBLOCK)) {
                entry->id = id;
                Object_MarkObjectAsLoaded(objectCtx, id);
            }
        }

        entry++;
    }
    #endif
}

s32 Object_GetSlot(ObjectContext* objectCtx, s16 objectId) {
    s32 slot = Object_GetNonPersistentSlot(objectCtx, objectId);

    if(slot == OBJECT_SLOT_NONE)
    {
        slot = Object_GetPersistentSlot(objectCtx, objectId);
    }

    return slot;

    // s32 slot = Object_GetPersistentSlot(objectCtx, objectId);

    // if(slot == OBJECT_SLOT_NONE)
    // {
    //     slot = Object_GetNonPersistentSlot(objectCtx, objectId);
    // }

    // return slot;

    // u32 i;
    // if(Object_IsLoadedById(objectCtx, objectId))
    // {
    //     for (i = 0; i < objectCtx->numEntries; i++) {
    //         if (ABS_ALT(objectCtx->slots[i].id) == objectId) {
    //             return i;
    //         }
    //     }
    // }

    // return OBJECT_SLOT_NONE;
}

s32 Object_GetPersistentSlot(ObjectContext* objectCtx, s16 objectId) 
{
    s32 i;

    // if(Object_IsLoadedById(objectCtx, objectId))
    {
        for (i = 0; i < objectCtx->numPersistentEntries; i++) {
            if (ABS_ALT(objectCtx->slots[i].id) == objectId) {
                return i;
            }
        }
    }

    return OBJECT_SLOT_NONE;
}

s32 Object_GetNonPersistentSlot(ObjectContext* objectCtx, s16 objectId) 
{
    s32 i;

    // if(Object_IsLoadedById(objectCtx, objectId))
    {
        for (i = objectCtx->numPersistentEntries; i < objectCtx->numEntries; i++) {
            if (ABS_ALT(objectCtx->slots[i].id) == objectId) {
                return i;
            }
        }
    }

    return OBJECT_SLOT_NONE;
}

void Object_MarkObjectAsLoaded(ObjectContext *ctx, s16 id)
{
    id = ABS(id);
    if(id < OBJECT_ID_MAX)
    {
        u32 byte_index = id >> 3;
        u32 bit_index = id & 0x7;
        gLoadedObjects[byte_index] |= 1 << bit_index;
    }
}

void Object_MarkObjectAsUnloaded(ObjectContext *ctx, s16 id)
{
    id = ABS(id);
    if(id < OBJECT_ID_MAX)
    {
        u32 byte_index = id >> 3;
        u32 bit_index = id & 0x7;
        gLoadedObjects[byte_index] &= ~(1 << bit_index);
    }
}

s32 Object_IsLoaded(ObjectContext* objectCtx, s32 slot) {
    return slot > OBJECT_SLOT_NONE && objectCtx->slots[slot].id > 0;
}

s32 Object_IsLoadedById(ObjectContext *objectCtx, s16 id) 
{
    // id = ABS(id);
    if(id > 0 && id < OBJECT_ID_MAX)
    {
        u32 byte_index = id >> 3;
        u32 bit_index = id & 0x7;

        return gLoadedObjects[byte_index] & (1 << bit_index);
    }
    
    return 0;
}

void Object_LoadAll(ObjectContext* objectCtx) {
    s32 i;
    s32 id;
    uintptr_t vromSize;

    bzero(gLoadedObjects, sizeof(gLoadedObjects));

    for (i = 0; i < objectCtx->numEntries; i++) {
        id = objectCtx->slots[i].id;
        vromSize = gObjectTable[id].vromEnd - gObjectTable[id].vromStart;

        if (vromSize == 0) {
            continue;
        }

        DmaMgr_RequestSync(objectCtx->slots[i].segment, gObjectTable[id].vromStart, vromSize);
        Object_MarkObjectAsLoaded(objectCtx, id);
    }
} 
/* Object_RequestOverwrite? */
void* Object_RequestOverwrite(ObjectContext* objectCtx, s32 slot, s16 id) {
    uintptr_t addr;
    uintptr_t vromSize;
    RomFile* fileTableEntry;

    if(objectCtx->slots[slot].id > 0)
    {
        Object_MarkObjectAsUnloaded(objectCtx, objectCtx->slots[slot].id);
    }

    objectCtx->slots[slot].id = -id;
    #ifdef USE_NEW_METHOD
    objectCtx->slots[slot].load_pending = false;
    #else
    objectCtx->slots[slot].dmaReq.vromAddr = 0;
    #endif

    fileTableEntry = &gObjectTable[id];
    vromSize = fileTableEntry->vromEnd - fileTableEntry->vromStart;

    // TODO: UB to cast void to u32
    addr = ((uintptr_t)objectCtx->slots[slot].segment) + vromSize;
    addr = ALIGN16(addr);

    return (void*)addr;
}

// SceneTableEntry Header Command 0x00: Spawn List
void Scene_CommandSpawnList(PlayState* play, SceneCmd* cmd) {
    s32 loadedCount;
    s16 playerObjectId;
    void* objectPtr;

    play->linkActorEntry =
        (ActorEntry*)Lib_SegmentedToVirtual(cmd->spawnList.segment) + play->setupEntranceList[play->curSpawn].spawn;
    if ((PLAYER_GET_START_MODE(play->linkActorEntry) == PLAYER_START_MODE_TELESCOPE) ||
        ((gSaveContext.respawnFlag == 2) && (gSaveContext.respawn[RESPAWN_MODE_RETURN].playerParams ==
                                             PLAYER_PARAMS(0xFF, PLAYER_START_MODE_TELESCOPE)))) {
        // Skull Kid Object
        Object_SpawnPersistent(&play->objectCtx, OBJECT_STK);
        return;
    }

    loadedCount = Object_SpawnPersistent(&play->objectCtx, OBJECT_LINK_CHILD);
    Object_MarkObjectAsUnloaded(&play->objectCtx, OBJECT_LINK_CHILD);
    objectPtr = play->objectCtx.slots[play->objectCtx.numEntries].segment;
    play->objectCtx.numEntries = loadedCount;
    play->objectCtx.numPersistentEntries = loadedCount;
    playerObjectId = gPlayerFormObjectIds[GET_PLAYER_FORM];
    gActorOverlayTable[ACTOR_PLAYER].profile->objectId = playerObjectId;
    Object_SpawnPersistent(&play->objectCtx, playerObjectId);

    play->objectCtx.slots[play->objectCtx.numEntries].segment = objectPtr;
}

// SceneTableEntry Header Command 0x01: Actor List
void Scene_CommandActorList(PlayState* play, SceneCmd* cmd) {
    play->numSetupActors = cmd->actorList.num;
    play->setupActorList = Lib_SegmentedToVirtual(cmd->actorList.segment);
    play->actorCtx.halfDaysBit = 0;
}

// SceneTableEntry Header Command 0x02: List of camera data for actor cutscenes
void Scene_CommandActorCutsceneCamList(PlayState* play, SceneCmd* cmd) {
    play->actorCsCamList = Lib_SegmentedToVirtual(cmd->actorCsCamList.segment);
}

// SceneTableEntry Header Command 0x03: Collision Header
void Scene_CommandCollisionHeader(PlayState* play, SceneCmd* cmd) {
    CollisionHeader* colHeaderTemp;
    CollisionHeader* colHeader;

    colHeaderTemp = Lib_SegmentedToVirtual(cmd->colHeader.segment);
    colHeader = colHeaderTemp;
    colHeader->vtxList = Lib_SegmentedToVirtual(colHeaderTemp->vtxList);
    colHeader->polyList = Lib_SegmentedToVirtual(colHeader->polyList);

    if (colHeader->surfaceTypeList != NULL) {
        colHeader->surfaceTypeList = Lib_SegmentedToVirtual(colHeader->surfaceTypeList);
    }

    if (colHeader->bgCamList != NULL) {
        colHeader->bgCamList = Lib_SegmentedToVirtual(colHeader->bgCamList);
    }

    if (colHeader->waterBoxes != NULL) {
        colHeader->waterBoxes = Lib_SegmentedToVirtual(colHeader->waterBoxes);
    }

    play->colCtx.colHeader = colHeader;
    Chaos_RandomizeMountainVillageClimb(play);

    BgCheck_Allocate(&play->colCtx, play, colHeader);
}

// SceneTableEntry Header Command 0x04: Room List
void Scene_CommandRoomList(PlayState* play, SceneCmd* cmd) {
    play->roomList.count = cmd->roomList.num;
    play->roomList.romFiles = Lib_SegmentedToVirtual(cmd->roomList.segment);
}

// SceneTableEntry Header Command 0x06: Entrance List
void Scene_CommandEntranceList(PlayState* play, SceneCmd* cmd) {
    play->setupEntranceList = Lib_SegmentedToVirtual(cmd->entranceList.segment);
}

// SceneTableEntry Header Command 0x07: Special Files
void Scene_CommandSpecialFiles(PlayState* play, SceneCmd* cmd) {
    // @note These quest hint files are identical to OoT's.
    // They are not relevant in this game and the system to process these scripts has been removed.
    static RomFile sNaviQuestHintFiles[2] = {
        ROM_FILE(elf_message_field),
        ROM_FILE(elf_message_ydan),
    };

    if (cmd->specialFiles.subKeepId != 0) {
        play->objectCtx.subKeepSlot = Object_SpawnPersistent(&play->objectCtx, cmd->specialFiles.subKeepId);
        // TODO: Segment number enum?
        gSegments[0x05] = OS_K0_TO_PHYSICAL(play->objectCtx.slots[play->objectCtx.subKeepSlot].segment);
    }

    if (cmd->specialFiles.naviQuestHintFileId != NAVI_QUEST_HINTS_NONE) {
        play->naviQuestHints = Play_LoadFile(play, &sNaviQuestHintFiles[cmd->specialFiles.naviQuestHintFileId - 1]);
    }
}

// SceneTableEntry Header Command 0x08: Room Behavior
void Scene_CommandRoomBehavior(PlayState* play, SceneCmd* cmd) {
    play->roomCtx.curRoom.type = cmd->roomBehavior.gpFlag1;
    play->roomCtx.curRoom.environmentType = cmd->roomBehavior.gpFlag2 & 0xFF;
    play->roomCtx.curRoom.lensMode = (cmd->roomBehavior.gpFlag2 >> 8) & 1;
    play->msgCtx.unk12044 = (cmd->roomBehavior.gpFlag2 >> 0xA) & 1;
    play->roomCtx.curRoom.enablePosLights = (cmd->roomBehavior.gpFlag2 >> 0xB) & 1;
    play->envCtx.stormState = (cmd->roomBehavior.gpFlag2 >> 0xC) & 1;
}

// SceneTableEntry Header Command 0x0A: Mesh Header
void Scene_CommandMesh(PlayState* play, SceneCmd* cmd) {
    play->roomCtx.curRoom.roomShape = Lib_SegmentedToVirtual(cmd->mesh.segment);
}

// SceneTableEntry Header Command 0x0B:  Object List
void Scene_CommandObjectList(PlayState* play, SceneCmd* cmd) {
    // s32 i;
    s32 non_persistent_entry_index = play->objectCtx.numPersistentEntries;
    s32 j;
    // s32 k;
    s32 object_list_entry_index = 0;
    ObjectEntry* object_slots = &play->objectCtx.slots[0];
    ObjectEntry* entry = &play->objectCtx.slots[non_persistent_entry_index];
    ObjectEntry* invalidatedEntry = NULL;
    s16* objectEntry = Lib_SegmentedToVirtual(cmd->objectList.segment);
    void* nextPtr = NULL;

    uintptr_t keep_start = (uintptr_t)play->objectCtx.slots[play->objectCtx.mainKeepSlot].segment;
    uintptr_t keep_end = keep_start + (gObjectTable[GAMEPLAY_KEEP].vromEnd - gObjectTable[GAMEPLAY_KEEP].vromStart);

    // objectEntry = Lib_SegmentedToVirtual(cmd->objectList.segment);
    // object_list_entry_index = 0;
    // non_persistent_entry_index = play->objectCtx.numPersistentEntries;
    // entry = &play->objectCtx.slots[non_persistent_entry_index];
    // object_slots = &play->objectCtx.slots[0];

    /* look for the first non-persistent entry that doesn't match the object list */
    while (non_persistent_entry_index < play->objectCtx.numEntries) {
        if (entry->id != *objectEntry) {
            /* entry doesn't match */
            invalidatedEntry = &play->objectCtx.slots[non_persistent_entry_index];

            /* so nuke everything after it */
            for (j = non_persistent_entry_index; j < play->objectCtx.numEntries; j++) {
                Object_MarkObjectAsUnloaded(&play->objectCtx, invalidatedEntry->id);
                invalidatedEntry->id = 0;
                // invalidatedEntry->request_index = MAX_OBJ_REQUESTS;
                invalidatedEntry++;
            }

            play->objectCtx.numEntries = non_persistent_entry_index;
            Actor_KillAllWithMissingObject(play, &play->actorCtx);

            continue;
        }
        non_persistent_entry_index++;
        object_list_entry_index++;
        objectEntry++;
        entry++;
    }

    while (object_list_entry_index < cmd->objectList.num) 
    {
        if(!Object_IsLoadedById(&play->objectCtx, *objectEntry) || *objectEntry == gChaosContext.loaded_object_id)
        {
            /* if the object isn't loaded, or if it's the same as the object currently loaded for chaos actors. 
            This is to guarantee that a normally spawned actor never uses the slot of a chaos actor */
            nextPtr = Object_RequestOverwrite(&play->objectCtx, non_persistent_entry_index, *objectEntry);

            if (non_persistent_entry_index < ARRAY_COUNT(play->objectCtx.slots) - 1) {
                object_slots[non_persistent_entry_index + 1].segment = nextPtr;
            }

            non_persistent_entry_index++;
        }

        object_list_entry_index++;
        objectEntry++;
    }

    play->objectCtx.numEntries = non_persistent_entry_index;
}

// SceneTableEntry Header Command 0x0C: Light List
void Scene_CommandLightList(PlayState* play, SceneCmd* cmd) {
    s32 i;
    LightInfo* lightInfo = Lib_SegmentedToVirtual(cmd->lightList.segment);

    for (i = 0; i < cmd->lightList.num; i++) {
        LightContext_InsertLight(play, &play->lightCtx, lightInfo);
        lightInfo++;
    }
}

// SceneTableEntry Header Command 0x0D: Path List
void Scene_CommandPathList(PlayState* play, SceneCmd* cmd) {
    play->setupPathList = Lib_SegmentedToVirtual(cmd->pathList.segment);
}

// SceneTableEntry Header Command 0x0E: Transition Actor List
void Scene_CommandTransitionActorList(PlayState* play, SceneCmd* cmd) {
    play->transitionActors.count = cmd->transitionActorList.num;
    play->transitionActors.list = Lib_SegmentedToVirtual(cmd->transitionActorList.segment);
    MapDisp_InitTransitionActorData(play, play->transitionActors.count, play->transitionActors.list);
}

// Init function for the transition system.
void Scene_ResetTransitionActorList(GameState* state, TransitionActorList* transitionActors) {
    transitionActors->count = 0;
}

// SceneTableEntry Header Command 0x0F: Environment Light Settings List
void Scene_CommandEnvLightSettings(PlayState* play, SceneCmd* cmd) {
    play->envCtx.numLightSettings = cmd->lightSettingList.num;
    play->envCtx.lightSettingsList = Lib_SegmentedToVirtual(cmd->lightSettingList.segment);
}

/**
 * Loads different texture files for each region of the world.
 * These later are stored in segment 0x06, and used in maps.
 */
void Scene_LoadAreaTextures(PlayState* play, s32 fileIndex) {
    static RomFile sSceneTextureFiles[9] = {
        ROM_FILE_UNSET, // Default
        ROM_FILE(scene_texture_01),
        ROM_FILE(scene_texture_02),
        ROM_FILE(scene_texture_03),
        ROM_FILE(scene_texture_04),
        ROM_FILE(scene_texture_05),
        ROM_FILE(scene_texture_06),
        ROM_FILE(scene_texture_07),
        ROM_FILE(scene_texture_08),
    };
    uintptr_t vromStart = sSceneTextureFiles[fileIndex].vromStart;
    size_t size = sSceneTextureFiles[fileIndex].vromEnd - vromStart;

    if (size != 0) {
        play->roomCtx.unk74 = THA_AllocTailAlign16(&play->state.tha, size);
        DmaMgr_RequestSync(play->roomCtx.unk74, vromStart, size);
    }
}

// SceneTableEntry Header Command 0x11: Skybox Settings
void Scene_CommandSkyboxSettings(PlayState* play, SceneCmd* cmd) {
    play->skyboxId = cmd->skyboxSettings.skyboxId & 3;
    play->envCtx.skyboxConfig = play->envCtx.changeSkyboxNextConfig = cmd->skyboxSettings.skyboxConfig;
    play->envCtx.lightMode = cmd->skyboxSettings.envLightMode;
    Scene_LoadAreaTextures(play, cmd->skyboxSettings.data1);
}

// SceneTableEntry Header Command 0x12: Skybox Disables
void Scene_CommandSkyboxDisables(PlayState* play, SceneCmd* cmd) {
    play->envCtx.skyboxDisabled = cmd->skyboxDisables.unk4;
    play->envCtx.sunDisabled = cmd->skyboxDisables.unk5;
}

// SceneTableEntry Header Command 0x10: Time Settings
void Scene_CommandTimeSettings(PlayState* play, SceneCmd* cmd) {
    if ((cmd->timeSettings.hour != 0xFF) && (cmd->timeSettings.min != 0xFF)) {
        gSaveContext.skyboxTime = gSaveContext.save.time =
            CLOCK_TIME_ALT2_F(cmd->timeSettings.hour, cmd->timeSettings.min);
    }

    if (cmd->timeSettings.timeSpeed != 0xFF) {
        play->envCtx.sceneTimeSpeed = cmd->timeSettings.timeSpeed;
    } else {
        play->envCtx.sceneTimeSpeed = 0;
    }

    // Increase time speed during first cycle
    if ((gSaveContext.save.saveInfo.inventory.items[SLOT_OCARINA] == ITEM_NONE) && (play->envCtx.sceneTimeSpeed != 0)) {
        play->envCtx.sceneTimeSpeed = 5;
    }

    if (gSaveContext.sunsSongState == SUNSSONG_INACTIVE) {
        R_TIME_SPEED = play->envCtx.sceneTimeSpeed;
    }

    play->envCtx.sunPos.x = -(Math_SinS(CURRENT_TIME - CLOCK_TIME(12, 0)) * 120.0f) * 25.0f;
    play->envCtx.sunPos.y = (Math_CosS(CURRENT_TIME - CLOCK_TIME(12, 0)) * 120.0f) * 25.0f;
    play->envCtx.sunPos.z = (Math_CosS(CURRENT_TIME - CLOCK_TIME(12, 0)) * 20.0f) * 25.0f;

    if ((play->envCtx.sceneTimeSpeed == 0) && (gSaveContext.save.cutsceneIndex < 0xFFF0)) {
        gSaveContext.skyboxTime = CURRENT_TIME;

        if ((gSaveContext.skyboxTime >= CLOCK_TIME(4, 0)) && (gSaveContext.skyboxTime < CLOCK_TIME(6, 30))) {
            gSaveContext.skyboxTime = CLOCK_TIME(5, 0);
        } else if ((gSaveContext.skyboxTime >= CLOCK_TIME(6, 30)) && (gSaveContext.skyboxTime < CLOCK_TIME(8, 0))) {
            gSaveContext.skyboxTime = CLOCK_TIME(8, 0);
        } else if ((gSaveContext.skyboxTime >= CLOCK_TIME(16, 0)) && (gSaveContext.skyboxTime < CLOCK_TIME(17, 0))) {
            gSaveContext.skyboxTime = CLOCK_TIME(17, 0);
        } else if ((gSaveContext.skyboxTime >= CLOCK_TIME(18, 0)) && (gSaveContext.skyboxTime < CLOCK_TIME(19, 0))) {
            gSaveContext.skyboxTime = CLOCK_TIME(19, 0);
        }
    }
}

// SceneTableEntry Header Command 0x05: Wind Settings
void Scene_CommandWindSettings(PlayState* play, SceneCmd* cmd) {
    s8 temp1 = cmd->windSettings.west;
    s8 temp2 = cmd->windSettings.vertical;
    s8 temp3 = cmd->windSettings.south;

    play->envCtx.windDirection.x = temp1;
    play->envCtx.windDirection.y = temp2;
    play->envCtx.windDirection.z = temp3;;
    play->envCtx.windSpeed = cmd->windSettings.clothIntensity;
}

// SceneTableEntry Header Command 0x13: Exit List
void Scene_CommandExitList(PlayState* play, SceneCmd* cmd) {
    play->setupExitList = Lib_SegmentedToVirtual(cmd->exitList.segment);
}

// SceneTableEntry Header Command 0x09: Undefined
void Scene_Command09(PlayState* play, SceneCmd* cmd) {
}

// SceneTableEntry Header Command 0x15: Sound Settings
void Scene_CommandSoundSettings(PlayState* play, SceneCmd* cmd) {
    play->sceneSequences.seqId = cmd->soundSettings.seqId;
    play->sceneSequences.ambienceId = cmd->soundSettings.ambienceId;

    if (gSaveContext.seqId == (u8)NA_BGM_DISABLED ||
        AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) == NA_BGM_FINAL_HOURS) {
        Audio_SetSpec(cmd->soundSettings.specId);
    }
}

// SceneTableEntry Header Command 0x16: Echo Setting
void Scene_CommandEchoSetting(PlayState* play, SceneCmd* cmd) {
    play->roomCtx.curRoom.echo = cmd->echoSettings.echo;
}

// SceneTableEntry Header Command 0x18: Alternate Header List
void Scene_CommandAltHeaderList(PlayState* play, SceneCmd* cmd) {
    SceneCmd** altHeaderList;
    SceneCmd* altHeader;

    if (gSaveContext.sceneLayer != 0) {
        altHeaderList = Lib_SegmentedToVirtual(cmd->altHeaders.segment);
        altHeader = altHeaderList[gSaveContext.sceneLayer - 1];

        if (altHeader != NULL) {
            Scene_ExecuteCommands(play, Lib_SegmentedToVirtual(altHeader));
            (cmd + 1)->base.code = 0x14;
        }
    }
}

// SceneTableEntry Header Command 0x17: Cutscene Script List
void Scene_CommandCutsceneScriptList(PlayState* play, SceneCmd* cmd) {
    play->csCtx.scriptListCount = cmd->scriptList.scriptListCount;
    play->csCtx.scriptList = Lib_SegmentedToVirtual(cmd->scriptList.segment);
}

// SceneTableEntry Header Command 0x1B: Cutscene List
void Scene_CommandCutsceneList(PlayState* play, SceneCmd* cmd) {
    CutsceneManager_Init(play, Lib_SegmentedToVirtual(cmd->cutsceneList.segment), cmd->cutsceneList.num);
}

// SceneTableEntry Header Command 0x1C: Map Data
void Scene_CommandMapData(PlayState* play, SceneCmd* cmd) {
    MapDisp_Init(play);
    MapDisp_InitMapData(play, cmd->mapData.segment);
}

// SceneTableEntry Header Command 0x1D: Undefined
void Scene_Command1D(PlayState* play, SceneCmd* cmd) {
}

// SceneTableEntry Header Command 0x1E: Map Data Chests
void Scene_CommandMapDataChests(PlayState* play, SceneCmd* cmd) {
    MapDisp_InitChestData(play, cmd->mapDataChests.num, cmd->mapDataChests.segment);
}

void Scene_CommandSetRoomVerts(PlayState *play, SceneCmd *cmd) {
    gChaosContext.room.vert_list_list[play->roomCtx.activeBufPage] = cmd->room_vert_list_list.segment;
}

// SceneTableEntry Header Command 0x19: Sets Region Visited Flag
void Scene_CommandSetRegionVisitedFlag(PlayState* play, SceneCmd* cmd) {
    s16 j = 0;
    s16 i = 0;

    while (true) {
        if (gSceneIdsPerRegion[i][j] == 0xFFFF) {
            i++;
            j = 0;

            if (i == REGION_MAX) {
                break;
            }
        }

        if (play->sceneId == gSceneIdsPerRegion[i][j]) {
            break;
        }

        j++;
    }

    if (i < REGION_MAX) {
        gSaveContext.save.saveInfo.regionsVisited =
            (gBitFlags[i] | gSaveContext.save.saveInfo.regionsVisited) | gSaveContext.save.saveInfo.regionsVisited;
    }
}

// SceneTableEntry Header Command 0x1A: Material Animations
void Scene_CommandAnimatedMaterials(PlayState* play, SceneCmd* cmd) {
    play->sceneMaterialAnims = Lib_SegmentedToVirtual(cmd->textureAnimations.segment);
}

/**
 * Sets the exit fade from the next entrance index.
 */
void Scene_SetExitFade(PlayState* play) {
    play->transitionType = Entrance_GetTransitionFlags(play->nextEntrance) & 0x7F;
}

void (*sSceneCmdHandlers[SCENE_CMD_MAX])(PlayState*, SceneCmd*) = {
    Scene_CommandSpawnList,            // SCENE_CMD_ID_SPAWN_LIST
    Scene_CommandActorList,            // SCENE_CMD_ID_ACTOR_LIST
    Scene_CommandActorCutsceneCamList, // SCENE_CMD_ID_ACTOR_CUTSCENE_CAM_LIST
    Scene_CommandCollisionHeader,      // SCENE_CMD_ID_COL_HEADER
    Scene_CommandRoomList,             // SCENE_CMD_ID_ROOM_LIST
    Scene_CommandWindSettings,         // SCENE_CMD_ID_WIND_SETTINGS
    Scene_CommandEntranceList,         // SCENE_CMD_ID_ENTRANCE_LIST
    Scene_CommandSpecialFiles,         // SCENE_CMD_ID_SPECIAL_FILES
    Scene_CommandRoomBehavior,         // SCENE_CMD_ID_ROOM_BEHAVIOR
    Scene_Command09,                   // SCENE_CMD_ID_UNK_09
    Scene_CommandMesh,                 // SCENE_CMD_ID_ROOM_SHAPE
    Scene_CommandObjectList,           // SCENE_CMD_ID_OBJECT_LIST
    Scene_CommandLightList,            // SCENE_CMD_ID_LIGHT_LIST
    Scene_CommandPathList,             // SCENE_CMD_ID_PATH_LIST
    Scene_CommandTransitionActorList,  // SCENE_CMD_ID_TRANSI_ACTOR_LIST
    Scene_CommandEnvLightSettings,     // SCENE_CMD_ID_ENV_LIGHT_SETTINGS
    Scene_CommandTimeSettings,         // SCENE_CMD_ID_TIME_SETTINGS
    Scene_CommandSkyboxSettings,       // SCENE_CMD_ID_SKYBOX_SETTINGS
    Scene_CommandSkyboxDisables,       // SCENE_CMD_ID_SKYBOX_DISABLES
    Scene_CommandExitList,             // SCENE_CMD_ID_EXIT_LIST
    NULL,                              // SCENE_CMD_ID_END
    Scene_CommandSoundSettings,        // SCENE_CMD_ID_SOUND_SETTINGS
    Scene_CommandEchoSetting,          // SCENE_CMD_ID_ECHO_SETTINGS
    Scene_CommandCutsceneScriptList,   // SCENE_CMD_ID_CUTSCENE_SCRIPT_LIST
    Scene_CommandAltHeaderList,        // SCENE_CMD_ID_ALTERNATE_HEADER_LIST
    Scene_CommandSetRegionVisitedFlag, // SCENE_CMD_ID_SET_REGION_VISITED
    Scene_CommandAnimatedMaterials,    // SCENE_CMD_ID_ANIMATED_MATERIAL_LIST
    Scene_CommandCutsceneList,         // SCENE_CMD_ID_ACTOR_CUTSCENE_LIST
    Scene_CommandMapData,              // SCENE_CMD_ID_MAP_DATA
    Scene_Command1D,                   // SCENE_CMD_ID_UNUSED_1D
    Scene_CommandMapDataChests,        // SCENE_CMD_ID_MAP_DATA_CHESTS
    Scene_CommandSetRoomVerts,         // SCENE_CMD_ID_SET_ROOM_VERTS
};

/**
 * Executes all of the commands in a scene or room header.
 */
s32 Scene_ExecuteCommands(PlayState* play, SceneCmd* sceneCmd) {
    u32 cmdId;

    
    Chaos_ClearActors();
    
    while (true) {
        cmdId = sceneCmd->base.code;
        
        if (cmdId == SCENE_CMD_ID_END) {
            break;
        }
        
        if (cmdId < SCENE_CMD_MAX) {
            sSceneCmdHandlers[cmdId](play, sceneCmd);
        }
        
        sceneCmd++;
    }

    return 0;
}

/**
 * Creates an entrance from the scene, spawn, and layer.
 */
u16 Entrance_Create(s32 scene, s32 spawn, s32 layer) {
    return (scene << 9) | (spawn << 4) | layer;
}

/**
 * Creates an layer 0 entrance from the current entrance and the given spawn.
 */
u16 Entrance_CreateFromSpawn(s32 spawn) {
    return Entrance_Create((u32)gSaveContext.save.entrance >> 9, spawn, 0);
}
