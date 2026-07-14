#pragma once
#include <m2/video/Sprite.h>
#include "AbstractButton.h"
#include <variant>
#include <functional>

namespace m2::widget {
	class CheckboxWithImage : public AbstractButton {
		bool _state;
		std::variant<m2g::pb::SpriteType, std::function<void(const CheckboxWithImage&)>> _content{};

	public:
		explicit CheckboxWithImage(UiPanel* parent, const UiWidgetBlueprint* blueprint);

		// Accessors

		[[nodiscard]] bool GetState() const { return _state; }

		// Modifiers

		void SetState(bool state) { _state = state; }
		void SetSpriteType(m2g::pb::SpriteType);

	protected:
		UiAction OnUpdate() override;
		void OnDraw() override;

	private:
		[[nodiscard]] const CheckboxWithImageBlueprint& VariantBlueprint() const { return std::get<CheckboxWithImageBlueprint>(blueprint->variant); }

		friend struct AbstractButton;
	};
}
