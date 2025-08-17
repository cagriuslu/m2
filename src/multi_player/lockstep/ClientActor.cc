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
		std::queue<MessageAndSender> messages;
		if (const auto success = _messagePasser->ReadMessages(messages); not success) {
			LOG_ERROR("Unrecoverable error while reading", success.error());
			return false;
		}
		// TODO process messages
	}

	GatherAndQueueOutgoingMessages();
	if (not writeableSockets.empty()) {
		if (const auto success = _messagePasser->SendOutgoingPackets(); not success) {
			LOG_ERROR("Unrecoverable error while sending", success.error());
			return false;
		}
	}

	return true;
}

void ClientActor::GatherAndQueueOutgoingMessages() {
	const auto flushOperationGenerator = [this](const network::IpAddressAndPort& address) {
		return [this, address](pb::LockstepMessage&& msg) {
			_messagePasser->QueueMessage(MessageAndReceiver{
				.message = std::move(msg),
				.receiver = address
			});
		};
	};

	std::queue<pb::LockstepMessage> outgoingMessages;
	// Server connection
	_serverConnection->GatherOutgoingMessages(_messagePasser->GetConnectionStatistics(_serverAddressAndPort), outgoingMessages);
	Flush(outgoingMessages, flushOperationGenerator(_serverAddressAndPort));
	// Peer connections
	// TODO gather messages to peers
}
