#ifndef Z64OBJECT_H
#define Z64OBJECT_H

#include "stdint.h"
#include "romfile.h"

struct GameState;

#define USE_NEW_METHOD

typedef struct {
    #ifdef USE_NEW_METHOD
    /* 0x00 */ s16          id; // Negative ids mean that the object is unloaded
               u16          load_pending;
    /* 0x04 */ void*        segment;
               uintptr_t    vrom_addr;
    #else
    /* 0x00 */ s16 id; // Negative ids mean that the object is unloaded
    /* 0x02 */ UNK_TYPE1 pad2[0x2];
    /* 0x04 */ void* segment;
    /* 0x08 */ DmaRequest dmaReq;
    /* 0x28 */ OSMesgQueue loadQueue;
    /* 0x40 */ OSMesg loadMsg;
    #endif
} ObjectEntry; // size = 0x44

struct ObjectLoadRequest
{
    DmaRequest  dma_req;
    OSMesgQueue load_queue;
    OSMesg      load_msg;
    s16         slot_index;
}; // size = 0x40

#define OBJECT_SLOT_NONE -1

#define MAX_OBJECT_REQUESTS 16
typedef struct {
    /* 0x000 */ void*           spaceStart;
    /* 0x004 */ void*           spaceEnd;
    /* 0x008 */ u8              numEntries; // total amount of used entries
    /* 0x009 */ u8              numPersistentEntries; // amount of entries that won't be reused when loading a new object list (when loading a new room)
    /* 0x00A */ u8              mainKeepSlot; // "gameplay_keep" slot
    /* 0x00B */ u8              subKeepSlot; // "gameplay_field_keep" or "gameplay_dangeon_keep" slot

    #ifdef USE_NEW_METHOD
                u8              pending_request_count;
                u8              next_available_request;
                u8              pad0[2];
    /* 0x010 */ ObjectEntry     slots[64];    
    /* 0x310 */ struct ObjectLoadRequest    load_requests[MAX_OBJECT_REQUESTS];
    // /* 0x510 */ u8 pad1[0x448];
    #else
    /* 0x010 */ ObjectEntry     slots[35];    
    #endif
} ObjectContext; // size = 0x958 

#define DEFINE_OBJECT(_name, enumValue) enumValue,
#define DEFINE_OBJECT_UNSET(enumValue) enumValue,
#define DEFINE_OBJECT_EMPTY(_name, enumValue) enumValue,

typedef enum ObjectId {
    #include "tables/object_table.h"
    /* 0x283 */ OBJECT_ID_MAX
} ObjectId;

#undef DEFINE_OBJECT
#undef DEFINE_OBJECT_UNSET
#undef DEFINE_OBJECT_EMPTY

s32 Object_SpawnPersistent(ObjectContext* objectCtx, s16 id);
void Object_InitContext(struct GameState* gameState, ObjectContext* objectCtx);
void Object_UpdateEntries(ObjectContext* objectCtx);
s32 Object_GetSlot(ObjectContext* objectCtx, s16 objectId);
s32 Object_IsLoaded(ObjectContext* objectCtx, s32 slot);
s32 Object_GetPersistentSlot(ObjectContext* objectCtx, s16 objectId);
s32 Object_GetNonPersistentSlot(ObjectContext* objectCtx, s16 objectId);
void Object_MarkObjectAsLoaded(ObjectContext *ctx, s16 id);
void Object_MarkObjectAsUnloaded(ObjectContext *ctx, s16 id);
s32 Object_IsLoadedById(ObjectContext *objectCtx, s16 id);
void Object_LoadAll(ObjectContext* objectCtx);
void* Object_RequestOverwrite(ObjectContext* objectCtx, s32 slot, s16 id);

extern ObjectId gObjectTableSize;
extern RomFile gObjectTable[OBJECT_ID_MAX];

#endif
