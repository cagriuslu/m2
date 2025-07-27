#pragma once
#include "TcpSocket.h"
#include "UdpSocket.h"
#include <vector>
#include <optional>

namespace m2::network {
	template <typename SocketT>
	using SocketHandles = std::vector<SocketT*>;

	template <typename SocketT>
	struct SelectResult {
		SocketHandles<SocketT> readableSockets, writableSockets;
	};

	using TcpSocketHandles = SocketHandles<TcpSocket>;
	using UdpSocketHandles = SocketHandles<UdpSocket>;

	struct Select {
		/// \brief Waits until given sockets are ready for reading or writing.
		/// \details Returns unexpected if an error occurs. Otherwise, returns nullopt if timeout occurs. Otherwise, returns
		/// SelectResult with readable and writable sockets.
		static expected<std::optional<SelectResult<TcpSocket>>> WaitUntilSocketsReady(const TcpSocketHandles& readSockets,
			const TcpSocketHandles& writeSockets, uint64_t timeoutMs);

		/// \brief Waits until given sockets are ready for reading or writing.
		/// \details Returns unexpected if an error occurs. Otherwise, returns nullopt if timeout occurs. Otherwise, returns
		/// SelectResult with readable and writable sockets.
		static expected<std::optional<SelectResult<UdpSocket>>> WaitUntilSocketsReady(const UdpSocketHandles& readSockets,
			const UdpSocketHandles& writeSockets, uint64_t timeoutMs);
	};
}
