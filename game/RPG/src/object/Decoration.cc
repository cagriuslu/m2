#include <rpg/Objects.h>
#include <m2/Game.h>
#include <m2/thirdparty/physics/ColliderCategory.h>

m2::void_expected rpg::create_decoration(m2::Object& obj, const m2::VecF& position, m2g::pb::SpriteType sprite_type) {
	const auto orientation = obj.GetType() == m2g::pb::FENCE_VERTICAL ? m2::PI_DIV2 : 0.0f;

	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(sprite_type));
	[[maybe_unused]] auto& gfx = obj.AddGraphic(m2g::pb::UprightGraphicsLayer::UPRIGHT_GRAPHICS_DEFAULT_LAYER, sprite_type);
	gfx.position = position;
	gfx.orientation = orientation;

	m2::thirdparty::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::thirdparty::physics::RigidBodyType::STATIC,
		.allowSleeping = true,
		.initiallyAwake = false,
		.isBullet = false
	};

	if (sprite.OriginalPb().regular().fixtures_size()) {
		if (sprite.OriginalPb().regular().fixtures(0).has_rectangle()) {
			auto& phy = obj.AddPhysique();
			rigidBodyDef.fixtures = {m2::thirdparty::physics::FixtureDefinition{
				.shape = m2::thirdparty::physics::RectangleShape::FromSpriteRectangleFixture(sprite.OriginalPb().regular().fixtures(0).rectangle(), sprite.Ppm()),
				.colliderFilter = m2::thirdparty::physics::gColliderCategoryToParams[m2::I(m2::thirdparty::physics::ColliderCategory::COLLIDER_CATEGORY_BACKGROUND_OBSTACLE)]
			}};
			phy.body = m2::thirdparty::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), position, orientation);
		} else if (sprite.OriginalPb().regular().fixtures(0).has_circle()) {
			auto& phy = obj.AddPhysique();
			rigidBodyDef.fixtures = {m2::thirdparty::physics::FixtureDefinition{
				.shape = m2::thirdparty::physics::CircleShape::FromSpriteCircleFixture(sprite.OriginalPb().regular().fixtures(0).circle(), sprite.Ppm()),
				.colliderFilter = m2::thirdparty::physics::gColliderCategoryToParams[m2::I(m2::thirdparty::physics::ColliderCategory::COLLIDER_CATEGORY_BACKGROUND_OBSTACLE)]
			}};
			phy.body = m2::thirdparty::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), position, orientation);
		}
	}

	return {};
}
