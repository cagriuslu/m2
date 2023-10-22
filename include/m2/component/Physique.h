#pragma once
#include "../Component.h"
#include "../box2d/ContactListener.h"
#include "../box2d/Body.h"
#include <box2d/b2_body.h>
#include <functional>

namespace m2 {
	struct Physique : public Component {
		static void default_debug_draw(Physique& phy);

		using Callback = std::function<void(Physique&)>;
		Callback pre_step{};
		Callback post_step{};
		Callback on_debug_draw{default_debug_draw};

		box2d::BodyUniquePtr body;
		std::function<void(Physique&, Physique&, const box2d::Contact&)> on_collision;
		std::function<void(Physique&, Physique&)> off_collision;

		Physique() = default;
		explicit Physique(Id object_id);
		// Copy not allowed
		Physique(const Physique& other) = delete;
		Physique& operator=(const Physique& other) = delete;
		// Move constructors
		Physique(Physique&& other) noexcept;
		Physique& operator=(Physique&& other) noexcept;

		static void default_begin_contact_cb(b2Contact& b2_contact);
		static void default_end_contact_cb(b2Contact& b2_contact);
	};

	// Utilities

	/// Returns a force multiplier that can be applied to a physics object with a speed limit
	float calculate_limited_force(float curr_speed, float speed_limit);
}
