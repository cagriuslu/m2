#pragma once
#include "Types.h"
#include <m2/Meta.h>
#include <vector>
#include <string>

namespace m2::network {
	expected<std::vector<IpAddress>> GetInterfaces();
	std::vector<IpAddress> InferLanAddresses(const std::vector<IpAddress>&);
}
