#include <m2/physics/Particle.h>
#include <m2/Error.h>

m2::Particle::Particle(const PhysicsPrimitive mass, const Vec<PhysicsPrimitive> initialPosition,
		const Vec<PhysicsPrimitive> gravitationalAcceleration, const PhysicsPrimitive damping)
		: _inverseMass(mass.Inverse()), _damping(damping), _position(initialPosition),
		_gravitationalAcceleration(gravitationalAcceleration) {
	// Make sure the mass isn't negative
	if (_inverseMass < PhysicsPrimitive{}) {
		throw M2_ERROR("Given mass in negative");
	}
	// Make sure the damping is in [0,1] range
	if (_damping < PhysicsPrimitive{} || PhysicsPrimitive{1} < _damping) {
		throw M2_ERROR("Given damping is out of range");
	}
}

void m2::Particle::ApplyForce(const Vec<PhysicsPrimitive>& force) {
	_acceleration += force * _inverseMass;
}
void m2::Particle::Integrate() {
	// Position is calculated with the following formula:
	//     NewPosition  =  OldPosition  +  Velocity * DeltaTime  +  1/2 * Acceleration * Square(DeltaTime)
	// First, move the particle based on the current velocity
	_position += _velocity * physicsDeltaTime;
	// Second, move the particle based on the acceleration
	// This second component is usually very small because deltaTime is usually very small.
	// If the performance becomes an issue, we can skip this component.
	const auto totalAcceleration = _gravitationalAcceleration + _acceleration;
	const auto deltaTimeSquaredAndAcceleration = totalAcceleration * physicsDeltaTimeSquared;
	_position += deltaTimeSquaredAndAcceleration / PhysicsPrimitive{2};
	// Third, update the velocity with the following formula:
	//     NewVelocity  =  OldVelocity * Damping^DeltaTime  +  Acceleration * DeltaTime
	// If the performance becomes an issue, the damping factor can be cached.
	const auto dampingFactor = _damping.Power(physicsDeltaTime);
	const auto oldVelocityAndDampingFactor = _velocity * dampingFactor;
	const auto accelerationAndDeltaTime = totalAcceleration * physicsDeltaTime;
	_velocity = oldVelocityAndDampingFactor + accelerationAndDeltaTime;
	// Clear the acceleration applied to the particle
	_acceleration = {};
}
