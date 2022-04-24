#ifndef IMPL_PLAYER_H
#define IMPL_PLAYER_H

#include "m2/Object.h"
#include "rpg/Character.h"
#include "rpg/fsm/CharacterAnimation.h"
#include <rpg/Consumable.h>

namespace obj {
	struct Player : public m2::ObjectImpl {
		chr::CharacterState char_state;
		m2::FSM<fsm::CharacterAnimation> char_animator;
		std::vector<itm::Consumable> consumables;

		Player(m2::Object&, const chr::CharacterBlueprint*);
		void add_consumable(const itm::Consumable& consumable);

		static M2Err init(m2::Object& obj, const chr::CharacterBlueprint* blueprint, m2::Vec2f pos);
	};
}

#endif //IMPL_PLAYER_H