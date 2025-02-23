#pragma once
#include "../Component.h"
#include "../box2d/ContactListener.h"
#include "../box2d/Body.h"
#include <functional>

namespace m2 {
	struct Physique : Component {
		static void DefaultDebugDraw(Physique& phy);

		using Callback = std::function<void(Physique&)>;
		Callback preStep{};
		Callback postStep{};
		Callback onDebugDraw{DefaultDebugDraw};

		// An object has either body or rigidBodyIndex.
		box2d::BodyUniquePtr body;
		std::optional<int> rigidBodyIndex;

		std::function<void(Physique&, Physique&, const box2d::Contact&)> onCollision;
		std::function<void(Physique&, Physique&)> offCollision;

		Physique() = default;
		explicit Physique(Id object_id);
		// Copy not allowed
		Physique(const Physique& other) = delete;
		Physique& operator=(const Physique& other) = delete;
		// Move constructors
		Physique(Physique&& other) noexcept;
		Physique& operator=(Physique&& other) noexcept;

		static void DefaultBeginContactCallback(b2Contact& b2_contact);
		static void DefaultEndContactCallback(b2Contact& b2_contact);
	};

	// Utilities

	/// Returns a force multiplier that can be applied to a physics object with a speed limit
	float CalculateLimitedForce(float curr_speed, float speed_limit);
}
