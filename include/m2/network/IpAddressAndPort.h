#pragma once
#include "../Meta.h"
#include <string>
#include <utility>
#include <cstdint>

namespace m2::network {
	expected<std::vector<std::string>> get_ip_addresses();
}
