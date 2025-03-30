#include <pinball/objects/Edge.h>
#include <m2/Game.h>
#include <pinball/Pinball.h>
#include <m2/box2d/Detail.h>
#include <box2d/b2_edge_shape.h>

void LoadEdge() {
	const auto it = m2::CreateObject({}, m2g::pb::NO_OBJECT);

	auto& phy = it->AddPhysique();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(0.0f, 0.0f);
	bodyDef.allowSleep = true;
	bodyDef.fixedRotation = true;
	bodyDef.enabled = true;
	bodyDef.userData.pointer = it->GetPhysiqueId();
	bodyDef.gravityScale = 0.0f;
	b2Body* body = M2_LEVEL.world->CreateBody(&bodyDef);

	{
		// Top edge
		b2FixtureDef fixtureDef;
		b2EdgeShape edgeShape;
		edgeShape.SetTwoSided(b2Vec2{0.0f, 0.0f}, b2Vec2{gLevelDimensions.x, 0.0f});
		fixtureDef.shape = &edgeShape;
		fixtureDef.friction = 0.1f;
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_OBSTACLE;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	}
	{
		// Left edge
		b2FixtureDef fixtureDef;
		b2EdgeShape edgeShape;
		edgeShape.SetTwoSided(b2Vec2{0.0f, 0.0f}, b2Vec2{0.0f, gLevelDimensions.y});
		fixtureDef.shape = &edgeShape;
		fixtureDef.friction = 0.1f;
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_OBSTACLE;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	}
	{
		// Rigth edge
		b2FixtureDef fixtureDef;
		b2EdgeShape edgeShape;
		edgeShape.SetTwoSided(b2Vec2{gLevelDimensions.x, 0.0f}, b2Vec2{gLevelDimensions.x, gLevelDimensions.y});
		fixtureDef.shape = &edgeShape;
		fixtureDef.friction = 0.1f;
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_OBSTACLE;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	}
	{
		// Bottom edge
		b2FixtureDef fixtureDef;
		b2EdgeShape edgeShape;
		edgeShape.SetTwoSided(b2Vec2{0.0f, gLevelDimensions.y / 2.0f}, b2Vec2{gLevelDimensions.x, gLevelDimensions.y / 2.0f});
		fixtureDef.shape = &edgeShape;
		fixtureDef.friction = 0.0f; // TODO
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_OBSTACLE;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	}

	phy.body = m2::box2d::BodyUniquePtr{body};
}
