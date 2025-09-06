#pragma once
#include <array>
#include <algorithm>
#include <optional>

namespace m2 {
	/// \brief A fixed size storage similar to std::array, but not every item is initialized at the beginning.
	/// \details The API is designed to resemble STL containers.
	template <typename T, size_t Size>
	class MicroPool {
	public:
		using ItemType = std::optional<T>;
		using StorageType = std::array<ItemType, Size>;

	private:
		StorageType _storage;

	public:
		class const_iterator {
			const MicroPool& _container;
			StorageType::const_iterator _iterator;
		public:
			const_iterator(const MicroPool& container, StorageType::const_iterator it) : _container(container), _iterator(it) {}
			const_iterator& operator++() {
				if (_iterator == _container._storage.cend()) { return *this; }
				do { ++_iterator; } while (not _iterator->has_value() && _iterator != _container._storage.cend());
				return *this;
			}
			bool operator==(const const_iterator& other) const { return _iterator == other._iterator; }
			const ItemType& operator*() const { return *_iterator; }
			const ItemType* operator->() const { return &*_iterator; }
			StorageType::const_iterator internal_iterator() const { return _iterator; }
		};
		class iterator {
			const MicroPool& _container;
			StorageType::iterator _iterator;
		public:
			iterator(const MicroPool& container, StorageType::iterator it) : _container(container), _iterator(it) {}
			iterator& operator++() {
				if (_iterator == _container._storage.cend()) { return *this; }
				do { ++_iterator; } while (not _iterator->has_value() && _iterator != _container._storage.cend());
				return *this;
			}
			bool operator==(const iterator& other) const { return _iterator == other._iterator; }
			ItemType& operator*() const { return *_iterator; }
			ItemType* operator->() const { return &*_iterator; }
			StorageType::iterator internal_iterator() const { return _iterator; }
		};

		[[nodiscard]] const_iterator cbegin() const { return const_iterator{*this, std::ranges::find_if(_storage, [](const auto& it) { return it.has_value(); })}; }
		[[nodiscard]] const_iterator cend() const { return const_iterator{*this, _storage.cend()}; }
		[[nodiscard]] bool empty() const { return std::ranges::all_of(_storage, [](const auto& it) { return not it.has_value(); }); }
		[[nodiscard]] size_t size() const { return std::ranges::count_if(_storage, [](const auto& it) { return it.has_value(); }); }

		iterator begin() { return iterator{*this, std::ranges::find_if(_storage, [](const auto& it) { return it.has_value(); })}; }
		iterator end() { return iterator{*this, _storage.end()}; }
		void clear() { _storage = {}; }
		template<class... Args> std::pair<iterator, bool> emplace(Args&&... args) {
			auto emptySpace = std::ranges::find_if(_storage, [](const auto& it) { return not it.has_value(); });
			if (emptySpace == _storage.end()) { return std::make_pair(end(), false); }
			emptySpace->emplace(std::forward<Args>(args)...);
			return std::make_pair(iterator{*this, emptySpace}, true);
		}
		iterator erase(const iterator& it) {
			it->reset();
			auto copy = it;
			return ++copy;
		}
	};
}
