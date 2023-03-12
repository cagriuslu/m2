#ifndef M2_CHARACTER_H
#define M2_CHARACTER_H

#include <m2g/Interaction.h>
#include "../Component.h"
#include "../Item.h"
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <variant>

namespace m2 {
	class Character : public Component {
	public:
		std::function<void(Character& self)> update;
		std::function<void(Character& self, Character& other, m2g::InteractionType)> interact;

		template <typename T> class IteratorImpl; // Forward declaration
		/// Iterator can be const or non-const
		/// Iterator uses pImpl pattern to allow polymorphism with templates
		template <typename T> class Iterator {
			static_assert(std::is_same_v<T, Item> || std::is_same_v<T, const Item>);
		protected:
			friend IteratorImpl<T>;
			std::unique_ptr<IteratorImpl<T>> _impl; // PImpl
			using Filter = std::variant<std::monostate,m2g::pb::ItemType,m2g::pb::ItemCategory>;
			Filter _filter;
			T* _item_ptr;
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = T*;
			using reference = T&;
			using iterator_category = std::forward_iterator_tag;

			Iterator(IteratorImpl<T>* impl, Filter filter, T* item_ptr) : _impl(impl), _filter(filter), _item_ptr(item_ptr) {}
			Iterator& operator++() { _impl->increment(*this); return *this; }
			bool operator==(const Iterator& other) const { return _item_ptr == other._item_ptr; }
			T& operator*() { return *_item_ptr; }
			T* operator->() { return _item_ptr; }
			const T& operator*() const { return *_item_ptr; }
			const T* operator->() const { return _item_ptr; }
		};
		/// IteratorImpl can be const or non-const
		/// IteratorImpl needs to be overridden by child Character types
		template <typename T> class IteratorImpl {
			static_assert(std::is_same_v<T, Item> || std::is_same_v<T, const Item>);
		public:
			virtual ~IteratorImpl() = default;
			virtual void increment(Iterator<T>& iter) = 0;

			static typename Iterator<T>::Filter get_filter(const Iterator<T>& iter) { return iter._filter; }
			static void set_filter(Iterator<T>& iter, typename Iterator<T>::Filter filter) { iter._filter = filter; }
			static T* get_item(const Iterator<T>& iter) { return iter._item_ptr; }
			static void set_item(Iterator<T>& iter, T* ptr) { iter._item_ptr = ptr; }
		};

		Character() = default;
		explicit Character(uint64_t object_id);

		virtual void automatic_update() = 0;
		static void execute_interaction(Character& first_char, m2g::InteractionType cause, Character& second_char, m2g::InteractionType effect);

		[[nodiscard]] bool has_item(m2g::pb::ItemType item_type) const;
		[[nodiscard]] bool has_item(m2g::pb::ItemCategory item_cat) const;
		[[nodiscard]] size_t count_item(m2g::pb::ItemType item_type) const;
		[[nodiscard]] size_t count_item(m2g::pb::ItemCategory item_cat) const;
		[[nodiscard]] virtual Iterator<Item> find_items(m2g::pb::ItemType item_type) = 0;
		[[nodiscard]] virtual Iterator<Item> find_items(m2g::pb::ItemCategory item_cat) = 0;
		[[nodiscard]] virtual Iterator<Item> begin_items() = 0;
		[[nodiscard]] virtual Iterator<Item> end_items() = 0;
		[[nodiscard]] virtual Iterator<const Item> cbegin_items() const = 0;
		[[nodiscard]] virtual Iterator<const Item> cend_items() const = 0;
		virtual void add_item(const Item& item) = 0;
		bool use_item(const Iterator<Item>& item_it, float resource_multiplier = 1.0f);
		virtual void remove_item(const Iterator<Item>& item) = 0;

		[[nodiscard]] virtual bool has_resource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float get_resource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float get_max_resource(m2g::pb::ResourceType resource_type) const = 0;
		virtual void set_max_resource(m2g::pb::ResourceType resource_type, float max) = 0;
		virtual float set_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float add_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float remove_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual void clear_resource(m2g::pb::ResourceType resource_type) = 0;
	};

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

