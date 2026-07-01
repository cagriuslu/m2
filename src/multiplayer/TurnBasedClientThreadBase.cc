#include <m2/multiplayer/TurnBasedClientThreadBase.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/mt/CooperativeSleep.h>

m2::network::detail::TurnBasedClientThreadBase::TurnBasedClientThreadBase(std::string addr, const char* threadNameForLogging)
		: TurnBasedClientSharedStateOwner(), ActorInterfaceBase(sharedClientState, std::move(addr), threadNameForLogging),
		_ready_token(RandomNonZero64()) {
	LOG_INFO("Constructing ClientThread with ready-token", _ready_token);
}

m2::pb::ClientThreadState m2::network::detail::TurnBasedClientThreadBase::locked_get_client_state() {
	const std::lock_guard lock(sharedClientState->mutex);
	return sharedClientState->state;
}
bool m2::network::detail::TurnBasedClientThreadBase::locked_has_server_update() {
	const std::lock_guard lock(sharedClientState->mutex);
	return static_cast<bool>(sharedClientState->receivedServerUpdate);
}
const m2::pb::TurnBasedServerUpdate* m2::network::detail::TurnBasedClientThreadBase::locked_peek_server_update() {
	const std::lock_guard lock(sharedClientState->mutex);
	if (sharedClientState->receivedServerUpdate) {
		return &sharedClientState->receivedServerUpdate->second;
	}
	return nullptr;
}
std::optional<std::pair<m2::network::SequenceNo,m2::pb::TurnBasedServerUpdate>> m2::network::detail::TurnBasedClientThreadBase::locked_pop_server_update() {
	const std::lock_guard lock(sharedClientState->mutex);
	if (sharedClientState->receivedServerUpdate) {
		auto tmp = std::move(sharedClientState->receivedServerUpdate);
		sharedClientState->receivedServerUpdate.reset();
		return tmp;
	} else {
		return std::nullopt;
	}
}
bool m2::network::detail::TurnBasedClientThreadBase::locked_has_server_command() {
	const std::lock_guard lock(sharedClientState->mutex);
	return static_cast<bool>(sharedClientState->receivedServerCommand);
}
std::optional<std::pair<m2::network::SequenceNo,m2g::pb::TurnBasedServerCommand>> m2::network::detail::TurnBasedClientThreadBase::locked_pop_server_command() {
	const std::lock_guard lock(sharedClientState->mutex);
	if (sharedClientState->receivedServerCommand) {
		auto tmp = std::move(sharedClientState->receivedServerCommand);
		sharedClientState->receivedServerCommand.reset();
		return tmp;
	} else {
		return std::nullopt;
	}
}

void m2::network::detail::TurnBasedClientThreadBase::locked_set_ready(bool ready) {
	auto state = locked_get_client_state();
	if (ready) {
		if (state != pb::CLIENT_CONNECTED && state != pb::CLIENT_RECONNECTED) {
			throw M2_ERROR("Unexpected state while signaling readiness");
		}
	} else {
		if (state != pb::CLIENT_READY) {
			throw M2_ERROR("Unexpected state while signaling unreadiness");
		}
	}

	LOG_INFO("Will send ready state", ready);

	{
		const std::lock_guard lock(sharedClientState->mutex);
		pb::TurnBasedNetworkMessage msg;
		msg.set_game_hash(M2_GAME.Hash());
		msg.mutable_client_update()->set_ready_token(ready ? _ready_token : 0);
		sharedClientState->outgoingQueue.push(std::move(msg));
		LOG_NETWORK("Readiness message queued");
	}

	auto locked_has_outgoing_message = [this] {
		const std::lock_guard lock(sharedClientState->mutex);
		return not sharedClientState->outgoingQueue.empty();
	};

	// Wait until the readiness message has been sent (the outgoing queue drains). On the web build this cooperatively
	// pumps the client actor (which drains the queue) instead of blocking the single thread; on native the actor drains
	// it on its own thread.
	CooperativeSleepUntil([&locked_has_outgoing_message] { return not locked_has_outgoing_message(); });
	locked_set_state(state ? pb::CLIENT_READY : pb::CLIENT_CONNECTED);
}
void m2::network::detail::TurnBasedClientThreadBase::locked_start_if_ready() {
	if (locked_get_client_state() == pb::CLIENT_READY) {
		locked_set_state(pb::CLIENT_STARTED);
	}
}
void m2::network::detail::TurnBasedClientThreadBase::locked_queue_client_command(const m2g::pb::TurnBasedClientCommand& cmd) {
	INFO_FN();

	pb::TurnBasedNetworkMessage msg;
	msg.set_game_hash(M2_GAME.Hash());
	msg.set_sequence_no(_nextClientCommandSequenceNo++);
	msg.mutable_client_command()->CopyFrom(cmd);

	{
		const std::lock_guard lock(sharedClientState->mutex);
		sharedClientState->outgoingQueue.push(std::move(msg));
	}
}
void m2::network::detail::TurnBasedClientThreadBase::locked_shutdown() {
	locked_set_state(pb::CLIENT_SHUTDOWN);
}

void m2::network::detail::TurnBasedClientThreadBase::unlocked_set_state(pb::ClientThreadState state) {
	LOG_NETWORK("Setting ClientThread state", pb::enum_name(state));
	sharedClientState->state = state;
}
void m2::network::detail::TurnBasedClientThreadBase::locked_set_state(pb::ClientThreadState state) {
	const std::lock_guard lock(sharedClientState->mutex);
	unlocked_set_state(state);
}
