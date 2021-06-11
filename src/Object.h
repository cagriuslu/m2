#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>

typedef struct _Object {
	uint32_t type;
} Object;

// TODO get rid of these and switch to ECS

// Definitions

#define OBJTYP_OBJECT (0x01)
#define OBJTYP_TILE   (0x02)

#define OBJECT_PLAYER       ((0x000 << 8)  | OBJTYP_OBJECT)
#define OBJECT_PLAYER_BASIC ((0x000 << 20) | OBJECT_PLAYER)
#define OBJECT_PLAYER_GOD   ((0x001 << 20) | OBJECT_PLAYER)

#define OBJECT_BULLET       ((0x001 << 8)  | OBJTYP_OBJECT)
#define OBJECT_BULLET_BASIC ((0x000 << 20) | OBJECT_BULLET)

#define OBJECT_ENEMY          ((0x002 << 8)  | OBJTYP_OBJECT)
#define OBJECT_ENEMY_BASIC    ((0x000 << 20) | OBJECT_ENEMY)
#define OBJECT_ENEMY_SKELETON ((0x001 << 20) | OBJECT_ENEMY)

#define OBJECT_ENEMYBULLET ((0x003 << 8)  | OBJTYP_OBJECT)

// Tests

#define IS_OBJECT(objTyp) (((objTyp) & 0xFF) == OBJTYP_OBJECT)
#define IS_TILE(objTyp)   (((objTyp) & 0xFF) == OBJTYP_TILE)

#define IS_PLAYER(objTyp)        (((objTyp) & 0xFFFFF) == OBJECT_PLAYER)
#define IS_PLAYER_BASIC(objType) (((objTyp) & 0xFFFFFFFF) == OBJECT_PLAYER_BASIC)
#define IS_PLAYER_GOD(objType)   (((objTyp) & 0xFFFFFFFF) == OBJECT_PLAYER_GOD)

#define IS_BULLET(objType)       (((objTyp) & 0xFFFFF) == OBJECT_BULLET)
#define IS_BULLET_BASIC(objType) (((objTyp) & 0xFFFFFFFF) == OBJECT_BULLET_BASIC)

#define IS_ENEMY(objTyp)           (((objTyp) & 0xFFFFF) == OBJECT_ENEMY)
#define IS_ENEMY_BASIC(objType)    (((objTyp) & 0xFFFFFFFF) == OBJECT_ENEMY_BASIC)
#define IS_ENEMY_SKELETON(objType) (((objTyp) & 0xFFFFFFFF) == OBJECT_ENEMY_SKELETON)

#endif
