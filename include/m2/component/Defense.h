#ifndef M2_DEFENSE_H
#define M2_DEFENSE_H

#include "../Component.h"

namespace m2 {
	struct Object;
}

namespace m2 {
	struct Defense : public Component {
		Defense() = default;
		explicit Defense(Id object_id);

		[[nodiscard]] Object& parent() const;
	};
}

#endif //M2_DEFENSE_H
