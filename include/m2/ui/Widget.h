#pragma once
#include "../Events.h"
#include "WidgetBlueprint.h"
#include "Action.h"
#include "../Sprite.h"
#include <optional>
#include <SDL.h>

namespace m2::ui {
	// Forward declaration
	struct State;

	struct Widget {
	private:
		State* _parent;

	public:
		bool enabled{true};
		bool focused{false};
		std::optional<float> disable_after; // TODO only implemented for Text
		const WidgetBlueprint* blueprint;
		RectI rect_px{};

		Widget(State* parent, const WidgetBlueprint* blueprint);
		virtual ~Widget() = default;

		virtual void on_position_update(const RectI& rect_px);
		virtual Action on_event(Events& events);
		virtual void on_focus_change();
		virtual Action on_update();
		virtual void on_draw();

		[[nodiscard]] State& parent() const { return *_parent; }

		// Helpers
		static void draw_background_color(const RectI& rect, const SDL_Color& color);
		static void draw_text(const RectI& rect, SDL_Texture& texture, TextAlignment align);
		static void draw_sprite(const Sprite& sprite, const RectI& dst_rect);
		static void draw_border(const RectI& rect, unsigned border_width_px, const SDL_Color& color = {255, 255, 255, 255});
	};
}
