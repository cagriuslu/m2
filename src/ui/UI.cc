#include <m2/ui/UI.h>
#include <m2/ui/element/NestedUI.h>
#include <m2/Event.hh>
#include <m2/Def.hh>
#include <m2/Game.hh>

static SDL_Rect calculate_element_rect(const SDL_Rect& root_rect_px, unsigned root_w, unsigned root_h, unsigned child_x, unsigned child_y, unsigned child_w, unsigned child_h) {
    auto pixels_per_unit_w = (float)root_rect_px.w / (float)root_w;
    auto pixels_per_unit_h = (float)root_rect_px.h / (float)root_h;
    return SDL_Rect{
            root_rect_px.x + (int)roundf((float)child_x * pixels_per_unit_w),
            root_rect_px.y + (int)roundf((float)child_y * pixels_per_unit_h),
            (int)roundf((float)child_w * pixels_per_unit_w),
            (int)roundf((float)child_h * pixels_per_unit_h)
    };
}

m2::ui::ElementState* m2::ui::UIState::find_element_by_pixel(const m2::Vec2i& mouse_pos) {
    m2::ui::ElementState* found_element = nullptr;
    for (const auto& element : elements) {
        if (std::holds_alternative<element::NestedUIBlueprint>(element->blueprint->blueprint_variant)) {
            found_element = dynamic_cast<element::NestedUIState*>(element.get())->ui->find_element_by_pixel(mouse_pos);
        } else {
            auto p = SDL_Point{ mouse_pos.x, mouse_pos.y };
            found_element = SDL_PointInRect(&p, &element->rect_px) ? element.get() : nullptr;
        }
        if (found_element) {
            break;
        }
    }
    return found_element;
}

m2::ui::ElementState* m2::ui::UIState::find_element_by_keyboard_shortcut(const uint8_t* raw_keyboard_state) {
    m2::ui::ElementState* found_element = nullptr;
    for (const auto& element : elements) {
        if (std::holds_alternative<element::NestedUIBlueprint>(element->blueprint->blueprint_variant)) {
            found_element = dynamic_cast<element::NestedUIState*>(element.get())->ui->find_element_by_keyboard_shortcut((raw_keyboard_state));
        } else {
            found_element = element->get_keyboard_shortcut_active(raw_keyboard_state) ? element.get() : nullptr;
        }
        if (found_element) {
            break;
        }
    }
    return found_element;
}

void m2::ui::UIState::draw_background_color(const SDL_Rect& rect, const SDL_Color& color) {
    if (color.a == 0) {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
    } else {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, color.r, color.g, color.b, color.a);
    }
    SDL_RenderFillRect(GAME.sdlRenderer, &rect);
}

void m2::ui::UIState::draw_border(const SDL_Rect& rect, unsigned border_width_px) {
    if (border_width_px) {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(GAME.sdlRenderer, &rect);
    }
}

m2::ui::UIState::UIState() : blueprint(nullptr), rect_px() {}

m2::ui::UIState::UIState(const UIBlueprint* blueprint) : blueprint(blueprint), rect_px({}) {
    for (const auto& element_blueprint : blueprint->elements) {
        elements.push_back(element_blueprint.get_state());
    }
}

void m2::ui::UIState::update_positions(const SDL_Rect &rect_px_) {
    this->rect_px = rect_px_;
    for (auto& element_state : elements) {
        SDL_Rect element_rect = calculate_element_rect(rect_px_, blueprint->w, blueprint->h, element_state->blueprint->x, element_state->blueprint->y, element_state->blueprint->w, element_state->blueprint->h);
        element_state->update_position(element_rect);
    }
}

void m2::ui::UIState::update_contents() {
    for (auto& element_state : elements) {
        element_state->update_content();
    }
}

std::optional<int> m2::ui::UIState::handle_events(const Events& evs) {
    std::optional<int> return_value;

    SDL_Point mousePosition = {.x = evs.mousePosition.x, .y = evs.mousePosition.y};

    if (evs.buttonsPressed[BUTTON_PRIMARY] && SDL_PointInRect(&mousePosition, &rect_px)) {
        auto* element_under_mouse = find_element_by_pixel(evs.mousePosition);
        if (element_under_mouse) {
            element_under_mouse->set_depressed(true);
        }
    }

    if (evs.buttonsReleased[BUTTON_PRIMARY]) {
        if (SDL_PointInRect(&mousePosition, &rect_px)) {
            auto* element_under_mouse = find_element_by_pixel(evs.mousePosition);
            if (element_under_mouse) {
                return_value = element_under_mouse->get_button_return_value();
            }
        }
        for (auto& element : elements) {
            element->set_depressed(false);
        }
    }

    {
        auto* keyboard_shortcut_pressed_element = find_element_by_keyboard_shortcut(evs.rawKeyStates);
        if (keyboard_shortcut_pressed_element) {
            return_value = keyboard_shortcut_pressed_element->get_button_return_value();
        }
    }

    return return_value;
}

void m2::ui::UIState::draw() {
    draw_background_color(rect_px, blueprint->background_color);
    // Draw elements
    for (auto& element : elements) {
        element->draw();
    }
    // Draw border
    draw_border(rect_px, blueprint->border_width_px);
}

ValueOrM2Err<int> m2::ui::execute_blocking(const UIBlueprint *blueprint) {
    UIState state(blueprint);
    state.update_positions(GAME.windowRect);
    state.update_contents();

    Events evs;
    while (true) {
        ////////////////////////////////////////////////////////////////////////
        //////////////////////////// EVENT HANDLING ////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        if (Events_Gather(&evs)) {
            if (evs.quitEvent) {
                return ValueOrM2Err<int>{std::in_place_index<1>, M2ERR_QUIT};
            }
            if (evs.windowResizeEvent) {
                Game_UpdateWindowDimensions(evs.windowDims.x, evs.windowDims.y);
                state.update_positions(GAME.windowRect);
            }
            auto pressed_button_return_value = state.handle_events(evs);
            if (pressed_button_return_value) {
                LOG_INFO_M2V(M2_BUTTON, Int32, pressed_button_return_value.value());
                return ValueOrM2Err<int>{std::in_place_index<0>, pressed_button_return_value.value()};
            }
        }
        //////////////////////// END OF EVENT HANDLING /////////////////////////
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        /////////////////////////////// GRAPHICS ///////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        // Draw ui
        state.update_contents();
        state.draw();
        // Present
        SDL_RenderPresent(GAME.sdlRenderer);
        /////////////////////////// END OF GRAPHICS ////////////////////////////
        ////////////////////////////////////////////////////////////////////////
    }
    return ValueOrM2Err<int>{std::in_place_index<1>, M2OK};
}
