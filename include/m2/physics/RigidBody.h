#pragma once
#include <m2/math/composites/Vec.h>
#include <m2/physics/Detail.h>
#include <vector>
#include <variant>

namespace m2 {
	struct RigidBodyPart {
		struct CircParams {
			const PhysicsPrimitive radius;
		};
		struct RectParams {
			const Vec<PhysicsPrimitive> dimensions;
		};

		/// Inverse of the mass of the body part. If zero, the mass is infinite and the body can't be moved at all.
		const PhysicsPrimitive inverseMass;
		/// Linear offset from the origin of the body. Origin of the body is at an arbitrary location.
		const Vec<PhysicsPrimitive> position;
		/// Angular offset from the orientation of the body.
		const PhysicsPrimitive orientation;
		/// Shape parameters
		const std::variant<CircParams, RectParams> shapeParameters;
	};

	class RigidBody {
		/// Body parts
		const std::vector<RigidBodyPart> _parts;
		/// The percentage of linear velocity sustained after 1 second. Valid range of values are [0,1].
		const PhysicsPrimitive _velocitySustainment;
		/// The percentage of angular velocity sustained after 1 second. Valid range of values are [0,1].
		const PhysicsPrimitive _angularVelocitySustainment;
		/// The percentage of the gravity the body will experience at its center of mass
		const PhysicsPrimitive _gravityInfluence;

		/// The position of the center of mass of the body in the 2D Cartesian space. Even though the object has an
		/// origin, the location of the center of mass is used during physics calculations. The origin and the position
		/// of the body parts can be calculated using the center of mass.
		Vec<PhysicsPrimitive> _positionOfCenterOfMass;
		/// The orientation of the body in radians about the center of mass
		PhysicsPrimitive _orientationAboutCenterOfMass;

		/// The linear velocity of the body
		Vec<PhysicsPrimitive> _velocityOfCenterOfMass{};
		/// The angular velocity of the body
		PhysicsPrimitive _angularVelocityAboutCenterOfMass{};

		/// The cumulative linear acceleration the body will experience during the next integration, excluding gravity.
		Vec<PhysicsPrimitive> _accelerationOfCenterOfMass{};
		/// The cumulative angular acceleration the body will experience during the next integration.
		PhysicsPrimitive _angularAccelerationAboutCenterOfMass{};

	public:
		RigidBody(std::vector<RigidBodyPart> bodyParts, Vec<PhysicsPrimitive> initialPositionOfCenterOfMass,
				PhysicsPrimitive initialOrientationAboutCenterOfMass, PhysicsPrimitive gravityInfluence = {},
				PhysicsPrimitive velocitySustainment = physicsDefaultVelocitySustainment,
				PhysicsPrimitive angularVelocitySustainment = physicsDefaultAngularVelocitySustainment);
		// Copy not allowed
		RigidBody(const RigidBody& other) = delete;
		RigidBody& operator=(const RigidBody& other) = delete;
		// Move constructors
		RigidBody(RigidBody&& other) noexcept = default;
		// Destructor
		~RigidBody() = default;

		// Accessors

		/// Offset of the center of mass from the origin of the object. The origin of the object is arbitrary. The
		/// location of the center of mass is determined based on the body parts.
		Vec<PhysicsPrimitive> OffsetOfCenterOfMass() const;
		/// Position of the center of mass in the physics world.
		Vec<PhysicsPrimitive> PositionOfCenterOfMass() const { return _positionOfCenterOfMass; }

		// Modifiers

		void ApplyForce(const Vec<PhysicsPrimitive>& force, const Vec<PhysicsPrimitive>& offsetFromCenterOfMass = {});
		void ApplyTorqueAboutCenterOfMass(const PhysicsPrimitive& torqueAmount);
		void Integrate();

	private:
		PhysicsPrimitive InverseOfTotalMass() const;
		PhysicsPrimitive InverseOfMomentOfInertiaAboutCenterOfMass() const;
	};
}
