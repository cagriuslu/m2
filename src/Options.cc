#include <Sprite.pb.h>
#include <m2/thirdparty/protobuf/Compare.h>
#include <m2/thirdparty/protobuf/Deserialize.h>
#include <m2/thirdparty/protobuf/Json.h>
#include <m2/thirdparty/compression/Inflate.h>
#include <m2/Options.h>
#include <m2/Log.h>
#include <m2/M2.h>
#include <M2.orm.h>

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

namespace {
	uint16_t gPort = 1162;
}
uint16_t options::GetPort() {
	return gPort;
}

void ExecuteUtility::GenerateEmptySpriteSheet::Execute() {
	LOG_INFO("Generating empty sprite sheet");
	pb::SpriteSheets ss;
	auto* sheet = ss.add_sheets();
	sheet->set_comment("Auto-generated");
	pb::for_each_enum_value<m2g::pb::SpriteType>([&sheet](const auto type) {
		sheet->add_sprites()->set_type(type);
	});
	printf("%s\n", pb::message_to_json_string(ss)->c_str());
}

void ExecuteUtility::CompareLockstepSaves::Execute() const {
	LOG_INFO("Comparing lockstep save files...");

	auto [expectDatabaseA, expectDatabaseB] = TransformTuple(std::make_tuple(a, b), [](const auto& fpath) {
		return genORM::database::open(fpath.c_str());
	});
	if (not expectDatabaseA) { LOG_ERROR("Unable to open A", expectDatabaseA.error()); return; }
	if (not expectDatabaseB) { LOG_ERROR("Unable to open B", expectDatabaseB.error()); return; }
	auto databases = std::make_tuple(&*expectDatabaseA, &*expectDatabaseB);

	const auto [expectMetadataA, expectMetadataB] = TransformTuple(databases, [](auto* db) {
		return orm::LockstepGameMetadata::find_by_rowid(*db, 1);
	});
	if (not expectMetadataA) { LOG_ERROR("Unable to read metadata from A", expectMetadataA.error()); return; }
	if (not expectMetadataB) { LOG_ERROR("Unable to read metadata from B", expectMetadataB.error()); return; }
	if (not expectMetadataA.value()) { LOG_ERROR("Metadata not found in A"); return; }
	if (not expectMetadataB.value()) { LOG_ERROR("Metadata not found in B"); return; }
	const auto metadatas = std::make_tuple(&**expectMetadataA, &**expectMetadataB);

	const auto [gitHashA, gitHashB] = TransformTuple(metadatas, [](const auto* metadata) {
		const auto& hash = metadata->get_git_short_commit_hash();
		return std::string{hash.begin(), hash.end()};
	});
	if (gitHashA != gitHashB) { LOG_INFO("Git short commit hash mismatch", gitHashA, gitHashB); }
	const auto [playerCountA, playerCountB] = TransformTuple(metadatas, [](const auto* metadata) {
		return metadata->get_player_count();
	});
	if (playerCountA != playerCountB) { LOG_INFO("Player count mismatch", playerCountA, playerCountB); }
	const auto [selfIndexA, selfIndexB] = TransformTuple(metadatas, [](const auto* metadata) {
		return metadata->get_self_index();
	});
	if (selfIndexA != selfIndexB) { LOG_INFO("Self index mismatch", selfIndexA, selfIndexB); }
	const auto [levelBlueprintA, levelBlueprintB] = TransformTuple(metadatas, [](const auto* metadata) {
		return thirdparty::protobuf::ToMessage<pb::Level>(metadata->get_level_blueprint());
	});
	if ((levelBlueprintA && not levelBlueprintB) || (not levelBlueprintA && levelBlueprintB)
			|| (levelBlueprintA && levelBlueprintB && not thirdparty::protobuf::IsEqual(*levelBlueprintA, *levelBlueprintB))) {
		const auto serializedA = levelBlueprintA ? thirdparty::protobuf::ToJsonString(*levelBlueprintA).value_or("<error>") : "<null>";
		const auto serializedB = levelBlueprintB ? thirdparty::protobuf::ToJsonString(*levelBlueprintB).value_or("<error>") : "<null>";
		LOG_INFO(std::format("Level blueprint mismatch: levelBlueprintA={} levelBlueprintB={}", serializedA, serializedB));
	}
	const auto [levelNameA, levelNameB] = TransformTuple(metadatas, [](const auto* metadata) {
		const auto& levelName = metadata->get_level_name();
		return std::string{levelName.begin(), levelName.end()};
	});
	if (levelNameA != levelNameB) { LOG_INFO("Level name mismatch", levelNameA, levelNameB); }
	const auto [initParamsA, initParamsB] = TransformTuple(metadatas, [](const auto* metadata) {
		return thirdparty::protobuf::ToMessage<m2g::pb::LockstepGameInitParams>(metadata->get_game_init_params());
	});
	if ((initParamsA && not initParamsB) || (not initParamsA && initParamsB)
			|| (initParamsA && initParamsB && not thirdparty::protobuf::IsEqual(*initParamsA, *initParamsB))) {
		const auto serializedA = initParamsA ? thirdparty::protobuf::ToJsonString(*initParamsA).value_or("<error>") : "<null>";
		const auto serializedB = initParamsB ? thirdparty::protobuf::ToJsonString(*initParamsB).value_or("<error>") : "<null>";
		LOG_INFO(std::format("Game init params mismatch: initParamsA={} initParamsB={}", serializedA, serializedB));
	}
	const auto [durationA, durationB] = TransformTuple(metadatas, [](const auto* metadata) {
		return metadata->get_duration_tc();
	});
	if (durationA != durationB) { LOG_INFO("Duration mismatch", durationA ? ToString(*durationA) : "<null>", durationB ? ToString(*durationB) : "<null>"); }

	for (uint64_t rowId = 1; true; ++rowId) {
		const auto expectPlayerInputA = orm::LockstepPlayerInput::find_by_rowid(*expectDatabaseA, rowId);
		const auto expectPlayerInputB = orm::LockstepPlayerInput::find_by_rowid(*expectDatabaseB, rowId);
		if (not expectPlayerInputA) { LOG_ERROR("Unable to read player input from A", expectPlayerInputA.error()); return; }
		if (not expectPlayerInputB) { LOG_ERROR("Unable to read player input from B", expectPlayerInputB.error()); return; }
		if (not expectPlayerInputA.value() && not expectPlayerInputB.value()) { break; } // Player inputs are exhausted
		if (not expectPlayerInputA.value()) { LOG_INFO(std::format("Missing player input in A: rowId={}", rowId)); break; }
		if (not expectPlayerInputB.value()) { LOG_INFO(std::format("Missing player input in B: rowId={}", rowId)); break; }
		const auto playerInputs = std::make_tuple(&**expectPlayerInputA, &**expectPlayerInputB);

		const auto [timecodeA, timecodeB] = TransformTuple(playerInputs, [](const auto* input) {
			return input->get_timecode();
		});
		if (timecodeA != timecodeB) { LOG_INFO(std::format("Player input timecode mismatch: rowId={} timecodeA={} timecodeB={}", rowId, timecodeA, timecodeB)); }
		const auto [playerIndexA, playerIndexB] = TransformTuple(playerInputs, [](const auto* input) {
			return input->get_player_index();
		});
		if (playerIndexA != playerIndexB) { LOG_INFO(std::format("Player input player index mismatch: rowId={} playerIndexA={} playerIndexB={}", rowId, playerIndexA, playerIndexB)); }
		const auto [playerInputA, playerInputB] = TransformTuple(playerInputs, [](const auto* input) {
			return thirdparty::protobuf::ToMessage<m2g::pb::LockstepPlayerInput>(input->get_player_input());
		});
		if ((playerInputA && not playerInputB) || (not playerInputA && playerInputB)
				|| (playerInputA && playerInputB && not thirdparty::protobuf::IsEqual(*playerInputA, *playerInputB))) {
			const auto serializedA = playerInputA ? thirdparty::protobuf::ToJsonString(*playerInputA).value_or("<error>") : "<null>";
			const auto serializedB = playerInputB ? thirdparty::protobuf::ToJsonString(*playerInputB).value_or("<error>") : "<null>";
			LOG_INFO(std::format("Player input mismatch: rowId={} playerInputA={} playerInputB={}", rowId, serializedA, serializedB));
		}
	}

	for (uint64_t rowId = 1; true; ++rowId) {
		const auto expectStateReportA = orm::LockstepDebugStateReport::find_by_rowid(*expectDatabaseA, rowId);
		const auto expectStateReportB = orm::LockstepDebugStateReport::find_by_rowid(*expectDatabaseB, rowId);
		if (not expectStateReportA) { LOG_ERROR("Unable to read debug state report from A", expectStateReportA.error()); return; }
		if (not expectStateReportB) { LOG_ERROR("Unable to read debug state report from B", expectStateReportB.error()); return; }
		if (not expectStateReportA.value() && not expectStateReportB.value()) { break; }
		if (not expectStateReportA.value()) { LOG_INFO(std::format("Missing debug state report in A: rowId={}", rowId)); break; }
		if (not expectStateReportB.value()) { LOG_INFO(std::format("Missing debug state report in B: rowId={}", rowId)); break; }
		const auto stateReports = std::make_tuple(&**expectStateReportA, &**expectStateReportB);

		const auto [timecodeA, timecodeB] = TransformTuple(stateReports, [](const auto* report) {
			return report->get_timecode();
		});
		if (timecodeA != timecodeB) { LOG_INFO(std::format("Debug state report timecode mismatch: rowId={} timecodeA={} timecodeB={}", rowId, timecodeA, timecodeB)); }
		const auto [reportUncompressedSizeA, reportUncompressedSizeB] = TransformTuple(stateReports, [](const auto* report) {
			return report->get_report_uncompressed_size();
		});
		if (reportUncompressedSizeA != reportUncompressedSizeB) { LOG_INFO(std::format("Debug state report uncompressed size mismatch: rowId={} uncompressedSizeA={} uncompressedSizeB={}", rowId, reportUncompressedSizeA, reportUncompressedSizeB)); }

		const auto reportA = [&] -> std::optional<pb::LockstepDebugStateReport> {
			const auto& compressedBytes = std::get<0>(stateReports)->get_report();
			if (const auto expectUncompressed = thirdparty::compression::Inflate(compressedBytes, reportUncompressedSizeA)) {
				return thirdparty::protobuf::ToMessage<pb::LockstepDebugStateReport>(*expectUncompressed);
			}
			return std::nullopt;
		}();
		const auto reportB = [&] -> std::optional<pb::LockstepDebugStateReport> {
			const auto& compressedBytes = std::get<1>(stateReports)->get_report();
			if (const auto expectUncompressed = thirdparty::compression::Inflate(compressedBytes, reportUncompressedSizeB)) {
				return thirdparty::protobuf::ToMessage<pb::LockstepDebugStateReport>(*expectUncompressed);
			}
			return std::nullopt;
		}();
		if ((reportA && not reportB) || (not reportA && reportB) || (reportA && reportB && not thirdparty::protobuf::IsEqual(*reportA, *reportB))) {
			const auto serializedA = reportA ? thirdparty::protobuf::ToJsonString(*reportA).value_or("<error>") : "<null>";
			const auto serializedB = reportB ? thirdparty::protobuf::ToJsonString(*reportB).value_or("<error>") : "<null>";
			LOG_INFO(std::format("Debug state report mismatch: rowId={} reportA={} reportB={}", rowId, serializedA, serializedB));
		}
	}

	LOG_INFO("Comparison done");
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

	if (const auto portStr = parse_argument(arg_list, "port")) {
		if (auto const port = strtol(std::string{*portStr}.c_str(), nullptr, 0); 0 < port) {
			gPort = static_cast<uint16_t>(port);
			LOG_INFO("Port", gPort);
		} else {
			return make_unexpected("Invalid port");
		}
	}

	if (parse_argument(arg_list, "generate-empty-sprite-sheet")) {
		return ExecuteUtility{
			.variant = ExecuteUtility::GenerateEmptySpriteSheet{}
		};
	}

	if (const auto saveA = parse_argument(arg_list, "compare-lockstep-save-a")) {
		if (const auto saveB = parse_argument(arg_list, "compare-lockstep-save-b")) {
			return ExecuteUtility{
				.variant = ExecuteUtility::CompareLockstepSaves{
					.a = std::string{*saveA},
					.b = std::string{*saveB}
				}
			};
		}
	}

	return LoadGame{};
}
