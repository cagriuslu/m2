#pragma once
#include "../Component.h"
#include "../Card.h"
#include <m2/containers/AssociativeList.h>
#include <m2/GameTypes.h>
#include <m2/Proxy.h>
#include <utility>
#include <vector>
#include <functional>
#include <variant>

namespace m2 {
	class Character : public Component {
	public:
		std::function<void(Character& self, const Stopwatch::Duration& delta)> update;
		std::function<std::unique_ptr<const Proxy::InterCharacterMessage>(Character& self, Character* other, const std::unique_ptr<const Proxy::InterCharacterMessage>& data)> onMessage;

		class Iterator {
		public:
			using Incrementor = std::function<void(Iterator&)>;
			using Filter = std::variant<std::monostate,m2g::pb::CardType,m2g::pb::CardCategory>;
		private:
			const Character& _character;
			Incrementor _incrementor;
			Filter _filter;
			size_t _index{}; // Some implementations may use index to aid with iteration
			const Card* _card_ptr;
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = const Card;
			using pointer = const Card*;
			using reference = const Card&;
			using iterator_category = std::forward_iterator_tag;

			Iterator(const Character& character, Incrementor incrementor, const Filter filter, const size_t index, const Card* ptr) : _character(character), _incrementor(std::move(incrementor)), _filter(filter), _index(index), _card_ptr(ptr) {}
			Iterator& operator++() { _incrementor(*this); return *this; }
			bool operator==(const Iterator& other) const { return _card_ptr == other._card_ptr; }
			explicit operator bool() const { return _card_ptr; }
			const Card& operator*() const { return *_card_ptr; }
			const Card* operator->() const { return _card_ptr; }

			[[nodiscard]] const Character& GetCharacter() const { return _character; }
			[[nodiscard]] Filter GetFilter() const { return _filter; }
			[[nodiscard]] const Card* Get() const { return _card_ptr; }
			void Set(const Card* ptr) { _card_ptr = ptr; }
			[[nodiscard]] size_t GetIndex() const { return _index; }
			void SetIndex(const size_t index) { _index = index; }
		};

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
		[[nodiscard]] virtual Iterator FindCards(m2g::pb::CardType card_type) const = 0;
		[[nodiscard]] virtual Iterator FindCards(m2g::pb::CardCategory card_cat) const = 0;
		[[nodiscard]] virtual Iterator EndCards() const = 0;
		virtual void AddCard(const Card& card) = 0;
		virtual void RemoveCard(const Iterator& card) = 0;

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

	class CompactCharacter final : public Character {
		const Card* _card{};
		std::pair<m2g::pb::VariableType, IFE> _variable;

	public:
		CompactCharacter() = default;
		explicit CompactCharacter(uint64_t object_id);

		[[nodiscard]] int32_t Hash(int32_t initialValue) const override;
		void Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const override;
		void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) override;

		[[nodiscard]] bool HasCard(m2g::pb::CardType) const override;
		[[nodiscard]] bool HasCard(m2g::pb::CardCategory) const override;
		[[nodiscard]] size_t CountCards(m2g::pb::CardType) const override;
		[[nodiscard]] size_t CountCards(m2g::pb::CardCategory) const override;
		[[nodiscard]] Iterator FindCards(m2g::pb::CardType card_type) const override;
		[[nodiscard]] Iterator FindCards(m2g::pb::CardCategory card_cat) const override;
		[[nodiscard]] Iterator EndCards() const override;
		void AddCard(const Card& card) override;
		void RemoveCard(const Iterator& card) override;

		[[nodiscard]] IFE GetVariable(m2g::pb::VariableType) const override;
		IFE SetVariable(m2g::pb::VariableType, IFE) override;
		void ClearVariable(m2g::pb::VariableType) override;
	};

	class FastCharacter final : public Character {
		std::vector<const Card*> _cards;
		std::vector<IFE> _variables = std::vector<IFE>(pb::enum_value_count<m2g::pb::VariableType>());

	public:
		FastCharacter() = default;
		explicit FastCharacter(uint64_t object_id);

		[[nodiscard]] int32_t Hash(int32_t initialValue) const override;
		void Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const override;
		void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) override;

		[[nodiscard]] bool HasCard(m2g::pb::CardType) const override;
		[[nodiscard]] bool HasCard(m2g::pb::CardCategory) const override;
		[[nodiscard]] size_t CountCards(m2g::pb::CardType) const override;
		[[nodiscard]] size_t CountCards(m2g::pb::CardCategory) const override;
		[[nodiscard]] Iterator FindCards(m2g::pb::CardType card_type) const override;
		[[nodiscard]] Iterator FindCards(m2g::pb::CardCategory card_cat) const override;
		[[nodiscard]] Iterator EndCards() const override;
		void AddCard(const Card& card) override;
		void RemoveCard(const Iterator& card) override;

		[[nodiscard]] IFE GetVariable(const m2g::pb::VariableType v) const override { return _variables[VariableIndex(v)]; }
		IFE SetVariable(const m2g::pb::VariableType v, const IFE ife) override { _variables[VariableIndex(v)] = ife; return ife; }
		void ClearVariable(const m2g::pb::VariableType v) override { _variables[VariableIndex(v)] = {}; }

		// Utilities

		[[nodiscard]] std::vector<m2g::pb::CardType> GetCardTypes() const;
		[[nodiscard]] std::vector<m2g::pb::CardType> GetCardTypes(m2g::pb::CardCategory) const;

	private:
		static int VariableIndex(const m2g::pb::VariableType v) { return pb::enum_index(v); }
		friend void FullCharacterIteratorIncrementor(Iterator&);
	};

	using CharacterVariant = std::variant<CompactCharacter,FastCharacter>;

	// Filters

	constexpr auto HasCardOfType(m2g::pb::CardType it) { return [it](const Character& c) { return c.HasCard(it); }; }

	// Transformers

	Character& ToCharacterBase(CharacterVariant& v);
	FastCharacter& ToFastCharacter(CharacterVariant& v);
	const FastCharacter& ToFastCharacter(const CharacterVariant& v);
	inline Object& ToOwnerOfCharacter(const Character& chr) { return chr.Owner(); }
}
