#include <Sprite.pb.h>
#include <m2/Options.h>
#include <m2/Log.h>
#include <m2/M2.h>

using namespace m2;

m2::pb::LogLevel m2::current_log_level = pb::LogLevel::INF;
bool m2::verbose = false;
bool m2::silent = false;
int m2::time_slowdown_factor = 1;
std::string m2::console_command;
bool m2::god_mode = false;
std::string m2::gOverrideResourceDir;

namespace {
	std::vector<std::string_view> to_argument_list(const int argc, char** argv) {
		std::vector<std::string_view> list;
		for (int i = 1; i < argc; ++i) {
			list.emplace_back(argv[i]);
		}
		return list;
	}

	std::optional<std::string_view> parse_argument(const std::vector<std::string_view>& argument_list, const std::string& raw_argument_key) {
		const std::string arg_flag = "--" + raw_argument_key; // Ex. --silent
		const std::string arg_key = "--" + raw_argument_key + "="; // Ex. --slowdown=2
		for (const auto& arg : argument_list) {
			if (arg == arg_flag) {
				return std::string_view{};
			}

			if (arg.starts_with(arg_key)) {
				auto raw_value = arg.substr(arg_key.size()); // Extract the string after '='
				// Check if the string has quotes (", ')
				if (2 <= raw_value.size() &&
					((raw_value.front() == '"' && raw_value.back() == '"') ||
					(raw_value.front() == '\'' && raw_value.back() == '\''))) {
						// Remove quotes
						return raw_value.substr(1, raw_value.size() - 2);
					}
				return raw_value;
			}
		}
		return std::nullopt;
	}
}

void ExecuteUtility::GenerateEmptySpriteSheet::Execute() const {
	LOG_INFO("Generating empty sprite sheet");
	pb::SpriteSheets ss;
	auto* sheet = ss.add_sheets();
	sheet->set_comment("Auto-generated");
	pb::for_each_enum_value<m2g::pb::SpriteType>([&sheet](const auto type) {
		sheet->add_sprites()->set_type(type);
	});
	printf("%s\n", pb::message_to_json_string(ss)->c_str());
}

expected<ExecutionStrategy> m2::load_options(const int argc, char** argv) {
	const auto arg_list = to_argument_list(argc, argv);

	if (const auto optional_log_level = parse_argument(arg_list, "log-level")) {
		if (not LogLevel_Parse(std::string{*optional_log_level}, &current_log_level)) {
			return make_unexpected("Invalid log level");
		}
		LOG_INFO("New log level", current_log_level);
	}

	if (parse_argument(arg_list, "verbose")) {
		LOG_INFO("Verbose mode activated");
		verbose = true;
	}

	if (parse_argument(arg_list, "silent")) {
		LOG_INFO("Silent mode activated");
		silent = true;
	}

	if (const auto slowdown_opt = parse_argument(arg_list, "slowdown")) {
		if (auto const slowdown_factor = strtol(std::string{*slowdown_opt}.c_str(), nullptr, 0);
			1 <= slowdown_factor) {
			time_slowdown_factor = static_cast<int>(slowdown_factor);
			LOG_INFO("New slowdown factor", time_slowdown_factor);
		} else {
			return make_unexpected("Invalid slowdown factor");
		}
	}

	if (auto console_opt = parse_argument(arg_list, "console")) {
		console_command = *console_opt;
		LOG_INFO("Console command", *console_opt);
	}

	if (parse_argument(arg_list, "god-mode")) {
		LOG_INFO("God mode");
		god_mode = true;
	}

	if (const auto resourceDir = parse_argument(arg_list, "resource-dir")) {
		LOG_INFO("Overriding resource dir", *resourceDir);
		gOverrideResourceDir = *resourceDir;
	}

	if (parse_argument(arg_list, "generate-empty-sprite-sheet")) {
		return ExecuteUtility{
			.variant = ExecuteUtility::GenerateEmptySpriteSheet{}
		};
	}

	return LoadGame{};
}
