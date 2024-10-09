#pragma once
#include "../Meta.h"
#include <sys/select.h>

namespace m2 {
	std::string to_string(const std::pair<in_addr_t,in_port_t>& address_and_port);

	namespace network {
		expected<int> select(int max_fd, fd_set* read, fd_set* write, uint64_t timeout_ms);

		m2::expected<std::vector<std::string>> get_ip_addresses();
	}
}
