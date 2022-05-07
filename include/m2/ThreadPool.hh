#ifndef M2_THREADPOOL_H
#define M2_THREADPOOL_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>

namespace m2 {
	class Semaphore {
		size_t _state;
		std::mutex _mutex;
		std::condition_variable _condvar;

	public:
		explicit Semaphore(size_t initial_state = 1);
		void down(size_t count = 1);
		void up(size_t count = 1);
	};

	using Job = std::function<void(void)>;

	class ThreadPool {
		std::vector<std::thread> _threads;
		std::mutex _mutex;
		std::condition_variable _condvar;
		Semaphore _idle_thread_count;
		bool _quit;
		std::queue<Job> _jobs;

	public:
		ThreadPool();
		~ThreadPool();
		void queue(const Job& job);
		void wait();

		[[nodiscard]] size_t thread_count() const;

	private:
		static void thread_func(ThreadPool* pool);
	};
}

#endif //M2_THREADPOOL_H
