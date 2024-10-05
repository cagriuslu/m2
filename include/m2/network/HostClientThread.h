#pragma once
#include "BaseClientThread.h"

namespace m2::network {
	class HostClientThread : private detail::BaseClientThread {
	public:
		HostClientThread() = default;
		explicit HostClientThread(mplayer::Type type);
		HostClientThread(const HostClientThread& other) = delete;
		HostClientThread& operator=(const HostClientThread& other) = delete;
		HostClientThread(HostClientThread&& other) = delete;
		HostClientThread& operator=(HostClientThread&& other) = delete;

		// Accessors
		bool is_shutdown();
		inline std::optional<m2g::pb::ServerCommand> pop_server_command() { return locked_pop_server_command(); }

		// Modifiers
		void start_if_ready() { locked_start_if_ready(); }
		inline void queue_client_command(const m2g::pb::ClientCommand& c) { locked_queue_client_command(c); }

	private:
		bool is_connected();
	};
}
