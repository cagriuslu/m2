#ifndef M2_GAMEPROXY_HH
#define M2_GAMEPROXY_HH

#include <m2/Object.h>

namespace m2 {
    struct game_proxy {
        static M2Err exec_entry_ui();
        static M2Err exec_pause_ui();
    };
}

#endif //M2_GAMEPROXY_HH
