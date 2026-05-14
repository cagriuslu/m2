#include <m2/network/discovery/NetworkDiscoveryActor.h>
#include <m2/Options.h>
#include <m2/network/Select.h>
#include <m2/Log.h>
#include <NetworkDiscovery.pb.h>

using namespace m2;
using namespace m2::network;
using namespace m2::network::discovery;

bool NetworkDiscoveryActor::Initialize(MessageBox<NetworkDiscoveryActorInput>&, MessageBox<NetworkDiscoveryActorOutput>&) {
	LOG_INFO("NetworkDiscoveryActor Initialize");

	if (std::holds_alternative<SpeakerParameters>(_role)) {
		auto expectSocket = UdpSocket::CreateSendOnlyMulticastSocket(std::get<SpeakerParameters>(_role).multicastInterface);
		if (not expectSocket) {
			LOG_ERROR("Unable to create a send-only multicast UDP socket", expectSocket.error());
			return false;
		}
		LOG_NETWORK_VERBOSE(std::format("Created UDP socket for sending multicast: groupAddr={} ifAddr={}",
			ToString(std::get<SpeakerParameters>(_role).multicastAddress),
			ToString(std::get<SpeakerParameters>(_role).multicastInterface)));
		_socket = std::move(*expectSocket);
	} else {
		auto expectSocket = UdpSocket::CreateReceiveOnlyMulticastSocket(std::get<ListenerParameters>(_role).multicastAddress.ipAddress, std::get<ListenerParameters>(_role).multicastAddress.port);
		if (not expectSocket) {
			LOG_ERROR("Unable to create a receive-only multicast UDP socket", expectSocket.error());
			return false;
		}
		LOG_NETWORK_VERBOSE(std::format("Created UDP socket for receiving multicast: groupAddr={}",
			ToString(std::get<ListenerParameters>(_role).multicastAddress)));
		_socket = std::move(*expectSocket);
	}

	return true;
}

bool NetworkDiscoveryActor::operator()(MessageBox<NetworkDiscoveryActorInput>&, MessageBox<NetworkDiscoveryActorOutput>& outputBox) {
	if (std::holds_alternative<SpeakerParameters>(_role)) {
		if (_lastHeartbeat.HasTimePassed(std::chrono::seconds(3))) {
			pb::NetworkDiscoveryUdpPacket packet;
			packet.set_game_hash(std::get<SpeakerParameters>(_role).gameHash);
			packet.set_game_port(std::get<SpeakerParameters>(_role).gamePort.GetInNetworkOrder());

			const auto bytes = packet.SerializeAsString();
			const auto success = _socket->Send(std::get<SpeakerParameters>(_role).multicastAddress, bytes.data(), bytes.size());
			if (not success) {
				LOG_ERROR(std::format("Unable to send multicast: groupAddr={} ifAddr={} err={}",
					ToString(std::get<SpeakerParameters>(_role).multicastAddress),
					ToString(std::get<SpeakerParameters>(_role).multicastInterface),
					success.error()));
				return false;
			}
			LOG_NETWORK_VERBOSE(std::format("Sent multicast: groupAddr={} ifAddr={}",
				ToString(std::get<SpeakerParameters>(_role).multicastAddress),
				ToString(std::get<SpeakerParameters>(_role).multicastInterface)));

			_lastHeartbeat.Reset();
		}
	} else {
		auto selectResult = Select::WaitUntilSocketReady(&*_socket, 100);
		m2SucceedOrThrowError(selectResult);
		if (not *selectResult) {
			return true; // Timeout occurred, try again later
		}
		if (not (*selectResult)->readableSockets.empty()) {
			auto recvResult = _socket->Recv(_recvBuffer, sizeof(_recvBuffer));
			m2SucceedOrThrowError(recvResult);

			if (pb::NetworkDiscoveryUdpPacket packet; packet.ParseFromArray(_recvBuffer, recvResult->first)) {
				if (packet.game_hash() == std::get<ListenerParameters>(_role).gameHash) {
					const auto serverIp = recvResult->second.ipAddress;
					const auto gamePort = Port::CreateFromNetworkOrder(packet.game_port());
					const auto speakerKey = IpAddressAndPort{.ipAddress = serverIp, .port = gamePort};

					if (std::ranges::find(_knownSpeakers, speakerKey) == _knownSpeakers.end()) {
						_knownSpeakers.emplace_back(speakerKey);
						LOG_INFO(std::format("Discovered new game server: addr={}", ToString(speakerKey)));
						outputBox.PushMessage(NetworkDiscoveryActorOutput{
							.discoveredPeers = _knownSpeakers
						});
					}
				}
			} else {
				LOG_WARN("Unable to parse received message");
			}
		}
	}

	return true;
}

