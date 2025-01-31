#include <pinball/objects/Flipper.h>
#include <m2/Game.h>
#include <box2d/b2_polygon_shape.h>
#include <m2/box2d/Detail.h>

m2::void_expected LoadFlipper(m2::Object& obj, const bool rightFlipper) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(rightFlipper ? m2g::pb::SPRITE_BASIC_FLIPPER_RIGHT : m2g::pb::SPRITE_BASIC_FLIPPER_LEFT));

	auto& phy = obj.add_physique();
	b2BodyDef bodyDef;
	bodyDef.type = b2_kinematicBody;
	bodyDef.position.Set(obj.position.x, obj.position.y);
	bodyDef.angle = obj.orientation;
	bodyDef.linearVelocity = {};
	bodyDef.angularVelocity = 0.0f;
	bodyDef.linearDamping = 0.1f;
	bodyDef.angularDamping = 0.1f;
	bodyDef.allowSleep = true;
	bodyDef.awake = true;
	bodyDef.fixedRotation = false;
	bodyDef.bullet = true;
	bodyDef.enabled = true;
	bodyDef.userData.pointer = obj.physique_id();
	bodyDef.gravityScale = 0.0f;
	b2Body* body = M2_LEVEL.world->CreateBody(&bodyDef);
	{
		// Top edge
		b2FixtureDef fixtureDef;
		b2PolygonShape polygonShape;
		polygonShape.SetAsBox(sprite.ForegroundColliderRectDimsM().x / 2.0f,
				sprite.ForegroundColliderRectDimsM().y / 2.0f,
				static_cast<b2Vec2>(sprite.ForegroundColliderOriginToOriginVecM()),
				obj.orientation);
		fixtureDef.shape = &polygonShape;
		fixtureDef.friction = 0.1f;
		fixtureDef.restitution = 1.0f;
		fixtureDef.restitutionThreshold = 0.0f;
		fixtureDef.density = 0.0f; // Kinematic object doesn't need mass
		fixtureDef.isSensor = false;
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	}
	phy.body = m2::box2d::BodyUniquePtr{body};

	MAYBE auto& gfx = obj.add_graphic(rightFlipper ? m2g::pb::SPRITE_BASIC_FLIPPER_RIGHT : m2g::pb::SPRITE_BASIC_FLIPPER_LEFT);

	if (rightFlipper) {
		phy.pre_step = [](m2::Physique& phy_) {
			if (M2_GAME.events.pop_key_press(m2::Key::RIGHT)) {
				phy_.body->SetAngularVelocity(2);
			}
		};
	} else {
		phy.pre_step = [](m2::Physique& phy_) {
			if (M2_GAME.events.pop_key_press(m2::Key::LEFT)) {
				phy_.body->SetAngularVelocity(-2);
			}
		};
	}

	return {};
}
