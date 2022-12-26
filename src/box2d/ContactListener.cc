#include <m2/box2d/ContactListener.h>

m2::box2d::ContactListener::ContactListener(std::function<void(b2Contact &)>&& begin_contact_cb, std::function<void(b2Contact&)>&& end_contact_cb) : _begin_contact_cb(begin_contact_cb), _end_contact_cb(end_contact_cb) {}

void m2::box2d::ContactListener::BeginContact(b2Contact *contact) {
    _begin_contact_cb(*contact);
}

void m2::box2d::ContactListener::EndContact(b2Contact *contact) {
	_end_contact_cb(*contact);
}
