#include <m2/ui/element/DynamicImage.h>
#include <m2/ui/ElementBlueprint.h>

m2::ui::element::DynamicImageState::DynamicImageState(const ElementBlueprint* blueprint) : ElementState(blueprint), texture_rect() {}

void m2::ui::element::DynamicImageState::update_content() {
    texture_rect = std::get<DynamicImageBlueprint>(blueprint->blueprint_variant).callback();
}
