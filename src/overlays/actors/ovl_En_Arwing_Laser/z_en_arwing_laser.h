#ifndef Z_EN_ARWING_LASER_H
#define Z_EN_ARWING_LASER_H

#include "ultra64.h"
#include "global.h"
#include "assets/objects/object_arwing/object_arwing.h"

typedef struct EnArwingLaser
{
    Actor               actor;
    ColliderCylinder    collider;
    u8                  life;

}EnArwingLaser;

#endif