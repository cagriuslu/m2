#pragma once
#include <m2/M2.h>
#include <m2/containers/Pool.h>
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

		[[nodiscard]] constexpr Id OwnerId() const { return _owner_id; }
        [[nodiscard]] Object& Owner() const;
	};

	// Filters

	/// Generates a filter that receives a Component, and returns true if the component belongs to object_id.
	constexpr auto IsComponentOfObject(Id object_id) { return [object_id](const Component& c) { return c.OwnerId() == object_id; }; }
	/// Generates a filter that receives a Component, and returns true if the component belongs to any descendants of parent_id.
	std::function<bool(const Component&)> IsComponentOfAnyDescendant(Id parent_id);

	// Transformers

	constexpr Id ToOwnerId(const Component& cmp) { return cmp.OwnerId(); }
	inline Object& ToOwner(const Component& cmp) { return cmp.Owner(); }
}
