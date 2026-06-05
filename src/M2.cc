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
