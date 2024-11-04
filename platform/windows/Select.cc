#include <m2/network/Select.h>

using namespace m2;
using namespace m2::network;

expected<std::optional<std::pair<TcpSocketHandles, TcpSocketHandles>>> Select::operator()
        (const TcpSocketHandles& sockets_to_read, const TcpSocketHandles& sockets_to_write, uint64_t timeout_ms) {
    return {};
}
