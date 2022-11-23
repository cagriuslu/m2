#ifndef COMPONENT_H
#define COMPONENT_H

#include <m2/M2.h>
#include "Pool.hh"
#include <SDL.h>

namespace m2 {
    // Forward declaration
    struct Object;

	struct Component {
		Id object_id{0};

		Component() = default;
		explicit Component(Id object_id);

        [[nodiscard]] Object& parent() const;
	};
}

#endif
