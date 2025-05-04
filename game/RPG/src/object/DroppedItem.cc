#include <rpg/Objects.h>
#include <Item.pb.h>
#include <m2/Game.h>
#include <m2/third_party/physics/ColliderCategory.h>

m2::void_expected rpg::create_dropped_item(m2::Object &obj, m2g::pb::ItemType item_type) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(M2_GAME.GetNamedItem(item_type).UiSprite()));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.fixtures = {m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::CircleShape::FromSpriteCircleFixture(sprite.OriginalPb().regular().fixtures(0).circle(), sprite.Ppm()),
			.isSensor = true,
			.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_FOREGROUND_FRIENDLY_ITEM)]
		}},
		.allowSleeping = true,
		.initiallyAwake = false,
		.isBullet = false
	};
	phy.body[I(m2::ForegroundLayer::F0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation);

	obj.AddGraphic(M2_GAME.GetNamedItem(item_type).UiSprite());

	phy.onCollision = [item_type](m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (auto* other_char = other.Owner().TryGetCharacter(); other_char) {
			m2g::pb::InteractionData data;
			data.set_item_type(item_type);
			other_char->ExecuteInteraction(data);
		}
		M2_DEFER(m2::CreateObjectDeleter(phy.OwnerId()));
	};

	return {};
}
