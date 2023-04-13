#ifndef M2_ACTION_H
#define M2_ACTION_H

#include <functional>

namespace m2 {
	struct Action {
		std::function<bool()> trigger_condition;
		std::function<void()> action;
	};
}

#endif //M2_ACTION_H
