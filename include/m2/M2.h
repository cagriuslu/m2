#pragma once
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

	constexpr auto FLAT_GRAPHICS_LAYERS = {
		pb::Layer::BACKGROUND_FLAT,
		pb::Layer::BEDROCK_FLAT,
		pb::Layer::SEABED_FLAT,
		pb::Layer::UNDER_WATER_FLAT,
		pb::Layer::SEA_LEVEL_FLAT,
		pb::Layer::ABOVE_GROUND_FLAT,
		pb::Layer::AIRBORNE_FLAT,
		pb::Layer::SPACE_FLAT,
		pb::Layer::FOREGROUND_FLAT,
	};
	constexpr int FLAT_GRAPHICS_LAYER_COUNT = FLAT_GRAPHICS_LAYERS.size();

	constexpr auto UPRIGHT_GRAPHICS_LAYERS = {
		pb::Layer::BEDROCK_UPRIGHT,
		pb::Layer::SEABED_UPRIGHT,
		pb::Layer::UNDER_WATER_UPRIGHT,
		pb::Layer::SEA_LEVEL_UPRIGHT,
		pb::Layer::ABOVE_GROUND_UPRIGHT,
		pb::Layer::AIRBORNE_UPRIGHT,
		pb::Layer::SPACE_UPRIGHT,
	};
	constexpr int UPRIGHT_GRAPHICS_LAYER_COUNT = UPRIGHT_GRAPHICS_LAYERS.size();

	// Pairs belong to the same physical world
	constexpr auto PHYSICS_LAYER_PAIRS = {
		std::array{pb::Layer::BEDROCK_FLAT, pb::Layer::BEDROCK_UPRIGHT},
		std::array{pb::Layer::SEABED_FLAT, pb::Layer::SEABED_UPRIGHT},
		std::array{pb::Layer::UNDER_WATER_FLAT, pb::Layer::UNDER_WATER_UPRIGHT},
		std::array{pb::Layer::SEA_LEVEL_FLAT, pb::Layer::SEA_LEVEL_UPRIGHT},
		std::array{pb::Layer::ABOVE_GROUND_FLAT, pb::Layer::ABOVE_GROUND_UPRIGHT},
		std::array{pb::Layer::AIRBORNE_FLAT, pb::Layer::AIRBORNE_UPRIGHT},
		std::array{pb::Layer::SPACE_FLAT, pb::Layer::SPACE_UPRIGHT},
	};
	constexpr int PHYSICS_LAYER_COUNT = PHYSICS_LAYER_PAIRS.size();

	/// Each physics layer designates a separate physical world. Objects in different layers cannot collide.
	enum class PhysicsLayer {
		PM1, // Underground in a top-down game
		P0, // Ground level in a top-down game
		P1, // Above ground in a top-down game
		_n // End sentinel
	};
	constexpr int gPhysicsLayerCount = static_cast<int>(PhysicsLayer::_n);

	/// Background layers are graphics-only layers. An object can have a Graphic component in any of the background
	/// layers. Background layers are drawn from back to front and B0 is drawn the last, thus it's on the front.
	/// Background Graphic components are not ordered, thus they should not overlap.
	enum class BackgroundDrawLayer {
		B0,
		B1,
		B2,
		B3,
		_n // End sentinel
	};
	constexpr int gBackgroundDrawLayerCount = static_cast<int>(BackgroundDrawLayer::_n);

	enum class ForegroundDrawLayer {
		FM1_BOTTOM,
		FM1_TOP,
		F0_BOTTOM,
		F0_TOP,
		F1_BOTTOM,
		F1_TOP,
		_n, // End sentinel
	};
	constexpr int gForegroundDrawLayerCount = static_cast<int>(ForegroundDrawLayer::_n);

	using DrawLayer = std::variant<BackgroundDrawLayer, ForegroundDrawLayer>;
	extern std::array<DrawLayer, 10> gDrawOrder;

	/// Returns the number of codepoints in a UTF-8 string.
	/// One codepoint does not always equate to one glyph, but it's a good estimation for most characters.
	size_t Utf8CodepointCount(const char* s);

	/// This function is called by main to store the initial stack position.
	void StoreInitialStackPosition();
	/// This function can be called to query the current stack position (the difference between initial and current stack pointers).
	size_t GetCurrentStackPosition();
}
