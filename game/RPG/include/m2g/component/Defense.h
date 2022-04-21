#ifndef IMPL_DEFENSE_H
#define IMPL_DEFENSE_H

#include "m2/component/Defense.h"
#include <functional>

namespace impl::component {
	struct Defense : public m2::component::Defense {
		float maxHp;
		float hp;
		std::function<void(Defense&)> on_hit;
		std::function<void(Defense&)> on_death;

		Defense() = default;
		explicit Defense(ID object_id);
	};
}

#endif //IMPL_DEFENSE_H
