#include <rpg/Objects.h>
#include <m2/Game.h>
#include <m2/third_party/physics/ColliderCategory.h>

m2::void_expected rpg::create_decoration(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	if (obj.GetType() == m2g::pb::FENCE_VERTICAL) {
		obj.orientation = m2::PI_DIV2;
	}

	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(sprite_type));
	[[maybe_unused]] auto& gfx = obj.AddGraphic(m2::ForegroundDrawLayer::F0_BOTTOM, sprite_type);

	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.allowSleeping = true,
		.initiallyAwake = false,
		.isBullet = false
	};

	if (sprite.OriginalPb().regular().fixtures_size()) {
		if (sprite.OriginalPb().regular().fixtures(0).has_rectangle()) {
			auto& phy = obj.AddPhysique();
			rigidBodyDef.fixtures = {m2::third_party::physics::FixtureDefinition{
				.shape = m2::third_party::physics::RectangleShape::FromSpriteRectangleFixture(sprite.OriginalPb().regular().fixtures(0).rectangle(), sprite.Ppm()),
				.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_BACKGROUND_OBSTACLE)]
			}};
			phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::pb::PhysicsLayer::SEA_LEVEL);
		} else if (sprite.OriginalPb().regular().fixtures(0).has_circle()) {
			auto& phy = obj.AddPhysique();
			rigidBodyDef.fixtures = {m2::third_party::physics::FixtureDefinition{
				.shape = m2::third_party::physics::CircleShape::FromSpriteCircleFixture(sprite.OriginalPb().regular().fixtures(0).circle(), sprite.Ppm()),
				.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_BACKGROUND_OBSTACLE)]
			}};
			phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::pb::PhysicsLayer::SEA_LEVEL);
		}
	}

	return {};
}
