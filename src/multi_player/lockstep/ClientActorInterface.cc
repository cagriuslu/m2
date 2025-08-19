#include <m2/multi_player/lockstep/ClientActorInterface.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

void ClientActorInterface::ProcessOutbox() {
	GetActorOutbox().PopMessages([this](const ClientActorOutput& msg) {
		if (std::holds_alternative<ClientActorOutput::ConnectionToServerStateUpdate>(msg.variant)) {
			_connectionToServerState = std::get<ClientActorOutput::ConnectionToServerStateUpdate>(msg.variant);
		}
		return true;
	}, 10);
}
