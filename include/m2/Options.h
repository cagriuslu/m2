#ifndef M2_OPTIONS_H
#define M2_OPTIONS_H

#include <Enums.pb.h>

namespace m2 {
	const std::string& to_string(const pb::LogLevel&);
	extern pb::LogLevel current_log_level;

	extern bool silent;
}

#endif //M2_OPTIONS_H
