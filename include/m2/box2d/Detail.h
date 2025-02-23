#pragma once
#include <BodyBlueprint.pb.h>
#include "../Game.h"
#include <box2d/b2_world.h>

namespace m2::box2d {
	constexpr uint16_t FIXTURE_CATEGORY_OBSTACLE_ON_BACKGROUND = (0x0001);
	constexpr uint16_t FIXTURE_CATEGORY_OBSTACLE_ON_FOREGROUND = (0x0002);
	constexpr uint16_t FIXTURE_CATEGORY_OBSTACLE = FIXTURE_CATEGORY_OBSTACLE_ON_BACKGROUND | FIXTURE_CATEGORY_OBSTACLE_ON_FOREGROUND;

	constexpr uint16_t FIXTURE_CATEGORY_FRIEND_ON_BACKGROUND = (0x0004);
	constexpr uint16_t FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND = (0x0008);
	constexpr uint16_t FIXTURE_CATEGORY_FRIEND_OFFENSE_ON_BACKGROUND = (0x0010);
	constexpr uint16_t FIXTURE_CATEGORY_FRIEND_OFFENSE_ON_FOREGROUND = (0x0020);
	constexpr uint16_t FIXTURE_CATEGORY_FRIEND = FIXTURE_CATEGORY_FRIEND_ON_BACKGROUND | FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND;
	constexpr uint16_t FIXTURE_CATEGORY_FRIEND_ALL_OFFENSE = FIXTURE_CATEGORY_FRIEND_OFFENSE_ON_BACKGROUND | FIXTURE_CATEGORY_FRIEND_OFFENSE_ON_FOREGROUND;
	constexpr uint16_t FIXTURE_CATEGORY_FRIEND_ALL_BACKGROUND = FIXTURE_CATEGORY_FRIEND_ON_BACKGROUND | FIXTURE_CATEGORY_FRIEND_OFFENSE_ON_BACKGROUND;
	constexpr uint16_t FIXTURE_CATEGORY_FRIEND_ALL_FOREGROUND = FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND | FIXTURE_CATEGORY_FRIEND_OFFENSE_ON_FOREGROUND;
	constexpr uint16_t FIXTURE_CATEGORY_FRIEND_ALL = FIXTURE_CATEGORY_FRIEND | FIXTURE_CATEGORY_FRIEND_ALL_OFFENSE;

	constexpr uint16_t FIXTURE_CATEGORY_FRIEND_ITEM_ON_FOREGROUND = (0x0040);

	constexpr uint16_t FIXTURE_CATEGORY_FOE_ON_BACKGROUND = (0x0100);
	constexpr uint16_t FIXTURE_CATEGORY_FOE_ON_FOREGROUND = (0x0200);
	constexpr uint16_t FIXTURE_CATEGORY_FOE_OFFENSE_ON_BACKGROUND = (0x0400);
	constexpr uint16_t FIXTURE_CATEGORY_FOE_OFFENSE_ON_FOREGROUND = (0x0800);
	constexpr uint16_t FIXTURE_CATEGORY_FOE = FIXTURE_CATEGORY_FOE_ON_BACKGROUND | FIXTURE_CATEGORY_FOE_ON_FOREGROUND;
	constexpr uint16_t FIXTURE_CATEGORY_FOE_ALL_OFFENSE = FIXTURE_CATEGORY_FOE_OFFENSE_ON_BACKGROUND | FIXTURE_CATEGORY_FOE_OFFENSE_ON_FOREGROUND;
	constexpr uint16_t FIXTURE_CATEGORY_FOE_ALL_BACKGROUND = FIXTURE_CATEGORY_FOE_ON_BACKGROUND | FIXTURE_CATEGORY_FOE_OFFENSE_ON_BACKGROUND;
	constexpr uint16_t FIXTURE_CATEGORY_FOE_ALL_FOREGROUND = FIXTURE_CATEGORY_FOE_ON_FOREGROUND | FIXTURE_CATEGORY_FOE_OFFENSE_ON_FOREGROUND;
	constexpr uint16_t FIXTURE_CATEGORY_FOE_ALL = FIXTURE_CATEGORY_FOE | FIXTURE_CATEGORY_FOE_ALL_OFFENSE;

	bool DoesCategoryHaveBackgroundBits(uint16_t category_bits);

    b2AABB ExpandAabb(const b2AABB& in, float amount);

	pb::BodyBlueprint ExampleBulletBodyBlueprint();

	/// b2Vec2 object's default constructor does not zero-initialize
	/// Provide our own.
	inline b2Vec2 Vec2Zero() { return b2Vec2{0.0f, 0.0f}; }
}
