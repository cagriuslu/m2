#pragma once

#include <thread>
#include <mutex>
#include <latch>

namespace m2::network {
	// When connecting two macOS machines, I've observed two machines may fail to connect to each other because one
	// machine doesn't respond to the ARP request of the other. It could also be the router that's failing to deliver
	// the ARP packets. Pinging the broadcast address may inadvertently broadcast the MAC address of one to the other.
	// I've observed that macOS allows us to ping the broadcast address, and other macOS machine even respond to pings
	// coming from the broadcast IP. Firewalls were disabled.
	class PingBroadcastThread {
		// Shared variables
		std::latch _latch{1};
		std::mutex _mutex{};
		bool _quit{};

		// Initialize the thread after the shared variables
		std::thread _thread;

	public:
		PingBroadcastThread();
		PingBroadcastThread(const PingBroadcastThread& other) = delete;
		PingBroadcastThread& operator=(const PingBroadcastThread& other) = delete;
		PingBroadcastThread(PingBroadcastThread&& other) = delete;
		PingBroadcastThread& operator=(PingBroadcastThread&& other) = delete;
		~PingBroadcastThread();

		// Modifiers
		void shutdown();

	private:
		[[nodiscard]] bool IsQuit();
		static void thread_func(PingBroadcastThread* context);
	};
}
