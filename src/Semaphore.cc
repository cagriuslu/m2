#include <m2/Semaphore.h>

m2::Semaphore::Semaphore(size_t initial_state) : _state(initial_state) {}

void m2::Semaphore::down(size_t count) {
	while (0 < count) {
		std::unique_lock<std::mutex> lock(_mutex);
		// Wait until some resource available
		while (_state == 0) {
			_condvar.wait(lock);
		}
		// Decrease as much as possible
		if (_state < count) {
			count -= _state;
			_state = 0;
		} else { // count <= _state
			_state -= count;
			count = 0;
		}
	}
}

void m2::Semaphore::up(size_t count) {
	_mutex.lock();
	_state += count;
	_mutex.unlock();
	_condvar.notify_all();
}
