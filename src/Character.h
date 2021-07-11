#ifndef CHARACTER_H
#define CHARACTER_H

#include "Component.h"
#include "Array.h"
#include "Error.h"

typedef enum _CharacterClass {
	CHARTYP_NONE = 0,
	CHARTYP_HUMAN,
} CharacterClass;

typedef struct _Character {
	CharacterClass class_;
	unsigned level;
	Array itemArray;
	ComponentDefense charDefense;
	ComponentOffense charOffense;
	// Preprocessed Values
	ComponentDefense projectileDefense;
	ComponentDefense meleeDefense;
	ComponentOffense projectileOffense;
	ComponentOffense meleeOffense;
} Character;

XErr Character_Init(Character* char_, CharacterClass class_, unsigned level, Array itemArray);
void Character_Term(Character* chr);

#endif
