#include <m2/multi_player/State.h>

void m2::mplayer::State::set_player_object_id(const int player_idx, const ObjectId id) {
    if (player_idx < _player_object_ids.size()) {
        _player_object_ids[player_idx] = id;
    } else {
        _player_object_ids.resize(player_idx + 1);
        _player_object_ids[player_idx] = id;
    }
}
