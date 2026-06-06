#pragma once
#include <m2/thirdparty/physics/RigidBody.h>
#include <m2/ProxyTypes.h>

namespace m2::physics {
	class DeterministicBody {
		std::function<void(DeterministicBody&)> _onStep{};

	public:
		DeterministicBody() = default;
		template <typename Op>
		explicit DeterministicBody(Op op) : _onStep(std::move(op)) {}
		static DeterministicBody CreateFromDefinition(const thirdparty::physics::RigidBodyDefinition&, Id physiqueId, const VecF& position, float angleInRads, m2g::pb::PhysicsLayer pl);
		// Copy not allowed
		DeterministicBody(const DeterministicBody& other) = delete;
		DeterministicBody& operator=(const DeterministicBody& other) = delete;
		// Move is allowed
		DeterministicBody(DeterministicBody&& other) = default;
		DeterministicBody& operator=(DeterministicBody&& other) = default;
		// Destructor
		~DeterministicBody() = default;

		// Accessors

		[[nodiscard]] bool IsEnabled() const { return static_cast<bool>(_onStep); }
		[[nodiscard]] VecF GetPosition() const;
		[[nodiscard]] FE GetAngle() const; // In radians
		[[nodiscard]] VecF GetLinearVelocity() const;
		[[nodiscard]] FE GetAngularVelocity() const;


		// Modifiers

		void SetEnabled(bool);
		void SetPosition(const VecFE&);
		void SetAngle(FE angle); // In radians
		void SetLinearVelocity(const VecFE&);
		void SetAngularVelocity(FE w);
		void ApplyForceToCenter(const VecFE&);
		void TeleportToAnother(const DeterministicBody& other);

		void OnStep() { _onStep(*this); }
	};
}
