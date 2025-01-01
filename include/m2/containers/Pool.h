#pragma once
#include <m2/Error.h>
#include <array>
#include <optional>
#include <functional>
#include <cstdint>
#include <cmath>

namespace m2 {
	using PoolId = uint16_t; // Each Pool has a unique ID, which later becomes part of each object's ID.
	using ShiftedPoolId = uint64_t; // PoolId is shifted to left 48 times to form ShiftedPoolId
	ShiftedPoolId next_shifted_pool_id();

	// Each object held in a Pool has a unique ID. [16 bit PoolId, 24 bit key, 24 bit index]. Key is a monotonically
	// increasing number. If an object is reallocated in-place, it'll have a different key, thus a different ID. Index
	// is the location of the object in the Pool's array.
	using Id = uint64_t;
	using PoolItemId = Id; // Another name for ID

	// Transformers
	inline uint64_t id_to_key(Id id) { return id & 0xFFFFFF000000ull; }
	inline uint64_t id_to_index(Id id) { return id & 0xFFFFFFull; }
	constexpr auto pool_iterator_to_data = [](const auto &it) { return it.data(); };
	constexpr auto to_id = [](const auto &it) { return it.id(); };

    template <typename T, uint64_t Capacity = 65536>
    class Pool {
        static_assert(Capacity <= 16777216, "Max Pool capacity is 16777216 because the index is limited to 24 bits");

	public:
		//<editor-fold desc="Iterator">
        class Iterator {
			Pool<T,Capacity>* _pool{};
			T* _data{};
			Id _id{};

		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;

			Iterator() = default;
			Iterator(Pool<T,Capacity>* pool, T* data, Id id) : _pool(pool), _data(data), _id(id) {}
			explicit operator bool() const { return _data && _id; }
			bool operator==(const Iterator& other) const { return _data == other._data && _id == other._id; }
			// Accessors
			[[nodiscard]] Id id() const { return _id; }
			[[nodiscard]] T* data() const { return _data; }
			T* operator->() const { return _data; }
			T& operator*() const { return *_data; }
			// Modifiers
            Iterator& operator++() {
                for (uint64_t i = id_to_index(_id) + 1; i <= _pool->_highest_allocated_index; ++i) {
                    auto& item = _pool->_array[i];
                    if (id_to_key(item.id)) {
						_data = &item.data;
						_id = item.id;
                        return *this;
                    }
                }
                // Failed
				_data = nullptr;
				_id = 0;
                return *this;
            }
			Iterator operator++(int) {
				auto temp = *this;
				++*this;
				return temp;
			}
        };
		//</editor-fold>

		//<editor-fold desc="Item">
		struct Item {
			T data;
			// If allocated: 16 bit ShiftedPoolId | 24 bit Key | 24 bit Index
			// If available: 16 bit zeros | 24 bit zeros | 24 bit NextFreeIndex
			Id id;
		};
		//</editor-fold>

    private:
		std::array<Item,Capacity> _array;
		ShiftedPoolId _shifted_pool_id{next_shifted_pool_id()};
        uint64_t _size{0}; // [0, Capacity]
        uint64_t _next_key{1}; // [1, Capacity]
	    uint64_t _highest_allocated_index{0};
	    uint64_t _lowest_allocated_index{0};
	    uint64_t _next_free_index{0};

    public:
		// Constructors
        Pool() {
			if (_shifted_pool_id == 0) {
				throw M2_ERROR("PoolId overflow");
			}
			for (uint64_t i = 0; i < Capacity; ++i) {
				// Each item points to the next item as next free index.
				_array[i].id = id_to_index(i + 1);
			}
        }

