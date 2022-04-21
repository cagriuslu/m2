#ifndef IMPL_OFFENSE_H
#define IMPL_OFFENSE_H

#include <impl/private/character/ExplosiveWeapon.h>
#include <impl/private/character/MeleeWeapon.h>
#include <impl/private/character/RangedWeapon.h>
#include <m2/component/Offense.h>
#include <variant>

namespace impl::component {
	struct Offense : public m2::component::Offense {
		ID originator;
		std::variant<
			std::monostate,
			character::ExplosiveState,
			character::MeleeState,
			character::ProjectileState
		> variant;

		Offense() = default;
		explicit Offense(ID object_id);
	};
}

#endif //IMPL_OFFENSE_H
