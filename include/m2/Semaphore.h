#ifndef M2_SEMAPHORE_H
#define M2_SEMAPHORE_H

#include <mutex>
#include <condition_variable>

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
}

#endif //M2_SEMAPHORE_H
