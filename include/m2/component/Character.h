#ifndef M2_CHARACTER_H
#define M2_CHARACTER_H

#include "../Component.h"
#include <functional>

namespace m2 {
	struct Character : public Component {
		using Callback = std::function<void(Character&)>;
		Callback update;

		Character() = default;
		explicit Character(uint64_t object_id);
	};
}

#endif //M2_CHARACTER_H
