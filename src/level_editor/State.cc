#include <m2/level_editor/State.h>
#include <m2/Game.h>
#include <m2/object/Ghost.h>
#include <m2/object/Placeholder.h>
#include <m2/game/Selection.h>

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	constexpr SDL_Color CONFIRMED_SELECTION_COLOR = {0, 255, 0, 80};
}

void m2::ledit::State::PaintMode::select_sprite_type(m2g::pb::SpriteType sprite_type) {
	if (selected_sprite_ghost_id) {
		GAME.add_deferred_action(m2::create_object_deleter(selected_sprite_ghost_id));
	}
	if (sprite_type) {
		selected_sprite_type = sprite_type;
		selected_sprite_ghost_id = obj::create_ghost(GAME.get_sprite(sprite_type));
	}
}
void m2::ledit::State::PaintMode::paint_sprite(const VecI& position) {
	if (!position.is_negative()) {
		// Delete previous placeholder
		EraseMode::erase_position(position, selected_layer);
		// Create/Replace placeholder
		LEVEL.level_editor_state->bg_placeholders[I(selected_layer)][position] = std::make_pair(obj::create_background_placeholder(VecF{position}, GAME.get_sprite(selected_sprite_type), selected_layer), selected_sprite_type);
	}
}
m2::ledit::State::PaintMode::~PaintMode() {
	if (selected_sprite_ghost_id) {
		LEVEL.deferred_actions.push_back(create_object_deleter(selected_sprite_ghost_id));
	}
}
void m2::ledit::State::EraseMode::erase_position(const VecI& position) {
	erase_position(position, selected_layer);
}
void m2::ledit::State::EraseMode::erase_position(const VecI &position, BackgroundLayer layer) {
	auto placeholders_it = LEVEL.level_editor_state->bg_placeholders[I(layer)].find(position);
	if (placeholders_it != LEVEL.level_editor_state->bg_placeholders[I(layer)].end()) {
		LEVEL.deferred_actions.push_back(create_object_deleter(placeholders_it->second.first));
		LEVEL.level_editor_state->bg_placeholders[I(layer)].erase(placeholders_it);
	}
}
void m2::ledit::State::PlaceMode::select_object_type(m2g::pb::ObjectType object_type) {
	if (selected_sprite_ghost_id) {
		GAME.add_deferred_action(create_object_deleter(selected_sprite_ghost_id));
	}
	if (object_type) {
		selected_object_type = object_type;
		selected_sprite_ghost_id = obj::create_ghost(GAME.get_sprite(GAME.level_editor_object_sprites[object_type]));
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
		LEVEL.level_editor_state->fg_placeholders[position] = std::make_pair(obj::create_foreground_placeholder(VecF{position}, GAME.get_sprite(GAME.level_editor_object_sprites[selected_object_type])), level_object);
	}
}
m2::ledit::State::PlaceMode::~PlaceMode() {
	if (selected_sprite_ghost_id) {
		LEVEL.deferred_actions.push_back(create_object_deleter(selected_sprite_ghost_id));
	}
}
void m2::ledit::State::RemoveMode::remove_object(const VecI &position) {
	auto placeholders_it = LEVEL.level_editor_state->fg_placeholders.find(position);
	if (placeholders_it != LEVEL.level_editor_state->fg_placeholders.end()) {
		LEVEL.deferred_actions.push_back(create_object_deleter(placeholders_it->second.first));
		LEVEL.level_editor_state->fg_placeholders.erase(placeholders_it);
	}
}
std::optional<m2g::pb::SpriteType> m2::ledit::State::PickMode::lookup_background_sprite(const VecI& position) {
	auto it = LEVEL.level_editor_state->bg_placeholders[I(selected_layer)].find(position);
	if (it != LEVEL.level_editor_state->bg_placeholders[I(selected_layer)].end()) {
		return it->second.second;
	}
	return {};
}
std::optional<m2::pb::LevelObject> m2::ledit::State::PickMode::lookup_foreground_object(const VecI& position) {
	auto it = LEVEL.level_editor_state->fg_placeholders.find(position);
	if (it != LEVEL.level_editor_state->fg_placeholders.end()) {
		return it->second.second;
	}
	return {};
}
m2::ledit::State::SelectMode::SelectMode() {
	// Enable selection
	Events::enable_primary_selection(RectI{GAME.dimensions().game});
}
m2::ledit::State::SelectMode::~SelectMode() {
	Events::disable_primary_selection();
}
void m2::ledit::State::SelectMode::on_draw() const {
	// Draw selection
	if (auto positions = SelectionResult{GAME.events}.primary_int_selection_position_m(); positions) {
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
	if (auto selection_result = SelectionResult{GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		auto min_x = positions->first.x;
		auto min_y = positions->first.y;
		auto max_y = positions->second.y;
		auto shift_count = abs(positions->first.x - positions->second.x) + 1;
		for (auto& placeholders : LEVEL.level_editor_state->bg_placeholders) {
			level_editor::shift_placeholders(placeholders, LEVEL.objects, min_x, INT32_MAX, min_y, max_y, shift_count, 0);
		}
		level_editor::shift_placeholders(LEVEL.level_editor_state->fg_placeholders, LEVEL.objects, min_x, INT32_MAX, min_y, max_y, shift_count, 0);
	}
}
void m2::ledit::State::SelectMode::shift_down() {
	if (auto selection_result = SelectionResult{GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		auto min_x = positions->first.x;
		auto max_x = positions->second.x;
		auto min_y = positions->first.y;
		auto shift_count = abs(positions->first.y - positions->second.y) + 1;
		for (auto& placeholders : LEVEL.level_editor_state->bg_placeholders) {
			level_editor::shift_placeholders(placeholders, LEVEL.objects, min_x, max_x, min_y, INT32_MAX, 0, shift_count);
		}
		level_editor::shift_placeholders(LEVEL.level_editor_state->fg_placeholders, LEVEL.objects, min_x, max_x, min_y, INT32_MAX, 0, shift_count);
	}
}
void m2::ledit::State::SelectMode::copy() {
	if (auto selection_result = SelectionResult{GAME.events}; selection_result.is_primary_selection_finished()) {
		clipboard_layer = selected_layer;
		auto positions = selection_result.primary_int_selection_position_m();
		clipboard_position_1 = positions->first;
		clipboard_position_2 = positions->second;
		GAME.events.reset_primary_selection();
	}
}
void m2::ledit::State::SelectMode::paste_bg() {
	if (auto selection_result = SelectionResult{GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		if (clipboard_position_1 && clipboard_position_2) {
			clipboard_position_1->for_each_cell_in_between(*clipboard_position_2, [&](const VecI& cell) {
				auto it = LEVEL.level_editor_state->bg_placeholders[I(*clipboard_layer)].find(cell);
				if (it != LEVEL.level_editor_state->bg_placeholders[I(*clipboard_layer)].end()) {
					auto new_position = positions->first + (cell - *clipboard_position_1);
					auto sprite_type = it->second.second;
					LEVEL.level_editor_state->bg_placeholders[I(selected_layer)][new_position] = std::make_pair(obj::create_background_placeholder(VecF{new_position}, GAME.get_sprite(sprite_type), selected_layer), sprite_type);
				}
			});
		}
	}
}
void m2::ledit::State::SelectMode::paste_fg() {
	if (auto selection_result = SelectionResult{GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		if (clipboard_position_1 && clipboard_position_2) {
			clipboard_position_1->for_each_cell_in_between(*clipboard_position_2, [&](const VecI& cell) {
				auto it = LEVEL.level_editor_state->fg_placeholders.find(cell);
				if (it != LEVEL.level_editor_state->fg_placeholders.end()) {
					auto new_position = positions->first + (cell - *clipboard_position_1);
					auto level_object = it->second.second;
					level_object.mutable_position()->set_x(new_position.x);
					level_object.mutable_position()->set_y(new_position.y);
					LEVEL.level_editor_state->fg_placeholders[new_position] = std::make_pair(obj::create_foreground_placeholder(VecF{new_position}, GAME.get_sprite(GAME.level_editor_object_sprites[level_object.type()])), level_object);
				}
			});
		}
	}
}
void m2::ledit::State::SelectMode::erase() {
	if (auto selection_result = SelectionResult{GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		positions->first.for_each_cell_in_between(positions->second, [&](const VecI& cell) {
			ledit::State::EraseMode::erase_position(cell, selected_layer);
		});
	}
}
void m2::ledit::State::SelectMode::remove() {
	if (auto selection_result = SelectionResult{GAME.events}; selection_result.is_primary_selection_finished()) {
		auto positions = selection_result.primary_int_selection_position_m();
		positions->first.for_each_cell_in_between(positions->second, [](const VecI& cell) {
			ledit::State::RemoveMode::remove_object(cell);
		});
	}
}
void m2::ledit::State::ShiftMode::shift(MAYBE const VecI& position) const {
	if (shift_type == ShiftType::RIGHT) {
		for (auto& placeholders : LEVEL.level_editor_state->bg_placeholders) {
			level_editor::shift_placeholders(placeholders, LEVEL.objects, position.x, INT32_MAX, INT32_MIN, INT32_MAX, 1, 0);
		}
		level_editor::shift_placeholders(LEVEL.level_editor_state->fg_placeholders, LEVEL.objects, position.x, INT32_MAX, INT32_MIN, INT32_MAX, 1, 0);
	} else if (shift_type == ShiftType::DOWN) {
		for (auto& placeholders : LEVEL.level_editor_state->bg_placeholders) {
			level_editor::shift_placeholders(placeholders, LEVEL.objects, INT32_MIN, INT32_MAX, position.y, INT32_MAX, 0, 1);
		}
		level_editor::shift_placeholders(LEVEL.level_editor_state->fg_placeholders, LEVEL.objects, INT32_MIN, INT32_MAX, position.y, INT32_MAX, 0, 1);
	} else if (shift_type == ShiftType::RIGHT_N_DOWN) {
		for (auto& placeholders : LEVEL.level_editor_state->bg_placeholders) {
			level_editor::shift_placeholders(placeholders, LEVEL.objects, position.x, INT32_MAX, position.y, INT32_MAX, 1, 1);
		}
		level_editor::shift_placeholders(LEVEL.level_editor_state->fg_placeholders, LEVEL.objects, position.x, INT32_MAX, position.y, INT32_MAX, 1, 1);
	}
}
void m2::ledit::State::deactivate_mode() {
	mode.emplace<std::monostate>();
}
void m2::ledit::State::activate_paint_mode() {
	mode.emplace<PaintMode>();
	std::get<PaintMode>(mode).select_sprite_type(GAME.level_editor_background_sprites[0]);
}
void m2::ledit::State::activate_erase_mode() {
	mode.emplace<EraseMode>();
}
void m2::ledit::State::activate_place_mode() {
	mode.emplace<PlaceMode>();
	std::get<PlaceMode>(mode).select_object_type(GAME.level_editor_object_sprites.begin()->first);
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
	for (int i = 0; i < I(BackgroundLayer::n); ++i) {
		for (const auto& [position, pair] : LEVEL.level_editor_state->bg_placeholders[i]) {
			protobuf::mutable_get_or_create(level.mutable_background_layers(), i);
			auto* row = protobuf::mutable_get_or_create(level.mutable_background_layers(i)->mutable_background_rows(), position.y); // Get or create row
			*(protobuf::mutable_get_or_create(row->mutable_items(), position.x)) = pair.second; // Set sprite type
		}
	}
	for (const auto& [position, pair] : LEVEL.level_editor_state->fg_placeholders) {
		level.add_objects()->CopyFrom(pair.second);
	}
	protobuf::message_to_json_file(level, *LEVEL.path());
}
