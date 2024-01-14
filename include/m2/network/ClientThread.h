#pragma once
#include "../multi_player/Type.h"
#include <thread>
#include <deque>
#include <mutex>
#include "../Object.h"

namespace m2::network {
	class ClientThread {
		MAYBE const mplayer::Type _type;
		const std::string _addr;
		std::thread _thread;

		// Shared variables
		std::mutex _mutex;
		pb::ClientState _state{pb::ClientState::CLIENT_NOT_READY};
		std::deque<pb::NetworkMessage> _message_queue;
		std::optional<pb::NetworkMessage> _prev_processed_server_update, _last_processed_server_update, _unprocessed_server_update;
		std::unordered_map<ObjectId,std::pair<ObjectId,bool>> _server_to_local_map;

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
		std::optional<pb::ServerUpdate> peek_unprocessed_server_update();
		std::optional<pb::ServerUpdate> last_processed_server_update();
		bool is_our_turn();
		unsigned total_player_count();

		// Modifiers
		void set_ready_blocking(bool state);
		expected<bool> process_server_update();
		void queue_client_command(const m2g::pb::ClientCommand& cmd);

	private:
		size_t message_count_locked();
		void set_state_unlocked(pb::ClientState state);
		void set_state_locked(pb::ClientState state);
		bool fetch_server_update_unlocked();

		static void thread_func(ClientThread* client_thread);
		[[nodiscard]] bool is_quit();
	};
}
