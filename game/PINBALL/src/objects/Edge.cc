#include <pinball/Objects.h>
#include <m2/Game.h>
#include <pinball/Pinball.h>

void LoadEdge() {
	const auto it = m2::CreateObject(m2g::pb::NO_OBJECT);

	auto& phy = it->AddPhysique();
	m2::thirdparty::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::thirdparty::physics::RigidBodyType::STATIC,
		.fixtures = {
			m2::thirdparty::physics::FixtureDefinition{
				.shape = m2::thirdparty::physics::EdgeShape{.pointA = {}, .pointB = {gLevelDimensions.GetX(), 0.0f}},
				.restitution = 1.0f,
				.colliderFilter = m2::thirdparty::physics::ColliderParams{
					.belongsTo = gPinballSharedLayer,
					.collidesWith = 0xFFFF
				}
			},
			m2::thirdparty::physics::FixtureDefinition{
				.shape = m2::thirdparty::physics::EdgeShape{.pointA = {}, .pointB = {0.0f, gLevelDimensions.GetY()}},
				.restitution = 1.0f,
				.colliderFilter = m2::thirdparty::physics::ColliderParams{
					.belongsTo = gPinballSharedLayer,
					.collidesWith = 0xFFFF
				}
			},
			m2::thirdparty::physics::FixtureDefinition{
				.shape = m2::thirdparty::physics::EdgeShape{.pointA = {gLevelDimensions.GetX(), 0.0f}, .pointB = {gLevelDimensions.GetX(), gLevelDimensions.GetY()}},
				.restitution = 1.0f,
				.colliderFilter = m2::thirdparty::physics::ColliderParams{
					.belongsTo = gPinballSharedLayer,
					.collidesWith = 0xFFFF
				}
			}
		},

	};
	phy.body = m2::thirdparty::physics::RigidBody::CreateFromDefinition(rigidBodyDef, it->GetPhysiqueId(), {}, {});
}
