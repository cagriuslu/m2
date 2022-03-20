#ifndef BOX2D_UTILS_H
#define BOX2D_UTILS_H

#include "Pool-old.hh"
#include "m2/Box2D.hh"

#define CATEGORY_STATIC_OBJECT (0x0001)
#define CATEGORY_STATIC_CLIFF (0x0002)
#define CATEGORY_PLAYER (0x0004)
#define CATEGORY_PLAYER_BULLET (0x0008)
#define CATEGORY_PLAYER_MELEE_WEAPON (0x0010)
#define CATEGORY_ENEMY (0x0020)
#define CATEGORY_ENEMY_BULLET (0x0040)
#define CATEGORY_ENEMY_MELEE_WEAPON (0x0080)

#define Box2DUtils_CreateStaticBox(phyId, position, category, dims)                           Box2DUtils_CreateBody((phyId), /* isDisk */ false, /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (dims),      /* boxCenterOffset */ m2::vec2f{}, /* boxAngle */ 0.0f, /* diskRadius */ 0.0f,     /* mass */ 0.0f,   /* lineaDamping */ 0.0f,      /* fixedRotation */ true)
#define Box2DUtils_CreateStaticDisk(phyId, position, category, radius)                        Box2DUtils_CreateBody((phyId), /* isDisk */ true,  /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ m2::vec2f{}, /* boxCenterOffset */ m2::vec2f{}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ 0.0f,   /* lineaDamping */ 0.0f,      /* fixedRotation */ true)
#define Box2DUtils_CreateDynamicBox(phyId, position, sleep, category, dims, mass, damping)    Box2DUtils_CreateBody((phyId), /* isDisk */ false, /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (dims),      /* boxCenterOffset */ m2::vec2f{}, /* boxAngle */ 0.0f, /* diskRadius */ 0.0f,     /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)
#define Box2DUtils_CreateDynamicDisk(phyId, position, sleep, category, radius, mass, damping) Box2DUtils_CreateBody((phyId), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ m2::vec2f{}, /* boxCenterOffset */ m2::vec2f{}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)
#define Box2DUtils_CreateBullet(phyId, position, category, radius, mass, damping)             Box2DUtils_CreateBody((phyId), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ false,   /* isBullet */ true,  /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ m2::vec2f{}, /* boxCenterOffset */ m2::vec2f{}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)

#define Box2DUtils_CreateStaticBoxSensor(phyId, position, category, dims)                           Box2DUtils_CreateBody((phyId), /* isDisk */ false, /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (dims),      /* boxCenterOffset */ m2::vec2f{}, /* boxAngle */ 0.0f, /* diskRadius */ 0.0f,     /* mass */ 0.0f,   /* lineaDamping */ 0.0f,      /* fixedRotation */ true)
#define Box2DUtils_CreateStaticDiskSensor(phyId, position, category, radius)                        Box2DUtils_CreateBody((phyId), /* isDisk */ true,  /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ m2::vec2f{}, /* boxCenterOffset */ m2::vec2f{}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ 0.0f,   /* lineaDamping */ 0.0f,      /* fixedRotation */ true)
#define Box2DUtils_CreateDynamicBoxSensor(phyId, position, sleep, category, dims, mass, damping)    Box2DUtils_CreateBody((phyId), /* isDisk */ false, /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (dims),      /* boxCenterOffset */ m2::vec2f{}, /* boxAngle */ 0.0f, /* diskRadius */ 0.0f,     /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)
#define Box2DUtils_CreateDynamicDiskSensor(phyId, position, sleep, category, radius, mass, damping) Box2DUtils_CreateBody((phyId), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ m2::vec2f{}, /* boxCenterOffset */ m2::vec2f{}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)
#define Box2DUtils_CreateBulletSensor(phyId, position, category, radius, mass, damping)             Box2DUtils_CreateBody((phyId), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ false,   /* isBullet */ true,  /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ m2::vec2f{}, /* boxCenterOffset */ m2::vec2f{}, /* boxAngle */ 0.0f, /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping), /* fixedRotation */ true)

Box2DBody* Box2DUtils_CreateBody(ID phyId, bool isDisk, bool isDynamic, m2::vec2f position, bool allowSleep, bool isBullet, bool isSensor, uint16_t categoryBits, uint16_t maskBits, m2::vec2f boxDims, m2::vec2f boxCenterOffset, float boxAngle, float diskRadius, float mass, float linearDamping, bool fixedRotation);

bool Box2DUtils_CheckEyeSight(m2::vec2f from, m2::vec2f to, uint16_t categoryMask);

#endif
