#include <m2/box2d/ContactListener.h>

m2::box2d::ContactListener::ContactListener(std::function<void(b2Contact &)>&& cb) : m_cb(cb) {}

void m2::box2d::ContactListener::BeginContact(b2Contact *contact) {
    m_cb(*contact);
}
