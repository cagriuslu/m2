#include <m2/multi_player/lockstep/ServerActorInterface.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

bool ServerActorInterface::IsLobbyOpen() {
	ProcessOutbox();
	return _serverStateUpdate.stateIndex == GetIndexInVariant<ServerActor::LobbyOpen, ServerActor::State>::value;
}

void ServerActorInterface::TryCloseLobby() {
	GetActorInbox().PushMessage(ServerActorInput{.variant = ServerActorInput::CloseLobby{}});
}

void ServerActorInterface::ProcessOutbox() {
	GetActorOutbox().PopMessages([this](const ServerActorOutput& msg) {
		if (std::holds_alternative<ServerActorOutput::ServerStateUpdate>(msg.variant)) {
			_serverStateUpdate = std::get<ServerActorOutput::ServerStateUpdate>(msg.variant);
		}
		return true;
	}, 10);
}

