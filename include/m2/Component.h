#pragma once
#include <m2/M2.h>
#include "Pool.h"
#include <SDL2/SDL.h>

namespace m2 {
    // Forward declaration
    struct Object;

	class Component {
	protected:
		Id _owner_id{0};

	public:
		Component() = default;
		explicit Component(Id owner_id);
		virtual ~Component() = default;

		[[nodiscard]] constexpr Id owner_id() const { return _owner_id; }
        [[nodiscard]] Object& owner() const;
	};

	// Filter Generators
	constexpr auto is_component_of_object(Id object_id) { return [object_id](const Component& c) { return c.owner_id() == object_id; }; }
	std::function<bool(const Component&)> is_component_of_child_object_of_parent(Id parent_id);
	// Transformers
	constexpr Id to_owner_id_of_component(const Component& cmp) { return cmp.owner_id(); }
	inline Object& to_owner_of_component(const Component& cmp) { return cmp.owner(); }
}
