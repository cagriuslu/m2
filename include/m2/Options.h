#pragma once
#include <Enums.pb.h>

#include "Meta.h"

namespace m2 {
	extern pb::LogLevel current_log_level;
	extern bool verbose;

	extern bool silent;

	extern int time_slowdown_factor;

	extern std::string console_command;

	extern bool god_mode;

	extern std::string gOverrideResourceDir;

	struct LoadGame {};
	struct ExecuteUtility {
		struct GenerateEmptySpriteSheet {
			void Execute() const;
		};
		std::variant<GenerateEmptySpriteSheet> variant;
	};
	using ExecutionStrategy = std::variant<LoadGame, ExecuteUtility>;
	/// Load options by parsing the program arguments
	expected<ExecutionStrategy> load_options(int argc, char** argv);
}
