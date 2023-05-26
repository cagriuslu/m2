#pragma once
#include <m2/M2.h>
#include "Pool.hh"
#include <SDL2/SDL.h>

namespace m2 {
    // Forward declaration
    struct Object;

	struct Component {
		Id object_id{0};

		Component() = default;
		explicit Component(Id object_id);
		virtual ~Component() = default;

        [[nodiscard]] Object& parent() const;
	};
}
