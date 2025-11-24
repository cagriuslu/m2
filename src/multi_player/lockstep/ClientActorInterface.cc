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
	ProcessOutbox(false); // Don't pop player inputs after the game is started
	return _connectionToServerState.stateIndex == GetIndexInVariant<ConnectionToServer::GameStarted, ConnectionToServer::State>::value;
}
const m2g::pb::LockstepGameInitParams* ClientActorInterface::GetGameInitParams() {
	ProcessOutbox();
	return _gameInitParams ? &*_gameInitParams : nullptr;
}

void ClientActorInterface::SetReadyState(const bool state) {
	_lastSetReadyState = state;
	GetActorInbox().PushMessage(ClientActorInput{
		.variant = ClientActorInput::SetReadyState{
			.state = state
		}
	});
}
void ClientActorInterface::CommitEmptyInputsToStartTheGame() {
	if (not IsGameStarted()) {
		LOG_NETWORK("Commiting empty inputs to signify start of game");
		GetActorInbox().PushMessage(ClientActorInput{
			.variant = ClientActorInput::QueueThisPlayerInput{}
		});
		return;
	}
	throw M2_ERROR("Game can be started only once");
}
void ClientActorInterface::QueueThisPlayerInput(m2g::pb::LockstepPlayerInput&& input) {
	LOG_NETWORK("Queueing inputs from this player for later");
	_thisPlayerInputBuffer.emplace_back(input);
}
void ClientActorInterface::CommitThisPlayerInputsAndPopReadyToSimulateInputsIfNecessary(std::optional<std::vector<std::deque<m2g::pb::LockstepPlayerInput>>>& out) {
	out.reset();
	if (IsGameStarted()) {
		_physicsSimulationsCounter = (_physicsSimulationsCounter + 1) % m2g::LockstepPhysicsSimulationCountPerGameTick;
		if (_physicsSimulationsCounter == 0) {
			LOG_NETWORK("Commiting inputs from this player");
			// Commit inputs from this player
			GetActorInbox().PushMessage(ClientActorInput{
				.variant = ClientActorInput::QueueThisPlayerInput{
					.inputs = std::move(_thisPlayerInputBuffer)
				}
			});
			_thisPlayerInputBuffer.clear();

			// This loop hangs until all inputs are received
			LOG_NETWORK("Waiting for inputs to simulate");
			while (not _readyToSimulatePlayersInputs) {
				ProcessOutbox();
				std::this_thread::sleep_for(std::chrono::milliseconds{5});
			}
			LOG_NETWORK("Returning inputs to simulate to the game loop");
			out = std::move(_readyToSimulatePlayersInputs);
			_readyToSimulatePlayersInputs.reset();
		}
	}
}

void ClientActorInterface::ProcessOutbox(const bool checkPlayerInputs) {
	// Message handler can handle any message
	const auto messageHandler = [this](const ClientActorOutput& msg) {
		if (std::holds_alternative<ClientActorOutput::ConnectionToServerStateUpdate>(msg.variant)) {
			_connectionToServerState = std::get<ClientActorOutput::ConnectionToServerStateUpdate>(msg.variant);
		} else if (std::holds_alternative<ClientActorOutput::PlayerInputsToSimulate>(msg.variant)) {
			if (_readyToSimulatePlayersInputs) {
				throw M2_ERROR("Next player inputs are received before the previous inputs are simulated");
			}
			LOG_NETWORK("Received inputs to simulate");
			_readyToSimulatePlayersInputs = std::move(std::get<ClientActorOutput::PlayerInputsToSimulate>(msg.variant).playerInputs);
			return false; // There could be multiple player inputs in the queue. Pop only one of them.
		}
		if (msg.gameInitParams) {
			_gameInitParams = std::move(msg.gameInitParams);
		}
		return true;
	};

	GetActorOutbox().PopMessagesIf([checkPlayerInputs](const ClientActorOutput& msg) {
		if (checkPlayerInputs) {
			return true;
		}
		// If checkPlayerInputs is false, don't find PlayerInputsToSimulate interesting.
		return not std::holds_alternative<ClientActorOutput::PlayerInputsToSimulate>(msg.variant);
	}, messageHandler, 10);
}
