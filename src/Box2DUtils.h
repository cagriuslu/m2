#ifndef BOX2D_UTILS_H
#define BOX2D_UTILS_H

#include "Box2DWrapper.h"
#include "Object.h"

#define ALLOW_SLEEP (true)
#define DONT_SLEEP (false)

#define STATIC_CATEGORY (0x0001)
#define PLAYER_CATEGORY (0x0002)
#define PLAYER_BULLET_CATEGORY (0x0004)
#define ENEMY_CATEGORY (0x0008)
#define ENEMY_BULLET_CATEGORY (0x0010)

#define Box2DUtilsCreateStaticBox(obj, position, category, dims)                           Box2DUtilsCreateBody((obj), /* isDisk */ false, /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (dims),                /* diskRadius */ 0.0f,     /* mass */ 0.0f,   /* lineaDamping */ 0.0f)
#define Box2DUtilsCreateStaticDisk(obj, position, category, radius)                        Box2DUtilsCreateBody((obj), /* isDisk */ true,  /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f},   /* diskRadius */ (radius), /* mass */ 0.0f,   /* lineaDamping */ 0.0f)
#define Box2DUtilsCreateDynamicBox(obj, position, sleep, category, dims, mass, damping)    Box2DUtilsCreateBody((obj), /* isDisk */ false, /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (dims),                /* diskRadius */ 0.0f,     /* mass */ (mass), /* lineaDamping */ (damping))
#define Box2DUtilsCreateDynamicDisk(obj, position, sleep, category, radius, mass, damping) Box2DUtilsCreateBody((obj), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f},   /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping))
#define Box2DUtilsCreateBullet(obj, position, category, radius, mass, damping)             Box2DUtilsCreateBody((obj), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ false,   /* isBullet */ true,  /* isSensor */ false, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f},   /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping))

#define Box2DUtilsCreateStaticBoxSensor(obj, position, category, dims)                           Box2DUtilsCreateBody((obj), /* isDisk */ false, /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (dims),                /* diskRadius */ 0.0f,     /* mass */ 0.0f,   /* lineaDamping */ 0.0f)
#define Box2DUtilsCreateStaticDiskSensor(obj, position, category, radius)                        Box2DUtilsCreateBody((obj), /* isDisk */ true,  /* isDynamic */ false, (position), /* allowSleep */ true,    /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f},   /* diskRadius */ (radius), /* mass */ 0.0f,   /* lineaDamping */ 0.0f)
#define Box2DUtilsCreateDynamicBoxSensor(obj, position, sleep, category, dims, mass, damping)    Box2DUtilsCreateBody((obj), /* isDisk */ false, /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (dims),                /* diskRadius */ 0.0f,     /* mass */ (mass), /* lineaDamping */ (damping))
#define Box2DUtilsCreateDynamicDiskSensor(obj, position, sleep, category, radius, mass, damping) Box2DUtilsCreateBody((obj), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ (sleep), /* isBullet */ false, /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f},   /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping))
#define Box2DUtilsCreateBulletSensor(obj, position, category, radius, mass, damping)             Box2DUtilsCreateBody((obj), /* isDisk */ true,  /* isDynamic */ true,  (position), /* allowSleep */ false,   /* isBullet */ true,  /* isSensor */ true, (category), /* mask */ 0, /* boxDims */ (Vec2F) {0.0f,0.0f},   /* diskRadius */ (radius), /* mass */ (mass), /* lineaDamping */ (damping))

Box2DBody* Box2DUtilsCreateBody(void* obj, bool isDisk, bool isDynamic, Vec2F position, bool allowSleep, bool isBullet, bool isSensor, uint16_t categoryBits, uint16_t maskBits, Vec2F boxDims, float diskRadius, float mass, float linearDamping);

#endif
