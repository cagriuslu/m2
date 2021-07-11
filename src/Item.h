#ifndef ITEM_H
#define ITEM_H

#include "Error.h"
#include "Array.h"

typedef enum _ItemType {
	ITEMTYP_NONE = 0,
	ITEMTYP_GUN,
	ITEMTYP_RIFLE,
	ITEMTYP_CROSSBOW,
	ITEMTYP_BOW,
	ITEMTYP_SWORD,
	ITEMTYP_SPEAR,
	ITEMTYP_PANTS,
	ITEMTYP_SHIRT,
	ITEMTYP_AMULET,
	ITEMTYP_SHINYBELT,
	ITEMTYP_LEFTRING,
	ITEMTYP_RIGHTRING
} ItemType;

typedef enum _ItemFlags {
	ITEMFLAG_EQUIPABLE = 0x01,
	ITEMFLAG_EQUIPPED = 0x02,
} ItemFlags;

typedef struct _Item {
	ItemType type;
	int variant;
	int flags;
	// OFFENSE
	unsigned offenseHitPoints;
	// DEFENSE
	unsigned defenseHitPoints;
} Item;

XErr Item_Init(Item* item, ItemType type, int variant);
void Item_Term(Item* item);

XErr Item_GenerateStandardItemSet(Array* arrayOfItems);

#endif
