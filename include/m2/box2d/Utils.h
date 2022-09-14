#ifndef M2_UTILS_H
#define M2_UTILS_H

#include "../Vec2f.h"
#include <BodyBlueprint.pb.h>
#include "../Game.hh"
#include <b2_body.h>
#include <b2_fixture.h>
#include <b2_world.h>
#include <memory>
#include <unordered_map>

namespace m2::box2d {
	constexpr uint16_t BODY_CATEGORY_OBSTACLE_BACKGROUND = (0x0001);
	constexpr uint16_t BODY_CATEGORY_OBSTACLE_FOREGROUND = (0x0002);
	constexpr uint16_t BODY_CATEGORY_OBSTACLE = (BODY_CATEGORY_OBSTACLE_BACKGROUND | BODY_CATEGORY_OBSTACLE_FOREGROUND);
	constexpr uint16_t BODY_CATEGORY_FRIEND_BACKGROUND = (0x0004);
	constexpr uint16_t BODY_CATEGORY_FRIEND_FOREGROUND = (0x0008);
	constexpr uint16_t BODY_CATEGORY_FRIEND = (BODY_CATEGORY_FRIEND_BACKGROUND | BODY_CATEGORY_FRIEND_FOREGROUND);
	constexpr uint16_t BODY_CATEGORY_FOE_BACKGROUND = (0x0010);
	constexpr uint16_t BODY_CATEGORY_FOE_FOREGROUND = (0x0020);
	constexpr uint16_t BODY_CATEGORY_FOE = (BODY_CATEGORY_FOE_BACKGROUND | BODY_CATEGORY_FOE_FOREGROUND);
	constexpr uint16_t BODY_CATEGORY_FRIEND_PICKUP = (0x0040);

	b2Body* create_body(b2World& world, Id physique_id, m2::Vec2f position, const pb::BodyBlueprint& blueprint);
	void destroy_body(b2Body*& body);

    b2AABB expand_aabb(const b2AABB& in, float amount);
	bool is_obstacle(const b2Fixture* fixture);
}

#endif //M2_UTILS_H
