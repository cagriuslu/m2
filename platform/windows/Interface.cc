#include <m2/network/Interface.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <vector>
#include <format>

#pragma comment(lib, "Iphlpapi.lib")

m2::expected<std::vector<m2::network::IpAddress>> m2::network::GetInterfaces() {
    constexpr ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;

    // The recommended initial buffer size is 15 KB. If it's not enough, GetAdaptersAddresses reports the
    // required size via buffer_size, so retry with a larger buffer.
    ULONG buffer_size = 15 * 1024;
    std::vector<uint8_t> buffer;
    ULONG result = ERROR_BUFFER_OVERFLOW;
    for (int attempt = 0; attempt < 3 && result == ERROR_BUFFER_OVERFLOW; ++attempt) {
        buffer.resize(buffer_size);
        result = ::GetAdaptersAddresses(AF_INET, flags, nullptr, reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data()), &buffer_size);
    }
    if (result != NO_ERROR) {
        return m2::make_unexpected(std::format("GetAdaptersAddresses failed: {}", result));
    }

    std::vector<IpAddress> interfaces;
    for (auto* adapter = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data()); adapter; adapter = adapter->Next) {
        for (auto* unicast = adapter->FirstUnicastAddress; unicast; unicast = unicast->Next) {
            const sockaddr* addr = unicast->Address.lpSockaddr;
            if (addr && addr->sa_family == AF_INET) {
                interfaces.emplace_back(IpAddress::CreateFromNetworkOrder(reinterpret_cast<const sockaddr_in*>(addr)->sin_addr.S_un.S_addr));
            }
        }
    }
    return interfaces;
}
