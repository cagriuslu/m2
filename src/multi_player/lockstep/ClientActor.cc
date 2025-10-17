#include <m2/multi_player/lockstep/ClientActor.h>
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
				_serverConnection->DeliverIncomingMessage(std::move(msg.message));
			}
			_serverConnection->DeliverIncomingMessageToPeer(std::move(msg));
		}
	}

	// Gather outgoing messages from connection managers
	const auto firstEverPlayerInputAvailable = not _unsentPlayerInputs.empty() && not _lastPlayerInputsSentAt;
	auto nextPlayerInputsToSimulate = GetNextPlayerInputsToSimulate();
	const auto timeToSendPlayerInputs = nextPlayerInputsToSimulate && _lastPlayerInputsSentAt && _lastPlayerInputsSentAt->HasTimePassed(M2G_PROXY.lockstepGameTickPeriod);
	if (firstEverPlayerInputAvailable || timeToSendPlayerInputs) {
		const auto timecode = _nextTimecode++;
		if (firstEverPlayerInputAvailable) {
			LOG_DEBUG("First player inputs are available with timecode, sending to peers...", timecode);
		} else {
			LOG_DEBUG("It is time to send inputs to peers with timecode", timecode);
		}
		_serverConnection->QueueOutgoingMessages(timecode, &_unsentPlayerInputs); // Sends to peers as well

		if (firstEverPlayerInputAvailable) {
			LOG_DEBUG("Waiting after first player inputs for main thread to catch up...");
			std::this_thread::sleep_for(std::chrono::seconds{1});
		}

		// If all player inputs are received, and game tick period has passed since the last time the inputs were sent
		// to other peers, we can return the inputs to the client interface for them to be simulated.
		if (timeToSendPlayerInputs) {
			const auto timecodeToSimulate = _nextSelfPlayerInputsToSimulate->first;
			LOG_DEBUG("It is time to simulate player inputs with timecode", timecodeToSimulate);
			outbox.PushMessage(ClientActorOutput{
				.variant = ClientActorOutput::PlayerInputsToSimulate{
					.playerInputs = std::move(*nextPlayerInputsToSimulate)
				}
			});
			_nextSelfPlayerInputsToSimulate.reset();
		}
		_nextSelfPlayerInputsToSimulate = std::make_pair(timecode, std::move(_unsentPlayerInputs));
		_unsentPlayerInputs.clear();
		_lastPlayerInputsSentAt = Stopwatch{};
	} else {
		_serverConnection->QueueOutgoingMessages(std::nullopt, nullptr); // Handles messages that needs to be sent to peers as well
	}

	if (not writeableSockets.empty()) {
		if (const auto success = _messagePasser->SendOutgoingPackets(); not success) {
			LOG_ERROR("Unrecoverable error while sending", success.error());
			return false;
		}
	}

	return true;
}

std::optional<std::vector<std::deque<m2g::pb::LockstepPlayerInput>>> ClientActor::GetNextPlayerInputsToSimulate() const {
	if (_serverConnection && _nextSelfPlayerInputsToSimulate) {
		const auto nextTimecodeToSimulate = _nextSelfPlayerInputsToSimulate->first;
		if (auto peerPlayerInputs = _serverConnection->GetPeerPlayerInputsForTimecode(nextTimecodeToSimulate)) {
			if (const auto selfIndex = _serverConnection->GetSelfIndex()) {
				// Add self player inputs to the vector
				(*peerPlayerInputs)[*selfIndex] = _nextSelfPlayerInputsToSimulate->second;
				return peerPlayerInputs;
			}
		}
	}
	return std::nullopt;
}

void ClientActor::ProcessOneMessageFromInbox(MessageBox<ClientActorInput>& inbox) {
	inbox.PopMessages([this](const ClientActorInput& msg) {
		if (std::holds_alternative<ClientActorInput::SetReadyState>(msg.variant)) {
			const auto& readyState = std::get<ClientActorInput::SetReadyState>(msg.variant).state;
			_serverConnection->SetReadyState(readyState);
		} else if (std::holds_alternative<ClientActorInput::QueuePlayerInput>(msg.variant)) {
			if (not _serverConnection) {
				throw M2_ERROR("Player inputs are received before server connection is established");
			}
			if (not _serverConnection->IsLobbyFrozen() && not _serverConnection->IsGameStarted()) {
				throw M2_ERROR("Player inputs are received in an invalid server connection state");
			}
			_serverConnection->MarkGameAsStarted(); // Mark game as started on this client
			auto& playerInput = std::get<ClientActorInput::QueuePlayerInput>(msg.variant);
			_unsentPlayerInputs.emplace_back(std::move(playerInput.playerInput));
		}
		return true;
	}, 1);
}
