#pragma once
#include "../Meta.h"
#include "FdSet.h"

namespace m2::network {
	expected<int> select(FdSet& read, FdSet& write, uint64_t timeout_ms);
}
