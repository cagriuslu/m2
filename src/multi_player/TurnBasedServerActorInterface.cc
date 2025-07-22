#include <m2/multi_player/TurnBasedServerActorInterface.h>
#include <m2/Log.h>
#include <m2/multi_player/ServerUpdate.h>

namespace {
	constexpr auto MAX_MESSAGES_TO_PROCESS_FROM_ACTOR_PER_CALL = 10;
}

std::optional<m2::pb::NetworkMessage> m2::TurnBasedServerActorInterface::PopCommandFromTurnHolderEvent() {
	ProcessOutbox();
	if (_clientEvent && std::holds_alternative<TurnBasedServerActorOutput::ClientEvent::CommandFromTurnHolder>(_clientEvent->eventVariant)) {
		auto tmp = std::move(std::get<TurnBasedServerActorOutput::ClientEvent::CommandFromTurnHolder>(_clientEvent->eventVariant).turnHolderCommand);
		_clientEvent.reset();

		const auto json = pb::message_to_json_string(tmp);
		LOG_DEBUG("Popping client command", _turnHolderIndex, json->c_str());

		return std::move(tmp);
	}
	return std::nullopt;
}
std::optional<int> m2::TurnBasedServerActorInterface::PopDisconnectedClientEvent() {
	ProcessOutbox();
	if (_clientEvent && std::holds_alternative<TurnBasedServerActorOutput::ClientEvent::DisconnectedClient>(_clientEvent->eventVariant)) {
		const auto clientIndex = std::move(std::get<TurnBasedServerActorOutput::ClientEvent::DisconnectedClient>(_clientEvent->eventVariant).clientIndex);
		_clientEvent.reset();
		return clientIndex;
	}
	return std::nullopt;
}
void m2::TurnBasedServerActorInterface::TryCloseLobby() {
	GetActorInbox().PushMessage(TurnBasedServerActorInput{.variant = TurnBasedServerActorInput::CloseLobby()});
}
void m2::TurnBasedServerActorInterface::SetTurnHolder(const int clientIndex) {
	LOG_INFO("New turn holder index", clientIndex);
	_turnHolderIndex = clientIndex;
	GetActorInbox().PushMessage(TurnBasedServerActorInput{.variant = TurnBasedServerActorInput::UpdateTurnHolder{.clientIndex = clientIndex}});
}
m2::SequenceNo m2::TurnBasedServerActorInterface::SendServerUpdate(const bool shutdownAfter) {
	INFO_FN();
	auto serverUpdate = GenerateServerUpdate(_nextServerUpdateSequenceNo, _turnHolderIndex, shutdownAfter);
	const auto serverUpdateSequenceNo = serverUpdate.sequence_no();
	GetActorInbox().PushMessage(TurnBasedServerActorInput{.variant = TurnBasedServerActorInput::SendServerUpdate{.serverUpdate = std::move(serverUpdate)}});
	return serverUpdateSequenceNo;
}
void m2::TurnBasedServerActorInterface::SendServerCommand(const m2g::pb::ServerCommand& command, const int receiverIndex) {
	GetActorInbox().PushMessage(TurnBasedServerActorInput{.variant = TurnBasedServerActorInput::SendServerCommand{.receiverIndex = receiverIndex, .serverCommand = command}});
}

void m2::TurnBasedServerActorInterface::ProcessOutbox() {
	// Process up to a certain number of messages
	auto nProcessedMessages = 0;
	std::optional<TurnBasedServerActorOutput> msg;
	while (nProcessedMessages++ < MAX_MESSAGES_TO_PROCESS_FROM_ACTOR_PER_CALL && GetActorOutbox().PopMessage(msg) && msg) {
		if (std::holds_alternative<TurnBasedServerActorOutput::StateUpdate>(msg->variant)) {
			_serverActorState = std::get<TurnBasedServerActorOutput::StateUpdate>(msg->variant);
		} else if (std::holds_alternative<TurnBasedServerActorOutput::ClientEvent>(msg->variant)) {
			if (_clientEvent) {
				// Do not process messages further until processed
				return;
			}
			_clientEvent = std::get<TurnBasedServerActorOutput::ClientEvent>(msg->variant);
		}
	}
}
