#include <m2/M2.h>

const std::string m2::gEmptyString;

namespace {
	uintptr_t gStoredStackPosition{};

	uintptr_t GetStackPointerValue() {
		void* p = nullptr;
		const auto ptr = reinterpret_cast<uintptr_t>(&p);
		return ptr - 8;
	}
}

std::string m2::ToString(const BackgroundLayer layer) {
	return "B" + ToString(I(layer));
}
std::string m2::ToString(const ForegroundLayer layer) {
	return "F" + ToString(I(layer));
}

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