	class TinyCharacter : public Character {
		std::optional<Item> _item;
		std::optional<std::pair<m2g::pb::ResourceType, internal::ResourceAmount>> _resource;

	public:
		template <typename T> class TinyCharacterIteratorImpl : public IteratorImpl<T> {
			static_assert(std::is_same_v<T, Item> || std::is_same_v<T, const Item>);
		public:
			void increment(Iterator<T>& iter) override { IteratorImpl<T>::set_item(iter, nullptr); }
		};

		TinyCharacter() = default;
		explicit TinyCharacter(uint64_t object_id);

		void automatic_update() override;

		Iterator<Item> find_items(m2g::pb::ItemType item_type) override;
		Iterator<Item> find_items(m2g::pb::ItemCategory item_cat) override;
		Iterator<Item> begin_items() override;
		Iterator<Item> end_items() override;
		Iterator<const Item> cbegin_items() const override;
		Iterator<const Item> cend_items() const override;
		void add_item(const Item& item) override;
		void remove_item(const Iterator<Item>& item) override;

		[[nodiscard]] bool has_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_resource(m2g::pb::ResourceType resource_type) const override;
		float get_max_resource(m2g::pb::ResourceType resource_type) const override;
		void set_max_resource(m2g::pb::ResourceType resource_type, float max) override;
		float set_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		void clear_resource(m2g::pb::ResourceType resource_type) override;
	};

	class FullCharacter : public Character {
		std::vector<Item> _items;
		std::array<internal::ResourceAmount, m2g::pb::ResourceType_ARRAYSIZE> _resources;

	public:
		template <typename T, typename CharacterType> class FullCharacterIteratorImpl : public IteratorImpl<T> {
			static_assert(std::is_same_v<Item, T> || std::is_same_v<const Item, T>);
			CharacterType* _character;
		public:
			explicit FullCharacterIteratorImpl(CharacterType* character) : _character(character) {}
			void increment(Iterator<T>& iter) override {
				auto curr_index = static_cast<size_t>(IteratorImpl<T>::get_item(iter) - _character->_items.data());
				auto filter = IteratorImpl<T>::get_filter(iter);
				if (std::holds_alternative<std::monostate>(filter)) {
					if (curr_index + 1 < _character->_items.size()) {
						// Next item
						IteratorImpl<T>::set_item(iter, &_character->_items[curr_index + 1]);
						return;
					}
				} else if (std::holds_alternative<m2g::pb::ItemType>(filter)) {
					for (size_t i = curr_index + 1; i < _character->_items.size(); ++i) {
						if (_character->_items[i].item().type() == std::get<m2g::pb::ItemType>(filter)) {
							// Found item
							IteratorImpl<T>::set_item(iter, &_character->_items[i]);
							return;
						}
					}
				} else if (std::holds_alternative<m2g::pb::ItemCategory>(filter)) {
					for (size_t i = curr_index + 1; i < _character->_items.size(); ++i) {
						if (_character->_items[i].item().category() == std::get<m2g::pb::ItemCategory>(filter)) {
							// Found item
							IteratorImpl<T>::set_item(iter, &_character->_items[i]);
							return;
						}
					}
				} else {
					throw M2FATAL("Invalid iterator filter");
				}
				// Item not found
				IteratorImpl<T>::set_item(iter, nullptr);
			}
		};

		FullCharacter() = default;
		explicit FullCharacter(uint64_t object_id);

		void automatic_update() override;

		Iterator<Item> find_items(m2g::pb::ItemType item_type) override;
		Iterator<Item> find_items(m2g::pb::ItemCategory item_cat) override;
		Iterator<Item> begin_items() override;
		Iterator<Item> end_items() override;
		[[nodiscard]] Iterator<const Item> cbegin_items() const override;
		[[nodiscard]] Iterator<const Item> cend_items() const override;
		void add_item(const Item& item) override;
		void remove_item(const Iterator<Item>& item) override;

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
	};

	using CharacterVariant = std::variant<TinyCharacter,FullCharacter>;
	Character& get_character_base(CharacterVariant& v);
}

#endif //M2_CHARACTER_H
