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

			std::visit(overloaded{
				[this, &msg](const LobbyOpen& lo) {
					auto* client = FindClient(msg.sender);
					if (not client) {
						LOG_INFO("received msg from an unknown source");
						// TODO
					} else {
						// TODO
					}
				},
				[](const std::monostate&) {},
			}, _state->Get());
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

ConnectionToClient* ServerActor::FindClient(const network::IpAddressAndPort& address) {
	return std::visit(overloaded {
		[&address](const StateWithClients auto& s) -> ConnectionToClient* {
			for (auto& clientAndAddress : s.clients) {
				if (clientAndAddress.address == address) {
					return clientAndAddress.client.get();
				}
			}
			return nullptr;
		},
		[](const auto&) -> ConnectionToClient* { return nullptr; }
	}, _state->Get());
}
