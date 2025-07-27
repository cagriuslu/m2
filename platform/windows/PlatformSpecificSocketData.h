#pragma once
#include <WinSock2.h>

namespace m2::network::detail {
	struct PlatformSpecificSocketData {
        addrinfo *address_info{};
        SOCKET socket{INVALID_SOCKET};
	};
}
