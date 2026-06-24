#pragma once
#include "../Component.h"
#include "../box2d/ContactListener.h"
#include <m2/thirdparty/physics/RigidBody.h>
#include <m2/physics/StaticBody.h>
#include <m2/common/math/VecF.h>
#include <m2/common/math/VecE.h>
#include <m2/common/BuildOptions.h>
#include <functional>
#include <type_traits>
#include <variant>

namespace m2 {
	struct Physique final : Component {
		std::function<void(Physique&, const Stopwatch::Duration&)> preStep{};
		std::function<void(Physique&, const Stopwatch::Duration&)> postStep{};

		using StaticBody = physics::StaticBody;
		using M2Body = int;
		using Box2dBody = thirdparty::physics::RigidBody;
		/// Either a Box2D body, or an index into the custom physics world's rigid bodies. Which one is active is chosen
		/// at compile time, as a component never has both.
		using DynamicBody = std::conditional_t<USE_M2_PHYSICS, M2Body, Box2dBody>;
		using Body = std::conditional_t<GAME_IS_DETERMINISTIC, std::variant<StaticBody>, std::variant<StaticBody, DynamicBody>>;
		Body body{};

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

		[[nodiscard]] VecFE GetPosition() const;
		[[nodiscard]] FE GetOrientation() const; // In radians
		void SetPosition(const VecFE&);
		void SetOrientation(const FE&); // In radians

		static void DefaultBeginContactCallback(b2Contact& b2_contact);
		static void DefaultEndContactCallback(b2Contact& b2_contact);
	};

	/// Returns a force multiplier that can be applied to a physics object with a speed limit
	float CalculateLimitedForce(float curr_speed, float speed_limit);
}
