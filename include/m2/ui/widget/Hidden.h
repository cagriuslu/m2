#pragma once
#include "../Widget.h"

namespace m2::ui::widget {
    class Hidden final : public Widget {
    public:
        explicit Hidden(State* parent, const WidgetBlueprint* blueprint) : Widget(parent, blueprint) {}

        Action on_event(Events& events) override;
        Action on_update() override;

    private:
        [[nodiscard]] const HiddenBlueprint& hidden_blueprint() const { return std::get<HiddenBlueprint>(blueprint->variant); }
    };
}
