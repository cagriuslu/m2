#pragma once
#include "Exception.h"
#include "Log.h"
#include "ThreadPool.h"
#include <array>
#include <optional>
#include <functional>
#include <cstdint>
#include <cmath>

namespace m2 {
	using Id = uint64_t;

    extern uint16_t g_pool_id;

    template <typename T, uint64_t Capacity = 65536>
    struct Pool {
        static_assert(Capacity <= 16777216);

        struct Item {
            T data;
			// If allocated: 0(16bit) | key(24bit) | index(24bit)
			// Else: 0(16bit) | 0(24bit) | nextFreeIndex(24bit)
            Id id;
        };

        struct Iterator {
            Pool<T,Capacity> *pool;
            T* data;
            Id id;

            Iterator& operator++() {
                const uint64_t curr_index = id & 0xFFFFFFull;
                for (uint64_t i = curr_index + 1; i <= pool->_highest_allocated_index; ++i) {
                    auto& item = pool->_array[i];
                    if (item.id & 0xFFFFFF000000ull) {
                        data = &item.data;
                        id = pool->_shifted_pool_id | item.id;
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
            std::pair<T*,Id> operator*() {
                return {data, id};
            }
        };

		struct Array {
			std::array<Item,Capacity> array;
			inline Array() {
				uint64_t i = 0;
				for (auto& item : array) {
					// Each itm points to next itm as free
					item.id = (i++ + 1) & 0xFFFFFFull;
				}
			}
			inline const Item& operator[](size_t i) const {
				return array[i];
			}
			inline Item& operator[](size_t i) {
				return array[i];
			}
		};

    private:
		Array _array{};
        Id _shifted_pool_id{static_cast<uint64_t>(g_pool_id++) << 48};
        uint64_t _size{0}; // [0, Capacity]
        // Key is monotonically increasing, and it is a part of the ID. This means if an object is deallocated,
        // and some other object is allocated at the same location, they will have different IDs.
        uint64_t _next_key{1}; // [1, Capacity]
	    uint64_t _highest_allocated_index{0};
	    uint64_t _lowest_allocated_index{0};
	    uint64_t _next_free_index{0};
    public:

        Pool() {
			if (g_pool_id == 0) {
				LOG_WARN("Pool ID overflowed");
			}
        }

		template <typename... Args>
        std::pair<T&, Id> alloc(Args... args) {
	        if (Capacity <= _size) {
		        throw M2ERROR("Max pool size exceeded");
	        }
	        // Find the item that will be allocated
	        const uint64_t index_to_alloc = _next_free_index;
	        Item &item = _array[index_to_alloc];
			// Create object inplace
			item.data.~T();
			new (&item.data) T{args...};
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
	        Id id = _shifted_pool_id | item.id;
	        return {item.data, id};
        }
		template <typename... Args>
		Id emplace(Args... args) {
			auto item = alloc(args...);
			return item.second;
		}

		void free(Id id) {
			auto* item_ptr = get_array_item(id);
            if (item_ptr) {
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
                        if (_array[i].id & 0xFFFFFF000000ull) {
                            break;
                        }
                    }
                }
                if (_lowest_allocated_index == index) {
                    // Search forward until lowest allocated index is found
                    for (uint64_t i = index + 1; i < Capacity; i++) {
                        _lowest_allocated_index = i;
                        if (_array[i].id & 0xFFFFFF000000ull) {
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
			if (item.id & 0xFFFFFF000000ull) {
				free(get_id(&item.data));
			}
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
        [[nodiscard]] std::optional<uint64_t> lowest_index() const {
			return _size ? _lowest_allocated_index : std::optional<uint64_t>{};
		}
		[[nodiscard]] std::optional<uint64_t> highest_index() const {
			return _size ? _highest_allocated_index : std::optional<uint64_t>{};
		}
		[[nodiscard]] bool contains(Id id) const {
            return get(id);
        }
        [[nodiscard]] bool contains(const T* data) const {
            return get_id(data);
        }

		T& operator[](Id id) {
			T* t = get(id);
			if (t) {
				return *t;
			} else {
				throw M2ERROR("Pool out of bounds");
			}
		}
	private:
		Item* get_array_item(Id id) {
			if (_shifted_pool_id == (id & 0xFFFF000000000000ull)) {
				const auto candidate_idx = (id & 0xFFFFFFull);
				auto& item = _array[candidate_idx];
				if (item.id == (id & 0xFFFFFFFFFFFFull)) {
					return &item;
				}
			}
			return nullptr;
		}
	public:
        T* get(Id id) {
			auto* item = get_array_item(id);
			if (item) {
				return &item->data;
			}
            return nullptr;
        }
        Id get_id(const T* data) const {
            const auto* byte_ptr = reinterpret_cast<const uint8_t*>(data);
            // Check if data is in range of items
            const auto* lowest_byte_ptr = reinterpret_cast<const uint8_t*>(&_array[_lowest_allocated_index].data);
            const auto* highest_byte_ptr = reinterpret_cast<const uint8_t*>(&_array[_highest_allocated_index].data);
            if (lowest_byte_ptr <= byte_ptr && byte_ptr <= highest_byte_ptr) {
				auto offset_of_data = reinterpret_cast<uint8_t*>(&(reinterpret_cast<Item*>(0)->data));
                const auto* item_ptr = reinterpret_cast<const Item*>(byte_ptr - offset_of_data);
                // Check if itm is allocated
                if (item_ptr->id & 0xFFFFFF000000ull) {
                    return _shifted_pool_id | item_ptr->id;
                }
            }
            return 0;
        }

        Iterator begin() {
            if (_size) {
                Item& item = _array[_lowest_allocated_index];
                return {.pool = this, .data = &item.data, .id = _shifted_pool_id | item.id};
            } else {
                return end();
            }
        }
		Iterator begin_after_index(size_t idx) {
			if (idx <= _highest_allocated_index) {
				// Search first allocated item starting from idx
				for (size_t i = idx; i <= _highest_allocated_index; i++) {
					Item& item = _array[i];
					if (item.id & 0xFFFFFF000000ull) { // If item allocated
						return {.pool = this, .data = &item.data, .id = _shifted_pool_id | item.id};
					}
				}
			}
			return end();
		}
        Iterator end() {
            return {.pool = this, .data = nullptr, .id = 0};
        }

		void parallel_for_each(ThreadPool& tpool, const std::function<void(Id, T&)>& func) {
			if (!size()) {
				return;
			}
			auto index_span = highest_index() - lowest_index() + 1;
			auto fair_item_count = (size_t) std::ceil((float) index_span / (float) tpool.thread_count());
			for (size_t alloc_item_count = 0; alloc_item_count < index_span;) {
				auto left_item_count = index_span - alloc_item_count;
				auto thread_item_count = (fair_item_count < left_item_count) ? fair_item_count : left_item_count;
				auto from_idx = lowest_index() + alloc_item_count;
				auto to_idx = from_idx + thread_item_count;
				auto it = begin_after_index(from_idx);
				if (it == end() || to_idx <= id2index(it.id)) {
					// Iterator is not valid or No allocated items in this batch
				} else {
					tpool.queue([this, func, it, to_idx]() {
						for (auto it_cp = it; it_cp != end() && id2index(it_cp.id) < to_idx; ++it_cp) {
							func(it_cp.id, *it_cp.data);
						}
					});
				}
				alloc_item_count += thread_item_count;
			}
			tpool.wait();
		}

		static size_t id2index(Id id) {
			return id & 0xFFFFFFull;
		}
    };
}