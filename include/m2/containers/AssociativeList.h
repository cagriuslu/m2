#pragma once
#include <m2/Meta.h>
#include "MicroPool.h"
#include <unordered_map>
#include <variant>
#include <algorithm>
#include <utility>

namespace m2 {
	/// \brief A key-value map with efficient storage.
	/// \details When small, the items are stored inside this object (like small string optimization). MinSequentialSize
	/// determines at least how many items to store sequentially. When the list gets larger, the items are moved into an
	/// unordered_map. The API is designed to resemble STL containers.
	template <typename KeyT, typename ValueT, size_t MinSequentialSize = 1>
	class AssociativeList {
	public:
		using ItemType = std::pair<const KeyT, ValueT>;
		using MapType = std::unordered_map<KeyT, ValueT>;
		static constexpr auto mapSize = sizeof(MapType);

		using PoolItemType = MicroPool<ItemType,1>::ItemType;
		static constexpr auto poolSize = std::max(mapSize / sizeof(PoolItemType) + 1, MinSequentialSize);
		using PoolType = MicroPool<ItemType, poolSize>;

		using StorageType = std::variant<PoolType, MapType>;
		using StorageConstIteratorType = std::variant<typename PoolType::const_iterator, typename MapType::const_iterator>;
		using StorageIteratorType = std::variant<typename PoolType::iterator, typename MapType::iterator>;

	private:
		StorageType _storage{};

	public:
		class const_iterator {
			StorageConstIteratorType _iterator;
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = ItemType;
			using pointer = ItemType*;
			using reference = ItemType&;
			using iterator_category = std::forward_iterator_tag;
			const_iterator() : _iterator() {}
			explicit const_iterator(StorageConstIteratorType it) : _iterator(std::move(it)) {}
			const_iterator& operator++() { std::visit([](auto& it) { ++it; }, _iterator); return *this; }
			const_iterator operator++(int) { return ++*this; }
			bool operator==(const const_iterator& other) const { return _iterator == other._iterator; }
			const ItemType& operator*() const { return std::visit([](const auto& it) -> const ItemType& { return *it; }, _iterator); }
			const ItemType* operator->() const { return &operator*(); }
		};
		class iterator {
			StorageIteratorType _iterator;
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = ItemType;
			using pointer = ItemType*;
			using reference = ItemType&;
			using iterator_category = std::forward_iterator_tag;
			iterator() : _iterator() {}
			explicit iterator(StorageIteratorType it) : _iterator(std::move(it)) {}
			iterator& operator++() { std::visit([](auto& it) { ++it; }, _iterator); return *this; }
			iterator operator++(int) { return ++*this; }
			bool operator==(const iterator& other) const { return _iterator == other._iterator; }
			ItemType& operator*() const { return std::visit([](auto& it) -> ItemType& { return *it; }, _iterator); }
			ItemType* operator->() const { return &operator*(); }
			StorageIteratorType internal_iterator() const { return _iterator; }
		};

		[[nodiscard]] const_iterator cbegin() const { return const_iterator{std::visit([](const auto& s) { return StorageConstIteratorType{s.cbegin()}; }, _storage)}; }
		[[nodiscard]] const_iterator cend() const { return const_iterator{std::visit([](const auto& s) { return StorageConstIteratorType{s.cend()}; }, _storage)}; }
		[[nodiscard]] const_iterator find(const KeyT& key) const {
			return const_iterator{std::visit(overloaded{
				[&](const PoolType& pool) { return StorageConstIteratorType{std::ranges::find_if(pool, [&key](const auto& kv) { return kv.first == key; })}; },
				[&](const MapType& map) { return StorageConstIteratorType{map.find(key)}; },
			}, _storage)};
		}
		[[nodiscard]] bool contains(const KeyT& key) const {
			return std::visit(overloaded{
				[&](const PoolType& pool) { return std::ranges::contains(pool.cbegin(), pool.cend(), key, [](const auto& it) { return it.first; }); },
				[&](const MapType& map) { return map.contains(key); },
			}, _storage);
		}
		[[nodiscard]] bool empty() const { return std::visit([](const auto& s) { return s.empty(); }, _storage); }
		[[nodiscard]] size_t size() const { return std::visit([](const auto& s) { return s.size(); }, _storage); }

		iterator begin() { return iterator{std::visit([](auto& s) { return StorageIteratorType{s.begin()}; }, _storage)}; }
		iterator end() { return iterator{std::visit([](auto& s) { return StorageIteratorType{s.end()}; }, _storage)}; }
		iterator find(const KeyT& key) {
			return iterator{std::visit(overloaded{
				[&](PoolType& pool) { return StorageIteratorType{std::ranges::find_if(pool, [&key](auto& kv) { return kv.first == key; })}; },
				[&](MapType& map) { return StorageIteratorType{map.find(key)}; },
			}, _storage)};
		}
		void clear() {
			_storage.~StorageType();
			new (&_storage) StorageType();
		}
		template<class... Args> bool emplace(Args&&... args) {
			auto item = ItemType{std::forward<Args>(args)...};
			if (auto it = find(item.first); it != end()) { return false; }
			if (std::holds_alternative<PoolType>(_storage) && std::get<PoolType>(_storage).full()) {
				MapType map;
				for (auto& it : std::get<PoolType>(_storage)) { map.emplace(std::move(it)); }
				_storage = std::move(map);
			}
			std::visit(overloaded{
				[&](PoolType& pool) { pool.emplace(std::move(item)); },
				[&](MapType& map) { map.emplace(std::move(item)); },
			}, _storage);
			return true;
		}
		iterator erase(const iterator& it) {
			return iterator{std::visit(overloaded{
				[this](typename PoolType::iterator&& pit) { return StorageIteratorType{std::get<PoolType>(_storage).erase(pit)}; },
				[this](typename MapType::iterator&& mit) { return StorageIteratorType{std::get<MapType>(_storage).erase(mit)}; },
			}, it.internal_iterator())};
		}
		void erase(const KeyT& key) {
			if (const auto it = find(key); it != end()) {
				erase(it);
			}
		}
	};
}
