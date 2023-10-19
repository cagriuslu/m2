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
		static Action create_execute_sync(const Blueprint* blueprint);
		static Action create_execute_sync(const Blueprint* blueprint, SDL_Rect rect);
		~State();

		Action execute(SDL_Rect rect);
        void update_positions(const SDL_Rect& rect);
        Action handle_events(Events& events);
        Action update_contents();
        void draw();

	private:
		std::unique_ptr<Widget> create_widget_state(const WidgetBlueprint& blueprint);
		void set_widget_focus_state(Widget& w, bool state);
		void clear_focus();

	public:
		// Helpers

		template<typename WidgetT>
		Widget* find_first_widget_of_type() {
			for (auto& w : widgets) {
				if (std::holds_alternative<WidgetT>(w->blueprint->variant)) {
					return w.get();
				}
			}
			return nullptr;
		}
	};

	// Helpers
	SDL_Rect calculate_widget_rect(const SDL_Rect& root_rect_px, unsigned root_w, unsigned root_h, int child_x, int child_y, unsigned child_w, unsigned child_h);
	Widget* find_text_widget(State& state, const std::string& text);

	extern const Blueprint console_ui;
	extern const Blueprint message_box_ui;
}
