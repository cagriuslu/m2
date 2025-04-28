#include <pinball/objects/Wall.h>
#include <m2/box2d/Shape.h>
#include <m2/Game.h>

m2::void_expected LoadWall(m2::Object& obj) {
	const auto type = obj.GetType();
	const auto spriteType = *M2_GAME.GetMainSpriteOfObject(type);
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC
	};
	for (const auto& fixturePb : sprite.OriginalPb().regular().fixtures()) {
		rigidBodyDef.fixtures.emplace_back(m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::ToShape(fixturePb, sprite.Ppm()),
			.friction = 0.2f,
			.restitution = 0.72f,
			.restitutionThresholdVelocity = 0.2f,
			.colliderFilter = m2::third_party::physics::ColliderParams{
				.belongsTo = 1,
				.collidesWith = 0xFFFF
			}
		});
	}
	phy.body[I(m2::ForegroundLayer::F0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation);

	obj.AddGraphic(spriteType);

	return {};
}
