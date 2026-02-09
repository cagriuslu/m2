#include <m2/ui/Console.h>
#include <m2/Game.h>
#include <regex>

m2::UiAction m2::HandleConsoleCommand(const std::string& command) {
	M2_GAME.console_output.emplace_back(">> " + command);

	if (command == "quit") {
		return MakeQuitAction();
	} else if (command == "close") {
		return MakeReturnAction();
	} else if (command.empty()) {
		// Do nothing
		return MakeContinueAction();
	} else {
		if (const auto result = M2_GAME.ExecuteCommand(command); std::holds_alternative<Game::UnknownCommand>(result)) {
			M2_GAME.console_output.emplace_back("Unknown command");
			return MakeContinueAction();
		} else if (std::holds_alternative<Game::CommandFail>(result)) {
			M2_GAME.console_output.emplace_back(std::get<Game::CommandFail>(result).error);
			return MakeContinueAction();
		} else {
			if (std::get<Game::CommandSuccess>(result).levelReplaced) {
				return MakeClearStackAction();
			} else {
				return MakeContinueAction();
			}
		}
	}
}
