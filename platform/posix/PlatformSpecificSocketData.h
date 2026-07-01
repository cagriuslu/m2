#pragma once

namespace m2::network::detail {
	struct PlatformSpecificTcpSocketData {
		int fd{};
	};
	struct PlatformSpecificUdpSocketData {
		int fd{};
	};
}
