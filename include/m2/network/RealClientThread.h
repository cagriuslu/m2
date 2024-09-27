#pragma once
#include "BaseClientThread.h"

namespace m2::network {
	class RealClientThread : private detail::BaseClientThread {
		/// When a ServerUpdate is received from the server, it's placed in BaseClientThread::_received_server_update.
		/// peek_unprocessed_server_update() can be used to take a peek at it.
		/// When process_server_update() is called, ServerUpdate is shifted as follows:
		/// _prev_processed_server_update << _last_processed_server_update << BaseClientThread::_received_server_update
		std::optional<pb::ServerUpdate> _prev_processed_server_update, _last_processed_server_update;

		/// Mapping of server object IDs to local object IDs. The boolean represents if the object has been visited during
		/// the processing of ServerUpdate or not. If an object is not visited, it must have been deleted on the server side.
		std::unordered_map<ObjectId,std::pair<ObjectId,bool>> _server_to_local_map;

	public:
		RealClientThread(mplayer::Type type, std::string addr);
		RealClientThread(const RealClientThread& other) = delete;
		RealClientThread& operator=(const RealClientThread& other) = delete;
		RealClientThread(RealClientThread&& other) = delete;
		RealClientThread& operator=(RealClientThread&& other) = delete;

		// Accessors
		bool is_connected();
		bool is_ready();
		bool is_started();
		bool is_shutdown();
		// Accessors (only available if is_started() is true)
		int total_player_count();
		std::optional<pb::ServerUpdate> last_processed_server_update();
		inline std::optional<m2g::pb::ServerCommand> pop_server_command() { return locked_pop_server_command(); }
		int self_index();
		int turn_holder_index();
		inline bool is_our_turn() { return self_index() == turn_holder_index(); }

		// Modifiers

		inline void set_ready(bool state) { locked_set_ready_sync(state); }
		inline void queue_client_command(const m2g::pb::ClientCommand& c) { locked_queue_client_command(c); }
		/// Returns true if a ServerUpdate is processed, otherwise returns false.
		/// Returns unexpected if an error occurs.
		expected<bool> process_server_update();
	};
}
