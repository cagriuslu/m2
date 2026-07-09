#include <m2/multiplayer/lockstep/ClientActor.h>
#include <m2/network/UdpSocket.h>
#include <m2/Game.h>
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

bool ClientActor::operator()(MessageBox<ClientActorInput>& inbox, MessageBox<ClientActorOutput>& outbox) {
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
		// Process messages
		while (not messages.empty()) {
			auto msg = std::move(messages.front()); messages.pop();
			if (msg.sender == _serverConnection->GetAddressAndPort()) {
				if (const auto status = _serverConnection->DeliverIncomingMessage(std::move(msg.message));
						status == ConnectionToServer::Status::STOP) {
					return false;
				}
			} else {
				_serverConnection->DeliverIncomingMessageToPeer(std::move(msg));
			}
		}
	}

	if (not _lastPlayerInputsSentAt && _unsentThisPlayerInputs) {
		// If input streaming hasn't yet started and inputs from this player have been committed, mark game as started on this client.
		_serverConnection->MarkGameAsStarted();
		if (_nextTimecode != 0) {
			throw M2_ERROR("First timecode should have been zero");
		}
		++_nextTimecode;

		if (not _unsentThisPlayerInputs->empty()) {
			// First set of inputs are sent automatically by the game engine to signal the beginning of the simulation.
			// It shouldn't contain a meaningful input.
			throw M2_ERROR("First set of player inputs should have been empty");
		}
		LOG_NETWORK("First player inputs are available, sending to peers...");
		_serverConnection->QueueOutgoingMessages(0, &*_unsentThisPlayerInputs, _lastReceivedGameStateHash, _unsentThisPlayerRngSeed);

		// Even though the first set of player inputs are empty, we still simulate it. Store them until the next tick.
		// Stored timecode becomes the expected timecode for inputs of the peers.
		_nextSelfPlayerInputsToSimulate = std::make_tuple(0, std::move(*_unsentThisPlayerInputs), std::nullopt);
		_unsentThisPlayerInputs.reset();
		_lastPlayerInputsSentAt = Stopwatch{};
	} else if (HasNextPlayerInputsToSimulate() && _unsentThisPlayerInputs) {
		// Input streaming has started, inputs from all peers have been received, and inputs from this player has been
		// committed. This signifies that previous inputs have been simulated by the game loop completely.
		const auto timecode = _nextTimecode++;
		LOG_NETWORK("It is time to send inputs to peers with timecode", timecode);
		_serverConnection->QueueOutgoingMessages(timecode, &*_unsentThisPlayerInputs, _lastReceivedGameStateHash, _unsentThisPlayerRngSeed);

		auto nextPlayerInputsToSimulate = GetNextPlayerInputsToSimulate(); // Available only if the inputs from every player, including the self, is available
		const auto timecodeToSimulate = std::get<network::Timecode>(*_nextSelfPlayerInputsToSimulate);
		LOG_NETWORK("It is time to simulate player inputs with timecode", timecodeToSimulate);
		outbox.PushMessage(ClientActorOutput{
			.variant = ClientActorOutput::PlayerInputsToSimulate{
				.timecode = timecodeToSimulate,
				.playerInputs = std::move(*nextPlayerInputsToSimulate)
			}
		});

		// Store this player inputs until the next tick. Stored timecode becomes the expected timecode for inputs of the peers.
		_nextSelfPlayerInputsToSimulate = std::make_tuple(timecode, std::move(*_unsentThisPlayerInputs), std::move(*_unsentThisPlayerRngSeed));
		_unsentThisPlayerInputs.reset();
		_unsentThisPlayerRngSeed.reset();
		_lastPlayerInputsSentAt = Stopwatch{};
	} else {
		_serverConnection->QueueOutgoingMessages(std::nullopt, nullptr, _lastReceivedGameStateHash, _unsentThisPlayerRngSeed); // Do regular housekeeping
	}

	if (not writeableSockets.empty()) {
		if (const auto success = _messagePasser->SendOutgoingPackets(); not success) {
			LOG_ERROR("Unrecoverable error while sending", success.error());
			return false;
		}
	}

	return true;
}

bool ClientActor::HasNextPlayerInputsToSimulate() const {
	if (_serverConnection && _nextSelfPlayerInputsToSimulate) {
		const auto nextTimecodeToSimulate = std::get<network::Timecode>(*_nextSelfPlayerInputsToSimulate);
		return _serverConnection->HasAllPeerInputsForTimecode(nextTimecodeToSimulate);
	}
	return false;
}
std::optional<std::vector<std::pair<std::deque<m2g::pb::LockstepPlayerInput>, uint64_t>>> ClientActor::GetNextPlayerInputsToSimulate() const {
	if (_serverConnection && _nextSelfPlayerInputsToSimulate) {
		const auto nextTimecodeToSimulate = std::get<network::Timecode>(*_nextSelfPlayerInputsToSimulate);
		if (auto peerPlayerInputs = _serverConnection->GetPeerPlayerInputsForTimecode(nextTimecodeToSimulate)) {
			if (const auto selfIndex = _serverConnection->GetSelfIndex()) {
				// Add self player inputs to the vector
				(*peerPlayerInputs)[*selfIndex].first = std::get<std::deque<m2g::pb::LockstepPlayerInput>>(*_nextSelfPlayerInputsToSimulate);
				if (const auto rngSeed = std::get<std::optional<uint64_t>>(*_nextSelfPlayerInputsToSimulate)) {
					(*peerPlayerInputs)[*selfIndex].second = *rngSeed;
				}
				return peerPlayerInputs;
			}
		}
	}
	return std::nullopt;
}

void ClientActor::ProcessOneMessageFromInbox(MessageBox<ClientActorInput>& inbox) {
	inbox.TryHandleMessages([this](ClientActorInput& msg) {
		if (std::holds_alternative<ClientActorInput::SetReadyState>(msg.variant)) {
			const auto& readyState = std::get<ClientActorInput::SetReadyState>(msg.variant).state;
			_serverConnection->SetReadyState(readyState);
		} else if (std::holds_alternative<ClientActorInput::QueueThisPlayerInput>(msg.variant)) {
			if (not _serverConnection) {
				throw M2_ERROR("Player inputs are received before server connection is established");
			}
			if (not _serverConnection->IsLobbyFrozen() && not _serverConnection->IsGameStarted()) {
				throw M2_ERROR("Player inputs are received in an invalid server connection state");
			}
			if (_unsentThisPlayerInputs) {
				throw M2_ERROR("Inputs from this player from previous tick are not sent yet");
			}
			if (_unsentThisPlayerRngSeed) {
				throw M2_ERROR("RNG seed from this player from previous tick are not sent yet");
			}
			LOG_NETWORK("Received committed inputs from the game loop");
			auto& playerInput = std::get<ClientActorInput::QueueThisPlayerInput>(msg.variant);
			_unsentThisPlayerInputs = std::move(playerInput.inputs);
			_unsentThisPlayerRngSeed = std::move(playerInput.rngSeed);
		} else if (std::holds_alternative<ClientActorInput::GameStateHash>(msg.variant)) {
			if (_lastReceivedGameStateHash) {
				throw M2_ERROR("Previously received game state hash wasn't used");
			}
			_lastReceivedGameStateHash = std::get<ClientActorInput::GameStateHash>(msg.variant);
		}
		return true;
	}, 1);
}
