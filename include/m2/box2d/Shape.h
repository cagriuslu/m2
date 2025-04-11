#pragma once
#include <Sprite.pb.h>
#include <box2d/b2_chain_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_edge_shape.h>
#include <box2d/b2_polygon_shape.h>

namespace m2::box2d {
	// The generated shapes are not rotated. If the object is originally skewed, the physics body should be rotated
	// **after** its created.

	b2PolygonShape GeneratePolygonShape(const pb::Fixture_RectangleFixture& rectFixture, int spritePpm);
	b2CircleShape GenerateCircleShape(const pb::Fixture_CircleFixture& circFixture, int spritePpm);
	b2ChainShape GenerateChainShape(const pb::Fixture_ChainFixture& chainFixture, int spritePpm);
}
