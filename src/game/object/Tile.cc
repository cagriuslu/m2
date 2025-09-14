#include <m2/game/object/Tile.h>
#include <m2/Game.h>
#include <m2/Object.h>
#include <m2/third_party/physics/RigidBody.h>
#include <M2.pb.h>

using namespace google::protobuf;
using namespace m2::third_party::physics;

namespace {
	std::vector<FixtureDefinition> ToFixtures(const RepeatedPtrField<m2::pb::Fixture>& pbFixtures,
			const m2g::pb::SpriteType spriteType, int ppm) {
		std::vector<FixtureDefinition> fixtures(pbFixtures.size());
		std::ranges::transform(pbFixtures, fixtures.begin(), [=](const auto& pbFixture) {
			FixtureDefinition fixtureDef = M2G_PROXY.TileFixtureDefinition(spriteType);
			fixtureDef.shape = ToShape(pbFixture, ppm);
			return fixtureDef;
		});
		return fixtures;
	}
}

m2::Pool<m2::Object>::Iterator m2::obj::CreateTile(const pb::FlatGraphicsLayer layer, const VecF& position, const m2g::pb::SpriteType spriteType) {
    const auto it = CreateObject();
	it->AddGraphic(layer, spriteType, position);

	if (const auto& spriteOrTextLabel = M2_GAME.GetSpriteOrTextLabel(spriteType);
			std::holds_alternative<Sprite>(spriteOrTextLabel)) {
		const auto& sprite = std::get<Sprite>(spriteOrTextLabel);

		// Add collider if necessary
		if (sprite.OriginalPb().regular().fixtures_size()) {
			const RigidBodyDefinition rigidBodyDef{
				.bodyType = RigidBodyType::STATIC,
				.fixtures = ToFixtures(sprite.OriginalPb().regular().fixtures(), spriteType, sprite.Ppm()),
				.allowSleeping = true,
				.initiallyAwake = false,
				.isBullet = false,
				.initiallyEnabled = true
			};
			auto& phy = it->AddPhysique();
			phy.body[I(pb::PhysicsLayer::SEA_LEVEL)] = third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, it->GetPhysiqueId(), position, 0.0f, pb::PhysicsLayer::SEA_LEVEL);
		}

		// Add foreground companion if necessary
		if (sprite.HasForegroundCompanion()) {
			const auto fg_it = CreateObject();
			auto& gfx = fg_it->AddGraphic(pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT, spriteType,
				position - sprite.CenterToOriginVecM() + sprite.ForegroundCompanionCenterToOriginVecM());
			gfx.drawForegroundCompanion = true;
		}
	}

    return it;
}
