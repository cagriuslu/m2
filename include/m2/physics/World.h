#pragma once
#include <m2/physics/RigidBody.h>
#include <vector>

namespace m2 {
	class World {
		std::vector<RigidBody> _rigidBodies;

	public:
		World() = default;

		// Accessors

		[[nodiscard]] const RigidBody& GetRigidBody(size_t index) const;

		// Modifiers

		/// Adds a new rigid body to the world, and returns it's index. Since rigid bodies cannot be removed yet, the
		/// index uniquely describes the body.
		template <typename ...Args>
		size_t AddRigidBody(Args... args) {
			_rigidBodies.emplace_back(std::forward<Args>(args)...);
			return _rigidBodies.size() - 1;
		}
		RigidBody& GetRigidBody(size_t index);
		void Integrate();
	};
}
