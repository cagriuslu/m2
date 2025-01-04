#include <m2/level_editor/State.h>
#include <m2/level_editor/Ui.h>
#include <m2/Game.h>
#include <m2/game/object/Ghost.h>
#include <m2/game/object/Placeholder.h>
#include <m2/game/Selection.h>

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	constexpr SDL_Color CONFIRMED_SELECTION_COLOR = {0, 255, 0, 80};
}

void m2::ledit::State::PaintMode::select_sprite_type(m2g::pb::SpriteType sprite_type) {
	if (selected_sprite_ghost_id) {
		M2_DEFER(m2::create_object_deleter(selected_sprite_ghost_id));
	}
	if (sprite_type) {
		selected_sprite_type = sprite_type;
		selected_sprite_ghost_id = obj::create_ghost(sprite_type);

		const auto sprite_name = pb::enum_name(sprite_type);
		M2_LEVEL.display_message(sprite_name);
	}
}
void m2::ledit::State::PaintMode::paint_sprite(const VecI& position) {
	if (!position.is_negative()) {
		// Delete previous placeholder
		EraseMode::erase_position(position, std::get<ledit::State>(M2_LEVEL.type_state).selected_layer);
		// Create/Replace placeholder
		std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders[I(std::get<ledit::State>(M2_LEVEL.type_state).selected_layer)][position] = std::make_pair(obj::create_background_placeholder(VecF{position}, selected_sprite_type, std::get<ledit::State>(M2_LEVEL.type_state).selected_layer), selected_sprite_type);
	}
}
m2::ledit::State::PaintMode::~PaintMode() {
	if (selected_sprite_ghost_id) {
		M2_LEVEL.deferred_actions.push(create_object_deleter(selected_sprite_ghost_id));
	}
}
void m2::ledit::State::EraseMode::erase_position(const VecI& position) {
	erase_position(position, std::get<ledit::State>(M2_LEVEL.type_state).selected_layer);
}
void m2::ledit::State::EraseMode::erase_position(const VecI &position, BackgroundLayer layer) {
	auto placeholders_it = std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders[I(layer)].find(position);
	if (placeholders_it != std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders[I(layer)].end()) {
		M2_LEVEL.deferred_actions.push(create_object_deleter(placeholders_it->second.first));
		std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders[I(layer)].erase(placeholders_it);
	}
}
void m2::ledit::State::PlaceMode::select_object_type(m2g::pb::ObjectType object_type) {
	if (selected_sprite_ghost_id) {
		M2_DEFER(create_object_deleter(selected_sprite_ghost_id));
	}
	if (object_type) {
		selected_object_type = object_type;
		selected_sprite_ghost_id = obj::create_ghost(M2_GAME.object_main_sprites[object_type]);
	}
}
void m2::ledit::State::PlaceMode::select_group_type(m2g::pb::GroupType group_type) { selected_group_type = group_type; }
void m2::ledit::State::PlaceMode::select_group_instance(unsigned group_instance) { selected_group_instance = group_instance; }
void m2::ledit::State::PlaceMode::place_object(const VecI& position) const {
	if (!position.is_negative()) {
		// Delete previous placeholder
		RemoveMode::remove_object(position);
		// Create/Replace placeholder
		pb::LevelObject level_object;
		level_object.mutable_position()->set_x(position.x);
		level_object.mutable_position()->set_y(position.y);
		level_object.set_type(selected_object_type);
		level_object.mutable_group()->set_type(selected_group_type);
		level_object.mutable_group()->set_instance(selected_group_instance);
		std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders[position] = std::make_pair(obj::create_foreground_placeholder(VecF{position}, M2_GAME.object_main_sprites[selected_object_type]), level_object);
	}
}
m2::ledit::State::PlaceMode::~PlaceMode() {
	if (selected_sprite_ghost_id) {
		M2_LEVEL.deferred_actions.push(create_object_deleter(selected_sprite_ghost_id));
	}
}
void m2::ledit::State::RemoveMode::remove_object(const VecI &position) {
	auto placeholders_it = std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders.find(position);
	if (placeholders_it != std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders.end()) {
		M2_LEVEL.deferred_actions.push(create_object_deleter(placeholders_it->second.first));
		std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders.erase(placeholders_it);
	}
}
std::optional<m2g::pb::SpriteType> m2::ledit::State::PickMode::lookup_background_sprite(const VecI& position) {
	auto it = std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders[I(std::get<ledit::State>(M2_LEVEL.type_state).selected_layer)].find(position);
	if (it != std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders[I(std::get<ledit::State>(M2_LEVEL.type_state).selected_layer)].end()) {
		return it->second.second;
	}
	return {};
}
std::optional<m2::pb::LevelObject> m2::ledit::State::PickMode::lookup_foreground_object(const VecI& position) {
	auto it = std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders.find(position);
	if (it != std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders.end()) {
		return it->second.second;
	}
	return {};
}
m2::ledit::State::SelectMode::SelectMode() {
	// Enable selection
	Events::enable_primary_selection(RectI{M2_GAME.Dimensions().Game()});
}
m2::ledit::State::SelectMode::~SelectMode() {
	Events::disable_primary_selection();
}
void m2::ledit::State::SelectMode::on_draw() const {
	// Draw selection
	if (auto positions = SelectionResult{M2_GAME.events}.primary_int_selection_position_m(); positions) {
		positions->first.for_each_cell_in_between(positions->second, [=](const VecI& cell) {
			Graphic::color_cell(cell, SELECTION_COLOR);
		});
	}
	// Draw clipboard
	if (clipboard_position_1 && clipboard_position_2) {
		clipboard_position_1->for_each_cell_in_between(*clipboard_position_2, [=](const VecI& cell) {
			Graphic::color_cell(cell, CONFIRMED_SELECTION_COLOR);
		});
	}
}
void m2::ledit::State::SelectMode::shift_right() {
	if (auto selection_result = SelectionResult{M2_GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		auto min_x = positions->first.x;
		auto min_y = positions->first.y;
		auto max_y = positions->second.y;
		auto shift_count = abs(positions->first.x - positions->second.x) + 1;
		for (auto& placeholders : std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders) {
			level_editor::shift_placeholders(placeholders, M2_LEVEL.objects, min_x, INT32_MAX, min_y, max_y, shift_count, 0);
		}
		level_editor::shift_placeholders(std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders, M2_LEVEL.objects, min_x, INT32_MAX, min_y, max_y, shift_count, 0);
	}
}
void m2::ledit::State::SelectMode::shift_down() {
	if (auto selection_result = SelectionResult{M2_GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		auto min_x = positions->first.x;
		auto max_x = positions->second.x;
		auto min_y = positions->first.y;
		auto shift_count = abs(positions->first.y - positions->second.y) + 1;
		for (auto& placeholders : std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders) {
			level_editor::shift_placeholders(placeholders, M2_LEVEL.objects, min_x, max_x, min_y, INT32_MAX, 0, shift_count);
		}
		level_editor::shift_placeholders(std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders, M2_LEVEL.objects, min_x, max_x, min_y, INT32_MAX, 0, shift_count);
	}
}
void m2::ledit::State::SelectMode::copy() {
	if (auto selection_result = SelectionResult{M2_GAME.events}; selection_result.is_primary_selection_finished()) {
		clipboard_layer = std::get<ledit::State>(M2_LEVEL.type_state).selected_layer;
		auto positions = selection_result.primary_int_selection_position_m();
		clipboard_position_1 = positions->first;
		clipboard_position_2 = positions->second;
		M2_GAME.events.reset_primary_selection();
	}
}
void m2::ledit::State::SelectMode::paste_bg() {
	if (auto selection_result = SelectionResult{M2_GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		if (clipboard_position_1 && clipboard_position_2) {
			clipboard_position_1->for_each_cell_in_between(*clipboard_position_2, [&](const VecI& cell) {
				auto it = std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders[I(*clipboard_layer)].find(cell);
				if (it != std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders[I(*clipboard_layer)].end()) {
					auto new_position = positions->first + (cell - *clipboard_position_1);
					auto sprite_type = it->second.second;
					EraseMode::erase_position(new_position, std::get<ledit::State>(M2_LEVEL.type_state).selected_layer);
					std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders[I(std::get<ledit::State>(M2_LEVEL.type_state).selected_layer)][new_position] = std::make_pair(obj::create_background_placeholder(VecF{new_position}, sprite_type, std::get<ledit::State>(M2_LEVEL.type_state).selected_layer), sprite_type);
				}
			});
		}
	}
}
void m2::ledit::State::SelectMode::paste_fg() {
	if (auto selection_result = SelectionResult{M2_GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		if (clipboard_position_1 && clipboard_position_2) {
			clipboard_position_1->for_each_cell_in_between(*clipboard_position_2, [&](const VecI& cell) {
				auto it = std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders.find(cell);
				if (it != std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders.end()) {
					auto new_position = positions->first + (cell - *clipboard_position_1);
					auto level_object = it->second.second;
					level_object.mutable_position()->set_x(new_position.x);
					level_object.mutable_position()->set_y(new_position.y);
					RemoveMode::remove_object(new_position);
					std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders[new_position] = std::make_pair(obj::create_foreground_placeholder(VecF{new_position}, M2_GAME.object_main_sprites[level_object.type()]), level_object);
				}
			});
		}
	}
}
void m2::ledit::State::SelectMode::erase() {
	if (auto selection_result = SelectionResult{M2_GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		positions->first.for_each_cell_in_between(positions->second, [&](const VecI& cell) {
			ledit::State::EraseMode::erase_position(cell, std::get<ledit::State>(M2_LEVEL.type_state).selected_layer);
		});
	}
}
void m2::ledit::State::SelectMode::remove() {
	if (auto selection_result = SelectionResult{M2_GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		positions->first.for_each_cell_in_between(positions->second, [](const VecI& cell) {
			ledit::State::RemoveMode::remove_object(cell);
		});
	}
}
m2::ui::Action m2::ledit::State::SelectMode::rfill() {
	if (auto selection_result = SelectionResult{M2_GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();

		auto action = ui::Panel::create_and_run_blocking(&level_editor::ui::fill_dialog);
		if (action.IsReturn() && not rfill_sprite_types.empty()) {
			positions->first.for_each_cell_in_between(positions->second, [&](const VecI& cell) {
				auto index = rand(static_cast<uint32_t>(rfill_sprite_types.size()));
				auto sprite_type = rfill_sprite_types[index];
				EraseMode::erase_position(cell, std::get<ledit::State>(M2_LEVEL.type_state).selected_layer);
				std::get<ledit::State>(M2_LEVEL.type_state)
				    .bg_placeholders[I(std::get<ledit::State>(M2_LEVEL.type_state).selected_layer)][cell] = std::make_pair(
							obj::create_background_placeholder(VecF{cell}, sprite_type, std::get<ledit::State>(M2_LEVEL.type_state).selected_layer),
							sprite_type);
			});
		} else {
			return action;
		}
	}
	return ui::MakeContinueAction();
}
void m2::ledit::State::ShiftMode::shift(MAYBE const VecI& position) const {
	if (shift_type == ShiftType::RIGHT) {
		for (auto& placeholders : std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders) {
			level_editor::shift_placeholders(placeholders, M2_LEVEL.objects, position.x, INT32_MAX, INT32_MIN, INT32_MAX, 1, 0);
		}
		level_editor::shift_placeholders(std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders, M2_LEVEL.objects, position.x, INT32_MAX, INT32_MIN, INT32_MAX, 1, 0);
	} else if (shift_type == ShiftType::DOWN) {
		for (auto& placeholders : std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders) {
			level_editor::shift_placeholders(placeholders, M2_LEVEL.objects, INT32_MIN, INT32_MAX, position.y, INT32_MAX, 0, 1);
		}
		level_editor::shift_placeholders(std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders, M2_LEVEL.objects, INT32_MIN, INT32_MAX, position.y, INT32_MAX, 0, 1);
	} else if (shift_type == ShiftType::RIGHT_N_DOWN) {
		for (auto& placeholders : std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders) {
			level_editor::shift_placeholders(placeholders, M2_LEVEL.objects, position.x, INT32_MAX, position.y, INT32_MAX, 1, 1);
		}
		level_editor::shift_placeholders(std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders, M2_LEVEL.objects, position.x, INT32_MAX, position.y, INT32_MAX, 1, 1);
	}
}
void m2::ledit::State::deactivate_mode() {
	mode.emplace<std::monostate>();
}
void m2::ledit::State::activate_paint_mode() {
	mode.emplace<PaintMode>();
	std::get<PaintMode>(mode).select_sprite_type(M2_GAME.level_editor_background_sprites[0]);
}
void m2::ledit::State::activate_erase_mode() {
	mode.emplace<EraseMode>();
}
void m2::ledit::State::activate_place_mode() {
	mode.emplace<PlaceMode>();
	std::get<PlaceMode>(mode).select_object_type(M2_GAME.object_main_sprites.begin()->first);
	std::get<PlaceMode>(mode).select_group_type(m2g::pb::GroupType::NO_GROUP);
	std::get<PlaceMode>(mode).select_group_instance(0);
}
void m2::ledit::State::activate_remove_mode() {
	mode.emplace<RemoveMode>();
}
void m2::ledit::State::activate_pick_mode() {
	mode.emplace<PickMode>();
}
void m2::ledit::State::activate_select_mode() {
	mode.emplace<SelectMode>();
}
void m2::ledit::State::activate_shift_mode() {
	mode.emplace<ShiftMode>();
}
void m2::ledit::State::save() {
	pb::Level level;
	// Start from the current level
	if (auto lb = M2_LEVEL.level_blueprint(); lb) {
		level = *lb;
	}

	// Clear fields that'll be filled here
	level.clear_background_layers();
	level.clear_objects();

	for (int i = 0; i < I(BackgroundLayer::n); ++i) {
		for (const auto& [position, pair] : std::get<ledit::State>(M2_LEVEL.type_state).bg_placeholders[i]) {
			pb::mutable_get_or_create(level.mutable_background_layers(), i);
			auto* row = pb::mutable_get_or_create(level.mutable_background_layers(i)->mutable_background_rows(), position.y); // Get or create row
			*(pb::mutable_get_or_create(row->mutable_items(), position.x)) = pair.second; // Set sprite type
		}
	}
	for (const auto& [position, pair] : std::get<ledit::State>(M2_LEVEL.type_state).fg_placeholders) {
		level.add_objects()->CopyFrom(pair.second);
	}
	pb::message_to_json_file(level, *M2_LEVEL.path());
}
