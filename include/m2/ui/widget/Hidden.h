#pragma once
#include "../UiWidget.h"

namespace m2::widget {
    class Hidden final : public UiWidget {
    public:
        Hidden(UiPanel* parent, const UiWidgetBlueprint* blueprint);

    protected:
        UiAction OnEvent(Events& events) override;
        UiAction OnUpdate() override;

    private:
        [[nodiscard]] const HiddenBlueprint& VariantBlueprint() const { return std::get<HiddenBlueprint>(blueprint->variant); }
    };
}
