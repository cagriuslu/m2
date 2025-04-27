#pragma once
#include <m2/containers/Pool.h>
#include <m2/math/VecF.h>
#include <m2/Meta.h>
#include <m2/M2.h>
#include <Sprite.pb.h>
#include <variant>
#include <vector>
#include <cstdint>


namespace m2::third_party::physics {
	enum class RigidBodyType {
		STATIC,
		KINEMATIC,
		DYNAMIC,
	};

	struct ColliderParams {
		// The layers that the collider itself belongs to.
		uint16_t belongsTo{};
		// The layers that the collider agreeing to colliding with. For a collision to occur, both objects must agree to
		// collide with each other. For example, for objects A and B to collide, object A's `collidesWith` must contain
		// at least one bit of object B's `belongsTo`, and object B's `collidesWith` must contain at least one bit of
		// object A's `belongsTo`.
		uint16_t collidesWith{};
	};

	struct RectangleShape {
		// Offset of the center of the rectangle from the origin of the body
		VecF offset;
		VecF dimensions;
		float angleInRads;

		static RectangleShape FromSpriteRectangleFixture(const pb::Fixture_RectangleFixture&, int ppm);
	};
	struct CircleShape {
		// Offset of the center of the circle from the origin of the body
		VecF offset;
		float radius;

		static CircleShape FromSpriteCircleFixture(const pb::Fixture_CircleFixture&, int ppm);
	};
	struct ChainShape {
		// Point coordinates with respect to the origin of the body. Points will be automatically completed into a loop.
		// At least 3 points are necessary.
		std::vector<VecF> points;

		static ChainShape FromSpriteChainFixture(const pb::Fixture_ChainFixture&, int ppm);
	};
	// Open-ended line with collisions from both sides
	struct EdgeShape {
		VecF pointA, pointB;
	};
	std::variant<RectangleShape,CircleShape,ChainShape,EdgeShape> ToShape(const pb::Fixture&, int ppm);

	struct FixtureDefinition {
		std::variant<RectangleShape,CircleShape,ChainShape,EdgeShape> shape;
		float friction{};
		float restitution{};
		float restitutionThresholdVelocity{};
		float isSensor{};
		ColliderParams colliderFilter;
	};

	struct RigidBodyDefinition {
		RigidBodyType bodyType;

		std::vector<FixtureDefinition> fixtures;

		// Properties applicable only to dynamic objects

		float linearDamping{};
		bool fixedRotation{true};
		float angularDamping{};
		float mass{};
		float inertia{};
		VecF offsetOfCenterOfMass; // Position of the center of mass with respect to the body origin
		float gravityScale{1.0};

		// Putting a body to sleep and waking it up is costly. Objects that are in constant movement shouldn't sleep.
		bool allowSleeping{true};
		bool initiallyAwake{true};
		// Prevent tunneling if the object is a bullet. Applicable to objects that move fast, and should never tunnel.
		bool isBullet{};
		bool initiallyEnabled{true};
	};

	class RigidBody {
		void* _ptr{};
		ForegroundLayer _foregroundLayer;

		RigidBody(void* ptr, const ForegroundLayer fl) : _ptr(ptr), _foregroundLayer(fl) {}

	public:
		static RigidBody CreateFromDefinition(const RigidBodyDefinition&, Id physiqueId, const VecF& position, float angleInRads, ForegroundLayer fl = ForegroundLayer::F0);
		// Copy not allowed
		RigidBody(const RigidBody& other) = delete;
		RigidBody& operator=(const RigidBody& other) = delete;
		// Move constructors
		RigidBody(RigidBody&& other) noexcept;
		RigidBody& operator=(RigidBody&& other) noexcept;
		// Destructor
		~RigidBody();

		// Accessors

		[[nodiscard]] void* GetThirdPartObject() const { return _ptr; }
		[[nodiscard]] bool IsEnabled() const;
		[[nodiscard]] VecF GetPosition() const;
		[[nodiscard]] float GetAngle() const; // In radians
		[[nodiscard]] VecF GetLinearVelocity() const;
		[[nodiscard]] float GetAngularVelocity() const;
		[[nodiscard]] bool HasJoint() const;

		[[nodiscard]] uint16_t GetAllLayersBelongingTo() const;
		[[nodiscard]] uint16_t GetAllLayersCollidingTo() const;

		// Modifiers

		void SetEnabled(bool);
		void SetPosition(const VecF&);
		void SetAngle(float angle); // In radians
		void SetLinearVelocity(const VecF&);
		void SetAngularVelocity(float w);
		void ApplyForceToCenter(const VecF&);

		/// Clone the properties (position, orientation, speed, etc.) of another body to self
		void TeleportToAnother(const RigidBody& other);
	};
}
