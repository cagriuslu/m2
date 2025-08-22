#include <m2/multi_player/lockstep/ConnectionToClient.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

ConnectionToClient::ConnectionToClient(network::IpAddressAndPort address, MessagePasser& messagePasser)
	: _addressAndPort(std::move(address)), _messagePasser(messagePasser) {}

void ConnectionToClient::SetLobbyAsClosed() {
	pb::LockstepMessage msg;
	msg.mutable_lobby_closed();
	LOG_INFO("Queueing lobby closure message to client", _addressAndPort);
	QueueOutgoingMessage(std::move(msg));
}

void ConnectionToClient::QueueOutgoingMessage(pb::LockstepMessage&& msg) {
	_messagePasser.QueueMessage(MessageAndReceiver{
		.message = std::move(msg),
		.receiver = _addressAndPort
	});
}
