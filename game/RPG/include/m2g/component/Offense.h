#ifndef IMPL_OFFENSE_H
#define IMPL_OFFENSE_H

#include "rpg/ExplosiveWeapon.h"
#include "rpg/MeleeWeapon.h"
#include "m2/component/Offense.h"
#include <variant>

namespace m2g {
	struct Offense : public m2::Offense {
		std::variant<
			std::monostate,
			chr::ExplosiveState,
			chr::MeleeState
		> variant;

		Offense() = default;
		explicit Offense(m2::Id object_id);
	};
}

#endif //IMPL_OFFENSE_H
