#include <pinball/objects/Wall.h>
#include <m2/box2d/Shape.h>
#include <m2/Game.h>

m2::void_expected LoadWall(m2::Object& obj) {
	const auto type = obj.GetType();
	const auto spriteType = M2_GAME.object_main_sprites[type];
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.fixtures = {
			m2::third_party::physics::FixtureDefinition{
				.shape = m2::third_party::physics::ToShape(sprite.OriginalPb().regular().fixtures(0), sprite.Ppm()),
				.restitution = 1.0f,
				.colliderFilter = m2::third_party::physics::ColliderParams{
					.belongsTo = 1,
					.collidesWith = 0xFFFF
				}
			}
		}
	};
	phy.body = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation);

	obj.AddGraphic(spriteType);

	return {};
}
