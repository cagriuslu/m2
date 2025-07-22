#pragma once
#include "TurnBasedClientThreadBase.h"

namespace m2::network {
	class TurnBasedHostClientThread final : private detail::TurnBasedClientThreadBase {
	public:
		TurnBasedHostClientThread() = default;
		explicit TurnBasedHostClientThread(mplayer::Type type);
		TurnBasedHostClientThread(const TurnBasedHostClientThread& other) = delete;
		TurnBasedHostClientThread& operator=(const TurnBasedHostClientThread& other) = delete;
		TurnBasedHostClientThread(TurnBasedHostClientThread&& other) = delete;
		TurnBasedHostClientThread& operator=(TurnBasedHostClientThread&& other) = delete;

		const char* thread_name() const override;

		// Accessors
		bool is_shutdown();
		std::optional<m2g::pb::ServerCommand> pop_server_command();

		// Modifiers
		void start_if_ready() { locked_start_if_ready(); }
		inline void queue_client_command(const m2g::pb::ClientCommand& c) { locked_queue_client_command(c); }

	private:
		bool is_connected();
	};
}
