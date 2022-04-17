#include <impl/public/ui/UI.h>
#include <impl/private/LevelBlueprint.h>
#include <m2/ui/UI.h>
#include "m2/GameProxy.hh"
#include "m2/Game.hh"
#include "SDL_image.h"

M2Err m2::game_proxy::exec_entry_ui() {
    auto button = m2::ui::execute_blocking(&impl::ui::entry);
    if (button.index() == 0) {
        int return_value = std::get<0>(button);
        if (return_value == impl::ui::ENTRY_NEW_GAME) {
            return Game_Level_Load(&impl::level::sp_000);
        } else {
            return M2ERR_QUIT;
        }
    } else {
        return std::get<1>(button);
    }
}

M2Err m2::game_proxy::exec_pause_ui() {
    auto button = m2::ui::execute_blocking(&impl::ui::pause);
    if (button.index() == 0) {
        int return_value = std::get<0>(button);
        if (return_value == impl::ui::PAUSE_RESUME_GAME) {
            return Game_Level_Load(&impl::level::sp_000);
        } else {
            return M2ERR_QUIT;
        }
    } else {
        return std::get<1>(button);
    }
}
