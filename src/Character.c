#include "Character.h"
#include <string.h>

XErr Character_Init(Character* char_, CharacterClass class_, unsigned level, Array itemArray) {
	memset(char_, 0, sizeof(Character));
	char_->class_ = class_;
	char_->level;
	char_->itemArray = itemArray;
	switch (class_) {
		case CHARTYP_HUMAN:
			switch (level) {
				case 1:
					char_->charDefense.maxHp = 100;
					char_->charDefense.hp = char_->charDefense.maxHp;
					char_->charOffense.hp = 15;
					char_->charOffense.projectileTicksLeft = 750;
					break;
			}
			break;
	}
	return 0;
}

void Character_Term(Character* chr) {
	memset(chr, 0, sizeof(Character));
}
