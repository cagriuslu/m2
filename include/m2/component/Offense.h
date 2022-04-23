#ifndef M2_OFFENSE_H
#define M2_OFFENSE_H

#include "../Component.h"

namespace m2 {
	struct Object;
}

namespace m2::comp {
	struct Offense : public Component {
		Offense() = default;
		explicit Offense(ID object_id);

		[[nodiscard]] Object& parent() const;
	};
}

#endif //M2_OFFENSE_H
