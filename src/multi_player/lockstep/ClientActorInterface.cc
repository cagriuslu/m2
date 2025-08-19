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
bool ClientActorInterface::IsWaitingForPlayers() {
	ProcessOutbox();
	return _connectionToServerState.stateIndex == GetIndexInVariant<ConnectionToServer::WaitForPlayers, ConnectionToServer::State>::value;
}

void ClientActorInterface::ProcessOutbox() {
	GetActorOutbox().PopMessages([this](const ClientActorOutput& msg) {
		if (std::holds_alternative<ClientActorOutput::ConnectionToServerStateUpdate>(msg.variant)) {
			_connectionToServerState = std::get<ClientActorOutput::ConnectionToServerStateUpdate>(msg.variant);
		}
		return true;
	}, 10);
}
