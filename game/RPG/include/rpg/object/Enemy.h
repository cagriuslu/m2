#ifndef IMPL_ENEMY_H
#define IMPL_ENEMY_H

#include "rpg/Character.h"
#include <m2/fsm/AnimationFsm.h>
#include "m2/Object.h"
#include "rpg/fsm/Chaser.h"
#include "rpg/fsm/DistanceKeeper.h"
#include "rpg/fsm/HitNRunner.h"
#include "rpg/fsm/Patroller.h"
#include <m2/Value.h>

namespace obj {
    struct Enemy : public m2::ObjectImpl {
		chr::CharacterState character_state;
		m2::fsm::AnimationFsm animation_fsm;
		using FSMVariant = std::variant<
			m2::Fsm<fsm::Chaser>,
			m2::Fsm<fsm::DistanceKeeper>,
			m2::Fsm<fsm::HitNRunner>,
			m2::Fsm<fsm::Patroller>
		>;
		FSMVariant fsm_variant; // TODO rename
		float on_hit_color_mod_ttl;

		Enemy(m2::Object&, const chr::CharacterBlueprint*);
		void stun();

        static m2::VoidValue init(m2::Object& obj, const chr::CharacterBlueprint* blueprint);
    };
}

#endif //IMPL_ENEMY_H
