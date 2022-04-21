#ifndef IMPL_OFFENSE_H
#define IMPL_OFFENSE_H

#include "rpg/ExplosiveWeapon.h"
#include "rpg/MeleeWeapon.h"
#include "rpg/RangedWeapon.h"
#include "m2/component/Offense.h"
#include <variant>

namespace m2g::component {
	struct Offense : public m2::component::Offense {
		ID originator;
		std::variant<
			std::monostate,
			chr::ExplosiveState,
			chr::MeleeState,
			chr::ProjectileState
		> variant;

		Offense() = default;
		explicit Offense(ID object_id);
	};
}

#endif //IMPL_OFFENSE_H
