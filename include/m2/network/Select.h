#pragma once
#include "TcpSocket.h"
#include "UdpSocket.h"
#include <vector>
#include <optional>

namespace m2::network {
	template <typename SocketT>
	using SocketHandles = std::vector<SocketT*>;
	using TcpSocketHandles = SocketHandles<TcpSocket>;
	using UdpSocketHandles = SocketHandles<UdpSocket>;

	template <typename SocketT>
	struct SelectResult {
		SocketHandles<SocketT> readableSockets, writableSockets;
	};
	using TcpSelectResult = SelectResult<TcpSocket>;
	using UdpSelectResult = SelectResult<UdpSocket>;

	struct Select {
		/// \brief Waits until given sockets are ready for reading or writing.
		/// \details Returns unexpected if an error occurs. Otherwise, returns nullopt if timeout occurs. Otherwise, returns
		/// SelectResult with readable and writable sockets.
		static expected<std::optional<TcpSelectResult>> WaitUntilSocketsReady(const TcpSocketHandles& readSockets,
			const TcpSocketHandles& writeSockets, uint64_t timeoutMs);

		/// \brief Waits until given sockets are ready for reading or writing.
		/// \details Returns unexpected if an error occurs. Otherwise, returns nullopt if timeout occurs. Otherwise, returns
		/// SelectResult with readable and writable sockets.
		static expected<std::optional<UdpSelectResult>> WaitUntilSocketsReady(const UdpSocketHandles& readSockets,
			const UdpSocketHandles& writeSockets, uint64_t timeoutMs);

		static expected<std::optional<UdpSelectResult>> WaitUntilSocketReady(UdpSocket* bidirectionalSocket, const uint64_t timeoutMs) {
			return WaitUntilSocketsReady(UdpSocketHandles{bidirectionalSocket}, UdpSocketHandles{bidirectionalSocket}, timeoutMs);
		}

		/// \brief Checks if the given socket is readable.
		/// \details This is a synchronous call that returns without blocking.
		static expected<bool> IsSocketReadable(UdpSocket* socket);
		/// \brief Checks if the given socket is writable.
		/// \details This is a synchronous call that returns without blocking.
		static expected<bool> IsSocketWritable(UdpSocket* socket);
	};
}
