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
		std::optional<m2g::pb::TurnBasedServerCommand> pop_server_command();

		// Modifiers
		void queue_client_command(const m2g::pb::TurnBasedClientCommand& c) { locked_queue_client_command(c); }
		std::optional<pb::TurnBasedServerUpdate> pop_server_update();
	};
}
