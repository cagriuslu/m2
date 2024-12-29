#pragma once
#include "TcpSocketManager.h"
#include "../multi_player/Type.h"
#include "TcpSocket.h"
#include <Network.pb.h>
#include <thread>
#include <deque>
#include <mutex>
#include "m2/Object.h"
#include <queue>
#include "PingBroadcastThread.h"
#include "ClientManager.h"
#include <latch>

namespace m2::network {
	class ServerThread {
		// Main thread variables
		const mplayer::Type _type{};
		const unsigned _max_connection_count{};

		// Shared variables
		std::latch _latch{1};
		std::mutex _mutex{};
		pb::ServerThreadState _state{pb::ServerThreadState::SERVER_INITIAL_STATE};
		std::vector<ClientManager> _clients;
		bool _has_reconnected_client{};
		int _turn_holder{};
		std::optional<pb::NetworkMessage> _received_client_command;

		// Inner thread variables
		std::optional<PingBroadcastThread> _ping_broadcast_thread;

		// Initialize the thread after the shared variables
		std::thread _thread;

	public:
		ServerThread() = default;
		ServerThread(mplayer::Type type, unsigned max_connection_count);
		ServerThread(const ServerThread& other) = delete;
		ServerThread& operator=(const ServerThread& other) = delete;
		ServerThread(ServerThread&& other) = delete;
		ServerThread& operator=(ServerThread&& other) = delete;
		~ServerThread();

		// Accessors
		[[nodiscard]] mplayer::Type type() const { return _type; }
		bool is_listening();
		int client_count();
		int ready_client_count();
		int turn_holder_index();
		inline bool is_our_turn() { return turn_holder_index() == 0; }
		std::optional<pb::NetworkMessage> pop_turn_holder_command();
		bool has_reconnected_client();
		std::optional<int> disconnected_client();
		std::optional<int> misbehaved_client();
		bool is_shutdown();

		// Modifiers
		void_expected close_lobby();
		void set_turn_holder(int index);
		void send_server_update(bool shutdown_as_well = false);
		void send_server_command(const m2g::pb::ServerCommand& command, int receiver_index = -1);

	private:
		pb::NetworkMessage prepare_server_update(bool shutdown);
		pb::ServerThreadState locked_get_state();
		void set_state_locked(pb::ServerThreadState state);
		void set_state_unlocked(pb::ServerThreadState state);

		// Thread functions
		static void thread_func(ServerThread* server_thread);
		[[nodiscard]] bool locked_is_quit();
	};
}
