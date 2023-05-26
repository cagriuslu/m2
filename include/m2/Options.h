#pragma once
#include <Enums.pb.h>

namespace m2 {
	const std::string& to_string(const pb::LogLevel&);
	extern pb::LogLevel current_log_level;

	extern bool silent;
}
