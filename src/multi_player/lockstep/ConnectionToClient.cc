#include <m2/multi_player/lockstep/ConnectionToClient.h>
#include <m2/math/Hash.h>
#include <m2/ProxyHelper.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

ConnectionToClient::ConnectionToClient(network::IpAddressAndPort address, MessagePasser& messagePasser)
	: _addressAndPort(std::move(address)), _messagePasser(messagePasser) {}

void ConnectionToClient::PublishPeerDetails(const pb::LockstepPeerDetails& details) {
	pb::LockstepMessage msg;
	msg.mutable_peer_details()->CopyFrom(details);
	LOG_NETWORK("Queueing peer details to client", _addressAndPort);
	QueueOutgoingMessage(std::move(msg));
	// Clear peer reachability state
	_allPeersReachable = false;
}
void ConnectionToClient::SetLobbyAsFrozen(const m2g::pb::LockstepGameInitParams& gameInitParams) {
	pb::LockstepMessage msg;
	msg.mutable_freeze_lobby_with_init_params()->CopyFrom(gameInitParams);
	LOG_NETWORK("Queueing lobby freeze message to client", _addressAndPort);
	QueueOutgoingMessage(std::move(msg));
}
void ConnectionToClient::StoreRunningInputHash(const pb::LockstepPlayerInputs& playerInputs) {
	pb::LockstepPlayerInputHashHelper hashHelper;
	if (_runningInputHash.empty()) {
		hashHelper.set_prev_hash(0);
	} else {
		hashHelper.set_prev_hash(_runningInputHash.back().hash);
	}
	hashHelper.set_ip(_addressAndPort.ipAddress.GetInNetworkOrder());
	hashHelper.set_port(_addressAndPort.port.GetInNetworkOrder());
	hashHelper.mutable_player_inputs()->CopyFrom(playerInputs);
	const auto serialized = hashHelper.SerializeAsString();
	const auto hash = HashI(serialized);
	_runningInputHash.emplace_back(Hash{.timecode = playerInputs.timecode(), .hash = hash});
	LOG_NETWORK("Calculated client player inputs hash", _addressAndPort, _runningInputHash.back().timecode, _runningInputHash.back().hash);
	// Keep the list limited to a capacity
	while (RunningInputHashCapacity < I(_runningInputHash.size())) {
		_runningInputHash.pop_front();
	}
}

void ConnectionToClient::QueueOutgoingMessage(pb::LockstepMessage&& msg) {
	_messagePasser.QueueMessage(MessageAndReceiver{
		.message = std::move(msg),
		.receiver = _addressAndPort
	});
}
