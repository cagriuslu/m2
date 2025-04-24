#include <pinball/objects/Edge.h>
#include <m2/Game.h>
#include <pinball/Pinball.h>

void LoadEdge() {
	const auto it = m2::CreateObject({}, m2g::pb::NO_OBJECT);

	auto& phy = it->AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.fixtures = {
			m2::third_party::physics::FixtureDefinition{
				.shape = m2::third_party::physics::EdgeShape{.pointA = {}, .pointB = {gLevelDimensions.x, 0.0f}},
				.restitution = 1.0f,
				.colliderFilter = m2::third_party::physics::ColliderParams{
					.belongsTo = 1,
					.collidesWith = 0xFFFF
				}
			},
			m2::third_party::physics::FixtureDefinition{
				.shape = m2::third_party::physics::EdgeShape{.pointA = {}, .pointB = {0.0f, gLevelDimensions.y}},
				.restitution = 1.0f,
				.colliderFilter = m2::third_party::physics::ColliderParams{
					.belongsTo = 1,
					.collidesWith = 0xFFFF
				}
			},
			m2::third_party::physics::FixtureDefinition{
				.shape = m2::third_party::physics::EdgeShape{.pointA = {gLevelDimensions.x, 0.0f}, .pointB = {gLevelDimensions.x, gLevelDimensions.y}},
				.restitution = 1.0f,
				.colliderFilter = m2::third_party::physics::ColliderParams{
					.belongsTo = 1,
					.collidesWith = 0xFFFF
				}
			}
		},

	};
	phy.body = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, it->GetPhysiqueId(), {}, {});
}
