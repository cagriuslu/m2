#include <m2/ProxyDetail.h>

int32_t m2::game_hash() {
	auto hash = std::hash<std::string_view>{}(m2g::game_name);
	return I(hash);
}
