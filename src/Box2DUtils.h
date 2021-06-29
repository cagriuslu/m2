#ifndef BOX2D_UTILS_H
#define BOX2D_UTILS_H

#include "Box2DWrapper.h"

#define ALLOW_SLEEP (true)
#define DONT_SLEEP (false)

#define STATIC_OBJECT_CATEGORY (0x0001)
#define STATIC_CLIFF_CATEGORY (0x0002)
#define PLAYER_CATEGORY (0x0004)
#define PLAYER_BULLET_CATEGORY (0x0008)
#define PLAYER_MELEE_WEAPON_CATEGORY (0x0010)
#define ENEMY_CATEGORY (0x0020)
#define ENEMY_BULLET_CATEGORY (0x0040)
#define ENEMY_MELEE_WEAPON_CATEGORY (0x0080)

#define Box2DUtilsCreateStaticBox(phyId, position, category, dims)                           Box2DUtilsCreateBody((phyId), /* isDisk */ false, /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (dims),              /* boxCenterOffset */ (Vec2F){0.0f, 0.0f}, /* boxAngle */ 0.0f, /* diskRadius */ 0.0f,     /* mass */ 0.0f,   /* lineaDamping */ 0.0f,      /* fixedRotation */ true)
#define Box2DUtilsCreateStaticDisk(phyId, position, category, radius)                        Box2DUtilsCreateBody((phyId), /* isDisk */ true,  /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f}, /* boxCenterOffset */ (Vec2F){0.0f, 0.0f}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ 0.0f,   /* lineaDamping */ 0.0f,      /* fixedRotation */ true)
#define Box2DUtilsCreateDynamicBox(phyId, position, sleep, category, dims, mass, damping)    Box2DUtilsCreateBody((phyId), /* isDisk */ false, /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (dims),              /* boxCenterOffset */ (Vec2F){0.0f, 0.0f}, /* boxAngle */ 0.0f, /* diskRadius */ 0.0f,     /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)
#define Box2DUtilsCreateDynamicDisk(phyId, position, sleep, category, radius, mass, damping) Box2DUtilsCreateBody((phyId), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f}, /* boxCenterOffset */ (Vec2F){0.0f, 0.0f}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)
#define Box2DUtilsCreateBullet(phyId, position, category, radius, mass, damping)             Box2DUtilsCreateBody((phyId), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ false,   /* isBullet */ true,  /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f}, /* boxCenterOffset */ (Vec2F){0.0f, 0.0f}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)

#define Box2DUtilsCreateStaticBoxSensor(phyId, position, category, dims)                           Box2DUtilsCreateBody((phyId), /* isDisk */ false, /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (dims),              /* boxCenterOffset */ (Vec2F){0.0f, 0.0f}, /* boxAngle */ 0.0f, /* diskRadius */ 0.0f,     /* mass */ 0.0f,   /* lineaDamping */ 0.0f,      /* fixedRotation */ true)
#define Box2DUtilsCreateStaticDiskSensor(phyId, position, category, radius)                        Box2DUtilsCreateBody((phyId), /* isDisk */ true,  /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f}, /* boxCenterOffset */ (Vec2F){0.0f, 0.0f}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ 0.0f,   /* lineaDamping */ 0.0f,      /* fixedRotation */ true)
#define Box2DUtilsCreateDynamicBoxSensor(phyId, position, sleep, category, dims, mass, damping)    Box2DUtilsCreateBody((phyId), /* isDisk */ false, /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (dims),              /* boxCenterOffset */ (Vec2F){0.0f, 0.0f}, /* boxAngle */ 0.0f, /* diskRadius */ 0.0f,     /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)
#define Box2DUtilsCreateDynamicDiskSensor(phyId, position, sleep, category, radius, mass, damping) Box2DUtilsCreateBody((phyId), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f}, /* boxCenterOffset */ (Vec2F){0.0f, 0.0f}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)
#define Box2DUtilsCreateBulletSensor(phyId, position, category, radius, mass, damping)             Box2DUtilsCreateBody((phyId), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ false,   /* isBullet */ true,  /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f}, /* boxCenterOffset */ (Vec2F){0.0f, 0.0f}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)

Box2DBody* Box2DUtilsCreateBody(uint64_t phyId, bool isDisk, bool isDynamic, Vec2F position, bool allowSleep, bool isBullet, bool isSensor, uint16_t categoryBits, uint16_t maskBits, Vec2F boxDims, Vec2F boxCenterOffset, float boxAngle, float diskRadius, float mass, float linearDamping, bool fixedRotation);

bool Box2DUtilsCheckEyeSight(Vec2F from, Vec2F to, uint16_t categoryMask);
bool Box2DUtilsCheckEyeSightWithWidth(Vec2F from, Vec2F to, uint16_t categoryMask, float shoulderWidth);

#endif
