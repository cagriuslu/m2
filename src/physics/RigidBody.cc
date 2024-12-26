#include <m2/physics/RigidBody.h>

m2::RigidBody::RigidBody(std::vector<RigidBodyPart> bodyParts,
		const Vec<PhysicsPrimitive> initialPositionOfCenterOfMass, PhysicsPrimitive initialOrientationAboutCenterOfMass,
		PhysicsPrimitive gravityInfluence, PhysicsPrimitive velocitySustainment,
		PhysicsPrimitive angularVelocitySustainment)
		: _parts(std::move(bodyParts)), _velocitySustainment(std::move(velocitySustainment)),
		_angularVelocitySustainment(std::move(angularVelocitySustainment)),
		_gravityInfluence(std::move(gravityInfluence)),
		_positionOfCenterOfMass(std::move(initialPositionOfCenterOfMass)),
		_orientationAboutCenterOfMass(std::move(initialOrientationAboutCenterOfMass)) {
	// Check body parts
	for (const auto& part : _parts) {
		if (part.inverseMass < PhysicsPrimitive{}) {
			throw M2_ERROR("Given body part mass is negative");
		}
		if (std::holds_alternative<RigidBodyPart::CircParams>(part.shapeParameters)) {
			if (std::get<RigidBodyPart::CircParams>(part.shapeParameters).radius < PhysicsPrimitive{}) {
				throw M2_ERROR("Given circle radius is negative");
			}
		} else if (std::holds_alternative<RigidBodyPart::RectParams>(part.shapeParameters)) {
			if (std::get<RigidBodyPart::RectParams>(part.shapeParameters).dimensions.X() < PhysicsPrimitive{}) {
				throw M2_ERROR("Given rectangle width is negative");
			}
			if (std::get<RigidBodyPart::RectParams>(part.shapeParameters).dimensions.Y() < PhysicsPrimitive{}) {
				throw M2_ERROR("Given rectangle height is negative");
			}
		}
	}
	// Check velocity sustainment
	if (_velocitySustainment < PhysicsPrimitive{} || PhysicsPrimitive{1} < _velocitySustainment) {
		throw M2_ERROR("Given velocity sustainment is out of range");
	}
	// Check angular velocity sustainment
	if (_angularVelocitySustainment < PhysicsPrimitive{} || PhysicsPrimitive{1} < _angularVelocitySustainment) {
		throw M2_ERROR("Given angular velocity sustainment is out of range");
	}
}

m2::Vec<m2::PhysicsPrimitive> m2::RigidBody::OffsetOfCenterOfMass() const {
	Vec<PhysicsPrimitive> cumulativePositionAndMass;
	PhysicsPrimitive cumulativeMass;
	for (const auto& part : _parts) {
		const auto partMass = part.inverseMass.Inverse();
		cumulativePositionAndMass += part.position * partMass;
		cumulativeMass += partMass;
	}
	return cumulativePositionAndMass / cumulativeMass;
}

