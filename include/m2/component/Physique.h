#pragma once
#include "../Component.h"
#include "../box2d/ContactListener.h"
#include <m2/third_party/physics/RigidBody.h>
#include <functional>

namespace m2 {
	struct Physique final : Component {
		using Callback = std::function<void(Physique&)>;
		Callback preStep{};
		Callback postStep{};

		// An object has either body or rigidBodyIndex.
		std::array<std::optional<third_party::physics::RigidBody>, gForegroundLayerCount> body{};
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
