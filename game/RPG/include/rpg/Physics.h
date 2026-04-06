#pragma once
#include <m2/thirdparty/physics/RigidBody.h>

/// Only fixture needs to be added
inline m2::thirdparty::physics::RigidBodyDefinition BasicBulletRigidBodyDefinition() {
	using namespace m2::thirdparty::physics;
	return RigidBodyDefinition{
		.bodyType = RigidBodyType::KINEMATIC,
		.allowSleeping = false,
		.initiallyAwake = true,
		.isBullet = true
	};
}
