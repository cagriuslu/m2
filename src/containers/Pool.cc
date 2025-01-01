#include <m2/containers/Pool.h>

m2::ShiftedPoolId m2::next_shifted_pool_id() {
	static PoolId global_pool_id = 1;
	return static_cast<uint64_t>(global_pool_id++) << 48;
}