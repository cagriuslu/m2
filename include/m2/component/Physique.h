#ifndef M2_PHYSIQUE_H
#define M2_PHYSIQUE_H

#include "../Component.h"
#include <box2d/b2_body.h>
#include <functional>

namespace m2 {
	struct Physique : public Component {
		using Callback = std::function<void(Physique&)>;
		Callback pre_step{};
		Callback post_step{};

		b2Body* body{};
		std::function<void(Physique&, Physique&)> on_collision;

		Physique() = default;
		explicit Physique(Id object_id);
		// Copy not allowed
		Physique(const Physique& other) = delete;
		Physique& operator=(const Physique& other) = delete;
		// Move constructors
		Physique(Physique&& other) noexcept;
		Physique& operator=(Physique&& other) noexcept;
		// Destructor
		~Physique();

		void draw_debug_shapes() const;

		static void contact_cb(b2Contact& contact);
	};

	// Utilities

	/// Returns a force multiplier that can be applied to a physics object with a speed limit
	/// speed_limit doesn't have a known unit
	float calculate_limited_force(float curr_velocity, float speed_limit);
}

#endif //M2_PHYSIQUE_H
