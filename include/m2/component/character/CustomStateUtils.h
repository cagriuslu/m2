#pragma once
#include <cstdint>

namespace m2::component::character {
	/// Set of functions that needs to be implemented by any CustomState
	template <typename CustomState>
	struct CustomStateUtils {
		static int32_t Hash(const CustomState&, const int32_t initialValue) { return initialValue; }
		static void Store(const CustomState&, pb::TurnBasedServerUpdate::ObjectDescriptor&) {}
		static void Load(CustomState&, const pb::TurnBasedServerUpdate::ObjectDescriptor&) {}
	};
}
