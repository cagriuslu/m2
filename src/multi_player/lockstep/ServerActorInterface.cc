#include <m2/multi_player/lockstep/ServerActorInterface.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

ServerActorInterface::ServerActorInterface(const int maxClientCount) : ActorInterfaceBase(maxClientCount),
	_stateUpdateProcessor([this](const ServerActorOutput& msg) {
		if (std::holds_alternative<ServerActorOutput::ServerStateUpdate>(msg.variant)) {
			this->_serverStateUpdate = std::get<ServerActorOutput::ServerStateUpdate>(msg.variant);
		}
	}) {}

bool ServerActorInterface::IsLobbyOpen() {
	GetActorOutbox().PopMessages(_stateUpdateProcessor);
	return _serverStateUpdate.stateIndex == GetIndexInVariant<ServerActor::LobbyOpen, ServerActor::State>::value;
}
bool ServerActorInterface::IsLobbyFrozenForEveryone() {
	auto question = ServerActorInput{.variant = ServerActorInput::IsAllOutgoingMessagesDelivered{}};
	const auto isResponseInteresting = [](const ServerActorOutput& msg) {
		return std::holds_alternative<ServerActorOutput::IsAllOutgoingMessagesDelivered>(msg.variant);
	};
	bool answer{};
	const auto interestingResponseHandler = [&answer](const ServerActorOutput& msg) {
		if (std::holds_alternative<ServerActorOutput::IsAllOutgoingMessagesDelivered>(msg.variant)) {
			answer = std::get<ServerActorOutput::IsAllOutgoingMessagesDelivered>(msg.variant).answer;
		}
	};
	const auto uninterestingResponseHandler = _stateUpdateProcessor;

	SendQuestionReceiveAnswerSync<ServerActorInput,ServerActorOutput>(GetActorInbox(), std::move(question),
		GetActorOutbox(), isResponseInteresting, interestingResponseHandler, uninterestingResponseHandler);

	return answer;
}

void ServerActorInterface::TryFreezeLobby(const m2g::pb::LockstepGameInitParams& gameInitParams) {
	GetActorInbox().PushMessage(ServerActorInput{.variant = ServerActorInput::FreezeLobby{
		.gameInitParams = gameInitParams
	}});
}
void ServerActorInterface::StoreGameStateHash(const network::Timecode tc, const int32_t hash) {
	GetActorInbox().PushMessage(ServerActorInput{
		.variant = ServerActorInput::GameStateHash{
			.timecode = tc,
			.hash = hash
		}
	});
}
