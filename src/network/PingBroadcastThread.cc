#include <m2/network/PingBroadcastThread.h>
#include <m2/Log.h>
#include <cstdlib>
#include <m2/mt/WaitOnCondition.h>

using namespace m2;
using namespace m2::network;

PingBroadcastThread::PingBroadcastThread() : _thread(PingBroadcastThread::thread_func, this) {
	_latch.count_down();
}

PingBroadcastThread::~PingBroadcastThread() {
	{
		const std::lock_guard lock(_mutex);
		_quit = true;
	}
	_thread.join();
}

void PingBroadcastThread::shutdown() {
	const std::lock_guard lock(_mutex);
	_quit = true;
}

bool PingBroadcastThread::is_quit() {
	const std::lock_guard lock(_mutex);
	return _quit;
}

void PingBroadcastThread::thread_func(PingBroadcastThread* context) {
	context->_latch.wait();
	SetThreadNameForLogging("PN");
	LOG_INFO("PingBroadcastThread function");

	while (not context->is_quit()) {
		// Ping the broadcast address only once with some timeout
		auto retval = system("ping -c 1 -t 1 255.255.255.255 1>/dev/null 2>/dev/null");
		if (retval == 0) {
			// Received at least one packet
		} else if (retval == 2) {
			// Received no packets
		} else {
			LOG_WARN("Error while pinging the broadcast address", retval);
		}

		// Wait some time before pinging again
		m2::mt::wait_on_condition(5000, [&context]() -> bool {
			return context->is_quit();
		});
	}

	LOG_INFO("PingBroadcastThread is quitting");
}