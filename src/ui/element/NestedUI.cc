#include <m2/ui/element/NestedUI.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/ui/UI.h>

m2::ui::element::NestedUIState::NestedUIState(const ElementBlueprint *blueprint) : ElementState(blueprint) {
    ui = std::make_unique<UIState>(std::get<NestedUIBlueprint>(blueprint->blueprint_variant).ui);
}

void m2::ui::element::NestedUIState::update_position(const SDL_Rect &rect_px) {
    this->rect_px = rect_px;
    ui->update_positions(rect_px);
}

void m2::ui::element::NestedUIState::update_content() {
    ui->update_contents();
}

void m2::ui::element::NestedUIState::draw() {
    ui->draw();
}
