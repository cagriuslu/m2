#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>

using namespace m2;
using namespace m2::widget;

TextSelection::TextSelection(UiPanel* parent, const UiWidgetBlueprint* blueprint) : UiWidget(parent, blueprint) {
	SetOptions(VariantBlueprint().options);
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

std::vector<TextSelectionBlueprint::ReturnValue> TextSelection::GetSelectedOptions() const {
	std::vector<TextSelectionBlueprint::ReturnValue> selections;
	for (auto& option : _options) {
		if (option.is_selected) {
			selections.emplace_back(option.blueprint_option.return_value);
		}
	}
	return selections;
}
std::optional<int> TextSelection::GetIndexOfFirstSelection() const {
	const auto currentSelectionIt = std::ranges::find_if(_options, [](auto& o) { return o.is_selected == true; });
	if (currentSelectionIt == _options.end()) {
		return std::nullopt;
	}
	return std::distance(_options.begin(), currentSelectionIt);
}
std::vector<int> TextSelection::GetSelectedIndexes() const {
	std::vector<int> indexes;
	for (int i = 0; i < I(_options.size()); ++i) {
		if (_options[i].is_selected) {
			indexes.emplace_back(i);
		}
	}
	return indexes;
}

void TextSelection::SetOptions(const TextSelectionBlueprint::Options& newOptions) {
	auto copy = newOptions;
	SetOptions(std::move(copy));
}
void TextSelection::SetOptions(TextSelectionBlueprint::Options&& newOptions) {
	_options.clear();
	_options.resize(newOptions.size());
	for (size_t i = 0; i < newOptions.size(); ++i) {
		_options[i].blueprint_option = std::move(newOptions[i]);
		_options[i].is_selected = _options[i].blueprint_option.initiallySelected;
	}
	// Option under the mouse might have changed
	if (IsHoverActive()) {
		RenewHoverIfNecessary();
	}
	// Applicable to +/- selection and dropdown, select the first item
	if (VariantBlueprint().line_count == 0 || VariantBlueprint().line_count == 1) {
		if (not _options.empty()) {
			_options[0].is_selected = true;
			if (VariantBlueprint().onAction) {
				VariantBlueprint().onAction(*this);
			}
		}
	}
}
void TextSelection::SetUniqueSelectionIndex(const int index) {
	if (0 <= index && index < I(_options.size())) {
		// Clear all selections
		for (auto& option : _options) {
			option.is_selected = false;
		}
		// Option under the mouse might have changed
		if (IsHoverActive()) {
			RenewHoverIfNecessary();
		}
		// Select the given option
		_options[index].is_selected = true;
		if (VariantBlueprint().onAction) {
			VariantBlueprint().onAction(*this);
		}
	}
}

void TextSelection::OnResize(MAYBE const RectI& oldRect, MAYBE const RectI& newRect) {
	// Invalidate every font texture cache
	for (auto& option : _options) {
		option.text_texture_and_destination.reset();
	}
	_plusTexture.reset();
	_minusTexture.reset();
	_upArrowTexture.reset();
	_downArrowTexture.reset();
}
void TextSelection::OnHover() {
	RenewHoverIfNecessary();
}
void TextSelection::OffHover() {
	_hoveredIndex.reset();
	if (VariantBlueprint().offHover) {
		VariantBlueprint().offHover(*this);
	}
}
UiAction TextSelection::OnEvent(Events& events) {
	// +/- selection
	if (auto line_count = VariantBlueprint().line_count; line_count == 0) {
		auto buttons_rect = Rect().TrimLeft(Rect().w - Rect().h / 2);
		auto inc_button_rect = buttons_rect.TrimBottom(buttons_rect.h / 2);
		auto dec_button_rect = buttons_rect.TrimTop(buttons_rect.h / 2);
		if (!_plusDepressed && events.PopMouseButtonPress(MouseButton::PRIMARY, inc_button_rect)) {
			_plusDepressed = true;
			_minusDepressed = false;
		} else if (!_minusDepressed && events.PopMouseButtonPress(MouseButton::PRIMARY, dec_button_rect)) {
			_minusDepressed = true;
			_plusDepressed = false;
		} else if (_plusDepressed && events.PopMouseButtonRelease(MouseButton::PRIMARY, inc_button_rect)) {
			_plusDepressed = false;
			return IncrementSelection();
		} else if (_minusDepressed && events.PopMouseButtonRelease(MouseButton::PRIMARY, dec_button_rect)) {
			_minusDepressed = false;
			return DecrementSelection();
		} else {
			// Check if scrolled
			if (auto scroll_amount = events.PopMouseWheelVerticalScroll(Rect()); 0 < scroll_amount) {
				m2Repeat(scroll_amount) {
					if (auto action = IncrementSelection(); not action.IsContinue()) {
						return action;
					}
				}
				return MakeContinueAction();
			} else if (scroll_amount < 0) {
				m2Repeat(scroll_amount) {
					if (auto action = DecrementSelection(); not action.IsContinue()) {
						return action;
					}
				}
				return MakeContinueAction();
			}
		}
	} else if (line_count == 1) {
		// Dropdown
		throw M2_ERROR("Not yet implemented");
	} else {
		// Scrollable selection

		auto scroll_bar_rect = Rect().TrimLeft(Rect().w - Rect().h / I(VariantBlueprint().line_count));
		auto up_arrow_rect = scroll_bar_rect.GetRow(I(VariantBlueprint().line_count), 0);
		auto down_button_rect = scroll_bar_rect.GetRow(I(VariantBlueprint().line_count), I(VariantBlueprint().line_count) - 1);

		// Check if scroll buttons are pressed
		if (events.PopMouseButtonPress(MouseButton::PRIMARY, up_arrow_rect)) {
			if (0 < _topIndex) {
				_topIndex--;
			}
		} else if (events.PopMouseButtonPress(MouseButton::PRIMARY, down_button_rect)) {
			if (_topIndex + VariantBlueprint().line_count < I(_options.size())) {
				_topIndex++;
			}
		} else {
			// Check if mouse wheel scrolled
			if (auto mouseScrollAmount = events.PopMouseWheelVerticalScroll(Rect()); 0 < mouseScrollAmount) {
				const auto linesBelowWindow = AtLeastZero(I(_options.size()) - VariantBlueprint().line_count - _topIndex);
				_topIndex += std::min(mouseScrollAmount, linesBelowWindow);
			} else if (mouseScrollAmount < 0) {
				_topIndex -= std::min(-mouseScrollAmount, _topIndex);
			}
		}

		// Option under the mouse might have changed
		if (IsHoverActive()) {
			RenewHoverIfNecessary();
		}

		// Check line items
		for (auto i = 0; i < VariantBlueprint().line_count; ++i) {
			// If the entry is in window
			if (_topIndex + i < I(_options.size())) {
				if (auto text_rect = Rect().GetRow(VariantBlueprint().line_count, i).TrimRight(scroll_bar_rect.w);
						events.PopMouseButtonPress(MouseButton::PRIMARY, text_rect)) {
					if (int pressed_item = _topIndex + i; _options[pressed_item].is_selected) {
						// If already selected
						_options[pressed_item].is_selected = false; // Deselect
						if (VariantBlueprint().onAction) {
							return VariantBlueprint().onAction(*this);
						}
					} else {
						// Clear selection if necessary
						if (not VariantBlueprint().allow_multiple_selection) {
							// Clear all selections
							for (auto& option : _options) {
								option.is_selected = false;
							}
						}
						_options[pressed_item].is_selected = true; // Select
						if (VariantBlueprint().onAction) {
							return VariantBlueprint().onAction(*this);
						}
					}
				}
			}
		}
	}
	return MakeContinueAction();
}
UiAction TextSelection::OnUpdate() {
	if (VariantBlueprint().onUpdate) {
		return VariantBlueprint().onUpdate(*this);
	}
	return MakeContinueAction();
}
void TextSelection::OnDraw() {
	draw_background_color();

	// +/- selection
	if (auto line_count = VariantBlueprint().line_count; line_count == 0) {
		// Selected option's text
		if (auto current_selection = std::ranges::find_if(_options, [](const auto& o) { return o.is_selected == true; });
			current_selection != _options.end()) {
			if (not current_selection->text_texture_and_destination) {
				// Render the text
				auto drawable_area = Rect().TrimRight(Rect().h / 2);
				auto fontSize = calculate_filled_text_rect(drawable_area, TextHorizontalAlignment::LEFT, I(Utf8CodepointCount(current_selection->blueprint_option.text.c_str()))).h;
				auto textTexture = m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, fontSize, current_selection->blueprint_option.text));
				auto destination_rect = calculate_filled_text_rect(drawable_area, TextHorizontalAlignment::LEFT, I(Utf8CodepointCount(current_selection->blueprint_option.text.c_str())));
				current_selection->text_texture_and_destination = sdl::TextTextureAndDestination{std::move(textTexture), destination_rect};
			}
			sdl::render_texture_with_color_mod(current_selection->text_texture_and_destination->first.Texture(),
				current_selection->text_texture_and_destination->second, current_selection->blueprint_option.text_color);
		}
		// + button
		auto buttons_rect = Rect().TrimLeft(Rect().w - Rect().h / 2);
		{
			auto inc_button_rect = buttons_rect.TrimBottom(buttons_rect.h / 2);
			if (not _plusTexture) {
				auto fontSize = inc_button_rect.h;
				auto textTexture = m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, fontSize, "+"));
				auto destination_rect = RectI::CreateCenteredAround(inc_button_rect.GetCenterPoint(), textTexture.Dimensions().x, textTexture.Dimensions().y);
				// TODO we may need to move the texture slightly up, check the font properties
				_plusTexture = {std::move(textTexture), destination_rect};
			}
			sdl::render_texture_with_color_mod(_plusTexture->first.Texture(), _plusTexture->second);
			draw_border(inc_button_rect, vertical_border_width_px(), horizontal_border_width_px());
		}
		// - button
		{
			auto dec_button_rect = buttons_rect.TrimTop(buttons_rect.h / 2);
			if (not _minusTexture) {
				auto fontSize = dec_button_rect.h;
				auto textTexture = m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, fontSize, "-"));
				auto destination_rect = RectI::CreateCenteredAround(dec_button_rect.GetCenterPoint(), textTexture.Dimensions().x, textTexture.Dimensions().y);
				// TODO we may need to move the texture slightly up, check the font properties
				_minusTexture = {std::move(textTexture), destination_rect};
			}
			sdl::render_texture_with_color_mod(_minusTexture->first.Texture(), _minusTexture->second);
			draw_border(dec_button_rect, vertical_border_width_px(), horizontal_border_width_px());
		}
	} else if (line_count == 1) {
		// Dropdown
		throw M2_ERROR("Not yet implemented");
	} else {
		// Scrollable selection
		for (auto i = 0; i < VariantBlueprint().line_count; ++i) {
			if (const auto optionIndexAndTextRect = GetOptionIndexAndTextRectOfRow(i)) {
				const auto [optionIndex, textRect] = *optionIndexAndTextRect;
				// If selected, color the rect blue
				if (_options[optionIndex].is_selected) {
					draw_rectangle(textRect, {0, 0, 255, 127});
				}

				// Draw text
				auto& current_line = _options[optionIndex];
				// Calculate how many characters can fit into the line
				const auto textLength = I(Utf8CodepointCount(current_line.blueprint_option.text.c_str()));
				const auto charWidthToHeightRatio = M2_GAME.FontLetterWidthToHeightRatio();
				const auto charWidth = charWidthToHeightRatio.GetN() * textRect.h / charWidthToHeightRatio.GetD();
				const auto maxCharCount = I(textRect.w / charWidth);
				// Render texture if necessary
				if (not current_line.text_texture_and_destination) {
					const auto fontSize = textRect.h;
					auto textTexture = m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, fontSize,
						textLength <= maxCharCount ? current_line.blueprint_option.text : current_line.blueprint_option.text.substr(0, maxCharCount - 1) + "…"));
					// Don't bother with destination_rect, because we're going to calculate that every time
					current_line.text_texture_and_destination = std::make_pair(std::move(textTexture), RectI{});
				}
				// Calculate the text rect, actual text rect might be narrower than the available text rect
				const auto actualTextRect = calculate_filled_text_rect(textRect, TextHorizontalAlignment::LEFT,
					textLength <= maxCharCount ? textLength : maxCharCount);
				current_line.text_texture_and_destination->second = actualTextRect;
				sdl::render_texture_with_color_mod(current_line.text_texture_and_destination->first.Texture(),
					current_line.text_texture_and_destination->second, current_line.blueprint_option.text_color);
			}
		}
		// Scroll bar
		if (VariantBlueprint().show_scroll_bar) {
			auto scroll_bar_rect = Rect().TrimLeft(Rect().w - Rect().h / VariantBlueprint().line_count);
			draw_rectangle(scroll_bar_rect, {0, 0, 0, 255});
			draw_border(scroll_bar_rect, vertical_border_width_px(), horizontal_border_width_px());
			// Up arrow
			{
				auto up_arrow_rect = scroll_bar_rect.GetRow(VariantBlueprint().line_count, 0);
				if (not _upArrowTexture) {
					auto fontSize = up_arrow_rect.h;
					auto textTexture = m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, fontSize, "-"));
					auto destination_rect = RectI::CreateCenteredAround(up_arrow_rect.GetCenterPoint(), textTexture.Dimensions().x, textTexture.Dimensions().y);
					// TODO we may need to move the texture slightly up, check the font properties
					_upArrowTexture = {std::move(textTexture), destination_rect};
				}
				sdl::render_texture_with_color_mod(_upArrowTexture->first.Texture(), _upArrowTexture->second);
				draw_border(up_arrow_rect, vertical_border_width_px(), horizontal_border_width_px());
			}
			// Down arrow
			{
				auto down_button_rect = scroll_bar_rect.GetRow(VariantBlueprint().line_count,
					VariantBlueprint().line_count - 1);
				if (not _downArrowTexture) {
					auto fontSize = down_button_rect.h;
					auto textTexture = m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, fontSize, "+"));
					auto destination_rect = RectI::CreateCenteredAround(down_button_rect.GetCenterPoint(), textTexture.Dimensions().x, textTexture.Dimensions().y);
					// TODO we may need to move the texture slightly up, check the font properties
					_downArrowTexture = {std::move(textTexture), destination_rect};
				}
				sdl::render_texture_with_color_mod(_downArrowTexture->first.Texture(), _downArrowTexture->second);
				draw_border(down_button_rect, vertical_border_width_px(), horizontal_border_width_px());
			}
		}
	}

	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px());
}

