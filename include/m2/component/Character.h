#pragma once
#include "../Component.h"
#include "../Item.h"
#include <m2/containers/AssociativeList.h>
#include <m2/GameTypes.h>
#include <m2g_Interaction.pb.h>
#include <utility>
#include <vector>
#include <functional>
#include <variant>

namespace m2 {
	class Character : public Component {
	public:
		std::function<void(Character& self, const Stopwatch::Duration& delta)> update;
		std::function<std::optional<m2g::pb::InteractionData>(Character& self, Character* other, const m2g::pb::InteractionData& data)> on_interaction;

		class Iterator {
		public:
			using Incrementor = std::function<void(Iterator&)>;
			using Filter = std::variant<std::monostate,m2g::pb::ItemType,m2g::pb::ItemCategory>;
		private:
			const Character& _character;
			Incrementor _incrementor;
			Filter _filter;
			size_t _index{}; // Some implementations may use index to aid with iteration
			const Item* _item_ptr;
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = const Item;
			using pointer = const Item*;
			using reference = const Item&;
			using iterator_category = std::forward_iterator_tag;

			Iterator(const Character& character, Incrementor incrementor, const Filter filter, const size_t index, const Item* ptr) : _character(character), _incrementor(std::move(incrementor)), _filter(filter), _index(index), _item_ptr(ptr) {}
			Iterator& operator++() { _incrementor(*this); return *this; }
			bool operator==(const Iterator& other) const { return _item_ptr == other._item_ptr; }
			explicit operator bool() const { return _item_ptr; }
			const Item& operator*() const { return *_item_ptr; }
			const Item* operator->() const { return _item_ptr; }

			[[nodiscard]] const Character& GetCharacter() const { return _character; }
			[[nodiscard]] Filter GetFilter() const { return _filter; }
			[[nodiscard]] const Item* Get() const { return _item_ptr; }
			void Set(const Item* ptr) { _item_ptr = ptr; }
			[[nodiscard]] size_t GetIndex() const { return _index; }
			void SetIndex(const size_t index) { _index = index; }
		};

		Character() = default;
		explicit Character(uint64_t object_id);

		std::optional<m2g::pb::InteractionData> ExecuteInteraction(Character& initiator, const m2g::pb::InteractionData& data);
		std::optional<m2g::pb::InteractionData> ExecuteInteraction(const m2g::pb::InteractionData& data);

		[[nodiscard]] bool HasItem(m2g::pb::ItemType item_type) const;
		[[nodiscard]] bool HasItem(m2g::pb::ItemCategory item_cat) const;
		[[nodiscard]] size_t CountItem(m2g::pb::ItemType item_type) const;
		[[nodiscard]] size_t CountItem(m2g::pb::ItemCategory item_cat) const;
		[[nodiscard]] virtual Iterator FindItems(m2g::pb::ItemType item_type) const = 0;
		[[nodiscard]] virtual Iterator FindItems(m2g::pb::ItemCategory item_cat) const = 0;
		[[nodiscard]] virtual Iterator BeginItems() const = 0;
		[[nodiscard]] virtual Iterator EndItems() const = 0;
		[[nodiscard]] std::vector<m2g::pb::ItemType> NamedItemTypes() const;
		[[nodiscard]] std::vector<m2g::pb::ItemType> NamedItemTypes(m2g::pb::ItemCategory item_cat) const;
		virtual void AddNamedItem(const Item& item) = 0;
		virtual void AddNamedItemWithoutBenefits(const Item& item) = 0;
		virtual void RemoveItem(const Iterator& item) = 0;
		virtual void ClearItems() = 0;

		[[nodiscard]] virtual bool HasResource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float GetResource(m2g::pb::ResourceType resource_type) const = 0;
		virtual float SetResource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float AddResource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float RemoveResource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual void ClearResource(m2g::pb::ResourceType resource_type) = 0;

		[[nodiscard]] virtual bool HasAttribute(m2g::pb::AttributeType attribute_type) const = 0;
		[[nodiscard]] virtual float GetAttribute(m2g::pb::AttributeType attribute_type) const = 0;

		[[nodiscard]] virtual IFE GetProperty(m2g::pb::PropertyType) const = 0;
		virtual void SetProperty(m2g::pb::PropertyType, int32_t) = 0;
		virtual void SetProperty(m2g::pb::PropertyType, FE) = 0;

		[[nodiscard]] virtual IFE GetVariable(m2g::pb::VariableType) const = 0;
		virtual IFE SetVariable(m2g::pb::VariableType, IFE) = 0;
		virtual void ClearVariable(m2g::pb::VariableType) = 0;
		virtual void ClearVariables() = 0;
	};

	class CompactCharacter final : public Character {
		const Item* _item{};
		std::pair<m2g::pb::ResourceType, float> _resource;
		std::pair<m2g::pb::AttributeType, float> _attribute;
		std::pair<m2g::pb::VariableType, IFE> _variable;

	public:
		CompactCharacter() = default;
		explicit CompactCharacter(uint64_t object_id);

		[[nodiscard]] int32_t Hash(int32_t initialValue) const;

