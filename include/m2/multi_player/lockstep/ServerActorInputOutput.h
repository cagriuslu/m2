#pragma once
#include <m2g_Lockstep.pb.h>
#include <variant>

namespace m2::multiplayer::lockstep {
	struct ServerActorInput {
		struct FreezeLobby {
			m2g::pb::LockstepGameInitParams gameInitParams;
		};
		struct IsAllOutgoingMessagesDelivered {};
		struct GameStateHash {
			network::Timecode timecode;
			int32_t hash;
		};
		std::variant<FreezeLobby, IsAllOutgoingMessagesDelivered, GameStateHash> variant;
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
