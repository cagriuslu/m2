#pragma once
#include "TurnBasedClientThreadBase.h"

namespace m2::network {
	class TurnBasedBotClientThread final : private detail::TurnBasedClientThreadBase {
	public:
		TurnBasedBotClientThread() = default;
		explicit TurnBasedBotClientThread(mplayer::Type type);
		TurnBasedBotClientThread(const TurnBasedBotClientThread& other) = delete;
		TurnBasedBotClientThread& operator=(const TurnBasedBotClientThread& other) = delete;
		TurnBasedBotClientThread(TurnBasedBotClientThread&& other) = delete;
		TurnBasedBotClientThread& operator=(TurnBasedBotClientThread&& other) = delete;

		const char* thread_name() const override;

		// Accessors
		bool is_active();
		std::optional<m2g::pb::ServerCommand> pop_server_command();

		// Modifiers
		void queue_client_command(const m2g::pb::ClientCommand& c) { locked_queue_client_command(c); }
		std::optional<pb::ServerUpdate> pop_server_update();
	};
}
