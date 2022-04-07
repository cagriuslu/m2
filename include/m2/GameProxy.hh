#ifndef M2_GAMEPROXY_HH
#define M2_GAMEPROXY_HH

#include <m2/Object.h>

namespace m2 {
    struct game_proxy {
        const char *textureMapFile;
        const char *textureMaskFile;

        M2Err activate() const;
        M2Err exec_entry_ui() const;
        M2Err exec_pause_ui() const;
    };
}

#endif //M2_GAMEPROXY_HH
