#include <m2/network/FdSet.h>

void m2::network::FdSet::remove_fd(int fd) {
	auto it = std::find(_fds.begin(), _fds.end(), fd);
	if (it != _fds.end()) {
		_fds.erase(it);
	}
}

void m2::network::FdSet::clear_fds() {
	_fds.clear();
}

std::pair<int, fd_set*> m2::network::FdSet::prepare() {
	int max = 0;

	FD_ZERO(&_fd_set);
	for (auto fd : _fds) {
		FD_SET(fd, &_fd_set);
		max = std::max(max, fd);
	}

	return std::make_pair(max + 1, &_fd_set);
}

bool m2::network::FdSet::is_set(int fd) const {
	return FD_ISSET(fd, &_fd_set);
}
