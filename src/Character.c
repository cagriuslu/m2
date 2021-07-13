#include "Character.h"
#include "Item.h"
#include <string.h>

XErr Character_Init(Character* char_, CharacterClass class_, unsigned level, Array itemArray) {
	memset(char_, 0, sizeof(Character));
	char_->class_ = class_;
	char_->level;
	char_->itemArray = itemArray;
	ComponentDefenseInit(&char_->charDefense, 0);
	ComponentOffenseInit(&char_->charOffense, 0);
	switch (class_) {
		case CHARTYP_HUMAN:
			switch (level) {
				case 1:
					char_->charDefense.maxHp = 100;
					break;
			}
			break;
	}
	ComponentDefenseInit(&char_->defense, 0);
	ComponentOffenseInit(&char_->projectileOffense, 0);
	ComponentOffenseInit(&char_->meleeOffense, 0);
	return Character_Preprocess(char_);
}

XErr Character_Preprocess(Character* char_) {
	Character_ClearPreprocessed(char_);
	char_->defense = char_->charDefense;
	char_->projectileOffense = char_->charOffense;
	char_->meleeOffense = char_->charOffense;
	for (size_t i = 0; i < Array_Length(&char_->itemArray); i++) {
		Item* item = Array_Get(&char_->itemArray, i);
		if (item->flags & ITEMFLAG_EQUIPPED) {
			char_->defense.maxHp += item->defenseHitPoints;
			switch (item->type) {
			case ITEMTYP_GUN:
			case ITEMTYP_RIFLE:
			case ITEMTYP_CROSSBOW:
			case ITEMTYP_BOW:
				char_->projectileOffense.hp += item->offenseHitPoints;
				char_->projectileOffense.ttl += item->offenseTTL;
				break;
			case ITEMTYP_SWORD:
			case ITEMTYP_SPEAR:
				char_->meleeOffense.hp += item->offenseHitPoints;
				char_->meleeOffense.ttl += item->offenseTTL;
				break;
			case ITEMTYP_PANTS:
			case ITEMTYP_SHIRT:
			case ITEMTYP_AMULET:
			case ITEMTYP_SHINYBELT:
			case ITEMTYP_LEFTRING:
			case ITEMTYP_RIGHTRING:
				char_->meleeOffense.hp += item->offenseHitPoints;
				char_->meleeOffense.ttl += item->offenseTTL;
				char_->projectileOffense.hp += item->offenseHitPoints;
				char_->projectileOffense.ttl += item->offenseTTL;
				break;
			}
		}
	}
	
	return 0;
}

XErr Character_ClearPreprocessed(Character* char_) {
	ComponentOffenseDeinit(&char_->meleeOffense);
	ComponentOffenseDeinit(&char_->projectileOffense);
	ComponentDefenseDeinit(&char_->defense);
	ComponentDefenseInit(&char_->defense, 0);
	ComponentOffenseInit(&char_->projectileOffense, 0);
	ComponentOffenseInit(&char_->meleeOffense, 0);
	return 0;
}

void Character_Term(Character* char_) {
	ComponentOffenseDeinit(&char_->meleeOffense);
	ComponentOffenseDeinit(&char_->projectileOffense);
	ComponentDefenseDeinit(&char_->defense);
	ComponentOffenseDeinit(&char_->charOffense);
	ComponentDefenseDeinit(&char_->charDefense);
	Array_Term(&char_->itemArray);
	memset(char_, 0, sizeof(Character));
}
