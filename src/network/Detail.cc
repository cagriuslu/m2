#include <m2/network/Detail.h>
#include <sys/select.h>

m2::expected<int> m2::network::select(FdSet& read, FdSet& write, uint64_t timeout_ms) {
	auto [max_read, read_fd_set] = read.prepare();
	auto [max_write, write_fd_set] = write.prepare();

	timeval tv{};
	tv.tv_sec = static_cast<int64_t>(timeout_ms) / 1000;
	tv.tv_usec = static_cast<int32_t>((timeout_ms % 1000) * 1000);

	int select_result = ::select(std::max(max_read, max_write) + 1, read_fd_set, write_fd_set, nullptr, &tv);
	if (select_result == -1) {
		return make_unexpected(strerror(errno));
	} else {
		return select_result;
	}
}
