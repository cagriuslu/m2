#pragma once
#include "../Meta.h"
#include <sys/select.h>

namespace m2::network {
	expected<int> select(int max_fd, fd_set* read, fd_set* write, uint64_t timeout_ms);
}
