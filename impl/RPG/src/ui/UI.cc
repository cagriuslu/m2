#include <impl/public/ui/UI.h>
#include <impl/private/ui/Callbacks.h>
#include <m2/ui/element/ButtonStaticText.h>
#include <m2/ui/ElementBlueprint.h>

const m2::ui::UIBlueprint impl::ui::entry = {
        .w = 100, .h = 100,
        .background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
        .elements = {
            m2::ui::ElementBlueprint{
                .x = 45, .y = 35, .w = 10, .h = 10,
                .border_width_px = 1,
                .blueprint_variant = m2::ui::element::ButtonStaticTextBlueprint{
                    .text = "NEW GAME",
                    .keyboard_shortcut = SDL_SCANCODE_N,
                    .return_value = impl::ui::ENTRY_NEW_GAME
                }
            },
            m2::ui::ElementBlueprint{
                .x = 45, .y = 55, .w = 10, .h = 10,
                .border_width_px = 1,
                .blueprint_variant = m2::ui::element::ButtonStaticTextBlueprint{
                     .text = "QUIT",
                     .keyboard_shortcut = SDL_SCANCODE_Q,
                     .return_value = impl::ui::ENTRY_QUIT
                }
            }
        }
};

const m2::ui::UIBlueprint impl::ui::pause = {
        .w = 100, .h = 100,
        .background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
        .elements = {
                m2::ui::ElementBlueprint{
                        .x = 45, .y = 35, .w = 10, .h = 10,
                        .border_width_px = 1,
                        .blueprint_variant = m2::ui::element::ButtonStaticTextBlueprint{
                                .text = "RESUME GAME",
                                .keyboard_shortcut = SDL_SCANCODE_R,
                                .return_value = impl::ui::PAUSE_RESUME_GAME
                        }
                },
                m2::ui::ElementBlueprint{
                        .x = 45, .y = 55, .w = 10, .h = 10,
                        .border_width_px = 1,
                        .blueprint_variant = m2::ui::element::ButtonStaticTextBlueprint{
                                .text = "QUIT",
                                .keyboard_shortcut = SDL_SCANCODE_Q,
                                .return_value = impl::ui::PAUSE_QUIT
                        }
                }
        }
};

const m2::ui::UIBlueprint impl::ui::left_hud = {
        .w = 19, .h = 72,
        .background_color = {.r = 50, .g = 50, .b = 50, .a = 255},
        .elements = {
                m2::ui::ElementBlueprint{
                        .x = 4, .y = 50, .w = 11, .h = 2,
                        .blueprint_variant = m2::ui::element::StaticTextBlueprint{
                                .text = "HP"
                        }
                },
                m2::ui::ElementBlueprint{
                        .x = 4, .y = 52, .w = 11, .h = 2,
                        .blueprint_variant = m2::ui::element::DynamicTextBlueprint{
                                .callback = hp_callback
                        }
                }
        }
};

const m2::ui::UIBlueprint impl::ui::right_hud = {
        .w = 19, .h = 72,
        .background_color = {.r = 50, .g = 50, .b = 50, .a = 255}
};
