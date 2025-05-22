#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/sdl/TextTexture.h>
#include <m2/ui/widget/TextInput.h>

using namespace m2;
using namespace m2::widget;

namespace {
	void reset_focus() {
		LOG_DEBUG("Stopping text input");
		SDL_StopTextInput();
	}
}  // namespace

TextInput::TextInput(UiPanel* parent, const UiWidgetBlueprint* blueprint) : UiWidget(parent, blueprint) {
	const auto& te_blueprint = std::get<TextInputBlueprint>(blueprint->variant);
	_text_input << te_blueprint.initial_text;
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

UiAction TextInput::OnEvent(Events& events) {
	if (events.PopMouseButtonPress(MouseButton::PRIMARY, RectI{Rect()})) {
		LOG_INFO("Regaining focus");
		return MakeContinueAction(true);
	}
	// Ignore all events if not focused
	if (not IsFocused()) {
		return MakeContinueAction();
	}

	if (events.PopKeyPress(m2g::pb::PAUSE)) {
		return MakeContinueAction(false);
	} else if (events.PopKeyPress(m2g::pb::RETURN) && std::get<TextInputBlueprint>(blueprint->variant).onAction) {
		auto [action, new_string] = std::get<TextInputBlueprint>(blueprint->variant).onAction(*this);
		if (new_string) {
			_text_input = std::stringstream{*new_string};
		}
		return std::move(action);
	} else if (events.PopKeyPress(m2g::pb::BACKSPACE)) {
		if (const auto text_input_str = _text_input.str(); not text_input_str.empty()) {
			_text_input = std::stringstream{text_input_str.substr(0, text_input_str.length() - 1)};
			_text_input.seekp(0, std::ios::end);
		}
	} else {
		if (const auto opt_text_input = events.PopTextInput(); opt_text_input) {
			_text_input << *opt_text_input;
		}
	}
	return MakeContinueAction();
}

void TextInput::OnFocusChange() {
	if (IsFocused()) {
		LOG_DEBUG("Starting text input");
		SDL_StartTextInput();
	} else {
		reset_focus();
	}
}

void TextInput::OnDraw() {
	draw_background_color();

	auto str = _text_input.str();
	if (not _text_texture_and_destination_cache || _text_texture_and_destination_cache->first.String() != str) {
		// Add '_' if focused
		if (IsFocused()) {
			str += '_';
		}
		// Generate text texture
		auto textTexture = m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font,
				M2G_PROXY.default_font_size, str));
		// Calculate destination rectangle
		auto destination_rect = calculate_filled_text_rect(drawable_area(), TextHorizontalAlignment::LEFT,
				I(Utf8CodepointCount(str.c_str())));
		// Save for later
		_text_texture_and_destination_cache = sdl::TextTextureAndDestination{std::move(textTexture), destination_rect};
	}

	sdl::render_texture_with_color_mod(_text_texture_and_destination_cache->first.Texture(),
		_text_texture_and_destination_cache->second);

	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px());
}
