#pragma once
#include <m2/protobuf/Detail.h>
#include "Chrono.h"
#include "Meta.h"
#include <M2.pb.h>
#include <array>

#define MAYBE [[maybe_unused]] // TODO get rid of this
#define IF(cond) if (cond) cond // TODO make this a function
#define IF_TYPE(ref, type) if (std::holds_alternative<type>(ref)) std::get<type>(ref) // TODO make this a function

namespace m2 {
	extern const std::string EMPTY_STRING;

	constexpr auto TIME_BETWEEN_PHYSICS_SIMULATIONS{std::chrono::duration_cast<Stopwatch::Duration>(std::chrono::milliseconds{10})};
	constexpr auto TIME_BETWEEN_FPS_LOGS{std::chrono::duration_cast<Stopwatch::Duration>(std::chrono::seconds{10})};

	constexpr int FLAT_GRAPHICS_LAYER_COUNT = pb::FlatGraphicsLayer_ARRAYSIZE;
	constexpr int UPRIGHT_GRAPHICS_LAYER_COUNT = pb::UprightGraphicsLayer_ARRAYSIZE;
	constexpr int PHYSICS_LAYER_COUNT = pb::PhysicsLayer_ARRAYSIZE;

	using DrawLayer = std::variant<pb::FlatGraphicsLayer, pb::UprightGraphicsLayer>;
	extern std::array<DrawLayer, 16> gDrawOrder;

	/// Returns the number of codepoints in a UTF-8 string.
	/// One codepoint does not always equate to one glyph, but it's a good estimation for most characters.
	size_t Utf8CodepointCount(const char* s);

	/// This function is called by main to store the initial stack position.
	void StoreInitialStackPosition();
	/// This function can be called to query the current stack position (the difference between initial and current stack pointers).
	size_t GetCurrentStackPosition();
}
