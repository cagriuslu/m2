#ifndef M2_CONTACTLISTENER_H
#define M2_CONTACTLISTENER_H

#include <b2_world_callbacks.h>
#include <functional>

namespace m2::box2d {
    class ContactListener : public b2ContactListener {
        std::function<void(b2Contact&)> m_cb;
    public:
        explicit ContactListener(std::function<void(b2Contact&)>&& cb);

        void BeginContact(b2Contact* contact) override;
    };
}

#endif //M2_CONTACTLISTENER_H
