#ifndef IMPL_ENEMY_H
#define IMPL_ENEMY_H

#include "rpg/character/Character.h"
#include "m2/Object.h"
#include "rpg/fsm/Chaser.h"
#include "rpg/fsm/DistanceKeeper.h"
#include "rpg/fsm/CharacterAnimation.h"
#include "rpg/fsm/HitNRunner.h"
#include "rpg/fsm/Patroller.h"

namespace impl::object {
    struct Enemy : public m2::ObjectImpl {
		character::CharacterState character_state;
		m2::FSM<impl::fsm::CharacterAnimation> char_animator;
		using FSMVariant = std::variant<
			m2::FSM<impl::fsm::Chaser>,
			m2::FSM<impl::fsm::DistanceKeeper>,
			m2::FSM<impl::fsm::HitNRunner>,
			m2::FSM<impl::fsm::Patroller>
		>;
		FSMVariant fsm_variant; // TODO rename
		float on_hit_color_mod_ttl;

		Enemy(m2::Object&, const character::CharacterBlueprint*);

        static M2Err init(m2::Object& obj, const character::CharacterBlueprint* blueprint, m2::Vec2f pos);
    };
}

#endif //IMPL_ENEMY_H
