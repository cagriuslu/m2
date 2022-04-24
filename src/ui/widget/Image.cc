#include <m2/ui/widget/Image.h>
#include <m2/ui/WidgetBlueprint.h>

m2::ui::wdg::ImageState::ImageState(const WidgetBlueprint* blueprint) : AbstractButtonState(blueprint), texture_rect(std::get<ImageBlueprint>(blueprint->variant).initial_texture_rect) {}

m2::ui::Action m2::ui::wdg::ImageState::update_content() {
    auto& image_blueprint = std::get<ImageBlueprint>(blueprint->variant);
    if (image_blueprint.update_callback) {
        auto[action, optional_rect] = image_blueprint.update_callback();
        if (action == Action::CONTINUE && optional_rect) {
            texture_rect = *optional_rect;
        }
        return action;
    } else {
        return Action::CONTINUE;
    }
}
