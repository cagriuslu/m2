#pragma once
#include <Enums.pb.h>

#include <m2/common/Meta.h>

namespace m2 {
	extern pb::LogLevel current_log_level;
	extern bool verbose;

	extern bool silent;
	extern bool diagnostics;

	/// If non-empty, the console should be opened and prefilled with the given string
	extern std::string console_command;
	/// If non-empty, the given command should be executed after initializing the game, but before the main menu.
	extern std::string background_command;

	extern std::string gOverrideResourceDir;

	namespace options {
		uint16_t GetPort();
		const std::string& GetMulticastDiscoveryAddress();
		uint16_t GetMulticastDiscoveryPort();
	}

	struct LoadGame {};
	struct ExecuteUtility {
		struct GenerateEmptySpriteSheet {
			static void Execute();
		};
		struct CompareLockstepSaves {
			std::string a, b;
			void Execute() const;
		};
		std::variant<GenerateEmptySpriteSheet, CompareLockstepSaves> variant;
	};
	using ExecutionStrategy = std::variant<LoadGame, ExecuteUtility>;
	/// Load options by parsing the program arguments
	expected<ExecutionStrategy> load_options(int argc, char** argv);
}
