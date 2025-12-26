#pragma once
#include "ClientActorInputOutput.h"
#include "ConnectionToServer.h"
#include "MessagePasser.h"
#include <m2/mt/actor/ActorBase.h>
#include <m2/network/Select.h>

namespace m2::multiplayer::lockstep {
	class ClientActor final : ActorBase<ClientActorInput,ClientActorOutput> {
		const network::IpAddressAndPort _serverAddressAndPort;
		std::optional<MessagePasser> _messagePasser;

		// Connections

		std::optional<ConnectionToServer> _serverConnection;
		// Player inputs from this instance are collected here, to be sent to every peer each tick.
		std::optional<std::deque<m2g::pb::LockstepPlayerInput>> _unsentThisPlayerInputs;
		// Last timepoint where the previous player inputs are sent to peers.
		std::optional<Stopwatch> _lastPlayerInputsSentAt;
		network::Timecode _nextTimecode{};
		// Player inputs from this instance that are already sent to peers are stored here until inputs from all other
		// peers are received.
		std::optional<std::pair<network::Timecode,std::deque<m2g::pb::LockstepPlayerInput>>> _nextSelfPlayerInputsToSimulate;
		std::optional<ClientActorInput::GameStateHash> _lastReceivedGameStateHash;

	public:
		explicit ClientActor(const network::IpAddressAndPort serverAddress) : ActorBase(), _serverAddressAndPort(serverAddress) {}
		~ClientActor() override = default;

		[[nodiscard]] const char* ThreadNameForLogging() const override { return "CL"; }

		bool Initialize(MessageBox<ClientActorInput>&, MessageBox<ClientActorOutput>&) override;

		bool operator()(MessageBox<ClientActorInput>&, MessageBox<ClientActorOutput>&) override;

		void Deinitialize(MessageBox<ClientActorInput>&, MessageBox<ClientActorOutput>&) override {}

	private:
		bool HasNextPlayerInputsToSimulate() const;
		std::optional<std::vector<std::deque<m2g::pb::LockstepPlayerInput>>> GetNextPlayerInputsToSimulate() const;

		void ProcessOneMessageFromInbox(MessageBox<ClientActorInput>&);
	};
}
