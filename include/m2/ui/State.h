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
	public:
		static SDL_Rect calculate_widget_rect(const SDL_Rect& root_rect_px, unsigned root_w, unsigned root_h, int child_x, int child_y, unsigned child_w, unsigned child_h);
	};

	Action execute_blocking(const Blueprint* blueprint);
	Action execute_blocking(const Blueprint* blueprint, SDL_Rect rect);

	extern const Blueprint console_ui;
	extern const Blueprint message_box_ui;
}
