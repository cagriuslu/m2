#pragma once
#include <m2/third_party/physics/RigidBody.h>

/// Only fixture needs to be added
inline m2::third_party::physics::RigidBodyDefinition BasicBulletRigidBodyDefinition() {
	using namespace m2::third_party::physics;
	return RigidBodyDefinition{
		.bodyType = RigidBodyType::KINEMATIC,
		.allowSleeping = false,
		.initiallyAwake = true,
		.isBullet = true
	};
}
