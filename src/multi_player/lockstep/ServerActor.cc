#include <unistd.h>
#include <m2/multi_player/lockstep/ServerActor.h>
#include <m2/multi_player/lockstep/ConnectionToServer.h>
#include <m2/network/Select.h>
#include <m2/Log.h>
#include <thread>

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
std::optional<int> ServerActor::ClientList::FindIndexOf(const network::IpAddressAndPort& address) const {
	for (int i = 0; i < I(_clients.size()); ++i) {
		if (_clients[i].GetAddressAndPort() == address) {
			return i;
		}
	}
	return std::nullopt;
}
const ConnectionToClient* ServerActor::ClientList::At(const int index) const {
	if (index < Size()) {
		return &_clients[index];
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
ConnectionToClient* ServerActor::ClientList::At(const int index) {
	if (index < Size()) {
		return &_clients[index];
	}
	return nullptr;
}
ConnectionToClient* ServerActor::ClientList::Add(const network::IpAddressAndPort& address, MessagePasser& msgPasser) {
	_clients.emplace_back(address, msgPasser);
	// Prepare peer details
	pb::LockstepPeerDetails peerDetails;
	for (const auto& client : _clients) {
		auto* peer = peerDetails.add_peers();
		peer->set_ip(client.GetAddressAndPort().ipAddress.GetInNetworkOrder());
		peer->set_port(client.GetAddressAndPort().port.GetInNetworkOrder());
	}
	// Publish details
	for (int i = 0; i < I(_clients.size()); ++i) {
		peerDetails.set_receiver_index(i);
		_clients[i].PublishPeerDetails(peerDetails);
	}
	return &_clients.back();
}

bool ServerActor::StateValidationState::IsAllSucceeded() const {
	return std::ranges::all_of(validationResults, Is(ValidationResult::SUCCESS));
}
bool ServerActor::StateValidationState::IsClientSucceeded(const int index) const {
	if (index < I(validationResults.size())) {
		return validationResults[index] == ValidationResult::SUCCESS;
	}
	throw M2_ERROR("Client index out of bounds");
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
		// Process all messages
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
			} else if (msg.message.all_peers_reachable()) {
				// Peer reachability received, only allowed for known clients when the lobby is open
				if (std::holds_alternative<LobbyOpen>(_state->Get())) {
					const auto& lobby = std::get<LobbyOpen>(_state->Get());
					if (lobby.clientList.Find(msg.sender)) {
						LOG_INFO("Marking peer as reachable to every other peer", msg.sender);
						_state->Mutate([&msg](State& state) {
							std::get<LobbyOpen>(state).clientList.Find(msg.sender)->MarkAsReachableToAllPeers();
						});
					}
				}
			} else if (msg.message.has_player_inputs()) {
				if (std::holds_alternative<LobbyFrozen>(_state->Get())) {
					_state->Mutate([&](State& state) {
						// Only if the message is received from a known client
						if (const auto& lobby = std::get<LobbyFrozen>(state); lobby.clientList.Find(msg.sender)) {
							state = LevelStarted{.clientList = std::move(std::get<LobbyFrozen>(state).clientList)};
						}
					});
				}
				if (std::holds_alternative<LevelStarted>(_state->Get())) {
					_state->MutateNoSideEffect([&](auto& state) {
						auto& levelStarted = std::get<LevelStarted>(state);
						// Only if the message is received from a known client
						if (auto* client = levelStarted.clientList.Find(msg.sender)) {
							// Calculate a running hash of each player's inputs
							client->StoreRunningInputHash(msg.message.player_inputs());

							// The timecode of the received message
							if (const auto timecode = msg.message.player_inputs().timecode(); levelStarted.currentStateValidation) {
								// The server is expecting state reports for the following timecode
								const auto currentValidationTimecode = levelStarted.currentStateValidation->expectedGameStateHash.timecode;
								// The following is the timecode of the next validation
								const auto nextValidationTimecode = currentValidationTimecode + ConnectionToServer::GAME_STATE_REPORT_PERIOD_IN_TICKS;
								// State reports are submitted with one "cycle" delay. That means, the state report for
								// `currentValidationTimecode` should be sent between the messages with timecode
								// `nextValidationTimecode` and `nextValidationTimecode + 1`. Thus, if a message with
								// timecode `nextValidationTimecode + 1` is received from a client, the state report of
								// that client for `currentValidationTimecode` should have been submitted already.
								if (nextValidationTimecode < timecode
										&& not levelStarted.currentStateValidation->IsClientSucceeded(*levelStarted.clientList.FindIndexOf(msg.sender))) {
									// This client has failed to fullfil the state validateion
									levelStarted.clientList.Find(msg.sender)->SetFaultOrCheatDetected();
								}
							}
						}
					});
				}
			} else if (msg.message.has_state_report()) {
				if (std::holds_alternative<LevelStarted>(_state->Get())) {
					_state->MutateNoSideEffect([&](auto& state) {
						auto& levelStarted = std::get<LevelStarted>(state);
						// Only if the message is received from a known client
						if (auto* client = levelStarted.clientList.Find(msg.sender)) {
							LOG_INFO("Received state report from client for timecode with game state hash and input hashes",
								msg.sender, msg.message.state_report().timecode(), msg.message.state_report().game_state_hash(),
								std::vector<int32_t>{msg.message.state_report().player_input_hashes().begin(),
									msg.message.state_report().player_input_hashes().end()});
							if (not levelStarted.currentStateValidation) {
								throw M2_ERROR("Unable to find state validation details in time");
							}
							const auto& stateReport = msg.message.state_report();
							auto& currentStateValidation = *levelStarted.currentStateValidation;
							if (stateReport.timecode() != currentStateValidation.expectedGameStateHash.timecode) {
								client->SetFaultOrCheatDetected();
								return;
							}
							if (stateReport.game_state_hash() != currentStateValidation.expectedGameStateHash.hash) {
								client->SetFaultOrCheatDetected();
								return;
							}
							if (stateReport.player_input_hashes_size() != levelStarted.clientList.Size()) {
								client->SetFaultOrCheatDetected();
								return;
							}
							const auto senderIndex = *levelStarted.clientList.FindIndexOf(msg.sender);
							for (int i = 0; i < stateReport.player_input_hashes_size(); ++i) {
								if (i == senderIndex) {
									if (stateReport.player_input_hashes(i) != 0) {
										client->SetFaultOrCheatDetected();
										return;
									}
								} else {
									const auto expectedInputHash = levelStarted.clientList.At(i)->GetInputHash(stateReport.timecode());
									if (not expectedInputHash) {
										throw M2_ERROR("Unable to find the input hash of client during validation");
									}
									if (stateReport.player_input_hashes(i) != *expectedInputHash) {
										levelStarted.clientList.At(i)->SetFaultOrCheatDetected();
									}
								}
							}
							currentStateValidation.validationResults[senderIndex] = StateValidationState::ValidationResult::SUCCESS;
							LOG_NETWORK("State validation is successful for client", msg.sender);
						}
					});
				}
			}
		}
	}

	// Retire the current state validation is necessary
	if (const auto* levelStarted = std::get_if<LevelStarted>(&_state->Get()); levelStarted
			&& levelStarted->currentStateValidation && levelStarted->currentStateValidation->IsAllSucceeded()
			&& levelStarted->nextStateValidation) {
		_state->MutateNoSideEffect([&](auto& state) {
			auto& mutLevelStarted = std::get<LevelStarted>(state);
			LOG_NETWORK("Validation of timecode is completed successfully", mutLevelStarted.currentStateValidation->expectedGameStateHash.timecode);
			mutLevelStarted.currentStateValidation = StateValidationState{std::move(*mutLevelStarted.nextStateValidation), mutLevelStarted.clientList.Size()};
			mutLevelStarted.nextStateValidation.reset();
		});
	}

	// Check faulty clients
	const auto gameEndReport = [&]() -> std::optional<pb::LockstepGameEndReport> {
		if (std::holds_alternative<LevelStarted>(_state->Get())) {
			const auto& levelStarted = std::get<LevelStarted>(_state->Get());
			pb::LockstepGameEndReport ger;
			for (const auto& client : levelStarted.clientList) {
				ger.add_faulty_or_cheater_player(client.IsFaultOrCheatDetected());
			}
			if (std::ranges::any_of(ger.faulty_or_cheater_player(), [](const auto& b) { return b; })) {
				return ger;
			}
		}
		return std::nullopt;
	}();
	if (gameEndReport) {
		LOG_INFO("Queueing game end report to clients");
		_state->MutateNoSideEffect([&](auto& state) {
			for (auto& client : std::get<LevelStarted>(state).clientList) {
				client.EndGame(*gameEndReport);
			}
		});
		return false;
	}

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
				if (std::ranges::all_of(lobby.clientList.begin(), lobby.clientList.end(), [](const ConnectionToClient& client) {
					return client.GetReadyState() && client.GetIfAllPeersReachable();
				})) {
					LOG_INFO("Freezing lobby");
					_state->Mutate([&lobbyFreezeMsg](State& state) {
						state = LobbyFrozen{.clientList = std::move(std::get<LobbyOpen>(state).clientList)};
						for (auto& client : std::get<LobbyFrozen>(state).clientList) {
							client.SetLobbyAsFrozen(lobbyFreezeMsg.gameInitParams);
						}
					});
				} else {
					LOG_INFO("Lobby closure requested but not every client is ready or connected to each other");
				}
			} else {
				throw M2_ERROR("Lobby closure requested while lobby isn't open");
			}
		} else if (std::holds_alternative<ServerActorInput::IsAllOutgoingMessagesDelivered>(msg.variant)) {
			const bool answer = std::visit(overloaded {
				[](const StateWithClientList auto& s) {
					return std::all_of(s.clientList.begin(), s.clientList.end(), [](const auto& client) -> bool {
						return client.IsAllOutgoingMessagesDelivered();
					});
				},
				[](const auto&) { return true; }
			}, _state->Get());
			outbox.PushMessage(ServerActorOutput{
				.variant = ServerActorOutput::IsAllOutgoingMessagesDelivered{.answer = answer}
			});
		} else if (std::holds_alternative<ServerActorInput::GameStateHash>(msg.variant)) {
			if (std::holds_alternative<LevelStarted>(_state->Get())) {
				_state->MutateNoSideEffect([&](State& state) {
					auto& levelStarted = std::get<LevelStarted>(state);
					if (levelStarted.nextStateValidation) {
						throw M2_ERROR("A new game state hash is received before the previous one is consumed");
					}
					levelStarted.nextStateValidation = std::get<ServerActorInput::GameStateHash>(msg.variant);
					LOG_NETWORK("Storing game state hash for future verification", levelStarted.nextStateValidation->timecode, levelStarted.nextStateValidation->hash);
					if (not levelStarted.currentStateValidation) {
						// If this is the first state validation
						levelStarted.currentStateValidation = StateValidationState{
							ServerActorInput::GameStateHash{*levelStarted.nextStateValidation},
							levelStarted.clientList.Size()};
						levelStarted.nextStateValidation.reset();
					}
				});
			} else {
				throw M2_ERROR("Game state hash is received while the level isn't started");
			}
		}
		return true;
	}, 1);
}
