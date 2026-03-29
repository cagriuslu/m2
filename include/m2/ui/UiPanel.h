#pragma once
#include "UiPanelBlueprint.h"
#include "UiPanelState.h"
#include "UiWidget.h"
#include "../math/RectI.h"
#include "../math/RectF.h"

namespace m2 {
	struct UiPanel {
		struct Fullscreen {};
		struct RelativeToWindow {
			RectF ratioToGameAndHudDimensions;
			static RelativeToWindow CreateAnchoredToPosition(const RectI& positionWithinToGameAndHud);
		};
		struct RelativeToWorld {
			VecF centeredAt;
			VecF dimensionsRelativeToGameAndHud;
		};
		using PanelPosition = std::variant<Fullscreen, RelativeToWindow, RelativeToWorld>;

	private:
		/// If this field contains an object, the UiPanel has returned a Return value, but the panel wasn't freed.
		/// Instead, the Panel is destructed and recreated without any content, and the return value is stored to be
		/// extracted later. This mechanism is usually used for Custom UI panels because they can't be destroyed right
		/// away, as the game code may contain references to them.
		struct Undead {
			AnyReturnContainer returnValue;
			bool wasAutoClose;
		};
		std::optional<Undead> _undeadContainer;
		bool _prev_text_input_state{};
		std::unique_ptr<UiPanelBlueprint> _owned_blueprint; // `blueprint` will point here if this object exists
		PanelPosition _panelPosition;
		std::optional<VecI> _lastScreenPositionOfCenterIfRelativeToWorld;
		sdl::TextureUniquePtr _background_texture; // TODO if the screen is resized, background looks bad
		std::optional<float> _timeout_s;

		/// Used by KillWithReturnValue()
		explicit UiPanel(AnyReturnContainer&& returnValueContainer, const bool autoClose) : _undeadContainer(Undead{std::move(returnValueContainer), autoClose}) {}

		UiAction run_blocking();

	public:
		bool enabled{true};
		const UiPanelBlueprint* blueprint{}; // TODO merge with _owned_blueprint, make private
		/// Pointer to the state. State is often used to store information that's closely related to the UI panel.
		std::unique_ptr<UiPanelStateBase> state;
		std::vector<std::unique_ptr<UiWidget>> widgets; // TODO make private

		/// Use this constructor for non-blocking operation
		explicit UiPanel(std::variant<const UiPanelBlueprint*, std::unique_ptr<UiPanelBlueprint>> static_or_unique_blueprint,
				const PanelPosition& panelPosition = {},
				sdl::TextureUniquePtr background_texture = {});
		/// Use this constructor for blocking operation
		static UiAction create_and_run_blocking(std::variant<const UiPanelBlueprint*, std::unique_ptr<UiPanelBlueprint>> static_or_unique_blueprint,
				const PanelPosition& panelPosition = {},
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
		[[nodiscard]] bool IsAutoClean() const;
		[[nodiscard]] RectI Rect() const;

		// Modifiers

		void KillWithReturnValue(AnyReturnContainer&&);
		/// Given position must be with respect to GameAndHUD area, which means {0,0} corresponds to top-left point of
		/// GameAndHud area.
		void SetTopLeftPosition(const VecI&);
		/// After timeout, the panel is disabled, which effectively also hides it.
		void SetTimeout(float timeoutS);
		void ClearTimeout();

		// Lifecycle Management

        void UpdatePosition();
		/// Handle the events. If `IsPanning` is true, the mouse state (button states) are not cleared after the
		/// handling so that the game objects can still observe that the mouse buttons are down.
        UiAction HandleEvents(Events& events, bool IsPanning = false);
        UiAction UpdateContents(float delta_time_s);
        void Draw();

	private:
		[[nodiscard]] int vertical_border_width_px() const;
		[[nodiscard]] int horizontal_border_width_px() const;

		std::unique_ptr<UiWidget> create_widget_state(const UiWidgetBlueprint& widget_blueprint);
		void set_widget_focus_state(UiWidget& w, bool);
		void clear_focus();

	public:
		// Helpers

		template <typename WidgetT>
		[[nodiscard]] WidgetT* FindWidget() const {
			for (auto& w : widgets) {
				if (auto* widget = dynamic_cast<WidgetT*>(w.get())) {
					return widget;
				}
			}
			return nullptr;
		}
		template <typename WidgetT>
		[[nodiscard]] WidgetT* FindWidget(const std::string& name) const {
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
	UiWidget* FindTextWidget(UiPanel& state, const std::string& text); // TODO is this really necessary?

	extern UiPanelBlueprint console_ui;
}
