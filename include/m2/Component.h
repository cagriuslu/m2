#pragma once
#include <m2/M2.h>
#include <m2/containers/Pool.h>

namespace m2 {
    // Forward declaration
    struct Object;

	class Component {
	protected:
		Id _ownerId{};

	public:
		Component() = default;
		explicit Component(const Id ownerId) : _ownerId(ownerId) {}
		virtual ~Component() = default;

		[[nodiscard]] constexpr Id GetOwnerId() const { return _ownerId; }
        [[nodiscard]] Object& GetOwner() const;
	};

	// Filters

	/// Generates a filter that receives a Component, and returns true if the component belongs to object_id.
	constexpr auto IsComponentOfObject(Id objectId) { return [objectId](const Component& c) { return c.GetOwnerId() == objectId; }; }
	/// Generates a filter that receives a Component, and returns true if the component belongs to any descendants of parent_id.
	std::function<bool(const Component&)> IsComponentOfAnyDescendant(Id parentId);
}
