#pragma once
#include "../Object.h"
#include <Network.pb.h>

namespace m2::mplayer {
	class State {
		std::optional<pb::NetworkMessage> _prev_server_update, _last_server_update;

	public:
		State() = default;
		explicit State(pb::NetworkMessage&& msg) : _last_server_update(std::move(msg)) {}

		void set_last_server_update(pb::NetworkMessage&& msg);
		const std::optional<pb::NetworkMessage>& prev_server_update() const { return _prev_server_update; }
		const std::optional<pb::NetworkMessage>& last_server_update() const { return _last_server_update; }
	};
}
