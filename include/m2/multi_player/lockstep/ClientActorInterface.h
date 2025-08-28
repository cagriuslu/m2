#pragma once
#include "ClientActor.h"
#include <m2/mt/actor/ActorInterfaceBase.h>
#include <m2g_Lockstep.pb.h>

namespace m2::multiplayer::lockstep {
	class ClientActorInterface final : public ActorInterfaceBase<ClientActor, ClientActorInput, ClientActorOutput> {
		ClientActorOutput::ConnectionToServerStateUpdate _connectionToServerState{};
		std::optional<m2g::pb::LockstepGameInitParams> _gameInitParams;
		std::optional<std::deque<m2g::pb::LockstepPlayerInput>> _readyToSimulatePlayersInputs;

	public:
		explicit ClientActorInterface(network::IpAddressAndPort serverAddress) : ActorInterfaceBase(std::move(serverAddress)) {}

		// Accessors

		bool IsSearchingForServer();
		/// \brief Returns true if the client is waiting in the game lobby.
		/// \details Readiness must be set to allow the server to close the lobby and start the game.
		bool IsWaitingInLobby();
		/// \brief Returns true if the lobby is frozen and the level must be built.
		/// \details GetGameInitParams can be used to build the level.
		bool IsLobbyFrozen();
		/// Returns the game initialization parameters, if it's received from the server. Game init params are received
		/// first when the lobby is frozen.
		const m2g::pb::LockstepGameInitParams* GetGameInitParams();

		// Modifiers

		void SetReadyState(bool state);
		void QueuePlayerInput(m2g::pb::LockstepPlayerInput&&);
		void PopReadyToSimulatePlayerInputs(std::optional<std::deque<m2g::pb::LockstepPlayerInput>>& out);

	private:
		void ProcessOutbox();
	};
}
