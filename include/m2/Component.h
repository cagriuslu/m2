#ifndef COMPONENT_H
#define COMPONENT_H

#include <m2/M2.h>
#include <SDL.h>

namespace m2 {
	struct Component {
		ID object_id{0};

		Component() = default;
		explicit Component(ID object_id);
	};
}

#endif
