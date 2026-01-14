#include <rpg/Objects.h>
#include <m2g/Proxy.h>
#include <m2/Game.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <Character.pb.h>

m2::void_expected rpg::create_dropped_card(m2::Object &obj, const m2::VecF& position, m2g::pb::CardType card_type) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(M2_GAME.GetNamedCard(card_type).UiSprite()));

	auto& phy = obj.AddPhysique();
	phy.position = position;
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.fixtures = {m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::CircleShape::FromSpriteCircleFixture(sprite.OriginalPb().regular().fixtures(0).circle(), sprite.Ppm()),
			.isSensor = true,
			.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_FOREGROUND_FRIENDLY_CARD)]
		}},
		.allowSleeping = true,
		.initiallyAwake = false,
		.isBullet = false
	};
	phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), position, {}, m2::pb::PhysicsLayer::SEA_LEVEL);

	obj.AddGraphic(m2::pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT, M2_GAME.GetNamedCard(card_type).UiSprite()).position = position;

	phy.onCollision = [card_type](m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (auto* other_char = other.Owner().TryGetCharacter(); other_char) {
			other_char->ExecuteInteraction(std::make_unique<m2g::Proxy::Card>(card_type));
		}
		M2_DEFER(m2::CreateObjectDeleter(phy.OwnerId()));
	};

	return {};
}
