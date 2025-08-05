#include <unistd.h>
#include <m2/multi_player/lockstep/ServerActor.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

namespace {
	template <typename StateT>
	concept StateWithClients = requires(StateT s) {
		s.clients;
	};
}

bool ServerActor::Initialize(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) {
	LOG_INFO("Lockstep ServerActor Initialize");
	auto expectSocket = network::UdpSocket::CreateServerSideSocket(network::Port::CreateFromHostOrder(1162));
	if (not expectSocket) {
		LOG_WARN("Unable to create a UDP socket", expectSocket.error());
		return false;
	}
	_messagePasser.emplace(std::move(*expectSocket));
	return true;
}

bool ServerActor::operator()(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) {
	auto selectResult = network::Select::WaitUntilSocketReady(&_messagePasser->GetSocket(), 50);
	m2SucceedOrThrowError(selectResult);
	if (not *selectResult) {
		return true; // Timeout occurred, try again later
	}
	const auto& [readableSockets, writeableSockets] = **selectResult;
	PullIncomingMessages(readableSockets);
	ProcessOneReceivedMessage();

	return true;
}

void ServerActor::PullIncomingMessages(const network::SocketHandles<network::UdpSocket>& readableSockets) {
	if (not readableSockets.empty()) {
		LOG_DEBUG("Socket is readable");
		const auto readResult = _messagePasser->ReadMessages(_receivedMessages);
		// TODO handle result
	}
}
void ServerActor::ProcessOneReceivedMessage() {
	if (_receivedMessages.empty()) {
		return;
	}
	const auto msg = std::move(_receivedMessages.front());
	_receivedMessages.pop();

	std::visit(overloaded{
		[this, &msg](LobbyOpen& lo) {
			auto* client = FindClient(msg.sender);
			if (not client) {
				LOG_INFO("received msg from an unknown source");
				// TODO
			} else {
				// TODO
			}
		},
		[](const std::monostate&) {},
	}, _state);
}

ConnectionToClient* ServerActor::FindClient(const network::IpAddressAndPort& address) {
	return std::visit(overloaded {
		[&address](StateWithClients auto& s) -> ConnectionToClient* {
			for (auto& clientAndAddress : s.clients) {
				if (clientAndAddress.address == address) {
					return clientAndAddress.client.get();
				}
			}
			return nullptr;
		},
		[](auto&) -> ConnectionToClient* { return nullptr; }
	}, _state);
}
