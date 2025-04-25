#pragma once
#include <SDL.h>

#include <optional>

#include "../video/Color.h"
#include "../Events.h"
#include <m2/video/Sprite.h>
#include "UiAction.h"
#include "UiWidgetBlueprint.h"

namespace m2 {
	// Forward declaration
	struct UiPanel;

	// Base class of all widgets instances
	struct UiWidget {
	private:
		UiPanel* _parent;
		RectI _rect_px{}; // Position on screen
		bool _focused{};

	public:
		/// For a widget, being enabled and being shown are the same things
		bool enabled{true};
		const UiWidgetBlueprint* blueprint;

		UiWidget(UiPanel* parent, const UiWidgetBlueprint* blueprint)
		    : _parent(parent), enabled(blueprint->initially_enabled), blueprint(blueprint) {}
		virtual ~UiWidget() = default;

		// Accessors

		[[nodiscard]] UiPanel& Parent() const { return *_parent; }
		[[nodiscard]] const RectI& Rect() const { return _rect_px; }
		[[nodiscard]] bool IsFocused() const { return _focused; }

		// Modifiers

		void SetRect(const RectI& rect_px);
		UiAction HandleEvents(Events&);
		void SetFocusState(bool);
		UiAction UpdateContents();
		void Draw();

	protected:
		// Virtual functions for widgets to implement

		virtual void OnResize(MAYBE const RectI& oldRect, MAYBE const RectI& newRect) {}
		virtual UiAction OnEvent(MAYBE Events& events) { return MakeContinueAction(); }
		virtual void OnFocusChange() {}
		virtual UiAction OnUpdate() { return MakeContinueAction(); }
		virtual void OnDraw() {}

		// Utilities for widgets to use

		void draw_background_color() const;

		[[nodiscard]] float horizontal_pixels_per_unit() const { return F(Rect().w) / F(blueprint->w); }
		[[nodiscard]] float vertical_pixels_per_unit() const { return F(Rect().h) / F(blueprint->h); }
		[[nodiscard]] int vertical_border_width_px() const;
		[[nodiscard]] int horizontal_border_width_px() const;
		[[nodiscard]] int vertical_padding_width_px() const;
		[[nodiscard]] int horizontal_padding_width_px() const;
		// Calculates and returns the drawable area (rect() - border - padding)
		[[nodiscard]] RectI drawable_area() const;

		/// Calculate the rect of the wrapped text to fill the drawable_area. The width of the text texture is assumed
		/// to be drawable_area.w because `create_wrapped` functions generates textures with a desired width.
		static RectI calculate_wrapped_text_rect(SDL_Texture* text_texture, RectI drawable_area, TextHorizontalAlignment align_h, TextVerticalAlignment align_v);
		/// Calculate the rect of the text to fill the drawable_area
		static RectI calculate_filled_text_rect(RectI drawable_area, TextHorizontalAlignment align, int text_length);

		static void draw_rectangle(const RectI& rect, const SDL_Color& color);
		static void DrawSpriteOrTextLabel(const std::variant<Sprite, pb::TextLabel>&, const RectI& dst_rect);
		static void draw_border(const RectI& rect, int vertical_border_width_px, int horizontal_border_width_px, const SDL_Color& color = {255, 255, 255, 255}); // TODO get rid of this, use the below
		static void draw_border(const RectI& rect, int vertical_border_width_px, int horizontal_border_width_px, const RGBA& color) { draw_border(rect, vertical_border_width_px, horizontal_border_width_px, SDL_Color{color.r, color.g, color.b, color.a}); }

		// Allow UiPanel to use the utilities
		friend struct UiPanel;
	};
}  // namespace m2
