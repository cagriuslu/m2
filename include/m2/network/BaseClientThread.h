#pragma once
#include "../multi_player/Type.h"
#include <thread>
#include <deque>
#include <mutex>
#include "../Object.h"
#include "PingBroadcastThread.h"

namespace m2::network::detail {
	/// Base class of ClientThreads
	class BaseClientThread {
		// Read only
		MAYBE const mplayer::Type _type{};
		const std::string _addr;
		const bool _ping_broadcast{};
		std::thread _thread;

		// Shared variables
		std::mutex _mutex;
		pb::ClientState _state{pb::ClientState::CLIENT_NOT_READY};
		std::deque<pb::NetworkMessage> _out_message_queue;
		std::optional<pb::ServerUpdate> _received_server_update;
		std::optional<m2g::pb::ServerCommand> _received_server_command;

		// Thread variables
		char _read_buffer[65536]{};
		std::optional<PingBroadcastThread> _ping_broadcast_thread;

	public:
		BaseClientThread() = default; // Does nothing
		BaseClientThread(mplayer::Type type, std::string addr, bool ping_broadcast);
		BaseClientThread(const BaseClientThread& other) = delete;
		BaseClientThread& operator=(const BaseClientThread& other) = delete;
		BaseClientThread(BaseClientThread&& other) = delete;
		BaseClientThread& operator=(BaseClientThread&& other) = delete;
		virtual ~BaseClientThread();

		// Accessors
		pb::ClientState locked_get_client_state();
		bool locked_has_server_update();
		const pb::ServerUpdate* locked_peek_server_update();
		std::optional<pb::ServerUpdate> locked_pop_server_update();
		bool locked_has_server_command();
		std::optional<m2g::pb::ServerCommand> locked_pop_server_command();

		// Modifiers
		void locked_set_ready_sync(bool state);
		void locked_queue_client_command(const m2g::pb::ClientCommand& cmd);

	private:
		// Private accessors
		bool locked_should_continue_running();
		size_t locked_get_outgoing_message_count();

		// Private modifiers
		void unlocked_set_state(pb::ClientState state);
		void locked_set_state(pb::ClientState state);
		pb::NetworkMessage locked_unsafe_pop_outgoing_message();

		static void base_client_thread_func(BaseClientThread* thread_manager);
	};
}
