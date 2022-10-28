#ifndef M2_PHYSIQUE_H
#define M2_PHYSIQUE_H

#include "../Component.h"
#include <box2d/b2_body.h>
#include <functional>

namespace m2 {
	struct Object;
}

namespace m2::comp {
	struct Physique : public Component {
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

		[[nodiscard]] Object& parent() const;

		static void contact_cb(b2Contact& contact);
	};
}

#endif //M2_PHYSIQUE_H
