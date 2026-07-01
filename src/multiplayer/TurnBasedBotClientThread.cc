#include <m2/multiplayer/TurnBasedBotClientThread.h>
#include <m2/Log.h>
#include <m2/mt/CooperativeSleep.h>

m2::network::TurnBasedBotClientThread::TurnBasedBotClientThread(std::in_place_t)
	: detail::TurnBasedClientThreadBase("127.0.0.1", "BC") {
	// Wait until the bot is connected
	LOG_NETWORK("Waiting until the bot is connected");
	CooperativeSleepUntilOrTimeout([this] { return locked_get_client_state() == pb::CLIENT_CONNECTED; }, 500);

	if (locked_get_client_state() == pb::CLIENT_CONNECTED) {
		LOG_INFO("TurnBasedBotClientThread connected, becoming ready...");
		locked_set_ready(true);
		LOG_INFO("TurnBasedBotClientThread became ready");
	} else {
		LOG_WARN("TurnBasedBotClientThread was unable to connect, player count limit may have reached");
	}
}

bool m2::network::TurnBasedBotClientThread::is_active() {
	auto state = locked_get_client_state();
	return state == pb::CLIENT_READY || state == pb::CLIENT_STARTED;
}
std::optional<m2g::pb::TurnBasedServerCommand> m2::network::TurnBasedBotClientThread::pop_server_command() {
	if (auto serverCommand = locked_pop_server_command()) {
		return serverCommand->second;
	}
	return std::nullopt;
}

std::optional<m2::pb::TurnBasedServerUpdate> m2::network::TurnBasedBotClientThread::pop_server_update() {
	if (auto serverUpdate = locked_pop_server_update()) {
		// Store the receiver index as well
		_receiverIndex = serverUpdate->second.receiver_index();
		return std::move(serverUpdate->second);
	}
	return std::nullopt;
}
