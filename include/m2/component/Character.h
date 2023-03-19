#ifndef M2_CHARACTER_H
#define M2_CHARACTER_H

#include "../SmartPointer.h"
#include "../Component.h"
#include "../Item.h"
#include <InteractionType.pb.h>
#include <utility>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <variant>

namespace m2 {
	namespace internal {
		class ResourceAmount {
			float _amount{};
			float _max_amount{INFINITY};
		public:
			inline explicit ResourceAmount(float amount = 0.0f, float max_amount = INFINITY) { set_max_amount(max_amount); set_amount(amount); }

			[[nodiscard]] inline float amount() const { return _amount; }
			[[nodiscard]] inline bool has_amount() const { return 0.0f < _amount; }

			inline float set_amount(float amount) { return _amount = std::clamp(amount, 0.0f, _max_amount); }
			inline float add_amount(float amount) { return set_amount(_amount + amount); }
			inline float remove_amount(float amount) { return set_amount(_amount - amount); }
			inline float clear_amount() { return _amount = 0.0f; }

			[[nodiscard]] inline float max_amount() const { return _max_amount; }
			float set_max_amount(float max_amount);
		};
	}

	class Character : public Component {
	public:
		std::function<void(Character& self)> update;
		std::function<void(Character& self, Character& other, m2g::pb::InteractionType)> interact;

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

			inline Iterator(const Character& character, Incrementor incrementor, Filter filter, size_t index, const Item* ptr) : _character(character), _incrementor(std::move(incrementor)), _filter(filter), _index(index), _item_ptr(ptr) {}
			inline Iterator& operator++() { _incrementor(*this); return *this; }
			inline bool operator==(const Iterator& other) const { return _item_ptr == other._item_ptr; }
			inline const Item& operator*() const { return *_item_ptr; }
			inline const Item* operator->() const { return _item_ptr; }

			[[nodiscard]] inline const Character& character() const { return _character; }
			[[nodiscard]] inline Filter get_filter() const { return _filter; }
			[[nodiscard]] inline const Item* get() const { return _item_ptr; }
			inline void set(const Item* ptr) { _item_ptr = ptr; }
			[[nodiscard]] inline size_t get_index() const { return _index; }
			inline void set_index(size_t index) { _index = index; }
		};

		Character() = default;
		explicit Character(uint64_t object_id);

		virtual void automatic_update() = 0;
		static void execute_interaction(Character& first_char, m2g::pb::InteractionType cause, Character& second_char, m2g::pb::InteractionType effect);

		[[nodiscard]] bool has_item(m2g::pb::ItemType item_type) const;
		[[nodiscard]] bool has_item(m2g::pb::ItemCategory item_cat) const;
		[[nodiscard]] size_t count_item(m2g::pb::ItemType item_type) const;
		[[nodiscard]] size_t count_item(m2g::pb::ItemCategory item_cat) const;
		[[nodiscard]] virtual Iterator find_items(m2g::pb::ItemType item_type) = 0;
		[[nodiscard]] virtual Iterator find_items(m2g::pb::ItemCategory item_cat) = 0;
		[[nodiscard]] virtual Iterator begin_items() const = 0;
		[[nodiscard]] virtual Iterator end_items() const = 0;
		virtual void add_item(SmartPointer<const Item>&& item) = 0;
		bool use_item(const Iterator& item_it, float resource_multiplier = 1.0f);
		virtual void remove_item(const Iterator& item) = 0;

		[[nodiscard]] virtual bool has_resource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float get_resource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float get_max_resource(m2g::pb::ResourceType resource_type) const = 0;
		virtual void set_max_resource(m2g::pb::ResourceType resource_type, float max) = 0;
		virtual float set_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float add_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float remove_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual void clear_resource(m2g::pb::ResourceType resource_type) = 0;
	};

	/// TinyCharacter can hold only one item, and can have only one resource
	class TinyCharacter : public Character {
		SmartPointer<const Item> _item;
		std::optional<std::pair<m2g::pb::ResourceType, internal::ResourceAmount>> _resource;

	public:
		TinyCharacter() = default;
		explicit TinyCharacter(uint64_t object_id);

		void automatic_update() override;

		Iterator find_items(m2g::pb::ItemType item_type) override;
		Iterator find_items(m2g::pb::ItemCategory item_cat) override;
		[[nodiscard]] Iterator begin_items() const override;
		[[nodiscard]] Iterator end_items() const override;
		void add_item(SmartPointer<const Item>&& item) override;
		void remove_item(const Iterator& item) override;

		[[nodiscard]] bool has_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_max_resource(m2g::pb::ResourceType resource_type) const override;
		void set_max_resource(m2g::pb::ResourceType resource_type, float max) override;
		float set_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		void clear_resource(m2g::pb::ResourceType resource_type) override;
	};

	/// FullCharacter can hold any number of items, and can have any Resource
	class FullCharacter : public Character {
		std::vector<SmartPointer<const Item>> _items;
		std::array<internal::ResourceAmount, m2g::pb::ResourceType_ARRAYSIZE> _resources;

	public:
		FullCharacter() = default;
		explicit FullCharacter(uint64_t object_id);

		void automatic_update() override;

		Iterator find_items(m2g::pb::ItemType item_type) override;
		Iterator find_items(m2g::pb::ItemCategory item_cat) override;
		[[nodiscard]] Iterator begin_items() const override;
		[[nodiscard]] Iterator end_items() const override;
		void add_item(SmartPointer<const Item>&& item) override;
		void remove_item(const Iterator& item) override;

		[[nodiscard]] bool has_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_max_resource(m2g::pb::ResourceType resource_type) const override;
		void set_max_resource(m2g::pb::ResourceType resource_type, float max) override;
		float set_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		void clear_resource(m2g::pb::ResourceType resource_type) override;

	private:
		static int resource_type_index(m2g::pb::ResourceType resource_type);
		friend void full_character_iterator_incrementor(Character::Iterator& it);
	};

	using CharacterVariant = std::variant<TinyCharacter,FullCharacter>;
	Character& get_character_base(CharacterVariant& v);
}

#endif //M2_CHARACTER_H
