#pragma once
#include "../multi_player/Type.h"
#include "Socket.h"
#include <Network.pb.h>
#include <thread>
#include <deque>
#include <mutex>

namespace m2::network {
	class ServerThread {
		enum class State {
			NotReady,
			Quit,
			Listening,
			Ready, // Lobby closed
			Started,
		};
		struct Client {
			Socket socket;
			std::optional<int32_t> sender_id;
		};
		struct QueueMessage {
			unsigned destination;
			pb::NetworkMessage message;
		};

		const mplayer::Type _type;
		const unsigned _max_connection_count;
		std::thread _thread;

		// Shared variables
		std::mutex _mutex;
		State _state{State::NotReady};
		std::vector<Client> _clients;
		std::deque<QueueMessage> _message_queue;

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

		// Modifiers
		bool close_lobby();
		void server_update(int destination);
		void queue_server_command(int destination, const m2g::pb::ServerCommand& cmd);

	private:
		size_t message_count_locked();
		void set_state_locked(State state);
		void queue_message_unlocked(const QueueMessage& msg);
		void queue_message_locked(const QueueMessage& msg);
		static void thread_func(ServerThread* server_thread);
	};
}
