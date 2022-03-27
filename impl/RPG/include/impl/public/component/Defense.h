#ifndef IMPL_DEFENSE_H
#define IMPL_DEFENSE_H

#include <m2/component/Defense.h>

namespace impl::component {
	struct Defense : public m2::component::Defense {
		float maxHp;
		float hp;
		void (*onHit)(Defense*);
		void (*onDeath)(Defense*);

		Defense() = default;
		explicit Defense(ID object_id);
	};
}

#endif //IMPL_DEFENSE_H
