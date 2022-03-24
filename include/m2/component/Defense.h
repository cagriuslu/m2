#ifndef M2_DEFENSE_H
#define M2_DEFENSE_H

#include "../Component.h"

namespace m2::component {
	struct Defense : public Component {
		Defense() = default;
		explicit Defense(ID object_id);
	};
}

#endif //M2_DEFENSE_H
