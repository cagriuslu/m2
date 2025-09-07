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

std::array<DrawLayer, 10> m2::gDrawOrder = {
	pb::FlatGraphicsLayer::BACKGROUND_FLAT,
	pb::FlatGraphicsLayer::BEDROCK_FLAT,
	ForegroundDrawLayer::FM1_BOTTOM,
	ForegroundDrawLayer::FM1_TOP,
	pb::FlatGraphicsLayer::SEABED_FLAT,
	pb::FlatGraphicsLayer::UNDER_WATER_FLAT,
	ForegroundDrawLayer::F0_BOTTOM,
	ForegroundDrawLayer::F0_TOP,
	ForegroundDrawLayer::F1_BOTTOM,
	ForegroundDrawLayer::F1_TOP,
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
