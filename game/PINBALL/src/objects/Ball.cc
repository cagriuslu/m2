#include <pinball/objects/Ball.h>
#include <m2/Game.h>
#include <box2d/b2_circle_shape.h>
#include <m2/box2d/Detail.h>

m2::void_expected LoadBall(m2::Object& obj) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPRITE_BASIC_BALL));

	auto& phy = obj.add_physique();
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(obj.position.x, obj.position.y);
	bodyDef.angle = obj.orientation;
	bodyDef.linearVelocity = {};
	bodyDef.angularVelocity = 0.0f;
	bodyDef.linearDamping = 0.1f;
	bodyDef.angularDamping = 0.1f;
	bodyDef.allowSleep = false;
	bodyDef.awake = true;
	bodyDef.fixedRotation = false;
	bodyDef.bullet = true;
	bodyDef.enabled = true;
	bodyDef.userData.pointer = obj.physique_id();
	bodyDef.gravityScale = 1.0f;
	b2Body* body = M2_LEVEL.world->CreateBody(&bodyDef);
	{
		b2FixtureDef fixtureDef;
		b2CircleShape circleShape;
		circleShape.m_radius = sprite.ForegroundColliderCircRadiusM();
		circleShape.m_p = static_cast<b2Vec2>(m2::VecF{});
		fixtureDef.shape = &circleShape;
		fixtureDef.friction = 0.0f; // TODO
		fixtureDef.restitution = 1.0f;
		fixtureDef.restitutionThreshold = 0.0f;
		fixtureDef.density = 0.0f; // Mass will be set statically
		fixtureDef.isSensor = false;
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_FOE_ON_FOREGROUND;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	}
	b2MassData massData{};
	massData.mass = 1.0f;
	massData.center = {};
	massData.I = 1.0f;
	body->SetMassData(&massData);
	phy.body = m2::box2d::BodyUniquePtr{body};

	phy.postStep = [](MAYBE m2::Physique& phy_) {};

	MAYBE auto& gfx = obj.add_graphic(m2g::pb::SPRITE_BASIC_BALL);

	return {};
}
