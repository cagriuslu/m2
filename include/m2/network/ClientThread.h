#pragma once
#include "../multi_player/Type.h"
#include <thread>
#include <deque>
#include <mutex>
#include "../Object.h"

namespace m2::network {
	class ClientThread {
		enum class State {
			NotReady,
			Quit,
			Connected,
			Ready,
			Started,
		};

		const mplayer::Type _type;
		const std::string _addr;
		std::thread _thread;

		// Shared variables
		std::mutex _mutex;
		State _state{State::NotReady};
		std::deque<pb::NetworkMessage> _message_queue;
		std::optional<pb::NetworkMessage> _last_server_update;

		// Thread variables
		char _read_buffer[65536]{};

	public:
		ClientThread(mplayer::Type type, std::string addr);
		ClientThread(const ClientThread& other) = delete;
		ClientThread& operator=(const ClientThread& other) = delete;
		ClientThread(ClientThread&& other) = delete;
		ClientThread& operator=(ClientThread&& other) = delete;
		~ClientThread();

		// Accessors
		bool is_not_connected();
		bool is_connected();
		bool is_ready();
		bool is_started();

		// Modifiers
		void set_ready_blocking(bool state);
		std::optional<pb::NetworkMessage> pop_server_update();

	private:
		size_t message_count_locked();
		void set_state_unlocked(State state);
		void set_state_locked(State state);
		void queue_ping_locked(int32_t sender_id);
		static void thread_func(ClientThread* client_thread);
	};
}
