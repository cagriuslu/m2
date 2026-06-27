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
			StorageType::const_iterator _iterator, _endIterator;
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = const T;
			using pointer = const T*;
			using reference = const T&;
			using iterator_category = std::forward_iterator_tag;
			const_iterator() : _iterator(), _endIterator() {}
			const_iterator(StorageType::const_iterator it, StorageType::const_iterator endIt) : _iterator(std::move(it)), _endIterator(std::move(endIt)) {}
			const_iterator& operator++() {
				if (_iterator == _endIterator) { return *this; }
				do { ++_iterator; } while (not _iterator->has_value() && _iterator != _endIterator);
				return *this;
			}
			const_iterator operator++(int) { return ++(*this); }
			bool operator==(const const_iterator& other) const { return _iterator == other._iterator; }
			const T& operator*() const { return _iterator->value(); }
			const T* operator->() const { return &_iterator->value(); }
		};
		class iterator {
			StorageType::iterator _iterator, _endIterator;
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = T*;
			using reference = T&;
			using iterator_category = std::forward_iterator_tag;
			iterator() : _iterator(), _endIterator() {}
			iterator(StorageType::iterator it, StorageType::iterator endIt) : _iterator(std::move(it)), _endIterator(std::move(endIt)) {}
			iterator& operator++() {
				if (_iterator == _endIterator) { return *this; }
				do { ++_iterator; } while (not _iterator->has_value() && _iterator != _endIterator);
				return *this;
			}
			iterator operator++(int) { return ++(*this); }
			bool operator==(const iterator& other) const { return _iterator == other._iterator; }
			T& operator*() const { return _iterator->value(); }
			T* operator->() const { return &_iterator->value(); }
			StorageType::iterator internal_iterator() const { return _iterator; }
		};

		[[nodiscard]] const_iterator cbegin() const { return const_iterator{std::ranges::find_if(_storage, [](const auto& it) { return it.has_value(); }), _storage.cend()}; }
		[[nodiscard]] const_iterator cend() const { return const_iterator{_storage.cend(), _storage.cend()}; }
		[[nodiscard]] bool empty() const { return std::ranges::all_of(_storage, [](const auto& it) { return not it.has_value(); }); }
		[[nodiscard]] bool full() const { return std::ranges::all_of(_storage, [](const auto& it) { return it.has_value(); }); }
		[[nodiscard]] size_t size() const { return std::ranges::count_if(_storage, [](const auto& it) { return it.has_value(); }); }

		iterator begin() { return iterator{std::ranges::find_if(_storage, [](const auto& it) { return it.has_value(); }), _storage.end()}; }
		iterator end() { return iterator{_storage.end(), _storage.end()}; }
		void clear() { _storage = {}; }
		template<class... Args> std::pair<iterator, bool> emplace(Args&&... args) {
			auto emptySpace = std::ranges::find_if(_storage, [](const auto& it) { return not it.has_value(); });
			if (emptySpace == _storage.end()) { return std::make_pair(end(), false); }
			emptySpace->emplace(std::forward<Args>(args)...);
			return std::make_pair(iterator{emptySpace, _storage.end()}, true);
		}
		iterator erase(const iterator& it) {
			it.internal_iterator()->reset();
			auto copy = it;
			return ++copy;
		}
	};
}
