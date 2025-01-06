#pragma once
#include "BaseClientThread.h"

namespace m2::network {
	enum class ServerUpdateStatus {
		// There was no ServerUpdate
		NOT_FOUND = 0,
		// ServerUpdate is processed successfully
		PROCESSED = 1,
		// ServerUpdate is processed but it was containing the shutdown flag
		PROCESSED_SHUTDOWN = 2
	};

	class RealClientThread final : private detail::BaseClientThread {
		/// When a ServerUpdate is received from the server, it's placed in BaseClientThread::_received_server_update.
		/// peek_unprocessed_server_update() can be used to take a peek at it.
		/// When process_server_update() is called, ServerUpdate is shifted as follows:
		/// _prev_processed_server_update << _last_processed_server_update << BaseClientThread::_received_server_update
		std::optional<std::pair<SequenceNo,pb::ServerUpdate>> _prev_processed_server_update, _last_processed_server_update;

		/// Mapping of server object IDs to local object IDs. The boolean represents if the object has been visited during
		/// the processing of ServerUpdate or not. If an object is not visited, it must have been deleted on the server side.
		std::unordered_map<ObjectId,std::pair<ObjectId,bool>> _server_to_local_map;

	public:
		RealClientThread(mplayer::Type type, std::string addr);
		RealClientThread(const RealClientThread& other) = delete;
		RealClientThread& operator=(const RealClientThread& other) = delete;
		RealClientThread(RealClientThread&& other) = delete;
		RealClientThread& operator=(RealClientThread&& other) = delete;

		const char* thread_name() const override;

		// Accessors

		/// Check whether the client has successfully connected to the server.
		bool is_connected() { return locked_get_client_state() == pb::CLIENT_CONNECTED; }
		/// Check whether the client has successfully signified to the server that it's ready.
		bool is_ready() { return locked_get_client_state() == pb::CLIENT_READY; }
		/// Check whether the game has started or not.
		bool is_started() { return locked_get_client_state() == pb::CLIENT_STARTED; }
		/// Check whether the client has successfully reconnect to the server after a disconnection or not.
		bool is_reconnected() { return locked_get_client_state() == pb::CLIENT_RECONNECTED; }
		/// Check whether the client has timed out while trying to reconnect to the server.
		bool has_reconnection_timed_out() { return locked_get_client_state() == pb::CLIENT_RECONNECTION_TIMEOUT_QUIT; }
		/// Check whether the game has gracefully shutdown by the server.
		bool is_shutdown() { return locked_get_client_state() == pb::CLIENT_SHUTDOWN; }
		/// Check whether the thread has quited. This happens if the connection is rejected by the server, most likely due to player count reaching the limit.
		bool IsQuit() { return locked_get_client_state() == pb::CLIENT_QUIT; }
		/// Check whether the server has shown an unexpected behavior.
		bool is_server_unrecognized() { return locked_get_client_state() == pb::CLIENT_MISBEHAVING_SERVER_QUIT; }

		// Accessors (only available if is_started() is true)

		/// Query the total number of players in the game.
		int total_player_count();
		/// Try to pop the ServerCommand.
		inline std::optional<m2g::pb::ServerCommand> pop_server_command() { return locked_pop_server_command(); }
		/// Query the index of this game instance in server's client list.
		int self_index();
		/// Query the index of the current turn holder.
		int turn_holder_index();
		/// Compare self_index and turn_holder_index to check if it's our turn.
		inline bool is_our_turn() { return self_index() == turn_holder_index(); }

		// Modifiers

		/// Signal readiness to server.
		inline void set_ready(bool state) { locked_set_ready(state); }
		/// Queue a ClientCommand to be sent to the server.
		inline void queue_client_command(const m2g::pb::ClientCommand& c) { locked_queue_client_command(c); }
		/// Returns true if there was a ServerUpdate and it was processed, otherwise returns false.
		/// Returns unexpected if an error occurs while processing.
		expected<ServerUpdateStatus> process_server_update();
		/// Shutdown the client. This should be called only if the last ServerUpdateStatus indicated shutdown.
		void shutdown();
	};
}
