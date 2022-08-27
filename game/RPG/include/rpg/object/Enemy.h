#ifndef IMPL_ENEMY_H
#define IMPL_ENEMY_H

#include "rpg/Character.h"
#include <m2/fsm/AnimationFSM.h>
#include "m2/Object.h"
#include "rpg/fsm/Chaser.h"
#include "rpg/fsm/DistanceKeeper.h"
#include "rpg/fsm/HitNRunner.h"
#include "rpg/fsm/Patroller.h"
#include "m2/LevelBlueprint.h"
#include <m2/Value.h>

namespace obj {
    struct Enemy : public m2::ObjectImpl {
		chr::CharacterState character_state;
		m2::fsm::AnimationFSM animation_fsm;
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

        static m2::VoidValue init(m2::Object& obj, const chr::CharacterBlueprint* blueprint, const m2::model::GroupBlueprint& group, m2::Vec2f pos);
    };
}

#endif //IMPL_ENEMY_H
