#include <m2/ui/widget/NestedUi.h>
#include <m2/ui/State.h>

using namespace m2::ui;
using namespace m2::ui::widget;

NestedUi::NestedUi(const WidgetBlueprint *blueprint) : Widget(blueprint) {
	ui = std::make_unique<State>(std::get<NestedUiBlueprint>(blueprint->variant).ui);
}

void NestedUi::update_position(const SDL_Rect &rect_px_) {
	this->rect_px = rect_px_;
	ui->update_positions(rect_px_);
}

Action NestedUi::handle_events(Events &events) {
	return ui->handle_events(events);
}

Action NestedUi::update_content() {
	return ui->update_contents();
}

void NestedUi::draw() {
	ui->draw();
}
