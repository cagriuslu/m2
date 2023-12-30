#include <m2/multi_player/State.h>

void m2::mplayer::State::set_last_server_update(pb::NetworkMessage&& msg) {
	if (_last_server_update) {
		_prev_server_update = std::move(_last_server_update);
	}
	_last_server_update = std::move(msg);
}
