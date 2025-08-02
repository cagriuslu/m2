#pragma once
#include "TurnBasedServerActor.h"
#include "Type.h"
#include <m2/network/SequenceNo.h>
#include <m2/mt/actor/ActorInterfaceBase.h>
#include <m2/Meta.h>

namespace m2 {
	class TurnBasedServerActorInterface final : public ActorInterfaceBase<TurnBasedServerActor, TurnBasedServerActorInput, TurnBasedServerActorOutput> {
		SequenceNo _nextServerUpdateSequenceNo{};
		int _turnHolderIndex{};
		TurnBasedServerActorOutput::StateUpdate _serverActorState{};
		std::optional<TurnBasedServerActorOutput::ClientEvent> _clientEvent;

	public:
		TurnBasedServerActorInterface(const int maxConnectionCount): ActorInterfaceBase(maxConnectionCount) {}

		// Accessors
		
		/// Returns true if the server is still listening for external connections.
		bool IsListening() { ProcessOutbox(); return _serverActorState.threadState == pb::SERVER_LOBBY_OPEN; }
		bool IsLobbyClosed() { ProcessOutbox(); return _serverActorState.threadState == pb::SERVER_LOBBY_CLOSED; }
		int GetClientCount() { ProcessOutbox(); return _serverActorState.clientCount; }
		int GetReadyClientCount() { ProcessOutbox(); return _serverActorState.readyClientCount; }
		int GetTurnHolderIndex() const { return _turnHolderIndex; }
		bool IsOurTurn() const { return GetTurnHolderIndex() == 0; }
		bool HasBeenShutdown() { ProcessOutbox(); return _serverActorState.threadState == pb::SERVER_GAME_FINISHED; }

		// Modifiers

		std::optional<pb::TurnBasedNetworkMessage> PopCommandFromTurnHolderEvent();
		std::optional<int> PopDisconnectedClientEvent();
		/// Tries to close the lobby. May fail if not all clients have indicated that they are ready. The status can be
		/// polled using the IsLobbyClosed method.
		void TryCloseLobby();
		void SetTurnHolder(int clientIndex);
		/// Returns the sequence number of the sent TurnBasedServerUpdate
		SequenceNo SendServerUpdate(bool shutdownAfter = false);
		void SendServerCommand(const m2g::pb::TurnBasedServerCommand& command, int receiverIndex = -1);

	private:
		void ProcessOutbox();
	};
}
