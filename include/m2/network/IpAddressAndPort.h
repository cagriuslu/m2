#pragma once
#include "../Meta.h"
#include <string>
#include <utility>
#include <cstdint>

namespace m2::network {
	using IpAddress = uint32_t;
	using Port = uint16_t;
	using IpAddressAndPort = std::pair<IpAddress,Port>;

	m2::expected<std::vector<std::string>> get_ip_addresses();
}

namespace m2 {
	std::string to_string(const network::IpAddressAndPort& ip_port);
}
