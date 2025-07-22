#pragma once
#include "TurnBasedBotClientThread.h"
#include "TurnBasedHostClientThread.h"
#include "TurnBasedServerActorInterface.h"
#include <list>
#include <optional>

namespace m2 {
	/// \brief Different components used by the server in a TurnBased game.
	/// \details Clients are listed after the server so that they are killed before the server. This prevents the server
	/// from hanging flushing the messages.
	struct TurnBasedServerComponents {
		std::optional<TurnBasedServerActorInterface> serverActorInterface;
		std::optional<network::TurnBasedHostClientThread> hostClientThread;
		std::list<network::TurnBasedBotClientThread> botClientThreads;
	};
}
