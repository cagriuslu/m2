#pragma once
#include "ConnectionToClient.h"
#include "ServerActorInputOutput.h"
#include <m2/mt/actor/ActorBase.h>
#include <m2/multi_player/lockstep/MessagePasser.h>
#include <m2/ManagedObject.h>
#include <vector>
#include <optional>

namespace m2::multiplayer::lockstep {
	class ServerActor final : ActorBase<ServerActorInput,ServerActorOutput> {
	public:
		class ClientList {
			std::vector<ConnectionToClient> _clients;
		public:
			// Accessors

			auto begin() { return _clients.begin(); }
			auto end() { return _clients.end(); }
			[[nodiscard]] auto begin() const { return _clients.cbegin(); }
			[[nodiscard]] auto end() const { return _clients.cend(); }
			[[nodiscard]] bool Contains(const network::IpAddressAndPort&) const;
			[[nodiscard]] int Size() const { return I(_clients.size()); }
			[[nodiscard]] const ConnectionToClient* Find(const network::IpAddressAndPort&) const;
			[[nodiscard]] std::optional<int> FindIndexOf(const network::IpAddressAndPort&) const;
			[[nodiscard]] const ConnectionToClient* At(int index) const;

			// Modifiers

			ConnectionToClient* Find(const network::IpAddressAndPort&);
			ConnectionToClient* At(int index);
			ConnectionToClient* Add(const network::IpAddressAndPort&, MessagePasser&);
		};

		struct StateValidationState {
			enum class ValidationResult {
				NOT_YET_RECEIVED = 0,
				SUCCESS,
			};

			ServerActorInput::GameStateHash expectedGameStateHash;
			std::vector<ValidationResult> validationResults;

			StateValidationState(ServerActorInput::GameStateHash&& nextExpected, const int clientCount) : expectedGameStateHash(nextExpected), validationResults(clientCount) {}
			[[nodiscard]] bool IsAllSucceeded() const;
			[[nodiscard]] bool IsClientSucceeded(int index) const;
		};

		struct LobbyOpen {
			ClientList clientList;
		};
		struct LobbyFrozen {
			ClientList clientList;
		};
		struct LevelStarted {
			ClientList clientList;
			std::optional<StateValidationState> currentStateValidation{};
			std::optional<ServerActorInput::GameStateHash> nextStateValidation{};
		};
		using State = std::variant<std::monostate, LobbyOpen, LobbyFrozen, LevelStarted>;

	private:
		const int _maxClientCount;
		std::optional<MessagePasser> _messagePasser;
		std::optional<ManagedObject<State>> _state;

	public:
		explicit ServerActor(const int maxClientCount) : ActorBase(), _maxClientCount(maxClientCount) {}
		~ServerActor() override = default;

		[[nodiscard]] const char* ThreadNameForLogging() const override { return "SR"; }

		bool Initialize(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override;

		bool operator()(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override;

		void Deinitialize(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override {}

	private:
		void ProcessOneMessageFromInbox(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&);
	};
}
