#ifndef IMPL_OFFENSE_H
#define IMPL_OFFENSE_H

#include <impl/private/ARPG_Cfg.hh>
#include <m2/component/Offense.h>

namespace impl::component {
	struct Offense : public m2::component::Offense {
		ID originator;
		union {
			ProjectileState projectile;
			MeleeState melee;
			ExplosiveState explosive;
		} state;

		Offense() = default;
		explicit Offense(ID object_id);
	};
}

#endif //IMPL_OFFENSE_H
