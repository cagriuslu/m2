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

	/// Generates a filter that receives a Component, and returns true if the component belongs to object_id.
	constexpr auto is_component_of_object(Id object_id) { return [object_id](const Component& c) { return c.owner_id() == object_id; }; }
	/// Generates a filter that receives a Component, and returns true if the component belongs to any descendants of parent_id.
	std::function<bool(const Component&)> is_component_of_parent_object(Id parent_id);

	// Transformers
	constexpr Id to_owner_id_of_component(const Component& cmp) { return cmp.owner_id(); }
	inline Object& to_owner_of_component(const Component& cmp) { return cmp.owner(); }
}
