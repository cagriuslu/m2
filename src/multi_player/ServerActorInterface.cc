#include <m2/multi_player/ServerActorInterface.h>
#include <m2/Log.h>
#include <m2/multi_player/ServerUpdate.h>

namespace {
	constexpr auto MAX_MESSAGES_TO_PROCESS_FROM_ACTOR_PER_CALL = 10;
}

std::optional<m2::pb::NetworkMessage> m2::ServerActorInterface::PopCommandFromTurnHolderEvent() {
	ProcessOutbox();
	if (_clientEvent && std::holds_alternative<ServerActorOutput::ClientEvent::CommandFromTurnHolder>(_clientEvent->eventVariant)) {
		auto tmp = std::move(std::get<ServerActorOutput::ClientEvent::CommandFromTurnHolder>(_clientEvent->eventVariant).turnHolderCommand);
		_clientEvent.reset();

		const auto json = pb::message_to_json_string(tmp);
		LOG_DEBUG("Popping client command", _turnHolderIndex, json->c_str());

		return std::move(tmp);
	}
	return std::nullopt;
}
std::optional<int> m2::ServerActorInterface::PopDisconnectedClientEvent() {
	ProcessOutbox();
	if (_clientEvent && std::holds_alternative<ServerActorOutput::ClientEvent::DisconnectedClient>(_clientEvent->eventVariant)) {
		const auto clientIndex = std::move(std::get<ServerActorOutput::ClientEvent::DisconnectedClient>(_clientEvent->eventVariant).clientIndex);
		_clientEvent.reset();
		return clientIndex;
	}
	return std::nullopt;
}
void m2::ServerActorInterface::TryCloseLobby() {
	GetActorInbox().PushMessage(ServerActorInput{.variant = ServerActorInput::CloseLobby()});
}
void m2::ServerActorInterface::SetTurnHolder(const int clientIndex) {
	LOG_INFO("New turn holder index", clientIndex);
	_turnHolderIndex = clientIndex;
	GetActorInbox().PushMessage(ServerActorInput{.variant = ServerActorInput::UpdateTurnHolder{.clientIndex = clientIndex}});
}
m2::SequenceNo m2::ServerActorInterface::SendServerUpdate(const bool shutdownAfter) {
	INFO_FN();
	auto serverUpdate = GenerateServerUpdate(_nextServerUpdateSequenceNo, _turnHolderIndex, shutdownAfter);
	const auto serverUpdateSequenceNo = serverUpdate.sequence_no();
	GetActorInbox().PushMessage(ServerActorInput{.variant = ServerActorInput::SendServerUpdate{.serverUpdate = std::move(serverUpdate)}});
	return serverUpdateSequenceNo;
}
void m2::ServerActorInterface::SendServerCommand(const m2g::pb::ServerCommand& command, const int receiverIndex) {
	GetActorInbox().PushMessage(ServerActorInput{.variant = ServerActorInput::SendServerCommand{.receiverIndex = receiverIndex, .serverCommand = command}});
}

void m2::ServerActorInterface::ProcessOutbox() {
	// Process up to a certain number of messages
	auto nProcessedMessages = 0;
	std::optional<ServerActorOutput> msg;
	while (nProcessedMessages++ < MAX_MESSAGES_TO_PROCESS_FROM_ACTOR_PER_CALL && GetActorOutbox().PopMessage(msg) && msg) {
		if (std::holds_alternative<ServerActorOutput::StateUpdate>(msg->variant)) {
			_serverActorState = std::get<ServerActorOutput::StateUpdate>(msg->variant);
		} else if (std::holds_alternative<ServerActorOutput::ClientEvent>(msg->variant)) {
			if (_clientEvent) {
				// Do not process messages further until processed
				return;
			}
			_clientEvent = std::get<ServerActorOutput::ClientEvent>(msg->variant);
		}
	}
}
