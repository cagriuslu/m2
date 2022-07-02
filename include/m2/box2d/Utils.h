#ifndef M2_UTILS_H
#define M2_UTILS_H

#include "../Vec2f.h"
#include "../Def.h"
#include "../Game.hh"
#include <b2_body.h>
#include <b2_world.h>
#include <memory>
#include <unordered_map>

namespace m2::box2d {
	constexpr uint16_t CAT_OBSTACLE       = 0x0001;
	constexpr uint16_t CAT_GND_OBSTACLE   = 0x0002;
	constexpr uint16_t CAT_PLAYER         = 0x0004;
	constexpr uint16_t CAT_PLAYER_AIR_OBJ = 0x0008;
	constexpr uint16_t CAT_PLAYER_AOE     = 0x0010;
	constexpr uint16_t CAT_PLAYER_ITEM    = 0x0020;
	constexpr uint16_t CAT_ENEMY          = 0x0040;
	constexpr uint16_t CAT_ENEMY_AIR_OBJ  = 0x0080;
	constexpr uint16_t CAT_ENEMY_AOE      = 0x0100;
	extern const std::unordered_map<uint16_t,uint16_t> collision_map;

    b2Body* create_body(b2World& world, ID physique_id, bool is_disk, bool is_dynamic, m2::Vec2f position, bool allow_sleep, bool is_bullet, bool is_sensor, uint16_t category_bits, uint16_t mask_bits, m2::Vec2f box_dims, m2::Vec2f box_center_offset, float box_angle, float disk_radius, float mass, float linearDamping, bool fixed_rotation);
    b2Body* create_static_box(b2World& world, ID physique_id, Vec2f position, bool is_sensor, uint16_t category, Vec2f box_dims);
    b2Body* create_static_disk(b2World& world, ID physique_id, Vec2f position, bool is_sensor, uint16_t category, float disk_radius);
    b2Body* create_dynamic_box(b2World& world, ID physique_id, Vec2f position, bool is_sensor, bool allow_sleep, uint16_t category_bits, Vec2f box_dims, float mass, float linear_damping);
    b2Body* create_dynamic_disk(b2World& world, ID physique_id, Vec2f position, bool is_sensor, bool allow_sleep, uint16_t category_bits, float disk_radius, float mass, float linear_damping);
    b2Body* create_bullet(b2World& world, ID physique_id, Vec2f position, bool is_sensor, uint16_t category_bits, float disk_radius, float mass, float linear_damping);

    b2AABB expand_aabb(const b2AABB& in, float amount);

	void destroy_body(b2Body*& body);
}

#endif //M2_UTILS_H
