#include <unistd.h>
#include <m2/multi_player/lockstep/ServerActor.h>
#include <m2/network/Select.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

ConnectionToClient* ServerActor::ClientList::Find(const network::IpAddressAndPort& address) {
	for (auto& client : _clients) {
		if (client.GetAddressAndPort() == address) {
			return &client;
		}
	}
	return nullptr;
}
ConnectionToClient* ServerActor::ClientList::Add(const network::IpAddressAndPort& address, MessagePasser& msgPasser) {
	_clients.emplace_back(address, msgPasser);
	return &_clients.back();
}

bool ServerActor::Initialize(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>& outbox) {
	LOG_INFO("Lockstep ServerActor Initialize");
	auto expectSocket = network::UdpSocket::CreateServerSideSocket(network::Port::CreateFromHostOrder(1162));
	if (not expectSocket) {
		LOG_WARN("Unable to create a UDP socket", expectSocket.error());
		return false;
	}
	_messagePasser.emplace(std::move(*expectSocket));
	_state.emplace([&outbox](const State& newState) {
		outbox.PushMessage(ServerActorOutput{
			.variant = ServerActorOutput::ServerStateUpdate{
				.stateIndex = newState.index()
			}
		});
	}, State{});
	_state->Emplace(LobbyOpen{});
	return true;
}

bool ServerActor::operator()(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) {
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
		// Process messages
		while (not messages.empty()) {
			const auto msg = std::move(messages.front());
			messages.pop();

			_state->Mutate([this, &msg](State& state) {
				std::visit(overloaded{
					[this, &msg](LobbyOpen& lobby) {
						auto* client = lobby.clientList.Find(msg.sender);
						if (not client) {
							LOG_INFO("Accepting peer to lobby", msg.sender);
							client = lobby.clientList.Add(msg.sender, *_messagePasser);
						}
						if (msg.message.type_case() == pb::LockstepMessage::TYPE_NOT_SET) {}
						else if (msg.message.has_set_ready_state()) {
							LOG_INFO("Setting ready state for peer", msg.sender, msg.message.set_ready_state());
							client->SetReadyState(msg.message.set_ready_state());
						}
						// TODO
					},
					[](const std::monostate&) {},
				}, state);
			});
		}
	}

	// TODO gather and queue outgoing messages
	if (not writeableSockets.empty()) {
		if (const auto success = _messagePasser->SendOutgoingPackets(); not success) {
			LOG_ERROR("Unrecoverable error while sending", success.error());
			return false;
		}
	}

	return true;
}
