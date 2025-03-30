#include <pinball/objects/Wall.h>
#include <m2/box2d/Shape.h>
#include <m2/box2d/Detail.h>
#include <m2/Game.h>

m2::void_expected LoadWall(m2::Object& obj) {
	const auto type = obj.GetType();
	const auto spriteType = M2_GAME.object_main_sprites[type];
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));

	auto& phy = obj.AddPhysique();
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(obj.position.x, obj.position.y);
	bodyDef.angle = obj.orientation;
	bodyDef.linearVelocity = {};
	bodyDef.angularVelocity = 0.0f;
	bodyDef.linearDamping = 0.0f;
	bodyDef.angularDamping = 0.0f;
	bodyDef.allowSleep = true;
	bodyDef.awake = false;
	bodyDef.fixedRotation = true;
	bodyDef.bullet = false;
	bodyDef.enabled = true;
	bodyDef.userData.pointer = obj.GetPhysiqueId();
	bodyDef.gravityScale = 0.0f;
	b2Body* body = M2_LEVEL.world->CreateBody(&bodyDef);
	if (const auto& foregroundFixtures = sprite.Pb().regular().foreground_fixtures();
			foregroundFixtures.rectangle_fixtures_size()) {
		b2FixtureDef fixtureDef;
		const auto polygon = m2::box2d::GeneratePolygonShape(foregroundFixtures.rectangle_fixtures(0), sprite.Ppm());
		fixtureDef.shape = &polygon;
		fixtureDef.friction = 0.0f; // TODO
		fixtureDef.restitution = 1.0f;
		fixtureDef.restitutionThreshold = 0.0f;
		fixtureDef.density = 0.0f;
		fixtureDef.isSensor = false;
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	} else if (foregroundFixtures.circle_fixtures_size()) {
		b2FixtureDef fixtureDef;
		const auto circle = m2::box2d::GenerateCircleShape(foregroundFixtures.circle_fixtures(0), sprite.Ppm());
		fixtureDef.shape = &circle;
		fixtureDef.friction = 0.0f; // TODO
		fixtureDef.restitution = 1.0f;
		fixtureDef.restitutionThreshold = 0.0f;
		fixtureDef.density = 0.0f;
		fixtureDef.isSensor = false;
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	} else if (foregroundFixtures.has_chain_fixture()) {
		b2FixtureDef fixtureDef;
		const auto chain = m2::box2d::GenerateChainShape(foregroundFixtures.chain_fixture(), sprite.Ppm());
		fixtureDef.shape = &chain;
		fixtureDef.friction = 0.0f; // TODO
		fixtureDef.restitution = 1.0f;
		fixtureDef.restitutionThreshold = 0.0f;
		fixtureDef.density = 0.0f;
		fixtureDef.isSensor = false;
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	}
	phy.body = m2::box2d::BodyUniquePtr{body};

	obj.AddGraphic(spriteType);

	return {};
}
