#pragma once
#include "Widget.h"

namespace m2::ui {
	struct Panel {
	private:
		bool _prev_text_input_state{};
		sdl::TextureUniquePtr _background_texture;
		std::unique_ptr<PanelBlueprint> _managed_blueprint; // blueprint will point here if this object exists

	public:
		bool enabled{true};
		const PanelBlueprint* blueprint{};
		RectI rect_px{};
		std::vector<std::unique_ptr<Widget>> widgets;

		Panel() = default;
		explicit Panel(std::variant<const PanelBlueprint*, std::unique_ptr<PanelBlueprint>> blueprint, sdl::TextureUniquePtr background_texture = {});
		static Action create_execute_sync(std::variant<const PanelBlueprint*, std::unique_ptr<PanelBlueprint>> blueprint);
		static Action create_execute_sync(std::variant<const PanelBlueprint*, std::unique_ptr<PanelBlueprint>> blueprint, RectI rect, sdl::TextureUniquePtr background_texture = {});
		~Panel();

		Action execute(RectI rect);
        void update_positions(const RectI& rect);
        Action handle_events(Events& events);
        Action update_contents();
        void draw();

	private:
		[[nodiscard]] int vertical_border_width_px() const;
		[[nodiscard]] int horizontal_border_width_px() const;

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

		template <typename WidgetT>
		[[nodiscard]] WidgetT* find_first_widget_by_name(const std::string& name) const {
			for (auto& w : widgets) {
				if (dynamic_cast<WidgetT*>(w.get()) != nullptr) {
					auto* widget_state = dynamic_cast<WidgetT*>(w.get());
					if (widget_state->blueprint->name == name) {
						return widget_state;
					}
				}
			}
			return nullptr;
		}
	};

	// Helpers
	RectI calculate_widget_rect(const RectI& root_rect_px, unsigned root_w, unsigned root_h, int child_x, int child_y, unsigned child_w, unsigned child_h);
	Widget* find_text_widget(Panel& state, const std::string& text);

	extern PanelBlueprint console_ui;
	extern const PanelBlueprint message_box_ui;
}
