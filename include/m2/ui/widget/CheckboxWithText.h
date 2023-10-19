#pragma once
#include "AbstractButton.h"
#include "../../sdl/Detail.hh"

namespace m2::ui::widget {
	class CheckboxWithText : public AbstractButton {
		bool _state;
		sdl::TextureUniquePtr _font_texture;

	public:
		explicit CheckboxWithText(State* parent, const WidgetBlueprint* blueprint);
		void on_draw() override;

		inline bool state() const { return _state; }

		friend struct AbstractButton;
	};
}