void m2::RigidBody::ApplyForce(const Vec<PhysicsPrimitive>& force, const Vec<PhysicsPrimitive>& offsetFromCenterOfMass) {
	// We need to find the torque and linear components of the force. Torque component is calculated with the following
	// formula: Torque = PointWhereForceIsApplied X Force, where X is the cross product. We're only interested in the Z
	// component of torque since our engine is 2D, which is (px x fy - py x fx).
	const auto torqueComponent1 = offsetFromCenterOfMass.X() * force.Y();
	const auto torqueComponent2 = offsetFromCenterOfMass.Y() * force.X();
	const auto torque = torqueComponent1 - torqueComponent2;
	ApplyTorqueAboutCenterOfMass(torque);
	// Apply the linear component
	_accelerationOfCenterOfMass += force * InverseOfTotalMass();
}
void m2::RigidBody::ApplyTorqueAboutCenterOfMass(const PhysicsPrimitive& torqueAmount) {
	_angularAccelerationAboutCenterOfMass += torqueAmount * InverseOfMomentOfInertiaAboutCenterOfMass();
}
void m2::RigidBody::Integrate() {
	// Position is calculated with the following formula:
	//     NewPosition  =  OldPosition  +  Velocity * DeltaTime  +  1/2 * Acceleration * Square(DeltaTime)
	// First, move the particle based on the current velocity
	_positionOfCenterOfMass += _velocityOfCenterOfMass * physicsDeltaTime;
	_orientationAboutCenterOfMass += _angularVelocityAboutCenterOfMass * physicsDeltaTime;
	// Second, move the particle based on the acceleration
	// This second component is usually very small because deltaTime is usually very small.
	// If the performance becomes an issue, we can skip this component.
	const auto gravitationalAcceleration = physicsDefaultGravity * _gravityInfluence;
	const auto totalAcceleration = gravitationalAcceleration + _accelerationOfCenterOfMass;
	const auto deltaTimeSquaredAndAcceleration = totalAcceleration * physicsDeltaTimeSquared;
	_positionOfCenterOfMass += deltaTimeSquaredAndAcceleration / PhysicsPrimitive{2};
	const auto deltaTimeSquaredAndAngularAcceleration = _angularAccelerationAboutCenterOfMass * physicsDeltaTimeSquared;
	_orientationAboutCenterOfMass += deltaTimeSquaredAndAngularAcceleration / PhysicsPrimitive{2};
	// Third, update the velocity with the following formula:
	//     NewVelocity  =  OldVelocity * Damping^DeltaTime  +  Acceleration * DeltaTime
	// If the performance becomes an issue, the damping factor can be cached.
	const auto sustainmentFactor = _velocitySustainment.Power(physicsDeltaTime);
	const auto oldVelocityAndSustainmentFactor = _velocityOfCenterOfMass * sustainmentFactor;
	const auto accelerationAndDeltaTime = totalAcceleration * physicsDeltaTime;
	_velocityOfCenterOfMass = oldVelocityAndSustainmentFactor + accelerationAndDeltaTime;
	const auto angularSustainmentFactor = _angularVelocitySustainment.Power(physicsDeltaTime);
	const auto oldAngularVelocityAndAngularSustainmentFactor = _angularVelocityAboutCenterOfMass * angularSustainmentFactor;
	const auto angularAccelerationAndDeltaTime = _angularAccelerationAboutCenterOfMass * physicsDeltaTime;
	_angularVelocityAboutCenterOfMass = oldAngularVelocityAndAngularSustainmentFactor + angularAccelerationAndDeltaTime;
	// Clear the accelerations applied to the particle
	_accelerationOfCenterOfMass = {};
	_angularAccelerationAboutCenterOfMass = {};
}

m2::PhysicsPrimitive m2::RigidBody::InverseOfTotalMass() const {
	PhysicsPrimitive cumulativeMass;
	for (const auto& part : _parts) {
		// If any of the body parts have infinite mass, immediately return infinite mass
		if (part.inverseMass == PhysicsPrimitive::Zero()) {
			return PhysicsPrimitive::Zero();
		}
		cumulativeMass += part.inverseMass.Inverse();
	}
	return cumulativeMass.Inverse();
}
m2::PhysicsPrimitive m2::RigidBody::InverseOfMomentOfInertiaAboutCenterOfMass() const {
	// TODO make a proper implementation
	const auto offsetOfCenterOfMass = OffsetOfCenterOfMass();
	PhysicsPrimitive cumulativeMomentOfInertia;
	for (const auto& part : _parts) {
		// If any of the body parts have infinite mass, immediately return infinite moment of inertia
		if (part.inverseMass == PhysicsPrimitive::Zero()) {
			return PhysicsPrimitive::Zero();
		}
		// For now, assume that each body part is a particle
		const auto partOriginToCenterOfMassVec = part.position - offsetOfCenterOfMass;
		const auto partDistanceSquareToCenterOfMass = partOriginToCenterOfMassVec.MagnitudeSquare();
		const auto partMass = part.inverseMass.Inverse();
		cumulativeMomentOfInertia += partMass * partDistanceSquareToCenterOfMass;
	}
	return cumulativeMomentOfInertia;
}
