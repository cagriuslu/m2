#pragma once
#include "Widget.h"

namespace m2::ui {
	struct State {
		bool enabled{true};
		const Blueprint* blueprint{};
		SDL_Rect rect_px{};
		std::vector<std::unique_ptr<Widget>> widgets;

		State() = default;
        explicit State(const Blueprint* blueprint);
        void update_positions(const SDL_Rect& rect);
        Action handle_events(Events& events);
        Action update_contents();
        void draw();

	private:
		static std::unique_ptr<Widget> create_widget_state(const WidgetBlueprint& blueprint);
	};

	Action execute_blocking(const Blueprint* blueprint);
	Action execute_blocking(const Blueprint* blueprint, SDL_Rect rect);

	extern const Blueprint console_ui;
	extern const Blueprint message_box_ui;
}
