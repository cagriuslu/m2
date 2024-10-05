#include <m2/network/HostClientThread.h>
#include <m2/Log.h>

m2::network::HostClientThread::HostClientThread(mplayer::Type type)
	: detail::BaseClientThread(type, "127.0.0.1", false) {
	INFO_FN();

	// Wait until the client is connected
	while (not is_connected()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	LOG_INFO("HostClientThread connected, becoming ready...");

	locked_set_ready(true);
	LOG_INFO("HostClientThread became ready");
}

bool m2::network::HostClientThread::is_shutdown() {
	return locked_get_client_state() == pb::CLIENT_SHUTDOWN;
}

bool m2::network::HostClientThread::is_connected() {
	return locked_get_client_state() == pb::CLIENT_CONNECTED;
}
