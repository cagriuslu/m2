#ifndef IMPL_PLAYER_H
#define IMPL_PLAYER_H

#include "m2/Object.h"
#include <m2/fsm/AnimationFSM.h>
#include "rpg/Character.h"
#include <rpg/ConsumableBlueprint.h>
#include <vector>
#include <m2/Value.h>

namespace obj {
	struct Player : public m2::ObjectImpl {
		chr::CharacterState char_state;
		m2::fsm::AnimationFSM animation_fsm;
		std::vector<itm::ConsumableBlueprint> consumables;

		Player(m2::Object&, const chr::CharacterBlueprint*);
		void add_consumable(const itm::ConsumableBlueprint& consumable);

		static m2::VoidValue init(m2::Object& obj, const chr::CharacterBlueprint* blueprint, m2::Vec2f pos);
	};
}

#endif //IMPL_PLAYER_H
