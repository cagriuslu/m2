#include <impl/private/ui/Callbacks.h>
#include <m2/Game.hh>

std::string impl::ui::hp_callback() {
    // Lookup player's health
    auto* player = GAME.objects.get(GAME.playerId);
    if (player) {
        auto& def = GAME.defenses[player->defense_id];
        return std::to_string(def.hp);
    }
    return {};
}
