#include <m2/box2d/Shape.h>
#include <m2/math/VecF.h>
#include <m2/M2.h>

using namespace m2;

b2PolygonShape m2::box2d::GeneratePolygonShape(const pb::Fixtures_RectangleFixture& rectFixture, const int spritePpm) {
	b2PolygonShape polygonShape;
	polygonShape.SetAsBox(
			rectFixture.rectangle_dimensions_px().w() / F(spritePpm),
			rectFixture.rectangle_dimensions_px().h() / F(spritePpm),
			static_cast<b2Vec2>(VecF{rectFixture.sprite_origin_to_rectangle_center_vec_px()} / F(spritePpm)),
			0.0f);
	return polygonShape;
}
b2CircleShape m2::box2d::GenerateCircleShape(const pb::Fixtures_CircleFixture& circFixture, const int spritePpm) {
	b2CircleShape circleShape;
	circleShape.m_radius = circFixture.circle_radius_px() / F(spritePpm);
	circleShape.m_p = static_cast<b2Vec2>(VecF{circFixture.sprite_origin_to_circle_center_vec_px()} / F(spritePpm));
	return circleShape;
}
b2ChainShape m2::box2d::GenerateChainShape(const pb::Fixtures_ChainFixture& chainFixture, const int spritePpm) {
	if (chainFixture.points_size() < 3) {
		throw M2_ERROR("Chain fixture must have at least 3 points");
	}
	// Convert to box2d vertices
	std::vector<b2Vec2> vertices;
	std::ranges::transform(chainFixture.points(), std::back_inserter(vertices), [&](const auto& point) {
		return static_cast<b2Vec2>(VecF{point} / F(spritePpm));
	});
	// Create loop
	b2ChainShape chainShape;
	chainShape.CreateLoop(vertices.data(), vertices.size());
	return chainShape;
}
