#include <../../include/m2/multi_player/TurnBasedHostClientThread.h>
#include <m2/Log.h>

m2::network::TurnBasedHostClientThread::TurnBasedHostClientThread(std::in_place_t)
	: detail::TurnBasedClientThreadBase("127.0.0.1", false) {
	latch();

	// Wait until the client is connected
	while (not is_connected()) {
		LOG_NETWORK("Waiting 25ms until the host client is connected");
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	LOG_INFO("TurnBasedHostClientThread connected, becoming ready...");

	locked_set_ready(true);
	LOG_INFO("TurnBasedHostClientThread became ready");
}

const char* m2::network::TurnBasedHostClientThread::thread_name() const {
	return "HC";
}

bool m2::network::TurnBasedHostClientThread::is_shutdown() {
	return locked_get_client_state() == pb::CLIENT_SHUTDOWN;
}
std::optional<m2g::pb::TurnBasedServerCommand> m2::network::TurnBasedHostClientThread::pop_server_command() {
	if (auto serverCommand = locked_pop_server_command()) {
		return serverCommand->second;
	}
	return std::nullopt;
}

bool m2::network::TurnBasedHostClientThread::is_connected() {
	return locked_get_client_state() == pb::CLIENT_CONNECTED;
}
