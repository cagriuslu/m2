#ifndef IMPL_ENEMY_H
#define IMPL_ENEMY_H

#include "rpg/Character.h"
#include "m2/Object.h"
#include "rpg/fsm/Chaser.h"
#include "rpg/fsm/DistanceKeeper.h"
#include "rpg/fsm/CharacterAnimation.h"
#include "rpg/fsm/HitNRunner.h"
#include "rpg/fsm/Patroller.h"
#include "m2/LevelBlueprint.h"

namespace obj {
    struct Enemy : public m2::ObjectImpl {
		chr::CharacterState character_state;
		m2::FSM<fsm::CharacterAnimation> char_animator;
		using FSMVariant = std::variant<
			m2::FSM<fsm::Chaser>,
			m2::FSM<fsm::DistanceKeeper>,
			m2::FSM<fsm::HitNRunner>,
			m2::FSM<fsm::Patroller>
		>;
		FSMVariant fsm_variant; // TODO rename
		float on_hit_color_mod_ttl;

		Enemy(m2::Object&, const chr::CharacterBlueprint*);
		void stun();

        static M2Err init(m2::Object& obj, const chr::CharacterBlueprint* blueprint, m2::GroupID group_id, m2::Vec2f pos);
    };
}

#endif //IMPL_ENEMY_H
