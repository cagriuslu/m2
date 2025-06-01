#include <m2/containers/Pool.h>

m2::ShiftedPoolId m2::NextShiftedPoolId() {
	static PoolId gPoolId = 1;
	return static_cast<uint64_t>(gPoolId++) << gPoolIdShiftCount;
}
