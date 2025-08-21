#include <m2/multi_player/lockstep/ClientActor.h>
#include <m2/network/UdpSocket.h>
#include <m2/Log.h>
#include <thread>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

bool ClientActor::Initialize(MessageBox<ClientActorInput>&, MessageBox<ClientActorOutput>& outbox) {
	LOG_INFO("Lockstep ClientActor Initialize");
	auto expectSocket = network::UdpSocket::CreateClientSideSocket();
	if (not expectSocket) {
		LOG_ERROR("Unable to create a UDP socket", expectSocket.error());
		return false;
	}
	_messagePasser.emplace(std::move(*expectSocket));
	_serverConnection.emplace(_serverAddressAndPort, *_messagePasser, outbox);
	return true;
}

bool ClientActor::operator()(MessageBox<ClientActorInput>& inbox, MessageBox<ClientActorOutput>&) {
	ProcessOneMessageFromInbox(inbox);

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

	_serverConnection->QueueOutgoingMessages();

	if (not writeableSockets.empty()) {
		if (const auto success = _messagePasser->SendOutgoingPackets(); not success) {
			LOG_ERROR("Unrecoverable error while sending", success.error());
			return false;
		}
	}

	return true;
}

void ClientActor::ProcessOneMessageFromInbox(MessageBox<ClientActorInput>& inbox) {
	inbox.PopMessages([this](const ClientActorInput& msg) {
		if (std::holds_alternative<ClientActorInput::SetReadyState>(msg.variant)) {
			const auto& readyState = std::get<ClientActorInput::SetReadyState>(msg.variant).state;
			_serverConnection->SetReadyState(readyState);
		}
		return true;
	}, 1);
}
