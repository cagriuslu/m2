#ifndef IMPL_ENEMY_H
#define IMPL_ENEMY_H

#include "rpg/Character.h"
#include <m2/Animation.h>
#include "m2/Object.h"
#include "rpg/fsm/Chaser.h"
#include "rpg/fsm/DistanceKeeper.h"
#include "rpg/fsm/HitNRunner.h"
#include "rpg/fsm/Patroller.h"
#include <m2/Value.h>

namespace obj {
    struct Enemy : public m2::ObjectImpl {
		chr::CharacterState character_state;
		m2::Fsm<m2::AnimationFsmBase> animation_fsm;
		using FSMVariant = std::variant<
			m2::Fsm<rpg::ChaserFsmBase>,
			m2::Fsm<rpg::DistanceKeeperFsmBase>,
			m2::Fsm<rpg::HitNRunnerFsmBase>,
			m2::Fsm<rpg::PatrollerFsmBase>
		>;
		FSMVariant fsm_variant; // TODO rename
		float on_hit_color_mod_ttl;

		Enemy(m2::Object&, const chr::CharacterBlueprint*);
		void stun();

        static m2::VoidValue init(m2::Object& obj, const chr::CharacterBlueprint* blueprint);
    };
}

#endif //IMPL_ENEMY_H