RectI TextSelection::GetTextRects() const {
	if (VariantBlueprint().line_count == 0 || VariantBlueprint().line_count == 1) {
		return Rect().TrimRight(Rect().h / 2);
	}
	return Rect().TrimRight(Rect().h / VariantBlueprint().line_count);
}
RectI TextSelection::GetTextRectOfRow(const int row) const {
	if (VariantBlueprint().line_count == 0 || VariantBlueprint().line_count == 1) {
		return GetTextRects();
	}
	return GetTextRects().GetRow(VariantBlueprint().line_count, row);
}
std::optional<std::pair<int,RectI>> TextSelection::GetOptionIndexAndTextRectOfRow(const int row) const {
	if (_options.empty()) {
		return std::nullopt;
	}

	if (VariantBlueprint().line_count == 0 || VariantBlueprint().line_count == 1) {
		const auto currentSelectionIt = std::ranges::find_if(_options, [](auto& o) { return o.is_selected == true; });
		const auto index = std::distance(_options.begin(), currentSelectionIt);
		return {{index, GetTextRectOfRow(0)}};
	}

	// If the index corresponding to the row is a valid index
	if (const auto optionIndex = _topIndex + row; optionIndex < I(_options.size())) {
		return {{optionIndex, GetTextRectOfRow(row)}};
	}
	return std::nullopt;
}
void TextSelection::RenewHoverIfNecessary() {
	if (not VariantBlueprint().onHover) {
		return;
	}

	// Find the current hovered index
	const auto newHoveredIndex = [this]() -> std::optional<int> {
		if (VariantBlueprint().line_count == 0 || VariantBlueprint().line_count == 1) {
			return GetIndexOfFirstSelection();
		}
		const auto widgetY = Rect().y;
		const auto widgetH = Rect().h;
		const auto rowH = widgetH / VariantBlueprint().line_count;
		const auto mouseY = M2_GAME.events.MousePosition().y;
		const auto mouseYOffset = mouseY - widgetY;
		if (const auto indexBeingHovered = mouseYOffset / rowH; indexBeingHovered < I(_options.size())) {
			return std::clamp(_topIndex + indexBeingHovered, 0, I(_options.size()) - 1) ;
		}
		return std::nullopt;
	}();

	// If state changed
	if (newHoveredIndex != _hoveredIndex) {
		// Turn off the old hover context
		if (_hoveredIndex) {
			OffHover();
		}
		// Turn on the new hover context
		if (newHoveredIndex) {
			VariantBlueprint().onHover(*this, *newHoveredIndex);
		}
		_hoveredIndex = newHoveredIndex;
	}
}
UiAction TextSelection::IncrementSelection() {
	if (auto current_selection = std::ranges::find_if(_options, [](auto& o) { return o.is_selected == true; });
		current_selection != _options.end()) {
		if (const auto next_selection = std::next(current_selection); next_selection != _options.end()) {
			current_selection->is_selected = false;
			next_selection->is_selected = true;
			// Option under the mouse might have changed
			if (IsHoverActive()) {
				RenewHoverIfNecessary();
			}
			if (VariantBlueprint().onAction) {
				return VariantBlueprint().onAction(*this);
			}
		}
	}
	return MakeContinueAction();
}
UiAction TextSelection::DecrementSelection() {
	if (auto current_selection = std::find_if(_options.rbegin(), _options.rend(), [](auto& o) { return o.is_selected == true; });
		current_selection != _options.rend()) {
		if (const auto prev_selection = std::next(current_selection); prev_selection != _options.rend()) {
			current_selection->is_selected = false;
			prev_selection->is_selected = true;
			// Option under the mouse might have changed
			if (IsHoverActive()) {
				RenewHoverIfNecessary();
			}
			if (VariantBlueprint().onAction) {
				return VariantBlueprint().onAction(*this);
			}
		}
	}
	return MakeContinueAction();
}
