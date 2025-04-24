#pragma once
#include <Enums.pb.h>

#include "Meta.h"

namespace m2 {
	extern pb::LogLevel current_log_level;

	extern bool silent;

	extern int time_slowdown_factor;

	extern std::string console_command;

	extern bool god_mode;

	extern std::string gOverrideResourceDir;

	/// Load options by parsing the program arguments
	void_expected load_options(int argc, char** argv);
}
