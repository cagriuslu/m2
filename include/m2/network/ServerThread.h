#pragma once
#include "server/Client.h"
#include "../multi_player/Type.h"
#include "Socket.h"
#include <Network.pb.h>
#include <thread>
#include <deque>
#include <mutex>
#include "server/Client.h"
#include "m2/Object.h"
#include <queue>

namespace m2::network {
	class ServerThread {
		// Main thread variables
		const mplayer::Type _type;
		const unsigned _max_connection_count;
		std::thread _thread;

		// Shared variables
		std::mutex _mutex;
		pb::ServerState _state{pb::ServerState::SERVER_NOT_READY};
		std::vector<server::Client> _clients;
		unsigned _turn_holder{};
		std::optional<pb::NetworkMessage> _received_client_command;

		// Thread variables
		char _read_buffer[65536]{}; // Shared among all clients

	public:
		ServerThread(mplayer::Type type, unsigned max_connection_count);
		ServerThread(const ServerThread& other) = delete;
		ServerThread& operator=(const ServerThread& other) = delete;
		ServerThread(ServerThread&& other) = delete;
		ServerThread& operator=(ServerThread&& other) = delete;
		~ServerThread();

		// Accessors
		[[nodiscard]] mplayer::Type type() const { return _type; }
		bool is_listening();
		size_t client_count();
		unsigned turn_holder();
		std::optional<pb::NetworkMessage> pop_turn_holder_command();

		// Modifiers
		void_expected close_lobby();
		void set_turn_holder_index(unsigned);
		void server_update();

	private:
		void set_state_locked(pb::ServerState state);

		// Thread functions
		static void thread_func(ServerThread* server_thread);
		[[nodiscard]] bool is_quit();
		int prepare_read_set(fd_set* set); // Return max fd
	};
}