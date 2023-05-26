#pragma once
#include "../VecF.h"
#include <box2d/b2_world_callbacks.h>
#include <box2d/b2_contact.h>
#include <functional>

namespace m2::box2d {
	struct Contact {
		VecF normal;
		VecF point; // Mid-point, in case of multiple points
		float separation; // Negative overlap amount. Average in case of multiple points

		explicit Contact(const b2Contact& contact);
	};

    class ContactListener : public b2ContactListener {
        std::function<void(b2Contact&)> _begin_contact_cb;
        std::function<void(b2Contact&)> _end_contact_cb;
    public:
        explicit ContactListener(std::function<void(b2Contact&)>&& begin_contact_cb, std::function<void(b2Contact&)>&& end_contact_cb = {});

        void BeginContact(b2Contact* contact) override;
        void EndContact(b2Contact* contact) override;
    };
}
