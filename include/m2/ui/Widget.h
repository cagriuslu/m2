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
	struct Panel;

	// Base class of all widgets instances
	struct Widget {
	private:
		Panel* _parent;
		RectI _rect_px{}; // Position on screen

	public:
		bool enabled{true};
		bool focused{false};
		std::optional<float> disable_after;  // TODO only implemented for Text
		const WidgetBlueprint* blueprint;

		Widget(Panel* parent, const WidgetBlueprint* blueprint)
		    : _parent(parent), enabled(blueprint->initially_enabled), blueprint(blueprint) {}
		virtual ~Widget() = default;

		// Accessors
		[[nodiscard]] Panel& parent() const { return *_parent; }
		[[nodiscard]] const RectI& rect() const { return _rect_px; }

		// Modifiers
		void set_rect(const RectI& rect_px) { _rect_px = rect_px; on_resize(); }
		virtual Action on_event(MAYBE Events& events) { return make_continue_action(); }
		virtual void on_focus_change() {}
		virtual Action on_update() { return make_continue_action(); }
		virtual void on_draw() {}

	protected:
		// Virtual functions for children to implement
		virtual void on_resize() {}

		// Utilities for child classes to use

		void draw_background_color() const;

		[[nodiscard]] float horizontal_pixels_per_unit() const { return F(rect().w) / F(blueprint->w); }
		[[nodiscard]] float vertical_pixels_per_unit() const { return F(rect().h) / F(blueprint->h); }
		[[nodiscard]] int vertical_border_width_px() const;
		[[nodiscard]] int horizontal_border_width_px() const;
		[[nodiscard]] int vertical_padding_width_px() const;
		[[nodiscard]] int horizontal_padding_width_px() const;
		// Calculates and returns the drawable area (rect() - border - padding)
		[[nodiscard]] RectI drawable_area() const;

		static RectI calculate_text_rect(SDL_Texture* text_texture, RectI drawable_area, TextHorizontalAlignment align);
		/// Calculate the rect of the text to fill the drawable_area
		static RectI calculate_filled_text_rect(RectI drawable_area, TextHorizontalAlignment align, int text_length);

		static void draw_rectangle(const RectI& rect, const SDL_Color& color);
		static void draw_sprite(const Sprite& sprite, const RectI& dst_rect);
		static void draw_border(const RectI& rect, int vertical_border_width_px, int horizontal_border_width_px, const SDL_Color& color = {255, 255, 255, 255});

		// Allow Panel to use the utilities
		friend struct Panel;
	};
}  // namespace m2::ui
