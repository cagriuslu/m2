#pragma once
#include "../SmartPointer.h"
#include "../Component.h"
#include "../Item.h"
#include <m2g_Interaction.pb.h>
#include <utility>
#include <vector>
#include <functional>
#include <variant>

namespace m2 {
	namespace internal {
		class ResourceAmount {
			float _amount{};
			float _maxAmount{INFINITY};
		public:
			explicit ResourceAmount(const float amount = 0.0f, const float max_amount = INFINITY) { SetMaxAmount(max_amount); SetAmount(amount); }

			[[nodiscard]] float Amount() const { return _amount; }
			[[nodiscard]] bool HasAmount() const { return 0.0f < _amount; }

			float SetAmount(const float amount) { return _amount = std::clamp(amount, 0.0f, _maxAmount); }
			float AddAmount(const float amount) { return SetAmount(_amount + amount); }
			float RemoveAmount(const float amount) { return SetAmount(_amount - amount); }
			void ClearAmount() { _amount = 0.0f; }

			[[nodiscard]] float MaxAmount() const { return _maxAmount; }
			float SetMaxAmount(float max_amount);
		};
	}

	class Character : public Component {
	public:
		std::function<void(Character& self)> update;
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

			Iterator(const Character& character, Incrementor incrementor, Filter filter, size_t index, const Item* ptr) : _character(character), _incrementor(std::move(incrementor)), _filter(filter), _index(index), _item_ptr(ptr) {}
			Iterator& operator++() { _incrementor(*this); return *this; }
			bool operator==(const Iterator& other) const { return _item_ptr == other._item_ptr; }
			explicit operator bool() const { return _item_ptr; }
			const Item& operator*() const { return *_item_ptr; }
			const Item* operator->() const { return _item_ptr; }

			[[nodiscard]] const Character& Character() const { return _character; }
			[[nodiscard]] Filter GetFilter() const { return _filter; }
			[[nodiscard]] const Item* Get() const { return _item_ptr; }
			void Set(const Item* ptr) { _item_ptr = ptr; }
			[[nodiscard]] size_t GetIndex() const { return _index; }
			void SetIndex(const size_t index) { _index = index; }
		};

		Character() = default;
		explicit Character(uint64_t object_id);

		virtual void AutomaticUpdate() = 0;
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
		virtual void AddUnnamedItem(std::unique_ptr<const UnnamedItem>&& item) = 0; // Item is moved
		virtual void AddNamedItem(const NamedItem& item) = 0;
		virtual void AddNamedItemWithoutBenefits(const NamedItem& item) = 0;
		bool UseItem(const Iterator& item_it, float resource_multiplier = 1.0f);
		virtual void RemoveItem(const Iterator& item) = 0;
		virtual void ClearItems() = 0;

		[[nodiscard]] virtual bool HasResource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float GetResource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float GetMaxResource(m2g::pb::ResourceType resource_type) const = 0;
		virtual void SetMaxResource(m2g::pb::ResourceType resource_type, float max) = 0;
		virtual float SetResource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float AddResource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float RemoveResource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual void ClearResource(m2g::pb::ResourceType resource_type) = 0;
		virtual void ClearResources() = 0;

