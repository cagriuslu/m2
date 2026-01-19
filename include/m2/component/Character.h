#pragma once
#include "../Component.h"
#include "../Card.h"
#include <m2/containers/AssociativeList.h>
#include <m2/GameTypes.h>
#include <m2/Proxy.h>
#include <functional>

namespace m2 {
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
		virtual void AddCard(m2g::pb::CardType) = 0;
		virtual void RemoveCard(m2g::pb::CardType) = 0;

		[[nodiscard]] virtual IFE GetVariable(m2g::pb::VariableType) const = 0;
		virtual IFE SetVariable(m2g::pb::VariableType, IFE) = 0;
		virtual void ClearVariable(m2g::pb::VariableType) = 0;

		// Utilities

		IFE SetVariable(const m2g::pb::VariableType vt, const int32_t i) { return SetVariable(vt, IFE{i}); }
		IFE SetVariable(const m2g::pb::VariableType vt, const FE fe) { return SetVariable(vt, IFE{fe}); }
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		constexpr IFE SetVariable(const m2g::pb::VariableType vt, const float value) requires (Enable) { return SetVariable(vt, IFE{FE{value}}); }

		int32_t AddVariable(m2g::pb::VariableType, int32_t value, std::optional<int32_t> maxValue = {});
		FE AddVariable(m2g::pb::VariableType, FE value, std::optional<FE> maxValue = {});
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		constexpr float AddVariable(const m2g::pb::VariableType vt, const float value, const std::optional<float> maxValue = {}) requires (Enable) {
			return AddVariable(vt, FE{value}, maxValue ? std::optional{FE{*maxValue}} : std::optional<FE>{}).ToFloat();
		}

		int32_t SubtractVariable(m2g::pb::VariableType, int32_t value, std::optional<int32_t> minValue = {});
		FE SubtractVariable(m2g::pb::VariableType, FE value, std::optional<FE> minValue = {});
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		constexpr float SubtractVariable(const m2g::pb::VariableType vt, const float value, const std::optional<float> minValue = {}) requires (Enable) {
			return SubtractVariable(vt, FE{value}, minValue ? std::optional{FE{*minValue}} : std::optional<FE>{}).ToFloat();
		}
	};

	// Filters

	constexpr auto HasCardOfType(m2g::pb::CardType it) { return [it](const Character& c) { return c.HasCard(it); }; }

	// Transformers

	inline Object& ToOwnerOfCharacter(const Character& chr) { return chr.Owner(); }
}
