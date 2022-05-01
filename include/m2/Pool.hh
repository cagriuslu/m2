#ifndef M2_POOL_HH
#define M2_POOL_HH

#include <m2/Error.h>
#include <m2/Def.h>
#include <array>
#include <optional>
#include <cstdint>

// TODO capacity can be increased:
// 16bit pool id +  0bit unused + 24bit key + 24bit index = 16M items
// 16bit pool id + 16bit unused + 16bit key + 16bit index = 65K items

namespace m2 {
    extern uint16_t g_pool_id;

    template <typename T, uint64_t Capacity = 65536>
    struct Pool {
        static_assert(Capacity <= 65536);

        struct Item {
            T data;
            ID id; // If allocated: key|index, else: 0|nextFreeIndex
        };

        struct Iterator {
            Pool<T,Capacity> *pool;
            T* data;
            ID id;

            Iterator& operator++() {
                const uint64_t curr_index = id & 0xFFFFFFull;
                for (uint64_t i = curr_index + 1; i <= pool->_highest_allocated_index; ++i) {
                    auto& item = pool->_items[i];
                    if (item.id & 0xFFFF000000ull) {
                        data = &item.data;
                        id = item.id;
                        return *this;
                    }
                }
                // Failed
                data = nullptr;
                id = 0;
                return *this;
            }
            bool operator==(const Iterator& other) const {
                return id == other.id;
            }
            std::pair<T*,ID> operator*() {
                return {data, id};
            }
        };

    private:
        std::array<Item, Capacity> _items;
        ID _shifted_pool_id;
        uint64_t _size; // [0, 65536]
        // Key is monotonically increasing, and it is a part of the ID. This means if an object is deallocated,
        // and some other object is allocated at the same location, they will have different IDs.
        uint64_t _next_key; // [1, 65536]
	    uint64_t _highest_allocated_index;
	    uint64_t _lowest_allocated_index;
	    uint64_t _next_free_index;
    public:

        Pool() : _items(), _size(0), _next_key(1), _highest_allocated_index(0), _lowest_allocated_index(0), _next_free_index(0) {
            _shifted_pool_id = (static_cast<uint64_t>(g_pool_id++)) << 48;
	        uint64_t i = 0;
            for (auto& item : _items) {
                // Each itm points to next itm as free
                item.id = (i++ + 1) & 0xFFFFFFull;
            }
        }

        std::pair<T&, ID> alloc() {
            if (_size < Capacity) {
                // Find the itm that will be allocated
                const uint64_t index_to_alloc = _next_free_index;
                Item& item = _items[index_to_alloc];
                // Store next free index
                _next_free_index = item.id & 0xFFFFFFull;
                // Set id of the new itm
                item.id = (_next_key << 24) | (index_to_alloc & 0xFFFFFFull);
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
                // Form ID
                ID id = _shifted_pool_id | item.id;
                return {item.data, id};
            } else {
                throw M2ERROR(M2ERR_LIMIT_EXCEEDED);
            }
        }

		void free(ID id) {
			static const Item model = {};
			static const auto t_ptr = reinterpret_cast<const char*>(&model.data);
			static const auto model_ptr = reinterpret_cast<const char*>(&model);
			static const auto offset = t_ptr - model_ptr;

            T* data = get(id);
            if (data) {
                auto* byte_ptr = reinterpret_cast<char*>(data);
                auto* item_ptr = reinterpret_cast<Item*>(byte_ptr - offset);
                // Get index of itm
                auto index = item_ptr->id & 0xFFFFFFull;
                // Clear itm (avoid swap-delete, objects might rely on `this`, ex. Pool ID lookups)
				item_ptr->data.~T();
				new (&item_ptr->data) T();
                item_ptr->id = _next_free_index & 0xFFFFFFull;
                // Set next free index
                _next_free_index = index;

                --_size;
                if (_highest_allocated_index == index) {
                    // Search backwards until highest allocated index is found
                    for (uint64_t i = index; i-- > 0; ) {
                        _highest_allocated_index = i;
                        if (_items[i].id & 0xFFFF000000ull) {
                            break;
                        }
                    }
                }
                if (_lowest_allocated_index == index) {
                    // Search forward until lowest allocated index is found
                    for (uint64_t i = index + 1; i < Capacity; i++) {
                        _lowest_allocated_index = i;
                        if (_items[i].id & 0xFFFF000000ull) {
                            break;
                        }
                    }
                }
            }
        }
        void free(const T* data) {
            free(get_id(data));
        }
        void clear() {
            while (_size) {
                auto it = begin();
                free(it.id);
            }
        }

        [[nodiscard]] uint64_t size() const {
            return _size;
        }
        [[nodiscard]] bool contains(ID id) const {
            return get(id);
        }
        [[nodiscard]] bool contains(const T* data) const {
            return get_id(data);
        }

		T& operator[](ID id) {
			T* t = get(id);
			if (t) {
				return *t;
			} else {
				throw M2ERROR(M2ERR_OUT_OF_BOUNDS);
			}
		}
        T* get(ID id) {
            if (_shifted_pool_id == (id & 0xFFFF000000000000ull)) {
                const auto candidate_idx = (id & 0xFFFFFFull);
                auto& item = _items[candidate_idx];
                if (item.id == (id & 0xFFFFFFFFFFull)) {
                    return &item.data;
                }
            }
            return nullptr;
        }
        ID get_id(const T* data) const {
            const auto* byte_ptr = reinterpret_cast<const uint8_t*>(data);
            // Check if data is in range of items
            const auto* lowest_byte_ptr = reinterpret_cast<const uint8_t*>(&_items[_lowest_allocated_index].data);
            const auto* highest_byte_ptr = reinterpret_cast<const uint8_t*>(&_items[_highest_allocated_index].data);
            if (lowest_byte_ptr <= byte_ptr && byte_ptr <= highest_byte_ptr) {
                const auto* item_ptr = reinterpret_cast<const Item*>(byte_ptr - offsetof(Item, data));
                // Check if itm is allocated
                if (item_ptr->id & 0xFFFF000000ull) {
                    return _shifted_pool_id | item_ptr->id;
                }
            }
            return 0;
        }

        Iterator begin() {
            if (_size) {
                Item& item = _items[_lowest_allocated_index];
                return {.pool = this, .data = &item.data, .id = _shifted_pool_id | item.id};
            } else {
                return end();
            }
        }
        Iterator end() {
            return {.pool = this, .data = nullptr, .id = 0};
        }
    };
}

#endif //M2_POOL_HH
