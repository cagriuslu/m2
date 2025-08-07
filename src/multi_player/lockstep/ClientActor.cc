#include <m2/multi_player/lockstep/ClientActor.h>
#include <m2/network/UdpSocket.h>
#include <m2/Log.h>
#include <thread>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

bool ClientActor::Initialize(MessageBox<ClientActorInput>&, MessageBox<ClientActorOutput>&) {
	LOG_INFO("Lockstep ClientActor Initialize");
	auto expectSocket = network::UdpSocket::CreateClientSideSocket();
	if (not expectSocket) {
		LOG_ERROR("Unable to create a UDP socket", expectSocket.error());
		return false;
	}
	_messagePasser.emplace(std::move(*expectSocket));
	_serverConnection.emplace();
	return true;
}

bool ClientActor::operator()(MessageBox<ClientActorInput>&, MessageBox<ClientActorOutput>&) {
	auto selectResult = network::Select::WaitUntilSocketReady(&_messagePasser->GetSocket(), 50);
	m2SucceedOrThrowError(selectResult);
	if (not *selectResult) {
		return true; // Timeout occurred, try again later
	}
	const auto& [readableSockets, writeableSockets] = **selectResult;
	if (not readableSockets.empty()) {
		// TODO read messages
	}
	if (not writeableSockets.empty()) {
		std::queue<pb::LockstepMessage> outgoingMessages;
		_serverConnection->GatherOutgoingMessages(_messagePasser->GetConnectionStatistics(_serverAddressAndPort), outgoingMessages);
		while (not outgoingMessages.empty()) {
			_messagePasser->SendMessage(MessageAndReceiver{
				.message = std::move(outgoingMessages.front()),
				.receiver = _serverAddressAndPort
			});
			outgoingMessages.pop();
		}
		// TODO gather other outgoing messages
	}

	return true;
}
