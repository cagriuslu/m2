#pragma once
#include "../UiWidget.h"

namespace m2::widget {
    class Hidden final : public UiWidget {
    public:
        Hidden(UiPanel* parent, const UiWidgetBlueprint* blueprint);

        UiAction on_event(Events& events) override;
        UiAction on_update() override;

    private:
        [[nodiscard]] const HiddenBlueprint& hidden_blueprint() const { return std::get<HiddenBlueprint>(blueprint->variant); }
    };
}
