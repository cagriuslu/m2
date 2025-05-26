#pragma once
#include <m2/math/VecF.h>
#include <m2/containers/Pool.h>
#include <box2d/b2_world_callbacks.h>
#include <box2d/b2_contact.h>
#include <functional>

namespace m2::box2d {
	struct Contact {
		VecF normal;
		VecF point; // Mid-point, in case of multiple points
		float separation; // Negative overlap amount. Average in case of multiple points
		std::array<std::pair<Id,int>, 2> fixtureIndexes; // PhysiqueId -> Fixture index

		explicit Contact(const b2Contact& contact);
	};

    class ContactListener final : public b2ContactListener {
        std::function<void(b2Contact&)> _beginContactCallback, _endContactCallback;

    public:
        explicit ContactListener(std::function<void(b2Contact&)>&& begin_contact_cb, std::function<void(b2Contact&)>&& end_contact_cb = {});

        void BeginContact(b2Contact* contact) override;
        void EndContact(b2Contact* contact) override;
    };
}
