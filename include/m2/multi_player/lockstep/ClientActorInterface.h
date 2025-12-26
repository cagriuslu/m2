#pragma once
#include "ClientActor.h"
#include <m2/mt/actor/ActorInterfaceBase.h>
#include <m2g_Lockstep.pb.h>

namespace m2::multiplayer::lockstep {
	class ClientActorInterface final : public ActorInterfaceBase<ClientActor, ClientActorInput, ClientActorOutput> {
		ClientActorOutput::ConnectionToServerStateUpdate _connectionToServerState{};
		std::optional<m2g::pb::LockstepGameInitParams> _gameInitParams;

		struct GameNotStarted {
			bool lastSetReadyState{};
		};
		/// Simulating previously received inputs, the player can queue new inputs to be commited later.
		struct SimulatingInputs {
			int32_t physicsSimulationsCounter{};
			std::deque<m2g::pb::LockstepPlayerInput> selfInputs;
		};
		/// Inputs previously queued by player have been commited, but inputs to simulate haven't been received yet.
		/// It's not possible to queue further inputs. Only graphics and non-impactful events can be handled.
		struct Lagging {};
	public:
		/// Inputs previously queued by the player have been commited, and next inputs to simulate have been received
		/// and should be handled right away. This is a transitional state and the interface never settles on it.
		struct ReadyToSimulate {
			network::Timecode timecode;
			std::vector<std::deque<m2g::pb::LockstepPlayerInput>> allInputs;
		};
	private:
		std::variant<GameNotStarted, SimulatingInputs, Lagging, ReadyToSimulate> _state;

	public:
		explicit ClientActorInterface(const network::IpAddressAndPort serverAddress) : ActorInterfaceBase(serverAddress) {}

		// Accessors

		bool IsSearchingForServer();
		/// \brief Returns true if the client is waiting in the game lobby.
		/// \details Readiness must be set to allow the server to close the lobby and start the game.
		bool IsWaitingInLobby();
		/// \brief Returns true if the lobby is frozen and the level must be built.
		/// \details GetGameInitParams can be used to fetch the parameters to build the level.
		bool IsLobbyFrozen();
		bool IsGameStarted();
		std::optional<int> GetSelfIndex() const { return _connectionToServerState.selfIndex; }
		int GetTotalPlayerCount() const { return _connectionToServerState.totalPlayerCount; }
		bool GetLastSetReadyState() const;
		/// Returns the game initialization parameters, if it's received from the server. Game init params are received
		/// first when the lobby is frozen.
		const m2g::pb::LockstepGameInitParams* GetGameInitParams();

		// Modifiers

		void SetReadyState(bool state);
		/// Starts the game for this instance
		void StartInputStreaming();
		/// Tries to queueu an input to be commited later. Depending on the state of the connection, the interface may
		/// not be willing to accept new input. The return value reflects whether the input was accepted.
		bool TryQueueInput(m2g::pb::LockstepPlayerInput&&);
		struct SkipPhysics {};
		struct SimulatePhysics {};
		using SwapResult = std::variant<SkipPhysics, SimulatePhysics>;
		/// Commit the inputs queued previously from this player to be sent to peers, and fetches the inputs previously
		/// received from peers for simulation.
		SwapResult SwapInputs();
		std::optional<ReadyToSimulate> PopSimulationInputs();
		void StoreGameStateHash(network::Timecode, int32_t);

	private:
		void ProcessOutbox();
	};
}
