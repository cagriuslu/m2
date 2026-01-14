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

		[[nodiscard]] bool HasCard(m2g::pb::CardType card_type) const;
		[[nodiscard]] bool HasCard(m2g::pb::CardCategory card_cat) const;
		[[nodiscard]] size_t CountCard(m2g::pb::CardType card_type) const;
		[[nodiscard]] size_t CountCard(m2g::pb::CardCategory card_cat) const;
		[[nodiscard]] virtual Iterator FindCards(m2g::pb::CardType card_type) const = 0;
		[[nodiscard]] virtual Iterator FindCards(m2g::pb::CardCategory card_cat) const = 0;
		[[nodiscard]] virtual Iterator BeginCards() const = 0;
		[[nodiscard]] virtual Iterator EndCards() const = 0;
		[[nodiscard]] std::vector<m2g::pb::CardType> NamedCardTypes() const;
		[[nodiscard]] std::vector<m2g::pb::CardType> NamedCardTypes(m2g::pb::CardCategory card_cat) const;
		virtual void AddNamedCard(const Card& card) = 0;
		virtual void AddNamedCardWithoutBenefits(const Card& card) = 0;
		virtual void RemoveCard(const Iterator& card) = 0;
		virtual void ClearCards() = 0;

		[[nodiscard]] virtual bool HasResource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float GetResource(m2g::pb::ResourceType resource_type) const = 0;
		virtual float SetResource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float AddResource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float RemoveResource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual void ClearResource(m2g::pb::ResourceType resource_type) = 0;

		[[nodiscard]] virtual bool HasAttribute(m2g::pb::AttributeType attribute_type) const = 0;
		[[nodiscard]] virtual float GetAttribute(m2g::pb::AttributeType attribute_type) const = 0;

		[[nodiscard]] virtual IFE GetVariable(m2g::pb::VariableType) const = 0;
		virtual IFE SetVariable(m2g::pb::VariableType, IFE) = 0;
		virtual void ClearVariable(m2g::pb::VariableType) = 0;
		virtual void ClearVariables() = 0;
	};

	class CompactCharacter final : public Character {
		const Card* _card{};
		std::pair<m2g::pb::ResourceType, float> _resource;
		std::pair<m2g::pb::AttributeType, float> _attribute;
		std::pair<m2g::pb::VariableType, IFE> _variable;

	public:
		CompactCharacter() = default;
		explicit CompactCharacter(uint64_t object_id);

		[[nodiscard]] int32_t Hash(int32_t initialValue) const;

		[[nodiscard]] Iterator FindCards(m2g::pb::CardType card_type) const override;
		[[nodiscard]] Iterator FindCards(m2g::pb::CardCategory card_cat) const override;
		[[nodiscard]] Iterator BeginCards() const override;
		[[nodiscard]] Iterator EndCards() const override;
		void AddNamedCard(const Card& card) override;
		void AddNamedCardWithoutBenefits(const Card& card) override;
		void RemoveCard(const Iterator& card) override;
		void ClearCards() override;

		[[nodiscard]] bool HasResource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float GetResource(m2g::pb::ResourceType resource_type) const override;
		float SetResource(m2g::pb::ResourceType resource_type, float amount) override;
		float AddResource(m2g::pb::ResourceType resource_type, float amount) override;
		float RemoveResource(m2g::pb::ResourceType resource_type, float amount) override;
		void ClearResource(m2g::pb::ResourceType resource_type) override;

		[[nodiscard]] bool HasAttribute(m2g::pb::AttributeType attribute_type) const override;
		[[nodiscard]] float GetAttribute(m2g::pb::AttributeType attribute_type) const override;

		[[nodiscard]] IFE GetVariable(m2g::pb::VariableType) const override;
		IFE SetVariable(m2g::pb::VariableType, IFE) override;
		void ClearVariable(m2g::pb::VariableType) override;
		void ClearVariables() override;
	};

	class FastCharacter final : public Character {
		std::vector<const Card*> _cards;
		std::vector<float> _resources = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>()); // TODO deprecated
		std::vector<float> _attributes = std::vector<float>(pb::enum_value_count<m2g::pb::AttributeType>()); // TODO deprecated
		std::vector<IFE> _variables = std::vector<IFE>(pb::enum_value_count<m2g::pb::VariableType>());

	public:
		FastCharacter() = default;
		explicit FastCharacter(uint64_t object_id);

		[[nodiscard]] int32_t Hash(int32_t initialValue) const;

		[[nodiscard]] Iterator FindCards(m2g::pb::CardType card_type) const override;
		[[nodiscard]] Iterator FindCards(m2g::pb::CardCategory card_cat) const override;
		[[nodiscard]] Iterator BeginCards() const override;
		[[nodiscard]] Iterator EndCards() const override;
		void AddNamedCard(const Card& card) override;
		void AddNamedCardWithoutBenefits(const Card& card) override;
		void RemoveCard(const Iterator& card) override;
		void ClearCards() override;

		[[nodiscard]] bool HasResource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float GetResource(m2g::pb::ResourceType resource_type) const override;
		float SetResource(m2g::pb::ResourceType resource_type, float amount) override;
		float AddResource(m2g::pb::ResourceType resource_type, float amount) override;
		float RemoveResource(m2g::pb::ResourceType resource_type, float amount) override;
		void ClearResource(m2g::pb::ResourceType resource_type) override;

		[[nodiscard]] bool HasAttribute(m2g::pb::AttributeType attribute_type) const override;
		[[nodiscard]] float GetAttribute(m2g::pb::AttributeType attribute_type) const override;

		[[nodiscard]] IFE GetVariable(const m2g::pb::VariableType v) const override { return _variables[VariableIndex(v)]; }
		IFE SetVariable(const m2g::pb::VariableType v, const IFE ife) override { _variables[VariableIndex(v)] = ife; return ife; }
		void ClearVariable(const m2g::pb::VariableType v) override { _variables[VariableIndex(v)] = {}; }
		void ClearVariables() override { _variables = std::vector<IFE>(pb::enum_value_count<m2g::pb::VariableType>()); }

	private:
		static int ResourceTypeIndex(m2g::pb::ResourceType);
		static int AttributeTypeIndex(m2g::pb::AttributeType);
		static int PropertyTypeIndex(m2g::pb::PropertyType);
		static int VariableIndex(const m2g::pb::VariableType v) { return pb::enum_index(v); }
		friend void FullCharacterIteratorIncrementor(Iterator&);
	};

	using CharacterVariant = std::variant<CompactCharacter,FastCharacter>;

	// Filters
	constexpr auto HasCardOfType(m2g::pb::CardType it) { return [it](const Character& c) { return c.HasCard(it); }; }
	std::function<std::vector<m2g::pb::CardType>(Character&)> GenerateNamedCardTypesFilter(m2g::pb::CardCategory card_category);
	std::function<std::vector<m2g::pb::CardType>(Character&)> GenerateNamedCardTypesFilter(std::initializer_list<m2g::pb::CardCategory> categoriesToFilter);
	// Transformers
	Character& ToCharacterBase(CharacterVariant& v);
	FastCharacter& ToFastCharacter(CharacterVariant& v);
	const FastCharacter& ToFastCharacter(const CharacterVariant& v);
	inline Object& ToOwnerOfCharacter(const Character& chr) { return chr.Owner(); }
}
