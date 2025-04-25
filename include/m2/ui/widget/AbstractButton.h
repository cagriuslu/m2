#pragma once
#include "../UiWidget.h"
#include <m2g_KeyType.pb.h>

namespace m2::widget {
	struct AbstractButton : public UiWidget {
		m2g::pb::KeyType keyboardShortcut;
		bool depressed;

		explicit AbstractButton(UiPanel* parent, const UiWidgetBlueprint* blueprint);

		// Modifiers

		UiAction trigger_action();

	protected:
		UiAction OnEvent(Events& events) final;
	};
}
