#ifndef M2_POOL_HH
#define M2_POOL_HH

#include <m2/Def.hh>
#include <array>
#include <optional>
#include <cstdint>

// TODO capacity can be increased: 16bit pool id + 24bit key + 24bit index = 16M items
// OR 24bit ket + 20bit key + 20bit index = 1M items
// OR make it configurable

namespace m2 {
    template <typename T, size_t Capacity>
    struct pool;

    template <typename T, size_t Capacity>
    struct pool_iterator {
        pool<T,Capacity> *pool;
        T& data;
        ID id;

        pool_iterator<T,Capacity>& operator++();
        bool operator==(const pool_iterator<T,Capacity>& other) const;
    };

    template <typename T, size_t Capacity>
    struct pool_const_iterator {
        const pool<T,Capacity> *pool;
        const T& data;
        ID id;

        pool_const_iterator<T,Capacity>& operator++();
        bool operator==(const pool_const_iterator<T,Capacity>& other) const;
    };

    template <typename T, size_t Capacity = 65536>
    struct pool {
        static_assert(Capacity <= 65536);

        struct pool_item {
            T data;
            uint32_t id; // If allocated: key|index, else: 0|nextFreeIndex
        };

        std::array<pool_item, Capacity> items;
        ID shifted_pool_id;
        size_t size; // [0, 65536]
        // Key is monotonically increasing, and it is a part of the ID. This means if an object is deallocated,
        // and some other object is allocated at the same location, they will have different IDs.
        size_t next_key; // [1, 65536]
        size_t highest_allocated_index;
        size_t lowest_allocated_index;
        size_t next_free_index;

        pool();

        std::pair<T&, ID> alloc();
        void free(ID id);
        void free(const T* data);
        void free_all();

        bool contains(ID id) const;
        bool contains(const T* data) const;

        pool_iterator<T,Capacity> begin();
        pool_const_iterator<T,Capacity> begin() const;
        pool_iterator<T,Capacity> end();
        pool_const_iterator<T,Capacity> end() const;

        T* get(ID id);
        const T* get(ID id) const;
        ID get_id(const T* data) const;
    };
}

#endif //M2_POOL_HH
