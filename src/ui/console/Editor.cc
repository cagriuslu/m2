#include <m2/ui/console/Editor.h>
#include <m2/Game.hh>
#include <regex>
#include <filesystem>

m2::ui::Action m2::ui::con::editor(const std::string &command) {
	std::smatch match_results;
	if (std::regex_match(command, match_results, std::regex{"editor\\s+(.+)"})) {
		std::filesystem::path file_path{match_results.str(1)};
		auto load_result = GAME.load_editor(file_path);
		if (load_result) {
			return Action::RETURN;
		}
		GAME.console_output.emplace_back(load_result.error());
	} else {
		GAME.console_output.emplace_back("editor usage:");
		GAME.console_output.emplace_back(".. file_name - open editor with file" );
	}
	return Action::CONTINUE;
}
