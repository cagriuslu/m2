#ifndef ITEM_H
#define ITEM_H

#include "Error.h"
#include "Array.h"

typedef enum _ItemType {
	ITEMTYP_NONE = 0,
	ITEMTYP_GUN = 0x0001,
	ITEMTYP_RIFLE = 0x0002,
	ITEMTYP_BOW = 0x0004,
	ITEMTYP_SWORD = 0x0008,
	ITEMTYP_SPEAR = 0x0010,
	ITEMTYP_DAGGER = 0x0020,
	ITEMTYP_PANTS = 0x0040,
	ITEMTYP_SHIRT = 0x0080,
	ITEMTYP_AMULET = 0x0100,
	ITEMTYP_SHINYBELT = 0x0200,
	ITEMTYP_LEFTRING = 0x0400,
	ITEMTYP_RIGHTRING = 0x0800,
} ItemType;

typedef enum _ItemFlags {
	ITEMFLAG_EQUIPABLE = 0x01,
	ITEMFLAG_EQUIPPED = 0x02,
	ITEMFLAG_PREEQUIPPED_NEXT = 0x04,
	ITEMFLAG_PREEQUIPPED_PREV = 0x08,
} ItemFlags;

typedef struct _Item {
	ItemType type;
	int flags;
	// OFFENSE
	unsigned offenseHitPoints;
	unsigned offenseTTL;
	// DEFENSE
	unsigned defenseHitPoints;
} Item;

XErr Item_Init(Item* item, ItemType type);
void Item_Term(Item* item);

XErr Item_GenerateStandardItemSet(Array* arrayOfItems);
Item* Item_FindItemByTypeByFlags(Array* arrayOfItems, ItemType type, int flags);

#endif
