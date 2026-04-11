#pragma once
#include "TurnBasedClientThreadBase.h"

namespace m2::network {
	class TurnBasedBotClientThread final : private detail::TurnBasedClientThreadBase {
		int _receiverIndex{-1}; /// Bot doesn't know its receiver index until it receives a message from the server.

	public:
		TurnBasedBotClientThread() = default;
		explicit TurnBasedBotClientThread(std::in_place_t);
		TurnBasedBotClientThread(const TurnBasedBotClientThread& other) = delete;
		TurnBasedBotClientThread& operator=(const TurnBasedBotClientThread& other) = delete;
		TurnBasedBotClientThread(TurnBasedBotClientThread&& other) = delete;
		TurnBasedBotClientThread& operator=(TurnBasedBotClientThread&& other) = delete;

		const char* thread_name() const override;

		// Accessors

		/// Returns the receiver index if known. It's unknown if no messages are received from the server yet.
		std::optional<int> GetReceiverIndex() const { return _receiverIndex < 0 ? std::optional<int>{} : _receiverIndex; }
		bool is_active();
		std::optional<m2g::pb::TurnBasedServerCommand> pop_server_command();

		// Modifiers

		void queue_client_command(const m2g::pb::TurnBasedClientCommand& c) { locked_queue_client_command(c); }
		std::optional<pb::TurnBasedServerUpdate> pop_server_update();
	};
}
