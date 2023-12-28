#pragma once

namespace m2::mplayer {
    enum class Type {
        TurnBased, // TCP Ex. HoMM
        LockStep, // TCP Ex. AoE
        StateCast, // UDP Ex. CoD
    };
}
