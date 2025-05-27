#include <pinball/Objects.h>
#include <m2/Game.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <m2/Log.h>

namespace {
	bool IsDown(const m2::ObjectId id) {
		return std::get<const m2::Sprite*>(M2_LEVEL.objects[id].GetGraphic().visual)->Type() == m2g::pb::SPRITE_DROP_GATE_DOWN;
	}

	void DropGate(m2::Graphic& gfx) {
		static const auto* downSprite = &std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPRITE_DROP_GATE_DOWN));

		if (std::get<const m2::Sprite*>(gfx.visual)->Type() == m2g::pb::SPRITE_DROP_GATE_UP) {
			gfx.visual = downSprite;
		}
	}
	void RaiseGate(const m2::ObjectId objId) {
		static const auto* upSprite = &std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPRITE_DROP_GATE_UP));

		auto& gfx = M2_LEVEL.objects[objId].GetGraphic();
		if (std::get<const m2::Sprite*>(gfx.visual)->Type() == m2g::pb::SPRITE_DROP_GATE_DOWN) {
			gfx.visual = upSprite;
		}
	}
}

m2::void_expected LoadDropGate(m2::Object& obj) {
	const auto type = obj.GetType();
	const auto spriteType = *M2_GAME.GetMainSpriteOfObject(type);
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.isBullet = true
	};
	const auto& fixturePb = sprite.OriginalPb().regular().fixtures(0);
	rigidBodyDef.fixtures.emplace_back(m2::third_party::physics::FixtureDefinition{
		.shape = m2::third_party::physics::ToShape(fixturePb, sprite.Ppm()),
		.isSensor = true,
		.colliderFilter = m2::third_party::physics::ColliderParams{
			.belongsTo = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT,
			.collidesWith = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT
		}
	});
	phy.body[I(m2::ForegroundLayer::F0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef,
		obj.GetPhysiqueId(), obj.position, obj.orientation);

	auto& gfx = obj.AddGraphic(spriteType);

	phy.onCollision = [&obj, &gfx](m2::Physique&, m2::Physique&, const m2::box2d::Contact&) {
		DropGate(gfx);

		auto& group = *obj.TryGetGroup();
		const auto allDown = std::ranges::all_of(group, [](auto memberObjectId) {
			return IsDown(memberObjectId);
		});
		if (allDown) {
			LOG_INFO("Raising the gate");
			std::ranges::for_each(group, [](auto memberId) {
				RaiseGate(memberId);
			});
		}
	};

	return {};
}
