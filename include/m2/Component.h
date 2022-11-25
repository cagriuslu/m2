#ifndef COMPONENT_H
#define COMPONENT_H

#include <m2/M2.h>
#include "Pool.hh"
#include <SDL2/SDL.h>

namespace m2 {
	struct Component {
		Id object_id{0};

		Component() = default;
		explicit Component(Id object_id);
	};
}

#endif
