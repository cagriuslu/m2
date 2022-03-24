#ifndef M2_OFFENSE_H
#define M2_OFFENSE_H

#include "../Component.h"

namespace m2::component {
	struct Offense : public Component {
		Offense() = default;
		explicit Offense(ID object_id);
	};
}

#endif //M2_OFFENSE_H