		// Accessors
		[[nodiscard]] uint64_t size() const { return _size; }
		[[nodiscard]] bool empty() const { return !_size; }
		[[nodiscard]] bool contains(Id id) const { return get(id); }
		[[nodiscard]] bool contains(const T* data) const { return get_id(data); }
		T* get(Id id) {
			if (auto* item = get_array_item(id)) {
				return &item->data;
			}
			return nullptr;
		}
		const T* get(Id id) const {
			if (const auto* item = get_array_item(id)) {
				return &item->data;
			}
			return nullptr;
		}
		T& operator[](Id id) {
			if (auto* t = get(id)) {
				return *t;
			}
			throw M2_ERROR("Out of bounds");
		}
		const T& operator[](Id id) const {
			if (const auto* t = get(id)) {
				return *t;
			}
			throw M2_ERROR("Out of bounds");
		}
		Id get_id(const T* data) const {
			const auto* byte_ptr = reinterpret_cast<const uint8_t*>(data);
			// Check if data is in range of items
			const auto* lowest_byte_ptr = reinterpret_cast<const uint8_t*>(&_array[_lowest_allocated_index].data);
			const auto* highest_byte_ptr = reinterpret_cast<const uint8_t*>(&_array[_highest_allocated_index].data);
			if (lowest_byte_ptr <= byte_ptr && byte_ptr <= highest_byte_ptr) {
				auto offset_of_data = reinterpret_cast<uint8_t*>(&(reinterpret_cast<Item*>(0)->data));
				const auto* item_ptr = reinterpret_cast<const Item*>(byte_ptr - offset_of_data);
				// Check if item is allocated
				if (id_to_key(item_ptr->id)) {
					return item_ptr->id;
				}
			}
			return 0;
		}

		// Iterators
		Iterator begin() {
			if (_size) {
				Item& item = _array[_lowest_allocated_index];
				return {this, &item.data, item.id};
			} else {
				return end();
			}
		}
		Iterator end() { return {this, nullptr, 0}; }

		// Modifiers
		template <typename... Args>
		Iterator emplace(Args&&... args) {
			if (Capacity <= _size) {
				throw M2_ERROR("Max pool size exceeded");
			}
			// Find the item that will be allocated
			const uint64_t index_to_alloc = _next_free_index;
			Item &item = _array[index_to_alloc];
			// Create object in-place
			item.data.~T();
			new (&item.data) T{std::forward<Args>(args)...};
			// Store next free index
			_next_free_index = id_to_index(item.id);
			// Set id of the new item
			item.id = _shifted_pool_id | (_next_key << 24) | id_to_index(index_to_alloc);
			// Adjust pool
			++_size;
			++_next_key;
			if (Capacity < _next_key) {
				// Rewind key to beginning
				_next_key = 1;
			}
			if (_highest_allocated_index < index_to_alloc) {
				_highest_allocated_index = index_to_alloc;
			}
			if (index_to_alloc < _lowest_allocated_index) {
				_lowest_allocated_index = index_to_alloc;
			}
			return {this, &item.data, item.id};
		}

		// TODO add erase(it)
		// TODO rename free to erase
		// TODO provide index to iterator function to replace free_index
		void free(Id id) {
			auto* item_ptr = get_array_item(id);
            if (item_ptr) {
                // Get index of item
                auto index = id_to_index(item_ptr->id);
                // Clear item (avoid swap-delete, objects might rely on `this`, ex. Pool ID lookups)
				item_ptr->data.~T();
				new (&item_ptr->data) T();
                item_ptr->id = id_to_index(_next_free_index);
                // Set next free index
                _next_free_index = index;

                --_size;
                if (_highest_allocated_index == index) {
                    // Search backwards until highest allocated index is found
                    for (uint64_t i = index; i-- > 0; ) {
                        _highest_allocated_index = i;
                        if (id_to_key(_array[i].id)) {
                            break;
                        }
                    }
                }
                if (_lowest_allocated_index == index) {
                    // Search forward until lowest allocated index is found
                    for (uint64_t i = index + 1; i < Capacity; i++) {
                        _lowest_allocated_index = i;
                        if (id_to_key(_array[i].id)) {
                            break;
                        }
                    }
                }
            }
        }
        void free(const T* data) {
            free(get_id(data));
        }
		void free_index(uint64_t idx) {
			auto& item = _array[idx];
			if (id_to_key(item.id)) {
				free(get_id(&item.data));
			}
		}
        void clear() {
            while (_size) {
                auto it = begin();
                free(it.id());
            }
        }

	private:
		Item* get_array_item(Id id) {
			auto& item = _array[id_to_index(id)];
			return item.id == id ? &item : nullptr;
		}
    	const Item* get_array_item(Id id) const {
			auto& item = _array[id_to_index(id)];
			return item.id == id ? &item : nullptr;
		}
    };
}