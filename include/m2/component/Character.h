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
			float _max_amount{INFINITY};
		public:
			explicit ResourceAmount(float amount = 0.0f, float max_amount = INFINITY) { set_max_amount(max_amount); set_amount(amount); }

			[[nodiscard]] float amount() const { return _amount; }
			[[nodiscard]] bool has_amount() const { return 0.0f < _amount; }

			float set_amount(float amount) { return _amount = std::clamp(amount, 0.0f, _max_amount); }
			float add_amount(float amount) { return set_amount(_amount + amount); }
			float remove_amount(float amount) { return set_amount(_amount - amount); }
			void clear_amount() { _amount = 0.0f; }

			[[nodiscard]] float max_amount() const { return _max_amount; }
			float set_max_amount(float max_amount);
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

			[[nodiscard]] const Character& character() const { return _character; }
			[[nodiscard]] Filter get_filter() const { return _filter; }
			[[nodiscard]] const Item* get() const { return _item_ptr; }
			void set(const Item* ptr) { _item_ptr = ptr; }
			[[nodiscard]] size_t get_index() const { return _index; }
			void set_index(size_t index) { _index = index; }
		};

		Character() = default;
		explicit Character(uint64_t object_id);

		virtual void automatic_update() = 0;
		std::optional<m2g::pb::InteractionData> execute_interaction(Character& initiator, const m2g::pb::InteractionData& data);
		std::optional<m2g::pb::InteractionData> execute_interaction(const m2g::pb::InteractionData& data);

		[[nodiscard]] bool has_item(m2g::pb::ItemType item_type) const;
		[[nodiscard]] bool has_item(m2g::pb::ItemCategory item_cat) const;
		[[nodiscard]] size_t count_item(m2g::pb::ItemType item_type) const;
		[[nodiscard]] size_t count_item(m2g::pb::ItemCategory item_cat) const;
		[[nodiscard]] virtual Iterator find_items(m2g::pb::ItemType item_type) const = 0;
		[[nodiscard]] virtual Iterator find_items(m2g::pb::ItemCategory item_cat) const = 0;
		[[nodiscard]] virtual Iterator begin_items() const = 0;
		[[nodiscard]] virtual Iterator end_items() const = 0;
		[[nodiscard]] std::vector<m2g::pb::ItemType> named_item_types() const;
		[[nodiscard]] std::vector<m2g::pb::ItemType> named_item_types(m2g::pb::ItemCategory item_cat) const;
		virtual void add_unnamed_item(std::unique_ptr<const UnnamedItem>&& item) = 0; // Item is moved
		virtual void add_named_item(const NamedItem& item) = 0;
		virtual void add_named_item_no_benefits(const NamedItem& item) = 0;
		bool use_item(const Iterator& item_it, float resource_multiplier = 1.0f);
		virtual void remove_item(const Iterator& item) = 0;
		virtual void clear_items() = 0;

		[[nodiscard]] virtual bool has_resource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float get_resource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float get_max_resource(m2g::pb::ResourceType resource_type) const = 0;
		virtual void set_max_resource(m2g::pb::ResourceType resource_type, float max) = 0;
		virtual float set_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float add_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float remove_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual void clear_resource(m2g::pb::ResourceType resource_type) = 0;
		virtual void clear_resources() = 0;

		[[nodiscard]] virtual bool has_attribute(m2g::pb::AttributeType attribute_type) const = 0;
		[[nodiscard]] virtual float get_attribute(m2g::pb::AttributeType attribute_type) const = 0;
		virtual float set_attribute(m2g::pb::AttributeType attribute_type, float value) = 0;
		virtual void clear_attribute(m2g::pb::AttributeType attribute_type) = 0;
		virtual void clear_attributes() = 0;
	};

	/// TinyCharacter can hold only one unnamed item, one named item, and can have only one resource
	class TinyCharacter : public Character {
		SmartPointer<const Item> _item;
		std::pair<m2g::pb::ResourceType, internal::ResourceAmount> _resource;
		std::pair<m2g::pb::AttributeType, float> _attribute;

	public:
		TinyCharacter() = default;
		explicit TinyCharacter(uint64_t object_id);

		void automatic_update() override;

		[[nodiscard]] Iterator find_items(m2g::pb::ItemType item_type) const override;
		[[nodiscard]] Iterator find_items(m2g::pb::ItemCategory item_cat) const override;
		[[nodiscard]] Iterator begin_items() const override;
		[[nodiscard]] Iterator end_items() const override;
		void add_unnamed_item(std::unique_ptr<const UnnamedItem>&& item) override;
		void add_named_item(const NamedItem& item) override;
		void add_named_item_no_benefits(const NamedItem& item) override;
		void remove_item(const Iterator& item) override;
		void clear_items() override;

		[[nodiscard]] bool has_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_max_resource(m2g::pb::ResourceType resource_type) const override;
		void set_max_resource(m2g::pb::ResourceType resource_type, float max) override;
		float set_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		void clear_resource(m2g::pb::ResourceType resource_type) override;
		void clear_resources() override;

		[[nodiscard]] bool has_attribute(m2g::pb::AttributeType attribute_type) const override;
		[[nodiscard]] float get_attribute(m2g::pb::AttributeType attribute_type) const override;
		float set_attribute(m2g::pb::AttributeType attribute_type, float value) override;
		void clear_attribute(m2g::pb::AttributeType attribute_type) override;
		void clear_attributes() override;
	};

	/// FullCharacter can hold any number of items, and can have any Resource
	class FullCharacter : public Character {
		std::vector<SmartPointer<const Item>> _items;
		std::vector<internal::ResourceAmount> _resources = std::vector<internal::ResourceAmount>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _attributes = std::vector<float>(pb::enum_value_count<m2g::pb::AttributeType>());

	public:
		FullCharacter() = default;
		explicit FullCharacter(uint64_t object_id);

		void automatic_update() override;

		[[nodiscard]] Iterator find_items(m2g::pb::ItemType item_type) const override;
		[[nodiscard]] Iterator find_items(m2g::pb::ItemCategory item_cat) const override;
		[[nodiscard]] Iterator begin_items() const override;
		[[nodiscard]] Iterator end_items() const override;
		void add_unnamed_item(std::unique_ptr<const UnnamedItem>&& item) override;
		void add_named_item(const NamedItem& item) override;
		void add_named_item_no_benefits(const NamedItem& item) override;
		void remove_item(const Iterator& item) override;
		void clear_items() override;

		[[nodiscard]] bool has_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_max_resource(m2g::pb::ResourceType resource_type) const override;
		void set_max_resource(m2g::pb::ResourceType resource_type, float max) override;
		float set_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		void clear_resource(m2g::pb::ResourceType resource_type) override;
		void clear_resources() override;

		[[nodiscard]] bool has_attribute(m2g::pb::AttributeType attribute_type) const override;
		[[nodiscard]] float get_attribute(m2g::pb::AttributeType attribute_type) const override;
		float set_attribute(m2g::pb::AttributeType attribute_type, float value) override;
		void clear_attribute(m2g::pb::AttributeType attribute_type) override;
		void clear_attributes() override;

	private:
		static int resource_type_index(m2g::pb::ResourceType resource_type);
		static int attribute_type_index(m2g::pb::AttributeType attribute_type);
		friend void full_character_iterator_incrementor(Character::Iterator& it);
	};

	using CharacterVariant = std::variant<TinyCharacter,FullCharacter>;

	// Filters
	constexpr auto has_item_of_type(m2g::pb::ItemType it) { return [it](const Character& c) { return c.has_item(it); }; }
	std::function<std::vector<m2g::pb::ItemType>(Character&)> generate_named_item_types_filter(m2g::pb::ItemCategory item_category);
	std::function<std::vector<m2g::pb::ItemType>(Character&)> generate_named_item_types_filter(std::initializer_list<m2g::pb::ItemCategory>&& item_categories);
	// Transformers
	Character& to_character_base(CharacterVariant& v);
	inline Object& to_owner_of_character(const Character& chr) { return chr.owner(); }
}
