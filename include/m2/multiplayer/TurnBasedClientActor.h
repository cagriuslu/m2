#pragma once
#include "Type.h"
#include "../Object.h"
#include "turnbased/MessagePasser.h"
#include <m2/network/Types.h>
#include <m2/mt/actor/ActorBase.h>
#include <m2/thirdparty/video/Detail.h>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <variant>

namespace m2::network::detail {
	/// State shared between the actor interface (TurnBasedClientThreadBase, accessed from the main thread) and
	/// the actor (TurnBasedClientActor, accessed from the actor thread). Every access must be guarded by the mutex.
	struct TurnBasedClientSharedState {
		std::mutex mutex;
		pb::ClientThreadState state{pb::ClientThreadState::CLIENT_INITIAL_STATE};
		std::queue<pb::TurnBasedNetworkMessage> outgoingQueue, incomingQueue;
		std::optional<std::pair<SequenceNo,pb::TurnBasedServerUpdate>> receivedServerUpdate;
		std::optional<std::pair<SequenceNo,m2g::pb::TurnBasedServerCommand>> receivedServerCommand;
	};
	struct TurnBasedClientSharedStateOwner {
		std::shared_ptr<TurnBasedClientSharedState> sharedClientState = std::make_shared<TurnBasedClientSharedState>();
	};

	/// The actor and the interface communicate through the shared state rather than the message boxes, so the actor's
	/// input and output message types are std::monostate.
	class TurnBasedClientActor final : public ActorBase<std::monostate, std::monostate> {
		std::shared_ptr<TurnBasedClientSharedState> _sharedState;
		const std::string _serverAddress;
		const char* _threadNameForLogging;

		SequenceNo _expectedServerUpdateSequenceNo{}, _expectedServerCommandSequenceNo{};
		uint64_t _levelToken{};
		std::variant<std::monostate, multiplayer::turnbased::MessagePasser, thirdparty::video::Ticks> _socketManagerOrTicksDisconnectedAt;
		/// When a connection attempt times out, the next attempt is deferred until this timepoint instead of blocking
		/// the actor with a sleep.
		std::optional<thirdparty::video::Ticks> _retryConnectionAfterTicks;

	public:
		TurnBasedClientActor(std::shared_ptr<TurnBasedClientSharedState> sharedState, std::string serverAddress, const char* threadNameForLogging)
			: _sharedState(std::move(sharedState)), _serverAddress(std::move(serverAddress)), _threadNameForLogging(threadNameForLogging) {}
		~TurnBasedClientActor() override = default;

		const char* ThreadNameForLogging() const override { return _threadNameForLogging; }

		bool Initialize(MessageBox<std::monostate>&, MessageBox<std::monostate>&) override;

		bool operator()(MessageBox<std::monostate>&, MessageBox<std::monostate>&) override;

		void Deinitialize(MessageBox<std::monostate>&, MessageBox<std::monostate>&) override {}
	};
}
