#include <m2/pixel_editor/State.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/game/object/Pixel.h>

void m2::pixel_editor::State::PaintMode::paint_color(const VecI& position) {
	// Delete existing pixel, if there is one
	auto it = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).pixels.find(position);
	if (it != std::get<pixel_editor::State>(M2_LEVEL.stateVariant).pixels.end()) {
		M2_DEFER(create_object_deleter(it->second.first));
		std::get<pixel_editor::State>(M2_LEVEL.stateVariant).pixels.erase(it);
	}
	auto pixel_id = obj::create_pixel(static_cast<VecF>(position), std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color);
	std::get<pixel_editor::State>(M2_LEVEL.stateVariant).pixels[position] = std::make_pair(pixel_id, std::get<pixel_editor::State>(M2_LEVEL.stateVariant).selected_color);
}
void m2::pixel_editor::State::EraseMode::erase_color(const VecI &position) {
	auto it = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).pixels.find(position);
	if (it != std::get<pixel_editor::State>(M2_LEVEL.stateVariant).pixels.end()) {
		M2_DEFER(create_object_deleter(it->second.first));
		std::get<pixel_editor::State>(M2_LEVEL.stateVariant).pixels.erase(it);
	}
}
void m2::pixel_editor::State::ColorPickerMode::pick_color(const VecI &position) {
	auto it = std::get<pixel_editor::State>(M2_LEVEL.stateVariant).pixels.find(position);
	if (it != std::get<pixel_editor::State>(M2_LEVEL.stateVariant).pixels.end()) {
		LOG_DEBUG("Selected color");
		std::get<pixel_editor::State>(M2_LEVEL.stateVariant).select_color(it->second.second);
	}
}
void m2::pixel_editor::State::deactivate_mode() {
	mode = std::monostate{};
}
void m2::pixel_editor::State::activate_paint_mode() {
	mode = PaintMode{};
}
void m2::pixel_editor::State::activate_erase_mode() {
	mode = EraseMode{};
}
void m2::pixel_editor::State::activate_color_picker_mode() {
	mode = ColorPickerMode{};
}
void m2::pixel_editor::State::save() {
	// TODO
}
