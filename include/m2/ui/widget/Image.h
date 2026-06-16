#pragma once
#include <m2/video/Sprite.h>
#include "AbstractButton.h"
#include <variant>
#include <functional>

namespace m2::widget {
	class Image : public AbstractButton {
		std::variant<m2g::pb::SpriteType, std::function<void(const Image&)>> _content{};

	public:
		explicit Image(UiPanel* parent, const UiWidgetBlueprint* blueprint);

		// Modifiers

		void SetSpriteType(m2g::pb::SpriteType);

	protected:
		UiAction OnUpdate() override;
		void OnDraw() override;

	private:
		[[nodiscard]] const ImageBlueprint& VariantBlueprint() const { return std::get<ImageBlueprint>(blueprint->variant); }
	};
}
