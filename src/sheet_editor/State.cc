#include <m2/sheet_editor/State.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/IntegerSelection.h>
#include <m2/protobuf/Detail.h>

#include <utility>

using namespace m2;
using namespace m2::sedit;

expected<m2::sedit::State> m2::sedit::State::create(const std::filesystem::path& path) {
	if (std::filesystem::exists(path)) {
		return protobuf::json_file_to_message<pb::SpriteSheets>(path)
		        .and_then([](const auto& _sheets_pb) -> expected<State> {
					return State{_sheets_pb};
				});
	} else {
		return State{{}}; // Empty sprite sheet
	}
}

ui::Action m2::sedit::State::execute_add_sheet_dialog(bool edit) {
	std::string initial_resource;
	unsigned initial_ppm = 32;
	if (edit && selected_sheet && *selected_sheet < sheets_pb.sheets_size()) {
		// Lookup initial resource and ppm
		initial_resource = sheets_pb.sheets(*selected_sheet).resource();
		initial_ppm = sheets_pb.sheets(*selected_sheet).ppm();
	}

	const ui::Blueprint add_sheet_dialog_blueprint = {
			.w = 160, .h = 90,
			.border_width_px = 0,
			.background_color = {0, 0, 0, 255},
			.widgets = {
					ui::WidgetBlueprint{
							.x = 60, .y = 10, .w = 40, .h = 10,
							.border_width_px = 0,
							.padding_width_px = 2,
							.variant = ui::widget::TextBlueprint{
									.initial_text = "Add Sheet"
							}
					},
					ui::WidgetBlueprint{
							.x = 18, .y = 30, .w = 40, .h = 10,
							.border_width_px = 0,
							.padding_width_px = 2,
							.variant = ui::widget::TextBlueprint{
									.initial_text = "Resource",
									.alignment = ui::TextAlignment::RIGHT
							}
					},
					ui::WidgetBlueprint{
							.x = 60, .y = 30, .w = 40, .h = 10,
							.variant = ui::widget::TextInputBlueprint{
								.initial_text = initial_resource
							}
					},
					ui::WidgetBlueprint{
							.x = 18, .y = 50, .w = 40, .h = 10,
							.border_width_px = 0,
							.padding_width_px = 2,
							.variant = ui::widget::TextBlueprint{
									.initial_text = "PPM",
									.alignment = ui::TextAlignment::RIGHT
							}
					},
					ui::WidgetBlueprint{
							.x = 60, .y = 50, .w = 40, .h = 10,
							.variant = ui::widget::IntegerSelectionBlueprint{
								.min_value = 1,
								.max_value = 4096,
								.initial_value = (int) initial_ppm
							}
					},
					ui::WidgetBlueprint{
							.x = 35, .y = 70, .w = 40, .h = 10,
							.border_width_px = 1,
							.padding_width_px = 2,
							.variant = ui::widget::TextBlueprint{
									.initial_text = "Cancel",
									.action_callback = []() -> ui::Action {
										return ui::Action::RETURN;
									}
							}
					},
					ui::WidgetBlueprint{
							.x = 85, .y = 70, .w = 40, .h = 10,
							.border_width_px = 1,
							.padding_width_px = 2,
							.variant = ui::widget::TextBlueprint{
									.initial_text = edit ? "Edit" : "Add",
									.action_callback = [edit]() -> ui::Action {
										// Check if resource entered
										auto resource = dynamic_cast<ui::widget::TextInput&>(*LEVEL.sheet_editor_state->add_sheet_dialog->widgets[2]).text_input.str();
										if (resource.empty()) {
											return ui::Action::CONTINUE;
										}
										// Gather input
										auto ppm = dynamic_cast<ui::widget::IntegerSelection&>(*LEVEL.sheet_editor_state->add_sheet_dialog->widgets[4]).value;
										// Add sheet
										auto result = LEVEL.sheet_editor_state->add_sheet(resource, ppm, edit);
										if (!result) {
											LOG_ERROR(result.error().c_str());
										}
										return ui::Action::RETURN;
									}
							}
					}
			}
	};

	add_sheet_dialog = std::make_unique<ui::State>(&add_sheet_dialog_blueprint);
	auto action = add_sheet_dialog->execute();
	add_sheet_dialog.reset();
	return action;
}

void_expected m2::sedit::State::add_sheet(const std::string& resource, unsigned ppm, bool edit) {
	if (edit) {
		if (selected_sheet && *selected_sheet < sheets_pb.sheets_size()) {
			sheets_pb.mutable_sheets(*selected_sheet)->set_resource(resource);
			sheets_pb.mutable_sheets(*selected_sheet)->set_ppm(ppm);
		} else {
			throw M2FATAL("Implementation error");
		}
	} else {
		// Check if there already exists such resource
		for (const auto& sheet : sheets_pb.sheets()) {
			if (sheet.resource() == resource) {
				return make_unexpected("Sheet with same resource already exists");
			}
		}
		// Add new sheet
		auto new_sheet = sheets_pb.add_sheets();
		new_sheet->set_resource(resource);
		new_sheet->set_ppm(ppm);
	}
	return {};
}

m2::sedit::State::State(pb::SpriteSheets _sheets) : sheets_pb(std::move(_sheets)) {}
