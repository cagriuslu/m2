#pragma once
#include "../Component.h"
#include "../box2d/ContactListener.h"
#include <m2/thirdparty/physics/RigidBody.h>
#include <m2/math/VecF.h>
#include <m2/math/VecE.h>
#include <m2/ProxyTypes.h>
#include <m2/BuildOptions.h>
#include <m2g_Layers.pb.h>
#include <functional>
#include <type_traits>

namespace m2 {
	struct Physique final : Component {
		VecFE position;
		FE orientation{}; /// In radians

		using Callback = std::function<void(Physique&, const Stopwatch::Duration&)>;
		Callback preStep{};
		Callback postStep{};

		/// Either an array of Box2D bodies (one optional slot per physics layer), or an index into the custom physics
		/// World's rigid bodies. Which one is active is chosen at compile time, as a component never has both.
		std::conditional_t<USE_CUSTOM_PHYSICS,
				std::optional<int>,
				std::array<std::optional<thirdparty::physics::RigidBody>, PHYSICS_LAYER_COUNT>> body{};

		std::function<void(Physique&, Physique&, const box2d::Contact&)> onCollision;
		std::function<void(Physique&, Physique&)> offCollision;

		Physique() = default;
		explicit Physique(Id ownerId, const VecFE& position = {});
		// Copy not allowed
		Physique(const Physique& other) = delete;
		Physique& operator=(const Physique& other) = delete;
		// Move not allowed
		Physique(Physique&& other) = delete;
		Physique& operator=(Physique&& other) = delete;

		// Accessors

		[[nodiscard]] std::optional<m2g::pb::PhysicsLayer> GetCurrentPhysicsLayer() const;

		static void DefaultBeginContactCallback(b2Contact& b2_contact);
		static void DefaultEndContactCallback(b2Contact& b2_contact);
	};

	// Utilities

	/// Returns a force multiplier that can be applied to a physics object with a speed limit
	float CalculateLimitedForce(float curr_speed, float speed_limit);
}
