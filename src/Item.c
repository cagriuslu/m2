#include "Item.h"
#include <string.h>

XErr Item_Init(Item* item, ItemType type, int variant) {
	memset(item, 0, sizeof(Item));
	item->type = type;
	item->variant = variant;
	switch (type) {
	case ITEMTYP_GUN:
	case ITEMTYP_RIFLE:
	case ITEMTYP_CROSSBOW:
	case ITEMTYP_BOW:
	case ITEMTYP_SWORD:
	case ITEMTYP_SPEAR:
	case ITEMTYP_PANTS:
	case ITEMTYP_SHIRT:
	case ITEMTYP_AMULET:
	case ITEMTYP_SHINYBELT:
	case ITEMTYP_LEFTRING:
	case ITEMTYP_RIGHTRING:
		item->flags |= ITEMFLAG_EQUIPABLE;
		break;
	}
	return 0;
}

void Item_Term(Item* item) {
	memset(item, 0, sizeof(Item));
}
