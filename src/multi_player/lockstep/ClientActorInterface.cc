#include <m2/multi_player/lockstep/ClientActorInterface.h>
#include <m2/multi_player/lockstep/ConnectionToServer.h>
#include <m2/ProxyHelper.h>
#include <m2/Meta.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

bool ClientActorInterface::IsSearchingForServer() {
	ProcessOutbox();
	return _connectionToServerState.stateIndex == GetIndexInVariant<ConnectionToServer::SearchForServer, ConnectionToServer::State>::value;
}
bool ClientActorInterface::IsWaitingInLobby() {
	ProcessOutbox();
	return _connectionToServerState.stateIndex == GetIndexInVariant<ConnectionToServer::WaitingInLobby, ConnectionToServer::State>::value;
}
bool ClientActorInterface::IsLobbyFrozen() {
	ProcessOutbox();
	return _connectionToServerState.stateIndex == GetIndexInVariant<ConnectionToServer::LobbyFrozen, ConnectionToServer::State>::value;
}
bool ClientActorInterface::IsGameStarted() {
	ProcessOutbox();
	return _connectionToServerState.stateIndex == GetIndexInVariant<ConnectionToServer::GameStarted, ConnectionToServer::State>::value;
}
bool ClientActorInterface::GetLastSetReadyState() const {
	if (not std::holds_alternative<GameNotStarted>(_state)) {
		throw M2_ERROR("Last set ready state was queried after the game has started");
	}
	return std::get<GameNotStarted>(_state).lastSetReadyState;
}
const m2g::pb::LockstepGameInitParams* ClientActorInterface::GetGameInitParams() {
	ProcessOutbox();
	return _gameInitParams ? &*_gameInitParams : nullptr;
}

void ClientActorInterface::SetReadyState(const bool state) {
	if (not std::holds_alternative<GameNotStarted>(_state)) {
		throw M2_ERROR("Ready state was set after the game has started");
	}
	std::get<GameNotStarted>(_state).lastSetReadyState = state;
	GetActorInbox().PushMessage(ClientActorInput{
		.variant = ClientActorInput::SetReadyState{
			.state = state
		}
	});
}
void ClientActorInterface::StartInputStreaming() {
	if (not IsLobbyFrozen()) {
		throw M2_ERROR("Attempt to start the game outside of lobby frozen state");
	}
	if (not std::holds_alternative<GameNotStarted>(_state)) {
		throw M2_ERROR("Game can be started only once");
	}
	LOG_NETWORK("Commiting empty inputs to start of game");
	GetActorInbox().PushMessage(ClientActorInput{
		.variant = ClientActorInput::QueueThisPlayerInput{}
	});
	_state.emplace<SimulatingInputs>();
}
bool ClientActorInterface::TryQueueInput(m2g::pb::LockstepPlayerInput&& input) {
	if (std::holds_alternative<GameNotStarted>(_state)) {
		throw M2_ERROR("Attempt to queue input while the game hasn't started");
	}
	if (std::holds_alternative<SimulatingInputs>(_state)) {
		LOG_NETWORK("Queueing input from player");
		std::get<SimulatingInputs>(_state).selfInputs.emplace_back(input);
		return true;
	}
	LOG_NETWORK("Unable to queue input from player");
	return false;
}
ClientActorInterface::SwapResult ClientActorInterface::SwapInputs() {
	if (std::holds_alternative<GameNotStarted>(_state)) {
		throw M2_ERROR("Attempt to swap inputs while the game hasn't started");
	}
	if (std::holds_alternative<ReadyToSimulate>(_state)) {
		throw M2_ERROR("Attempt to swap input while there are inputs to simulate");
	}
	if (std::holds_alternative<SimulatingInputs>(_state)) {
		auto& simulatingInputs = std::get<SimulatingInputs>(_state);
		if (simulatingInputs.physicsSimulationsCounter == m2g::LOCKSTEP_PHYSICS_SIMULATION_COUNT_PER_GAME_TICK) {
			LOG_NETWORK("Commiting inputs from this player");
			GetActorInbox().PushMessage(ClientActorInput{
				.variant = ClientActorInput::QueueThisPlayerInput{
					.inputs = std::move(simulatingInputs.selfInputs)
				}
			});
			// Switch to lagging state and check inputs to simulate
			_state.emplace<Lagging>();
			// Give some time for client actor to publish the inputs to simulate
			std::this_thread::sleep_for(std::chrono::milliseconds{5});
		} else {
			++simulatingInputs.physicsSimulationsCounter;
			return SimulatePhysics{};
		}
		// Intentional fallthrough
	}
	if (std::holds_alternative<Lagging>(_state)) {
		ProcessOutbox();
		if (std::holds_alternative<Lagging>(_state)) {
			LOG_NETWORK("Simulation inputs haven't arrived yet");
			return SkipPhysics{};
		}
		if (std::holds_alternative<ReadyToSimulate>(_state)) {
			LOG_NETWORK("Simulation inputs have arrived");
			return SimulatePhysics{};
		}
	}
	throw M2_ERROR("Implementation error");
}
std::optional<ClientActorInterface::ReadyToSimulate> ClientActorInterface::PopSimulationInputs() {
	if (std::holds_alternative<ReadyToSimulate>(_state)) {
		LOG_NETWORK("Simulation inputs are popped");
		auto retval = std::move(std::get<ReadyToSimulate>(_state));
		// Return to simulating state
		_state.emplace<SimulatingInputs>();
		return std::move(retval);
	}
	return std::nullopt;
}
void ClientActorInterface::StoreGameStateHash(const network::Timecode tc, const int32_t hash) {
	GetActorInbox().PushMessage(ClientActorInput{
		.variant = ClientActorInput::GameStateHash{
			.timecode = tc,
			.hash = hash
		}
	});
}

void ClientActorInterface::ProcessOutbox() {
	GetActorOutbox().TryHandleMessages([this](ClientActorOutput& msg) {
		if (std::holds_alternative<ClientActorOutput::ConnectionToServerStateUpdate>(msg.variant)) {
			_connectionToServerState = std::get<ClientActorOutput::ConnectionToServerStateUpdate>(msg.variant);
		} else if (std::holds_alternative<ClientActorOutput::PlayerInputsToSimulate>(msg.variant)) {
			auto& playerInputsToSimulate = std::get<ClientActorOutput::PlayerInputsToSimulate>(msg.variant);
			if (std::holds_alternative<GameNotStarted>(_state)) {
				throw M2_ERROR("Received inputs to simulate before the game has started");
			}
			if (std::holds_alternative<SimulatingInputs>(_state)) {
				throw M2_ERROR("Received inputs to simulate before commiting next inputs");
			}
			if (std::holds_alternative<Lagging>(_state)) {
				LOG_NETWORK("Received inputs to simulate");
				_state.emplace<ReadyToSimulate>(playerInputsToSimulate.timecode, std::move(playerInputsToSimulate.playerInputs));
				return false; // There could be multiple player inputs in the queue. Pop only one of them.
			}
			if (std::holds_alternative<ReadyToSimulate>(_state)) {
				throw M2_ERROR("Received inputs to simulate before simulating previous inputs");
			}
		}
		if (msg.gameInitParams) {
			_gameInitParams = std::move(msg.gameInitParams);
		}
		return true;
	}, 10);
}
