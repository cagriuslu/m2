#include <m2/network/BotClientThread.h>
#include <m2/Log.h>

m2::network::BotClientThread::BotClientThread(mplayer::Type type)
	: detail::BaseClientThread(type, "127.0.0.1", false) {
	INFO_FN();

	// Wait until the bot is connected
	while (locked_get_client_state() != pb::CLIENT_CONNECTED) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	LOG_INFO("BotClientThread connected, becoming ready...");

	locked_set_ready_sync(true);
	LOG_INFO("BotClientThread became ready");
}
