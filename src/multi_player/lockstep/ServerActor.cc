#include <unistd.h>
#include <m2/multi_player/lockstep/ServerActor.h>
#include <m2/network/Select.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

namespace {
	/// A concept that requires a state to have a clientList of type ClientList
	template <typename StateT>
	concept StateWithClientList = std::same_as<decltype(std::declval<StateT>().clientList), ServerActor::ClientList>;
}

bool ServerActor::ClientList::Contains(const network::IpAddressAndPort& address) const {
	return std::ranges::any_of(_clients, [&address](const ConnectionToClient& client) {
		return client.GetAddressAndPort() == address;
	});
}
const ConnectionToClient* ServerActor::ClientList::Find(const network::IpAddressAndPort& address) const {
	for (const auto& client : _clients) {
		if (client.GetAddressAndPort() == address) {
			return &client;
		}
	}
	return nullptr;
}

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

bool ServerActor::operator()(MessageBox<ServerActorInput>& inbox, MessageBox<ServerActorOutput>& outbox) {
	ProcessOneMessageFromInbox(inbox, outbox);

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
			const auto msg = std::move(messages.front()); messages.pop();
			if (msg.message.type_case() == pb::LockstepMessage::TYPE_NOT_SET) {
				// Empty message received, it is used for sign-up when the lobby is open
				if (std::holds_alternative<LobbyOpen>(_state->Get())) {
					const auto& lobby = std::get<LobbyOpen>(_state->Get());
					if (not lobby.clientList.Contains(msg.sender) && lobby.clientList.Size() < _maxClientCount) {
						LOG_INFO("Accepting peer to lobby", msg.sender);
						_state->Mutate([this, &msg](State& state) {
							std::get<LobbyOpen>(state).clientList.Add(msg.sender, *_messagePasser);
						});
					}
				}
			} else if (msg.message.has_set_ready_state()) {
				// Ready state received, only allowed for known clients when the lobby is open
				if (std::holds_alternative<LobbyOpen>(_state->Get())) {
					const auto& lobby = std::get<LobbyOpen>(_state->Get());
					if (const auto* client = lobby.clientList.Find(msg.sender); client && client->GetReadyState() != msg.message.set_ready_state()) {
						LOG_INFO("Setting ready state for peer", msg.sender, msg.message.set_ready_state());
						_state->Mutate([&msg](State& state) {
							std::get<LobbyOpen>(state).clientList.Find(msg.sender)->SetReadyState(msg.message.set_ready_state());
						});
					}
				}
			} else if (msg.message.has_player_inputs()) {
				if (std::holds_alternative<LobbyFrozen>(_state->Get())) {
					_state->Mutate([](State& state) {
						state = LevelStarted{.clientList = std::move(std::get<LobbyFrozen>(state).clientList)};
					});
				}
				// Calculate a running hash of each player's inputs
				_state->MutateNoSideEffect([&](auto& state) {
					auto& level = std::get<LevelStarted>(state);
					if (auto* client = level.clientList.Find(msg.sender)) {
						client->StoreRunningInputHash(msg.message.player_inputs());
					}
				});
			}
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

void ServerActor::ProcessOneMessageFromInbox(MessageBox<ServerActorInput>& inbox, MessageBox<ServerActorOutput>& outbox) {
	inbox.PopMessages([this, &outbox](const ServerActorInput& msg) {
		if (std::holds_alternative<ServerActorInput::FreezeLobby>(msg.variant)) {
			const auto& lobbyFreezeMsg = std::get<ServerActorInput::FreezeLobby>(msg.variant);
			if (std::holds_alternative<LobbyOpen>(_state->Get())) {
				const auto& lobby = std::get<LobbyOpen>(_state->Get());
				if (std::ranges::all_of(lobby.clientList.cbegin(), lobby.clientList.cend(), [](const ConnectionToClient& client) {
					return client.GetReadyState();
				})) {
					LOG_INFO("Freezing lobby");
					_state->Mutate([&lobbyFreezeMsg](State& state) {
						state = LobbyFrozen{.clientList = std::move(std::get<LobbyOpen>(state).clientList)};
						for (auto& client : std::get<LobbyFrozen>(state).clientList) {
							client.SetLobbyAsFrozen(lobbyFreezeMsg.gameInitParams);
						}
					});
				} else {
					LOG_INFO("Lobby closure requested but not every client is ready");
				}
			} else {
				throw M2_ERROR("Lobby closure requested while lobby isn't open");
			}
		} else if (std::holds_alternative<ServerActorInput::IsAllOutgoingMessagesDelivered>(msg.variant)) {
			const bool answer = std::visit(overloaded {
				[](const StateWithClientList auto& s) {
					return std::all_of(s.clientList.cbegin(), s.clientList.cend(), [](const auto& client) -> bool {
						return client.IsAllOutgoingMessagesDelivered();
					});
				},
				[](const auto&) { return true; }
			}, _state->Get());
			outbox.PushMessage(ServerActorOutput{
				.variant = ServerActorOutput::IsAllOutgoingMessagesDelivered{.answer = answer}
			});
		}
		return true;
	}, 1);
}
