#include <m2/M2.h>

using namespace m2;

namespace {
	uintptr_t gStoredStackPosition{};

	uintptr_t GetStackPointerValue() {
		void* p = nullptr;
		const auto ptr = reinterpret_cast<uintptr_t>(&p);
		return ptr - 8;
	}
}

const std::string m2::EMPTY_STRING;

static_assert((std::chrono::duration_cast<Stopwatch::Duration>(std::chrono::seconds{1}).count() % TIME_BETWEEN_PHYSICS_SIMULATIONS.count()) == 0,
		"Physics simulation frequency isn't a whole integer");

std::array<DrawLayer, 16> m2::gDrawOrder = {
	pb::FlatGraphicsLayer::BACKGROUND_FLAT,

	pb::FlatGraphicsLayer::BEDROCK_FLAT,
	pb::UprightGraphicsLayer::BEDROCK_UPRIGHT,

	pb::FlatGraphicsLayer::SEABED_FLAT,
	pb::UprightGraphicsLayer::SEABED_UPRIGHT,

	pb::FlatGraphicsLayer::UNDER_WATER_FLAT,
	pb::UprightGraphicsLayer::UNDER_WATER_UPRIGHT,

	pb::FlatGraphicsLayer::SEA_LEVEL_FLAT,
	pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT,

	pb::FlatGraphicsLayer::ABOVE_GROUND_FLAT,
	pb::UprightGraphicsLayer::ABOVE_GROUND_UPRIGHT,

	pb::FlatGraphicsLayer::AIRBORNE_FLAT,
	pb::UprightGraphicsLayer::AIRBORNE_UPRIGHT,

	pb::FlatGraphicsLayer::SPACE_FLAT,
	pb::UprightGraphicsLayer::SPACE_UPRIGHT,

	pb::FlatGraphicsLayer::FOREGROUND_FLAT
};

size_t m2::Utf8CodepointCount(const char* s) {
	size_t len = 0;
	while (*s) {
		len += ((*s & 0xc0) != 0x80) ? 1 : 0;
		++s;
	}
	return len;
}

void m2::StoreInitialStackPosition() {
	gStoredStackPosition = GetStackPointerValue();
}
size_t m2::GetCurrentStackPosition() {
	return gStoredStackPosition - GetStackPointerValue();
}
