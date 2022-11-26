#ifndef M2_CHARACTER_H
#define M2_CHARACTER_H

#include "../Component.h"

namespace m2 {
	struct Character : public Component {
		Character() = default;
		explicit Character(uint64_t object_id);
	};
}

#endif //M2_CHARACTER_H
