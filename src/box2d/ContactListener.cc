#include <m2/box2d/ContactListener.h>

m2::box2d::Contact::Contact(const b2Contact& contact) {
	b2WorldManifold world_manifold;
	contact.GetWorldManifold(&world_manifold);

	normal = VecF{world_manifold.normal};

	VecF p;
	auto point_count = contact.GetManifold()->pointCount;
	for (decltype(point_count) i = 0; i < point_count; ++i) {
		p += VecF{world_manifold.points[i]};
	}
	point = p / point_count;

	float s{};
	for (decltype(point_count) i = 0; i < point_count; ++i) {
		s += world_manifold.separations[i];
	}
	separation = s / static_cast<float>(point_count);

	fixtureIndexes[0].first = contact.GetFixtureA()->GetBody()->GetUserData().pointer;
	fixtureIndexes[0].second = contact.GetChildIndexA();
	fixtureIndexes[1].first = contact.GetFixtureB()->GetBody()->GetUserData().pointer;
	fixtureIndexes[1].second = contact.GetChildIndexB();
}

m2::box2d::ContactListener::ContactListener(std::function<void(b2Contact &)>&& begin_contact_cb, std::function<void(b2Contact&)>&& end_contact_cb) : _beginContactCallback(begin_contact_cb), _endContactCallback(end_contact_cb) {}

void m2::box2d::ContactListener::BeginContact(b2Contact *contact) {
    _beginContactCallback(*contact);
}

void m2::box2d::ContactListener::EndContact(b2Contact *contact) {
	_endContactCallback(*contact);
}
