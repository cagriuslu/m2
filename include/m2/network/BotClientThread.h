#pragma once
#include "BaseClientThread.h"

namespace m2::network {
	class BotClientThread final : private detail::BaseClientThread {
	public:
		BotClientThread() = default;
		explicit BotClientThread(mplayer::Type type);
		BotClientThread(const BotClientThread& other) = delete;
		BotClientThread& operator=(const BotClientThread& other) = delete;
		BotClientThread(BotClientThread&& other) = delete;
		BotClientThread& operator=(BotClientThread&& other) = delete;

		const char* thread_name() const override;

		// Accessors
		bool is_active();
		std::optional<m2g::pb::ServerCommand> pop_server_command();

		// Modifiers
		void queue_client_command(const m2g::pb::ClientCommand& c) { locked_queue_client_command(c); }
		std::optional<pb::ServerUpdate> pop_server_update();
	};
}
