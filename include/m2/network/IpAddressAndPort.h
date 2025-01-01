#pragma once
#include "../Meta.h"
#include <string>
#include <utility>
#include <cstdint>

namespace m2::network {
	/// In network byte order
	using IpAddress = uint32_t;
	/// In network byte order
	using Port = uint16_t;
	using IpAddressAndPort = std::pair<IpAddress,Port>;

	expected<std::vector<std::string>> get_ip_addresses();
}

namespace m2 {
	std::string ToString(const network::IpAddressAndPort& ip_port);
}
