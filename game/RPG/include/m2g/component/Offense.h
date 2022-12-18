#ifndef IMPL_OFFENSE_H
#define IMPL_OFFENSE_H

#include "m2/component/Offense.h"
#include <variant>

namespace m2g {
	struct Offense : public m2::Offense {
		Offense() = default;
		explicit Offense(m2::Id object_id);
	};
}

#endif //IMPL_OFFENSE_H
