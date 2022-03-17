#include "m2/ThreadPool.hh"
#include <thread>
#include <queue>
#include <array>
#include <condition_variable>

class CountingSemaphore {
	unsigned state;
	std::mutex mutex;
	std::condition_variable condvar;

public:
	CountingSemaphore(unsigned initialState = 1) : state(initialState) {}

	void down(unsigned count = 1) {
		std::unique_lock<std::mutex> lock(mutex);
		while (state < count) {
			condvar.wait(lock);
		}
		state -= count;
	}

	void up(unsigned count = 1) {
		mutex.lock();
		state += count;
		mutex.unlock();
		condvar.notify_all();
	}
};

constexpr unsigned THREAD_COUNT = 4;

struct ThreadPoolImpl {
	std::array<std::thread, THREAD_COUNT> threads;

	std::mutex mutex;
	std::condition_variable condvar;
	CountingSemaphore idleThreadCount{THREAD_COUNT};
	// First check if quit flag set
	bool quit;
	// Then, check if there is a job queued
	std::queue<ThreadPoolJob> jobQueue;

	ThreadPoolImpl() : quit(false) {}
};

void ThreadPool_ThreadFunc(ThreadPoolImpl* pool) {
	while (true) {
		ThreadPoolJob job = {};

		{
			std::unique_lock<std::mutex> lock(pool->mutex);

			// Wait until quit or job
			while (pool->quit == false || pool->jobQueue.empty()) {
				pool->condvar.wait(lock);
			}
			// Check if quit
			if (pool->quit) {
				return;
			}
			// Otherwise, job
			job = pool->jobQueue.front();
			pool->jobQueue.pop();
		}

		pool->idleThreadCount.down();
		if (job.func) {
			(*job.func)(job.arg);
		}
		pool->idleThreadCount.up();
	}
}

ThreadPool* ThreadPool_Create() {
	auto* threadPoolImpl = new ThreadPoolImpl();

	for (auto it = threadPoolImpl->threads.begin(); it != threadPoolImpl->threads.end(); it++) {
		*it = std::thread(ThreadPool_ThreadFunc, threadPoolImpl);
	}

	return static_cast<ThreadPool*>(threadPoolImpl);
}

void ThreadPool_QueueJob(ThreadPool *pool, ThreadPoolJob job) {
	auto* threadPoolImpl = static_cast<ThreadPoolImpl*>(pool);

	threadPoolImpl->mutex.lock();
	threadPoolImpl->jobQueue.push(job);
	threadPoolImpl->mutex.unlock();
	threadPoolImpl->condvar.notify_one();
}

void ThreadPool_WaitJobs(ThreadPool *pool) {
	auto* threadPoolImpl = static_cast<ThreadPoolImpl*>(pool);

	threadPoolImpl->idleThreadCount.down(THREAD_COUNT);
	threadPoolImpl->idleThreadCount.up(THREAD_COUNT);
}

void ThreadPool_Destroy(ThreadPool *pool) {
	auto* threadPoolImpl = static_cast<ThreadPoolImpl*>(pool);

	threadPoolImpl->mutex.lock();
	threadPoolImpl->quit = true;
	threadPoolImpl->mutex.unlock();
	threadPoolImpl->condvar.notify_all();

	for (auto& thread : threadPoolImpl->threads) {
		thread.join();
	}

	delete threadPoolImpl;
}
