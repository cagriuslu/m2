#pragma once
#include <sys/select.h>

namespace m2::network {
	class FdSet {
		std::vector<int> _fds;
		fd_set _fd_set;

	public:
		void add_fd(int fd) { _fds.emplace_back(fd); }
		void remove_fd(int fd);
		void clear_fds();

		/// Returns max_fd + 1
		std::pair<int, fd_set*> prepare();

		bool is_set(int fd) const;
	};
}