		[[nodiscard]] virtual bool HasAttribute(m2g::pb::AttributeType attribute_type) const = 0;
		[[nodiscard]] virtual float GetAttribute(m2g::pb::AttributeType attribute_type) const = 0;
		virtual float SetAttribute(m2g::pb::AttributeType attribute_type, float value) = 0;
		virtual void ClearAttribute(m2g::pb::AttributeType attribute_type) = 0;
		virtual void ClearAttributes() = 0;
	};

	/// TinyCharacter can hold only one unnamed item, one named item, and can have only one resource
	class TinyCharacter : public Character {
		SmartPointer<const Item> _item;
		std::pair<m2g::pb::ResourceType, internal::ResourceAmount> _resource;
		std::pair<m2g::pb::AttributeType, float> _attribute;

	public:
		TinyCharacter() = default;
		explicit TinyCharacter(uint64_t object_id);

		void AutomaticUpdate() override;

		[[nodiscard]] Iterator FindItems(m2g::pb::ItemType item_type) const override;
		[[nodiscard]] Iterator FindItems(m2g::pb::ItemCategory item_cat) const override;
		[[nodiscard]] Iterator BeginItems() const override;
		[[nodiscard]] Iterator EndItems() const override;
		void AddUnnamedItem(std::unique_ptr<const UnnamedItem>&& item) override;
		void AddNamedItem(const NamedItem& item) override;
		void AddNamedItemWithoutBenefits(const NamedItem& item) override;
		void RemoveItem(const Iterator& item) override;
		void ClearItems() override;

		[[nodiscard]] bool HasResource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float GetResource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float GetMaxResource(m2g::pb::ResourceType resource_type) const override;
		void SetMaxResource(m2g::pb::ResourceType resource_type, float max) override;
		float SetResource(m2g::pb::ResourceType resource_type, float amount) override;
		float AddResource(m2g::pb::ResourceType resource_type, float amount) override;
		float RemoveResource(m2g::pb::ResourceType resource_type, float amount) override;
		void ClearResource(m2g::pb::ResourceType resource_type) override;
		void ClearResources() override;

		[[nodiscard]] bool HasAttribute(m2g::pb::AttributeType attribute_type) const override;
		[[nodiscard]] float GetAttribute(m2g::pb::AttributeType attribute_type) const override;
		float SetAttribute(m2g::pb::AttributeType attribute_type, float value) override;
		void ClearAttribute(m2g::pb::AttributeType attribute_type) override;
		void ClearAttributes() override;
	};

	/// FullCharacter can hold any number of items, and can have any Resource
	class FullCharacter : public Character {
		std::vector<SmartPointer<const Item>> _items;
		std::vector<internal::ResourceAmount> _resources = std::vector<internal::ResourceAmount>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _attributes = std::vector<float>(pb::enum_value_count<m2g::pb::AttributeType>());

	public:
		FullCharacter() = default;
		explicit FullCharacter(uint64_t object_id);

		void AutomaticUpdate() override;

		[[nodiscard]] Iterator FindItems(m2g::pb::ItemType item_type) const override;
		[[nodiscard]] Iterator FindItems(m2g::pb::ItemCategory item_cat) const override;
		[[nodiscard]] Iterator BeginItems() const override;
		[[nodiscard]] Iterator EndItems() const override;
		void AddUnnamedItem(std::unique_ptr<const UnnamedItem>&& item) override;
		void AddNamedItem(const NamedItem& item) override;
		void AddNamedItemWithoutBenefits(const NamedItem& item) override;
		void RemoveItem(const Iterator& item) override;
		void ClearItems() override;

		[[nodiscard]] bool HasResource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float GetResource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float GetMaxResource(m2g::pb::ResourceType resource_type) const override;
		void SetMaxResource(m2g::pb::ResourceType resource_type, float max) override;
		float SetResource(m2g::pb::ResourceType resource_type, float amount) override;
		float AddResource(m2g::pb::ResourceType resource_type, float amount) override;
		float RemoveResource(m2g::pb::ResourceType resource_type, float amount) override;
		void ClearResource(m2g::pb::ResourceType resource_type) override;
		void ClearResources() override;

		[[nodiscard]] bool HasAttribute(m2g::pb::AttributeType attribute_type) const override;
		[[nodiscard]] float GetAttribute(m2g::pb::AttributeType attribute_type) const override;
		float SetAttribute(m2g::pb::AttributeType attribute_type, float value) override;
		void ClearAttribute(m2g::pb::AttributeType attribute_type) override;
		void ClearAttributes() override;

	private:
		static int ResourceTypeIndex(m2g::pb::ResourceType resource_type);
		static int AttributeTypeIndex(m2g::pb::AttributeType attribute_type);
		friend void FullCharacterIteratorIncrementor(Character::Iterator& it);
	};

	using CharacterVariant = std::variant<TinyCharacter,FullCharacter>;

	// Filters
	constexpr auto HasItemOfType(m2g::pb::ItemType it) { return [it](const Character& c) { return c.HasItem(it); }; }
	std::function<std::vector<m2g::pb::ItemType>(Character&)> GenerateNamedItemTypesFilter(m2g::pb::ItemCategory item_category);
	std::function<std::vector<m2g::pb::ItemType>(Character&)> GenerateNamedItemTypesFilter(std::initializer_list<m2g::pb::ItemCategory> categoriesToFilter);
	// Transformers
	Character& ToCharacterBase(CharacterVariant& v);
	inline Object& ToOwnerOfCharacter(const Character& chr) { return chr.owner(); }
}
