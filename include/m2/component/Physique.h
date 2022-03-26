#ifndef M2_PHYSIQUE_H
#define M2_PHYSIQUE_H

#include "../Component.h"

namespace m2::component {
	struct Physique : public Component {
		b2Body* body;
		void (*onCollision)(Physique&, Physique&);

		Physique() = default;
		explicit Physique(ID object_id);
		// Copy not allowed
		Physique(const Physique& other) = delete;
		Physique& operator=(const Physique& other) = delete;
		// Move constructors
		Physique(Physique&& other) noexcept;
		Physique& operator=(Physique&& other) noexcept;
		// Destructor
		~Physique();

		static void contact_cb(b2Contact& contact);
	};
}

#endif //M2_PHYSIQUE_H
