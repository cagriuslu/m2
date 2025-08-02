#pragma once

namespace m2::multiplayer::lockstep {
	enum class ControlMessage {
		DISABLE_RETRANSMISSIONS_RESET_STATISTICS,
		ENABLE_RETRANSMISSIONS_RESET_STATISTICS,
	};
}
