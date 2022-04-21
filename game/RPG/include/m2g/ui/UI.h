#ifndef IMPL_UI_H
#define IMPL_UI_H

#include "m2/ui/UI.h"
#include "m2/Def.h"

namespace impl::ui {
    enum EntryReturnValue {
        ENTRY_NEW_GAME = 0,
        ENTRY_QUIT = M2ERR_QUIT
    };
    extern const m2::ui::UIBlueprint entry;

    enum PauseReturnValue {
        PAUSE_RESUME_GAME = 0,
        PAUSE_QUIT = M2ERR_QUIT
    };
    extern const m2::ui::UIBlueprint pause;

    extern const m2::ui::UIBlueprint left_hud;
    extern const m2::ui::UIBlueprint right_hud;
}

#endif //IMPL_UI_H
