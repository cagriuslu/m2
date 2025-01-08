#pragma once
#include "UiPanelBlueprint.h"
#include "UiWidget.h"
#include "../math/RectI.h"
#include "../math/RectF.h"

namespace m2 {
	struct UiPanel {
	private:
		/// If this field contains an object, the UiPanel has returned a Return value, but the panel wasn't destroyed
		/// completely. Instead, the Panel is destroyed, recreated without any content, and the return value is stored
		/// inside, to be extracted later. HUD UI panels can't be destroyed right away, because the Proxy code may
		/// contain references to them.
		std::optional<AnyReturnContainer> _returnValueContainer;
		bool _prev_text_input_state{};
		std::unique_ptr<UiPanelBlueprint> _owned_blueprint; // `blueprint` will point here if this object exists
		RectF _relation_to_game_and_hud_dims;
		sdl::TextureUniquePtr _background_texture; // TODO if the screen is resized, background looks bad
		std::optional<float> _timeout_s;

		/// Used by KillWithReturnValue()
		explicit UiPanel(AnyReturnContainer&& returnValueContainer)
				: _returnValueContainer(std::move(returnValueContainer)) {}

		// Modifiers

		UiAction run_blocking();

	public:
		bool enabled{true};
		const UiPanelBlueprint* blueprint{};
		std::vector<std::unique_ptr<UiWidget>> widgets;

		/// Use this constructor for non-blocking operation
		explicit UiPanel(std::variant<const UiPanelBlueprint*, std::unique_ptr<UiPanelBlueprint>> static_or_unique_blueprint,
				const std::variant<std::monostate, RectI, RectF>& fullscreen_or_pixel_rect_or_relation_to_game_and_hud = {},
				sdl::TextureUniquePtr background_texture = {});
		/// Use this constructor for blocking operation
		static UiAction create_and_run_blocking(std::variant<const UiPanelBlueprint*, std::unique_ptr<UiPanelBlueprint>> static_or_unique_blueprint,
				const std::variant<std::monostate, RectI, RectF>& fullscreen_or_pixel_rect_or_relation_to_game_and_hud = {},
				sdl::TextureUniquePtr background_texture = {});
		// Copy not allowed
		UiPanel(const UiPanel& other) = delete;
		UiPanel& operator=(const UiPanel& other) = delete;
		// Move not allowed, because Widgets hold raw pointer to UiPanel
		UiPanel(UiPanel&& other) noexcept = delete;
		UiPanel& operator=(UiPanel&& other) noexcept = delete;
		~UiPanel();

		// Accessors

		[[nodiscard]] const std::string& Name() const;
		/// Check if the UI panel is already killed
		[[nodiscard]] bool IsKilled() const;
		/// Peek the return value contained inside the killed panel
		[[nodiscard]] const AnyReturnContainer* PeekReturnValueContainer() const;
		[[nodiscard]] RectI rect_px() const;

		// Modifiers

		void KillWithReturnValue(AnyReturnContainer&&);
		/// Given position must be with respect to GameAndHUD area, which means {0,0} corresponds to top-left point of
		/// GameAndHud area.
		void SetTopLeftPosition(const VecI&);
		/// After timeout, the panel is disabled, which effectively also hides it.
		void SetTimeout(float timeoutS);
		void ClearTimeout();

		// Lifecycle Management

        void update_positions();
		/// Handle the events. If `IsPanning` is true, the mouse state (button states) are not cleared after the
		/// handling so that the game objects can still observe that the mouse buttons are down.
        UiAction HandleEvents(Events& events, bool IsPanning = false);
        UiAction update_contents(float delta_time_s);
        void draw();

	private:
		[[nodiscard]] int vertical_border_width_px() const;
		[[nodiscard]] int horizontal_border_width_px() const;

		std::unique_ptr<UiWidget> create_widget_state(const UiWidgetBlueprint& widget_blueprint);
		void set_widget_focus_state(UiWidget& w, bool state);
		void clear_focus();

	public:
		// Helpers

		template <typename WidgetBlueprintT>
		[[nodiscard]] UiWidget* find_first_widget_of_blueprint_type() const {
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
				if (auto* widget = dynamic_cast<WidgetT*>(w.get())) {
					return widget;
				}
			}
			return nullptr;
		}

		template <typename WidgetT>
		[[nodiscard]] WidgetT* find_first_widget_by_name(const std::string& name) const {
			for (auto& w : widgets) {
				if (auto* widget = dynamic_cast<WidgetT*>(w.get()); widget && widget->blueprint->name == name) {
					return widget;
				}
			}
			return nullptr;
		}
	};

	// Helpers
	RectI CalculateWidgetRect(const RectI& root_rect_px, unsigned root_w, unsigned root_h, int child_x, int child_y, unsigned child_w, unsigned child_h);
	UiWidget* FindTextWidget(UiPanel& state, const std::string& text);

	extern UiPanelBlueprint console_ui;
}
