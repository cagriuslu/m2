#pragma once
#include "../multi_player/Type.h"
#include "Socket.h"
#include <Network.pb.h>
#include <thread>
#include <deque>
#include <mutex>

#include "m2/Object.h"

namespace m2::network {
	class ServerThread {
		struct Client {
			std::optional<Socket> socket;
			std::optional<int32_t> sender_id;
		};
		struct QueueMessage {
			unsigned destination;
			pb::NetworkMessage message;
		};

		// Main thread variables
		const mplayer::Type _type;
		const unsigned _max_connection_count;
		std::thread _thread;

		// Shared variables
		std::mutex _mutex;
		pb::ServerState _state{pb::ServerState::SERVER_NOT_READY};
		std::vector<Client> _clients;
		int _turn_holder_index{};
		std::deque<QueueMessage> _outgoing_message_queue;
		std::optional<pb::NetworkMessage> _received_client_command;

		// Thread variables
		char _read_buffer[65536]{};

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
		int turn_holder_index();
		std::optional<pb::NetworkMessage> pop_client_command();

		// Modifiers
		bool close_lobby();
		void set_turn_holder_index(int);
		void server_update();
		void queue_server_command(int destination, const m2g::pb::ServerCommand& cmd);

	private:
		size_t message_count_locked();
		void set_state_locked(pb::ServerState state);
		void queue_message_unlocked(const QueueMessage& msg);
		void queue_message_unlocked(QueueMessage&& msg);
		void queue_message_locked(const QueueMessage& msg);
		void queue_message_locked(QueueMessage&& msg);
		static void thread_func(ServerThread* server_thread);
	};
}
