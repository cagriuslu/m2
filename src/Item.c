#include "Item.h"
#include <string.h>

XErr Item_Init(Item* item, ItemType type) {
	memset(item, 0, sizeof(Item));
	item->type = type;
	switch (type) {
	case ITEMTYP_GUN:
	case ITEMTYP_RIFLE:
	case ITEMTYP_BOW:
	case ITEMTYP_SWORD:
	case ITEMTYP_SPEAR:
	case ITEMTYP_DAGGER:
	case ITEMTYP_PANTS:
	case ITEMTYP_SHIRT:
	case ITEMTYP_AMULET:
	case ITEMTYP_SHINYBELT:
	case ITEMTYP_LEFTRING:
	case ITEMTYP_RIGHTRING:
		item->flags |= ITEMFLAG_EQUIPABLE;
		break;
	default:
		break;
	}
	return 0;
}

void Item_Term(Item* item) {
	memset(item, 0, sizeof(Item));
}

XErr Item_GenerateStandardItemSet(Array* arrayOfItems) {
	Array_Clear(arrayOfItems);

	Item* stockGun = Array_Append(arrayOfItems, NULL);
	stockGun->type = ITEMTYP_GUN;
	stockGun->flags = ITEMFLAG_EQUIPABLE | ITEMFLAG_EQUIPPED;
	stockGun->offenseHitPoints = 15;
	stockGun->offenseTTL = 1250;
	stockGun->defenseHitPoints = 0;

	Item* stockRifle = Array_Append(arrayOfItems, NULL);
	stockRifle->type = ITEMTYP_RIFLE;
	stockRifle->flags = ITEMFLAG_EQUIPABLE | ITEMFLAG_PREEQUIPPED_NEXT;
	stockRifle->offenseHitPoints = 45;
	stockRifle->offenseTTL = 2000;
	stockRifle->defenseHitPoints = 0;

	Item* stockBow = Array_Append(arrayOfItems, NULL);
	stockBow->type = ITEMTYP_BOW;
	stockBow->flags = ITEMFLAG_EQUIPABLE | ITEMFLAG_PREEQUIPPED_PREV;
	stockBow->offenseHitPoints = 35;
	stockBow->offenseTTL = 1500;
	stockBow->defenseHitPoints = 0;

	Item* stockSword = Array_Append(arrayOfItems, NULL);
	stockSword->type = ITEMTYP_SWORD;
	stockSword->flags = ITEMFLAG_EQUIPABLE | ITEMFLAG_EQUIPPED;
	stockSword->offenseHitPoints = 45;
	stockSword->offenseTTL = 150;
	stockSword->defenseHitPoints = 0;

	Item* stockSpear = Array_Append(arrayOfItems, NULL);
	stockSpear->type = ITEMTYP_SPEAR;
	stockSpear->flags = ITEMFLAG_EQUIPABLE | ITEMFLAG_PREEQUIPPED_NEXT;
	stockSpear->offenseHitPoints = 85;
	stockSpear->offenseTTL = 250;
	stockSpear->defenseHitPoints = 0;

	Item* stockDagger = Array_Append(arrayOfItems, NULL);
	stockDagger->type = ITEMTYP_DAGGER;
	stockDagger->flags = ITEMFLAG_EQUIPABLE | ITEMFLAG_PREEQUIPPED_PREV;
	stockDagger->offenseHitPoints = 25;
	stockDagger->offenseTTL = 100;
	stockDagger->defenseHitPoints = 0;

	return 0;
}

Item* Item_FindItemByTypeByFlags(Array* arrayOfItems, ItemType type, int flags) {
	for (size_t i = 0; i < Array_Length(arrayOfItems); i++) {
		Item* item = Array_Get(arrayOfItems, i);
		if ((item->type & type) && (item->flags & flags)) {
			return item;
		}
	}
	return NULL;
}
