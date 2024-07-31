#pragma once
#include <m2/M2.h>
#include "Pool.h"
#include <SDL2/SDL.h>

namespace m2 {
    // Forward declaration
    struct Object;

	struct Component {
		Id object_id{0};

		Component() = default;
		explicit Component(Id object_id);
		virtual ~Component() = default;

		[[nodiscard]] Id parent_id() const { return object_id; }
        [[nodiscard]] Object& parent() const;
	};

	// Filter Generators
	constexpr auto is_component_of_parent(Id parent_id) { return [parent_id](const Component& c) { return c.object_id == parent_id; }; }
	// Transformers
	constexpr Id to_parent_id_of_component(const Component& cmp) { return cmp.parent_id(); }
}
