#pragma once
#include "../multi_player/Type.h"
#include <thread>
#include <deque>
#include <mutex>
#include "../Object.h"
#include "PingBroadcastThread.h"
#include "TcpSocketManager.h"
#include <latch>

namespace m2::network::detail {
	/// Base class of ClientThreads
	class BaseClientThread {
		// Read only
		MAYBE const mplayer::Type _type{};
		const std::string _addr;
		const bool _ping_broadcast{};
		uint64_t _ready_token{};

		// Shared variables
		std::latch _latch{1};
		std::mutex _mutex;
		pb::ClientState _state{pb::ClientState::CLIENT_INITIAL_STATE};
		std::queue<pb::NetworkMessage> _outgoing_queue, _incoming_queue;
		std::optional<pb::ServerUpdate> _received_server_update;
		std::optional<m2g::pb::ServerCommand> _received_server_command;

		// Inner thread variables
		uint64_t _level_token{};

		// Initialize the thread after the shared variables
		std::thread _thread;

	protected:
		void latch() { _latch.count_down(); } // This function must be called from the inherited class' constructor

	public:
		BaseClientThread() = default; // Does nothing
		BaseClientThread(mplayer::Type type, std::string addr, bool ping_broadcast);
		BaseClientThread(const BaseClientThread& other) = delete;
		BaseClientThread& operator=(const BaseClientThread& other) = delete;
		BaseClientThread(BaseClientThread&& other) = delete;
		BaseClientThread& operator=(BaseClientThread&& other) = delete;
		virtual ~BaseClientThread();

		virtual const char* thread_name() const = 0;

		// Accessors

		pb::ClientState locked_get_client_state();
		bool locked_has_server_update();
		const pb::ServerUpdate* locked_peek_server_update();
		std::optional<pb::ServerUpdate> locked_pop_server_update();
		bool locked_has_server_command();
		std::optional<m2g::pb::ServerCommand> locked_pop_server_command();

		// Modifiers

		void locked_set_ready(bool state);
		/// Only for HostClientThread and BotClientThread
		void locked_start_if_ready();
		void locked_queue_client_command(const m2g::pb::ClientCommand& cmd);
		void locked_shutdown();

	private:
		// Private modifiers
		void unlocked_set_state(pb::ClientState state);
		void locked_set_state(pb::ClientState state);

		static void base_client_thread_func(BaseClientThread* thread_manager);
	};
}
