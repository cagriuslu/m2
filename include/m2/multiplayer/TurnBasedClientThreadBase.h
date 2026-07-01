#pragma once
#include "TurnBasedClientActor.h"
#include <m2/mt/actor/ActorInterfaceBase.h>

namespace m2::network::detail {
	/// Base class of turn based client threads // TODO use message boxes instead of shared state
	/// Base classes are constructed in declaration order, the shared state is fully constructed before
	/// ActorInterfaceBase builds the actor.
	class TurnBasedClientThreadBase : TurnBasedClientSharedStateOwner, public ActorInterfaceBase<TurnBasedClientActor> {
		uint64_t _ready_token{};
		SequenceNo _nextClientCommandSequenceNo{};

	public:
		TurnBasedClientThreadBase(std::string addr, const char* threadNameForLogging);
		TurnBasedClientThreadBase(const TurnBasedClientThreadBase& other) = delete;
		TurnBasedClientThreadBase& operator=(const TurnBasedClientThreadBase& other) = delete;
		TurnBasedClientThreadBase(TurnBasedClientThreadBase&& other) = delete;
		TurnBasedClientThreadBase& operator=(TurnBasedClientThreadBase&& other) = delete;

		// Accessors

		pb::ClientThreadState locked_get_client_state();
		bool locked_has_server_update();
		const pb::TurnBasedServerUpdate* locked_peek_server_update();
		std::optional<std::pair<SequenceNo,pb::TurnBasedServerUpdate>> locked_pop_server_update();
		bool locked_has_server_command();
		std::optional<std::pair<SequenceNo,m2g::pb::TurnBasedServerCommand>> locked_pop_server_command();

		// Modifiers

		void locked_set_ready(bool state);
		/// Only for TurnBasedHostClientThread and TurnBasedBotClientThread
		void locked_start_if_ready();
		void locked_queue_client_command(const m2g::pb::TurnBasedClientCommand& cmd);
		void locked_shutdown();

	private:
		// Private modifiers
		void unlocked_set_state(pb::ClientThreadState state);
		void locked_set_state(pb::ClientThreadState state);
	};
}
