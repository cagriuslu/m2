#pragma once
#include "../Widget.h"

namespace m2::ui::widget {
    class Hidden final : public Widget {
    public:
        Hidden(Panel* parent, const WidgetBlueprint* blueprint);

        Action on_event(Events& events) override;
        Action on_update() override;

    private:
        [[nodiscard]] const HiddenBlueprint& hidden_blueprint() const { return std::get<HiddenBlueprint>(blueprint->variant); }
    };
}
