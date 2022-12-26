#ifndef M2_CONTACTLISTENER_H
#define M2_CONTACTLISTENER_H

#include <box2d/b2_world_callbacks.h>
#include <functional>

namespace m2::box2d {
    class ContactListener : public b2ContactListener {
        std::function<void(b2Contact&)> _begin_contact_cb;
        std::function<void(b2Contact&)> _end_contact_cb;
    public:
        explicit ContactListener(std::function<void(b2Contact&)>&& begin_contact_cb, std::function<void(b2Contact&)>&& end_contact_cb = {});

        void BeginContact(b2Contact* contact) override;
        void EndContact(b2Contact* contact) override;
    };
}

#endif //M2_CONTACTLISTENER_H
