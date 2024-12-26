#pragma once
#include <m2/math/composites/Vec.h>
#include <m2/physics/Detail.h>

namespace m2 {
	class Particle {
		/// Inverse of the mass of the particle, 1/m. Zero means a particle with infinite, un-moveable mass. Infinite
		/// should not be used as a value.
		const PhysicsPrimitive _inverseMass;
		/// Amount of damping applied to linear motion. Valid range of values are [0,1]. When zero, the object cannot
		/// sustain any velocity. When one, the object sustains its velocity perfectly.
		const PhysicsPrimitive _damping;
		/// The position of the particle in the 2D Cartesian space
		Vec<PhysicsPrimitive> _position;
		/// The velocity of the particle
		Vec<PhysicsPrimitive> _velocity;
		/// The continuous acceleration the particle will experience regardless of the user defined forces.
		const Vec<PhysicsPrimitive> _gravitationalAcceleration;
		/// The cumulative acceleration the particle will experience during the next integration.
		Vec<PhysicsPrimitive> _acceleration;

	public:
		explicit Particle(PhysicsPrimitive mass, Vec<PhysicsPrimitive> initialPosition, Vec<PhysicsPrimitive> gravitationalAcceleration = {}, PhysicsPrimitive damping = physicsDefaultVelocitySustainment);
		// Copy not allowed
		Particle(const Particle& other) = delete;
		Particle& operator=(const Particle& other) = delete;
		// Move constructors
		Particle(Particle&& other) noexcept = default;
		// Destructor
		~Particle() = default;

		// Accessors

		[[nodiscard]] const Vec<PhysicsPrimitive>& Position() const { return _position; }
		[[nodiscard]] const Vec<PhysicsPrimitive>& Velocity() const { return _velocity; }
		[[nodiscard]] Vec<PhysicsPrimitive> Acceleration() const { return _gravitationalAcceleration + _acceleration; }

		// Modifiers

		/// Applies a new force to the particle.
		void ApplyForce(const Vec<PhysicsPrimitive>& force);
		/// Advances the simulation for the particle and clears the non-persistent acceleration.
		void Integrate();
	};
}
