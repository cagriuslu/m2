#pragma once
#include <m2g_Lockstep.pb.h>
#include <variant>

namespace m2::multiplayer::lockstep {
	struct ServerActorInput {
		struct FreezeLobby {
			m2g::pb::LockstepGameInitParams gameInitParams;
		};
		struct IsAllOutgoingMessagesDelivered {};
		std::variant<FreezeLobby, IsAllOutgoingMessagesDelivered> variant;
	};

	struct ServerActorOutput {
		struct ServerStateUpdate {
			size_t stateIndex{};
		};
		struct IsAllOutgoingMessagesDelivered {
			bool answer{};
		};
		std::variant<ServerStateUpdate, IsAllOutgoingMessagesDelivered> variant;
	};
}
