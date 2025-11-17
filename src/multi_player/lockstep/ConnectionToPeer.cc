#include <m2/multi_player/lockstep/ConnectionToPeer.h>
#include <m2/math/Hash.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

namespace {
	constexpr int N_RESPONSES_TO_ASSUME_CONNECTION = 3;
}

bool ConnectionToPeer::DoPlayerInputsForTimecodeExist(const network::Timecode tc) const {
	return std::visit(overloaded{
		[tc](const ConnectedToPeer& connection) { return connection._inputs.contains(tc); },
		[](const auto&) { return false; }
	}, _state);
}
std::optional<std::deque<m2g::pb::LockstepPlayerInput>> ConnectionToPeer::GetPlayerInputsForTimecode(const network::Timecode tc) const {
	return std::visit(overloaded{
		[tc](const ConnectedToPeer& connection) -> std::optional<std::deque<m2g::pb::LockstepPlayerInput>> {
			if (const auto it = connection._inputs.find(tc); it != connection._inputs.end()) {
				return std::deque<m2g::pb::LockstepPlayerInput>{it->second.playerInputs.player_inputs().begin(), it->second.playerInputs.player_inputs().end()};
			}
			return std::nullopt;
		},
		[](const auto&) -> std::optional<std::deque<m2g::pb::LockstepPlayerInput>> { return std::nullopt; }
	}, _state);
}

void ConnectionToPeer::QueueOutgoingMessages() {
	const auto queuePing = [this] {
		_messagePasser.QueueMessage(MessageAndReceiver{
			.message = {},
			.receiver = _addressAndPort
		});
	};

	if (std::holds_alternative<SearchForPeer>(_state)) {
		if (const auto* connStats = _messagePasser.GetConnectionStatistics(_addressAndPort); not connStats) {
			LOG_NETWORK("Queueing first ping toward peer", _addressAndPort);
			queuePing();
		} else if (const auto nAckedMsgs = connStats->GetTotalAckedOutgoingSmallMessages(); nAckedMsgs < N_RESPONSES_TO_ASSUME_CONNECTION) {
			// Not enough ping-pongs have been made with the peer. Ping the peer if all previous pings have been ACKed.
			if (connStats->GetTotalQueuedOutgoingSmallMessages() == nAckedMsgs) {
				LOG_NETWORK("Queueing another ping toward peer", _addressAndPort);
				queuePing();
			}
		} else { // nAckedMsgs == N_RESPONSES_TO_ASSUME_CONNECTION
			// Enough ping-pongs have been made with the peer.
			LOG_INFO("Connected to peer", _addressAndPort);
			_state = ConnectedToPeer{};
		}
	}
}
void ConnectionToPeer::QueueOutgoingMessage(pb::LockstepMessage&& msg) {
	_messagePasser.QueueMessage(MessageAndReceiver{
		.message = std::move(msg),
		.receiver = _addressAndPort
	});
}
void ConnectionToPeer::StorePlayerInputsReceivedFrom(const pb::LockstepPlayerInputs& input) {
	if (not std::holds_alternative<ConnectedToPeer>(_state)) {
		throw M2_ERROR("Player inputs received from unconnected peer");
	}
	auto& inputs = std::get<ConnectedToPeer>(_state)._inputs;
	// Calculate hash
	pb::LockstepPlayerInputHashHelper hashHelper;
	if (inputs.empty()) {
		hashHelper.set_prev_hash(0);
	} else {
		hashHelper.set_prev_hash(inputs.rbegin()->second.hash);
	}
	hashHelper.set_ip(_addressAndPort.ipAddress.GetInNetworkOrder());
	hashHelper.set_port(_addressAndPort.port.GetInNetworkOrder());
	hashHelper.mutable_player_inputs()->CopyFrom(input);
	const auto serialized = hashHelper.SerializeAsString();
	const auto hash = HashI(serialized);
	// Store inputs and hash
	inputs.emplace(input.timecode(), PlayerInputsAndHash{
		.playerInputs = input,
		.hash = hash
	});
	LOG_NETWORK("Storing inputs from peer with timecode and hash", _addressAndPort, input.timecode(), hash);
	// Keep the list limited to a capacity
	while (InputCapacity < I(inputs.size())) {
		inputs.erase(inputs.begin());
	}
}
