#include <m2/Options.h>
#include <m2/M2.h>

m2::pb::LogLevel m2::current_log_level = m2::pb::LogLevel::INF;
bool m2::silent = false;

const std::string& m2::to_string(const pb::LogLevel& lvl) {
	return pb::LogLevel_Name(lvl);
}
