#pragma once
#include "Widget.h"

namespace m2::ui {
	struct State {
	private:
		bool _prev_text_input_state{};

	public:
		bool enabled{true};
		const Blueprint* blueprint{};
		RectI rect_px{};
		std::vector<std::unique_ptr<Widget>> widgets;

		State() = default;
		explicit State(const Blueprint* blueprint);
		static Action create_execute_sync(const Blueprint* blueprint);
		static Action create_execute_sync(const Blueprint* blueprint, RectI rect);
		~State();

		Action execute(RectI rect);
        void update_positions(const RectI& rect);
        Action handle_events(Events& events);
        Action update_contents();
        void draw();

	private:
		std::unique_ptr<Widget> create_widget_state(const WidgetBlueprint& widget_blueprint);
		void set_widget_focus_state(Widget& w, bool state);
		void clear_focus();

	public:
		// Helpers

		template <typename WidgetBlueprintT>
		[[nodiscard]] Widget* find_first_widget_of_blueprint_type() const {
			for (auto& w : widgets) {
				if (std::holds_alternative<WidgetBlueprintT>(w->blueprint->variant)) {
					return w.get();
				}
			}
			return nullptr;
		}

		template <typename WidgetT>
		[[nodiscard]] WidgetT* find_first_widget_of_type() const {
			for (auto& w : widgets) {
				if (dynamic_cast<WidgetT*>(w.get()) != nullptr) {
					return dynamic_cast<WidgetT*>(w.get());
				}
			}
			return nullptr;
		}
	};

	// Helpers
	RectI calculate_widget_rect(const RectI& root_rect_px, unsigned root_w, unsigned root_h, int child_x, int child_y, unsigned child_w, unsigned child_h);
	Widget* find_text_widget(State& state, const std::string& text);

	extern Blueprint console_ui;
	extern const Blueprint message_box_ui;
}
