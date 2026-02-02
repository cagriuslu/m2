#pragma once
#include "../Component.h"
#include "../Card.h"
#include <m2/containers/AssociativeList.h>
#include <m2/math/IVFE.h>
#include <m2/Proxy.h>
#include <functional>

namespace m2 {
	/// Even though each object has the option to store their data in impl, there are some benefits to storing the data
	/// inside the character: (1) In turn-based games, the character states are shared automatically. (2) In lockstep
	/// games, the synchronization of the clients are compared automatically. (3) Enables automatic saving and loading
	/// of levels.
	class Character : public Component {
	public:
		std::function<void(Character& self, const Stopwatch::Duration& delta)> update;
		std::function<std::unique_ptr<const Proxy::InterCharacterMessage>(Character& self, Character* other, const std::unique_ptr<const Proxy::InterCharacterMessage>& data)> onMessage;

		Character() = default;
		explicit Character(uint64_t object_id);

		std::unique_ptr<const Proxy::InterCharacterMessage> ExecuteInteraction(Character& initiator, std::unique_ptr<const Proxy::InterCharacterMessage>&& data);
		std::unique_ptr<const Proxy::InterCharacterMessage> ExecuteInteraction(std::unique_ptr<const Proxy::InterCharacterMessage>&& data);

		[[nodiscard]] virtual int32_t Hash(int32_t initialValue) const = 0;
		virtual void Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const = 0;
		virtual void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) = 0;

		[[nodiscard]] virtual bool HasCard(m2g::pb::CardType) const = 0;
		[[nodiscard]] virtual bool HasCard(m2g::pb::CardCategory) const = 0;
		[[nodiscard]] virtual size_t CountCards(m2g::pb::CardType) const = 0;
		[[nodiscard]] virtual size_t CountCards(m2g::pb::CardCategory) const = 0;
		[[nodiscard]] virtual std::optional<m2g::pb::CardType> GetFirstCardType(m2g::pb::CardCategory) const = 0;
		virtual expected<void> TryAddCard(m2g::pb::CardType) = 0;
		virtual void UnsafeAddCard(m2g::pb::CardType) = 0;
		virtual void RemoveCard(m2g::pb::CardType) = 0;

		[[nodiscard]] virtual IVFE GetVariable(m2g::pb::VariableType) const = 0;
		virtual expected<IVFE> TrySetVariable(m2g::pb::VariableType, IVFE) = 0;
		virtual IVFE UnsafeSetVariable(m2g::pb::VariableType, IVFE) = 0;
		virtual void ClearVariable(m2g::pb::VariableType) = 0;

		// Utilities

		int32_t UnsafeAddVariable(m2g::pb::VariableType, int32_t value, std::optional<int32_t> maxValue = {});
		FE UnsafeAddVariable(m2g::pb::VariableType, FE value, std::optional<FE> maxValue = {});
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		constexpr float UnsafeAddVariable(const m2g::pb::VariableType vt, const float value, const std::optional<float> maxValue = {}) requires (Enable) {
			return UnsafeAddVariable(vt, FE{value}, maxValue ? std::optional{FE{*maxValue}} : std::optional<FE>{}).ToFloat();
		}

		int32_t UnsafeSubtractVariable(m2g::pb::VariableType, int32_t value, std::optional<int32_t> minValue = {});
		FE UnsafeSubtractVariable(m2g::pb::VariableType, FE value, std::optional<FE> minValue = {});
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		constexpr float UnsafeSubtractVariable(const m2g::pb::VariableType vt, const float value, const std::optional<float> minValue = {}) requires (Enable) {
			return UnsafeSubtractVariable(vt, FE{value}, minValue ? std::optional{FE{*minValue}} : std::optional<FE>{}).ToFloat();
		}
	};

	// Filters

	constexpr auto HasCardOfType(m2g::pb::CardType it) { return [it](const Character& c) { return c.HasCard(it); }; }

	// Transformers

	inline Object& ToOwnerOfCharacter(const Character& chr) { return chr.Owner(); }
	m2g::pb::CardCategory ToCategoryOfCard(m2g::pb::CardType);
}
