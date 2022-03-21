#ifndef BOX2D_H
#define BOX2D_H

#include <b2_world_callbacks.h>
#include <m2/vec2f.hh>
#include "m2/AABB.hh"
#include <cstdint>

class ContactListener : public b2ContactListener {
	void (*m_cb)(b2Contact*);
public:
	ContactListener(void (*cb)(b2Contact*));

	void BeginContact(b2Contact* contact) override;
};

class RayCastCallback : public b2RayCastCallback {
	float (*m_cb)(b2Fixture*, m2::vec2f point, m2::vec2f normal, float fraction, void* userData);
	uint16_t m_categoryMask;
	void* m_userData;
public:
	RayCastCallback(float (*cb)(b2Fixture*, m2::vec2f point, m2::vec2f normal, float fraction, void* userData), uint16_t categoryMask, void* userData);

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;
};

class QueryCallback : public b2QueryCallback {
	bool (*m_cb)(b2Fixture*, void* userData);
	void* m_userData;
public:
	QueryCallback(bool (*cb)(b2Fixture*, void* userData), void* userData);

	bool ReportFixture(b2Fixture* fixture) override;
};

#endif