		[[nodiscard]] Iterator FindItems(m2g::pb::ItemType item_type) const override;
		[[nodiscard]] Iterator FindItems(m2g::pb::ItemCategory item_cat) const override;
		[[nodiscard]] Iterator BeginItems() const override;
		[[nodiscard]] Iterator EndItems() const override;
		void AddNamedItem(const Item& item) override;
		void AddNamedItemWithoutBenefits(const Item& item) override;
		void RemoveItem(const Iterator& item) override;
		void ClearItems() override;

		[[nodiscard]] bool HasResource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float GetResource(m2g::pb::ResourceType resource_type) const override;
		float SetResource(m2g::pb::ResourceType resource_type, float amount) override;
		float AddResource(m2g::pb::ResourceType resource_type, float amount) override;
		float RemoveResource(m2g::pb::ResourceType resource_type, float amount) override;
		void ClearResource(m2g::pb::ResourceType resource_type) override;

		[[nodiscard]] bool HasAttribute(m2g::pb::AttributeType attribute_type) const override;
		[[nodiscard]] float GetAttribute(m2g::pb::AttributeType attribute_type) const override;

		[[nodiscard]] IFE GetProperty(m2g::pb::PropertyType) const override { throw M2_ERROR("CompactCharacter doesn't support properties"); }
		void SetProperty(m2g::pb::PropertyType, int32_t) override { throw M2_ERROR("CompactCharacter doesn't support properties"); }
		void SetProperty(m2g::pb::PropertyType, FE) override { throw M2_ERROR("CompactCharacter doesn't support properties"); }

		[[nodiscard]] IFE GetVariable(m2g::pb::VariableType) const override;
		IFE SetVariable(m2g::pb::VariableType, IFE) override;
		void ClearVariable(m2g::pb::VariableType) override;
		void ClearVariables() override;
	};

	class FastCharacter final : public Character {
		std::vector<const Item*> _items;
		std::vector<float> _resources = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>()); // TODO deprecated
		std::vector<float> _attributes = std::vector<float>(pb::enum_value_count<m2g::pb::AttributeType>()); // TODO deprecated
		std::vector<IFE> _properties = std::vector<IFE>(pb::enum_value_count<m2g::pb::PropertyType>());
		std::vector<IFE> _variables = std::vector<IFE>(pb::enum_value_count<m2g::pb::VariableType>());

	public:
		FastCharacter() = default;
		explicit FastCharacter(uint64_t object_id);

		[[nodiscard]] int32_t Hash(int32_t initialValue) const;

		[[nodiscard]] Iterator FindItems(m2g::pb::ItemType item_type) const override;
		[[nodiscard]] Iterator FindItems(m2g::pb::ItemCategory item_cat) const override;
		[[nodiscard]] Iterator BeginItems() const override;
		[[nodiscard]] Iterator EndItems() const override;
		void AddNamedItem(const Item& item) override;
		void AddNamedItemWithoutBenefits(const Item& item) override;
		void RemoveItem(const Iterator& item) override;
		void ClearItems() override;

		[[nodiscard]] bool HasResource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float GetResource(m2g::pb::ResourceType resource_type) const override;
		float SetResource(m2g::pb::ResourceType resource_type, float amount) override;
		float AddResource(m2g::pb::ResourceType resource_type, float amount) override;
		float RemoveResource(m2g::pb::ResourceType resource_type, float amount) override;
		void ClearResource(m2g::pb::ResourceType resource_type) override;

		[[nodiscard]] bool HasAttribute(m2g::pb::AttributeType attribute_type) const override;
		[[nodiscard]] float GetAttribute(m2g::pb::AttributeType attribute_type) const override;

		[[nodiscard]] bool HasProperty(const m2g::pb::PropertyType pt) const { return static_cast<bool>(_properties[PropertyTypeIndex(pt)]); }
		[[nodiscard]] IFE GetProperty(const m2g::pb::PropertyType pt) const override { return _properties[PropertyTypeIndex(pt)]; }
		void SetProperty(const m2g::pb::PropertyType pt, const int32_t value) override { _properties[PropertyTypeIndex(pt)] = IFE{value}; }
		void SetProperty(const m2g::pb::PropertyType pt, FE value) override { _properties[PropertyTypeIndex(pt)] = IFE{value}; }
		void SetProperty(const m2g::pb::PropertyType pt, IFE value) { _properties[PropertyTypeIndex(pt)] = value; }
		void ClearProperty(const m2g::pb::PropertyType pt) { _properties[PropertyTypeIndex(pt)] = {}; }
		void ClearProperties() { _properties = std::vector<IFE>(pb::enum_value_count<m2g::pb::PropertyType>()); }

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
	constexpr auto HasItemOfType(m2g::pb::ItemType it) { return [it](const Character& c) { return c.HasItem(it); }; }
	std::function<std::vector<m2g::pb::ItemType>(Character&)> GenerateNamedItemTypesFilter(m2g::pb::ItemCategory item_category);
	std::function<std::vector<m2g::pb::ItemType>(Character&)> GenerateNamedItemTypesFilter(std::initializer_list<m2g::pb::ItemCategory> categoriesToFilter);
	// Transformers
	Character& ToCharacterBase(CharacterVariant& v);
	FastCharacter& ToFastCharacter(CharacterVariant& v);
	const FastCharacter& ToFastCharacter(const CharacterVariant& v);
	inline Object& ToOwnerOfCharacter(const Character& chr) { return chr.Owner(); }
}
