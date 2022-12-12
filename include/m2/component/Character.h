#ifndef M2_CHARACTER_H
#define M2_CHARACTER_H

#include <m2g/Interaction.h>
#include "../Component.h"
#include "../Item.h"
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace m2 {
	class CharacterBase : public Component {
	public:
		std::function<void(CharacterBase& self)> update;
		std::function<void(CharacterBase& self, CharacterBase& other, m2g::InteractionType)> interact;

		template <typename T> class IteratorImpl;
		template <typename T> class Iterator {
		protected:
			friend IteratorImpl<T>;
			std::unique_ptr<IteratorImpl<T>> _impl;
			using Filter = std::variant<std::monostate,m2g::pb::ItemType,m2g::pb::ItemCategory>;
			Filter _filter;
			T* _item_ptr; // Item or const Item
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
		template <typename T> class IteratorImpl {
		public:
			virtual ~IteratorImpl() = default;
			virtual void increment(Iterator<T>& iter) = 0;

			static typename Iterator<T>::Filter get_filter(const Iterator<T>& iter) { return iter._filter; }
			static void set_filter(Iterator<T>& iter, typename Iterator<T>::Filter filter) { iter._filter = filter; }
			static T* get_item(const Iterator<T>& iter) { return iter._item_ptr; }
			static void set_item(Iterator<T>& iter, T* ptr) { iter._item_ptr = ptr; }
		};

		CharacterBase() = default;
		explicit CharacterBase(uint64_t object_id);

		virtual void automatic_update() = 0;

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
		virtual float add_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual float remove_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual void clear_resource(m2g::pb::ResourceType resource_type) = 0;
	};

	class TinyCharacter : public CharacterBase {
		std::optional<Item> _item;
		std::optional<std::pair<m2g::pb::ResourceType,float>> _resource;

	public:
		template <typename T> class TinyCharacterIteratorImpl : public IteratorImpl<T> {
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
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		void clear_resource(m2g::pb::ResourceType resource_type) override;
	};

	class FullCharacter : public CharacterBase {
		std::vector<Item> _items;
		std::array<float, m2g::pb::ResourceType_ARRAYSIZE> _resources{};

	public:
		template <typename T, typename CharacterType> class FullCharacterIteratorImpl : public IteratorImpl<T> {
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
						if (_character->_items[i]->type() == std::get<m2g::pb::ItemType>(filter)) {
							// Found item
							IteratorImpl<T>::set_item(iter, &_character->_items[i]);
							return;
						}
					}
				} else if (std::holds_alternative<m2g::pb::ItemCategory>(filter)) {
					for (size_t i = curr_index + 1; i < _character->_items.size(); ++i) {
						if (_character->_items[i]->category() == std::get<m2g::pb::ItemCategory>(filter)) {
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
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		void clear_resource(m2g::pb::ResourceType resource_type) override;
	};

	using CharacterVariant = std::variant<TinyCharacter,FullCharacter>;
	CharacterBase& get_character_base(CharacterVariant& v);
}

#endif //M2_CHARACTER_H
