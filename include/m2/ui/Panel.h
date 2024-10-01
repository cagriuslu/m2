#pragma once
#include "PanelBlueprint.h"
#include "Widget.h"
#include "../RectI.h"
#include "../RectF.h"

namespace m2::ui {
	struct Panel {
	private:
		bool _prev_text_input_state{};
		std::unique_ptr<PanelBlueprint> _owned_blueprint; // `blueprint` will point here if this object exists
		RectF _relation_to_game_and_hud_dims;
		sdl::TextureUniquePtr _background_texture; // TODO if the screen is resized, background looks bad
		std::optional<float> _timeout_s;

		// Modifiers
		Action run_blocking();
	public:
		bool enabled{true};
		const PanelBlueprint* blueprint{};
		std::vector<std::unique_ptr<Widget>> widgets;

		Panel() = default;
		explicit Panel(std::variant<const PanelBlueprint*, std::unique_ptr<PanelBlueprint>> static_or_unique_blueprint,
			std::variant<std::monostate, RectI, RectF> fullscreen_or_pixel_rect_or_relation_to_game_and_hud = {},
			sdl::TextureUniquePtr background_texture = {});
		static Action create_and_run_blocking(
			std::variant<const PanelBlueprint*, std::unique_ptr<PanelBlueprint>> static_or_unique_blueprint,
			std::variant<std::monostate, RectI, RectF> fullscreen_or_pixel_rect_or_relation_to_game_and_hud = {},
			sdl::TextureUniquePtr background_texture = {});
		// Copy not allowed
		Panel(const Panel& other) = delete;
		Panel& operator=(const Panel& other) = delete;
		// Move is allowed
		Panel(Panel&& other) noexcept = default;
		Panel& operator=(Panel&& other) noexcept = default;
		~Panel();

		// Accessors
		[[nodiscard]] RectI rect_px() const;

		// Modifiers
		void set_timeout(float in_seconds) { _timeout_s = in_seconds; }

		// Lifecycle Management
        void update_positions();
        Action handle_events(Events& events);
        Action update_contents(float delta_time_s);
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
