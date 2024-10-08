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
		std::thread _thread;

		// Shared variables
		std::latch _latch{1};
		std::mutex _mutex;
		pb::ServerState _state{pb::ServerState::SERVER_INITIAL_STATE};
		std::vector<ClientManager> _clients;
		bool _has_reconnected_client{};
		int _turn_holder{};
		std::optional<pb::NetworkMessage> _received_client_command;

		// Thread variables
		std::optional<PingBroadcastThread> _ping_broadcast_thread;

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
		pb::NetworkMessage prepare_server_update();
		void send_server_update();
		void send_server_command(const m2g::pb::ServerCommand& command, int receiver_index);
		void shutdown();

	private:
		pb::ServerState locked_get_state();
		void set_state_locked(pb::ServerState state);
		void set_state_unlocked(pb::ServerState state);

		// Thread functions
		static void thread_func(ServerThread* server_thread);
		[[nodiscard]] bool locked_is_quit();
		int prepare_read_fd_set(fd_set* set); // Return max fd
		int prepare_write_fd_set(fd_set* set); // Return max fd
	};
}
