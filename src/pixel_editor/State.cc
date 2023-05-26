#include <m2/pixel_editor/State.h>
#include <m2/Game.h>
#include <m2/object/Pixel.h>

void m2::pedit::State::PaintMode::paint_color(const VecI& position) {
	// Delete existing pixel, if there is one
	auto it = LEVEL.pixel_editor_state->pixels.find(position);
	if (it != LEVEL.pixel_editor_state->pixels.end()) {
		GAME.add_deferred_action(create_object_deleter(it->second.first));
		LEVEL.pixel_editor_state->pixels.erase(it);
	}
	auto pixel_id = obj::create_pixel(static_cast<VecF>(position), LEVEL.pixel_editor_state->selected_color);
	LEVEL.pixel_editor_state->pixels[position] = std::make_pair(pixel_id, LEVEL.pixel_editor_state->selected_color);
}
void m2::pedit::State::EraseMode::erase_color(const VecI &position) {
	auto it = LEVEL.pixel_editor_state->pixels.find(position);
	if (it != LEVEL.pixel_editor_state->pixels.end()) {
		GAME.add_deferred_action(create_object_deleter(it->second.first));
		LEVEL.pixel_editor_state->pixels.erase(it);
	}
}
void m2::pedit::State::ColorPickerMode::pick_color(const VecI &position) {
	auto it = LEVEL.pixel_editor_state->pixels.find(position);
	if (it != LEVEL.pixel_editor_state->pixels.end()) {
		LOG_DEBUG("Selected color");
		LEVEL.pixel_editor_state->select_color(it->second.second);
	}
}
void m2::pedit::State::deactivate_mode() {
	mode = std::monostate{};
}
void m2::pedit::State::activate_paint_mode() {
	mode = PaintMode{};
}
void m2::pedit::State::activate_erase_mode() {
	mode = EraseMode{};
}
void m2::pedit::State::activate_color_picker_mode() {
	mode = ColorPickerMode{};
}
void m2::pedit::State::save() {
	// TODO
}
