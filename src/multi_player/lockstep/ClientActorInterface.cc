#include <m2/multi_player/lockstep/ClientActorInterface.h>
#include <m2/multi_player/lockstep/ConnectionToServer.h>
#include <m2/Meta.h>

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
const m2g::pb::LockstepGameInitParams* ClientActorInterface::GetGameInitParams() {
	ProcessOutbox();
	return _gameInitParams ? &*_gameInitParams : nullptr;
}

void ClientActorInterface::SetReadyState(const bool state) {
	GetActorInbox().PushMessage(ClientActorInput{
		.variant = ClientActorInput::SetReadyState{
			.state = state
		}
	});
}

void ClientActorInterface::ProcessOutbox() {
	GetActorOutbox().PopMessages([this](const ClientActorOutput& msg) {
		if (std::holds_alternative<ClientActorOutput::ConnectionToServerStateUpdate>(msg.variant)) {
			_connectionToServerState = std::get<ClientActorOutput::ConnectionToServerStateUpdate>(msg.variant);
		}
		if (msg.gameInitParams) {
			_gameInitParams = std::move(msg.gameInitParams);
		}
		return true;
	}, 10);
}
