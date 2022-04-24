#include <m2/ui/widget/NestedUI.h>
#include <m2/ui/WidgetBlueprint.h>
#include <m2/ui/UI.h>

using namespace m2::ui;
using namespace m2::ui::wdg;

NestedUIState::NestedUIState(const WidgetBlueprint *blueprint) : WidgetState(blueprint) {
    ui = std::make_unique<UIState>(std::get<NestedUIBlueprint>(blueprint->variant).ui);
}

void NestedUIState::update_position(const SDL_Rect &rect_px_) {
    this->rect_px = rect_px_;
    ui->update_positions(rect_px_);
}

Action NestedUIState::handle_events(Events &events) {
    return ui->handle_events(events);
}

Action NestedUIState::update_content() {
    return ui->update_contents();
}

void NestedUIState::draw() {
    ui->draw();
}