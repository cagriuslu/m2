#pragma once
#include "BaseClientThread.h"

namespace m2::network {
	class BotClientThread : private detail::BaseClientThread {
	public:
		BotClientThread() = default;
		explicit BotClientThread(mplayer::Type type);
		BotClientThread(const BotClientThread& other) = delete;
		BotClientThread& operator=(const BotClientThread& other) = delete;
		BotClientThread(BotClientThread&& other) = delete;
		BotClientThread& operator=(BotClientThread&& other) = delete;

		// Accessors
		inline std::optional<m2g::pb::ServerCommand> pop_server_command() { return locked_pop_server_command(); }

		// Modifiers
		inline void queue_client_command(const m2g::pb::ClientCommand& c) { locked_queue_client_command(c); }
		inline std::optional<m2::pb::ServerUpdate> pop_server_update() { return locked_pop_server_update(); }
	};
}
