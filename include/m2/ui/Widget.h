#pragma once
#include <SDL.h>

#include <optional>

#include "../Color.h"
#include "../Events.h"
#include "../Sprite.h"
#include "Action.h"
#include "WidgetBlueprint.h"

namespace m2::ui {
	// Forward declaration
	struct State;

	// Base class of all widgets instances
	struct Widget {
	   private:
		State* _parent;

	   public:
		bool enabled{true};
		bool focused{false};
		std::optional<float> disable_after;  // TODO only implemented for Text
		const WidgetBlueprint* blueprint;
		RectI rect_px{};

		Widget(State* parent, const WidgetBlueprint* blueprint)
		    : _parent(parent), enabled(blueprint->initially_enabled), blueprint(blueprint) {}
		virtual ~Widget() = default;

		virtual void on_position_update(const RectI& rect_px_) { this->rect_px = rect_px_; }
		virtual Action on_event(MAYBE Events& events) { return make_continue_action(); }
		virtual void on_focus_change() {}
		virtual Action on_update() { return make_continue_action(); }
		virtual void on_draw() {}

		[[nodiscard]] State& parent() const { return *_parent; }

	   protected:
		// Utilities for child classes to use

		[[nodiscard]] int vertical_border_width_px() const;
		[[nodiscard]] int horizontal_border_width_px() const;

		/// Calculate the text rect assuming that the whole widget will be filled with the text
		RectI calculate_text_rect(float font_size_unitless, TextHorizontalAlignment align, SDL_Texture* text_texture) const;
		/// If font_size_unitless is zero, the whole container will be filled with the text.
		/// container_height_unitless can be left empty in this case.
		static RectI calculate_text_rect(RectI container, int container_height_unitless, float font_size_unitless,
			TextHorizontalAlignment align, SDL_Texture* text_texture);

		static void draw_background_color(const RectI& rect, const SDL_Color& color);
		static void draw_text(const RectI& rect, SDL_Texture* text_texture, const RGB& color_mod = {255, 255, 255});
		static void draw_sprite(const Sprite& sprite, const RectI& dst_rect);
		static void draw_border(const RectI& rect, unsigned border_width_px, const SDL_Color& color = {255, 255, 255, 255});
		static void draw_border(const RectI& rect, int vertical_border_width_px, int horizontal_border_width_px, const SDL_Color& color = {255, 255, 255, 255});

		// Allow State to use the utilities
		friend struct State;
	};
}  // namespace m2::ui
