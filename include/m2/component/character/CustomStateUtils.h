#pragma once
#include <cstdint>

namespace m2::component::character {
	/// Set of functions that needs to be implemented by any CustomState
	template <typename CustomState> struct CustomStateUtils {};
	/// Default implementation
	template <> struct CustomStateUtils<std::monostate> {
		static int32_t Hash(const std::monostate&, const int32_t initialValue) { return initialValue; }
		static void Store(const std::monostate&, pb::TurnBasedServerUpdate::ObjectDescriptor&) {}
		static void Load(std::monostate&, const pb::TurnBasedServerUpdate::ObjectDescriptor&) {}
	};
}
