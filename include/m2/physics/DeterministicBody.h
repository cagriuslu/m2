#pragma once
#include <m2/thirdparty/physics/RigidBody.h>
#include <m2/math/VecE.h>
#include <m2/ProxyTypes.h>

namespace m2::physics {
	class DeterministicBody {
		std::function<void(DeterministicBody&)> _onStep{};
		VecFE _position{};
		FE _orientation{}; // In radians

	public:
		DeterministicBody() = default;
		template <typename Op>
		explicit DeterministicBody(Op op, VecFE pos = {}, FE angle = {})
		    : _onStep(std::move(op)), _position(pos), _orientation(angle) {}
		static DeterministicBody CreateFromDefinition(const thirdparty::physics::RigidBodyDefinition&, Id physiqueId, const VecF& position, float angleInRads);
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
		[[nodiscard]] VecFE GetPosition() const { return _position; }
		[[nodiscard]] FE GetAngle() const { return _orientation; } // In radians
		[[nodiscard]] VecFE GetLinearVelocity() const;
		[[nodiscard]] FE GetAngularVelocity() const;


		// Modifiers

		void SetEnabled(bool);
		void SetPosition(const VecFE& position_) { _position = position_; }
		void SetAngle(const FE angle) { _orientation = angle; } // In radians
		void SetLinearVelocity(const VecFE&);
		void SetAngularVelocity(FE w);
		void ApplyForceToCenter(const VecFE&);
		void TeleportToAnother(const DeterministicBody& other);

		void OnStep() { _onStep(*this); }
	};
}
