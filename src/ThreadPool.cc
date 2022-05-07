#include "m2/ThreadPool.hh"

m2::Semaphore::Semaphore(size_t initial_state) : _state(initial_state) {}

void m2::Semaphore::down(size_t count) {
	for (size_t i = 0; i < count; i++) {
		std::unique_lock<std::mutex> lock(_mutex);
		while (_state == 0) {
			_condvar.wait(lock);
		}
		_state--;
	}
}

void m2::Semaphore::up(size_t count) {
	_mutex.lock();
	_state += count;
	_mutex.unlock();
	_condvar.notify_all();
}

m2::ThreadPool::ThreadPool() : _idle_thread_count(std::thread::hardware_concurrency()), _quit(false) {
	auto thread_count = std::thread::hardware_concurrency();
	_threads.resize(thread_count);
	for (auto& thread : _threads) {
		thread = std::thread(thread_func, this);
	}
}

m2::ThreadPool::~ThreadPool() {
	_mutex.lock();
	_quit = true;
	_mutex.unlock();
	_condvar.notify_all();

	for (auto& thread : _threads) {
		thread.join();
	}
}

void m2::ThreadPool::queue(const Job& job) {
	_mutex.lock();
	_jobs.push(job);
	_mutex.unlock();
	_condvar.notify_one();
}

void m2::ThreadPool::wait() {
	unsigned downed = 0;
	while (downed < _threads.size()) {
		_idle_thread_count.down();
		downed++;
	}
	_idle_thread_count.up(_threads.size());
}

size_t m2::ThreadPool::thread_count() const {
	return _threads.size();
}

void m2::ThreadPool::thread_func(m2::ThreadPool* pool) {
	while (true) {
		Job job{};

		{
			std::unique_lock<std::mutex> lock(pool->_mutex);
			// Wait until quit or job
			while (!pool->_quit || pool->_jobs.empty()) {
				pool->_condvar.wait(lock);
			}
			// Check if quit
			if (pool->_quit) {
				return;
			}
			// Otherwise, job
			job = pool->_jobs.front();
			pool->_jobs.pop();
		}

		pool->_idle_thread_count.down();
		if (job) {
			job();
		}
		pool->_idle_thread_count.up();
	}
}
