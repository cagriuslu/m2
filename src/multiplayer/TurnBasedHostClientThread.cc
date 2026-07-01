#include <m2/multiplayer/TurnBasedHostClientThread.h>
#include <m2/Log.h>
#include <m2/mt/CooperativeSleep.h>

m2::network::TurnBasedHostClientThread::TurnBasedHostClientThread(std::in_place_t)
	: TurnBasedClientThreadBase("127.0.0.1", "HC") {
	CooperativeSleepUntil([this] { return is_connected(); });
	LOG_INFO("TurnBasedHostClientThread connected, becoming ready...");

	locked_set_ready(true);
	LOG_INFO("TurnBasedHostClientThread became ready");
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
