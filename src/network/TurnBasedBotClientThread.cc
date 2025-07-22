#include <m2/network/TurnBasedBotClientThread.h>
#include <m2/Log.h>

m2::network::TurnBasedBotClientThread::TurnBasedBotClientThread(mplayer::Type type)
	: detail::TurnBasedClientThreadBase(type, "127.0.0.1", false) {
	latch();

	// Wait until the bot is connected. The Bot thread waits 100 millisecond to ensure that the connection to host is not
	// closed from the host side. Let's wait a bit longer than that.
	LOG_DEBUG("Waiting 150ms until the bot is connected");
	std::this_thread::sleep_for(std::chrono::milliseconds(150));

	if (locked_get_client_state() == pb::CLIENT_CONNECTED) {
		LOG_INFO("TurnBasedBotClientThread connected, becoming ready...");
		locked_set_ready(true);
		LOG_INFO("TurnBasedBotClientThread became ready");
	} else {
		LOG_WARN("TurnBasedBotClientThread was unable to connect, player count limit may have reached");
	}
}

const char* m2::network::TurnBasedBotClientThread::thread_name() const {
	return "BC";
}

bool m2::network::TurnBasedBotClientThread::is_active() {
	auto state = locked_get_client_state();
	return state == pb::CLIENT_READY || state == pb::CLIENT_STARTED;
}
std::optional<m2g::pb::ServerCommand> m2::network::TurnBasedBotClientThread::pop_server_command() {
	if (auto serverCommand = locked_pop_server_command()) {
		return serverCommand->second;
	}
	return std::nullopt;
}

std::optional<m2::pb::ServerUpdate> m2::network::TurnBasedBotClientThread::pop_server_update() {
	if (auto serverUpdate = locked_pop_server_update()) {
		return serverUpdate->second;
	}
	return std::nullopt;
}
