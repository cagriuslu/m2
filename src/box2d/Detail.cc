#include <m2/box2d/Detail.h>
#include <box2d/b2_circle_shape.h>
#include <exception>

bool m2::box2d::does_category_have_background_bits(uint16_t category_bits) {
	return (category_bits & FIXTURE_CATEGORY_OBSTACLE_ON_BACKGROUND) ||
		(category_bits & FIXTURE_CATEGORY_FRIEND_ON_BACKGROUND) ||
		(category_bits & FIXTURE_CATEGORY_FRIEND_OFFENSE_ON_BACKGROUND) ||
		(category_bits & FIXTURE_CATEGORY_FOE_ON_BACKGROUND) ||
		(category_bits & FIXTURE_CATEGORY_FOE_OFFENSE_ON_BACKGROUND);
}

b2AABB m2::box2d::expand_aabb(const b2AABB& in, float amount) {
    return b2AABB{
		.lowerBound = b2Vec2{in.lowerBound.x - amount, in.lowerBound.y - amount},
		.upperBound = b2Vec2{in.upperBound.x + amount, in.upperBound.y + amount}
    };
}

m2::pb::BodyBlueprint m2::box2d::example_bullet_body_blueprint() {
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::KINEMATIC);
	bp.set_allow_sleep(false);
	bp.set_is_bullet(true);
	return bp;
}
