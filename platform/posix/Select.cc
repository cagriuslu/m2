#include <m2/network/Select.h>
#include <sys/select.h>
#include <sys/types.h>

m2::expected<int> m2::network::select(int max_fd, fd_set* read, fd_set* write, uint64_t timeout_ms) {
	timeval tv{};
	tv.tv_sec = static_cast<int64_t>(timeout_ms) / 1000;
	tv.tv_usec = static_cast<int32_t>((timeout_ms % 1000) * 1000);

	int select_result = ::select(max_fd + 1, read, write, nullptr, &tv);
	if (select_result == -1) {
		return make_unexpected(strerror(errno));
	} else {
		return select_result;
	}
}
