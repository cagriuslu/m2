#include <m2/pool.hh>
#include <m2/error.hh>

uint16_t g_pool_id = 1;

template <typename T, size_t Capacity>
m2::pool_iterator<T,Capacity>& m2::pool_iterator<T,Capacity>::operator++() {
    // TODO actual increment
    return *this;
}

template <typename T, size_t Capacity>
bool m2::pool_iterator<T,Capacity>::operator==(const pool_iterator<T,Capacity>& other) const {
    return id == other.id;
}

template <typename T, size_t Capacity>
m2::pool_const_iterator<T,Capacity>& m2::pool_const_iterator<T,Capacity>::operator++() {
    // TODO actual increment
    return *this;
}

template <typename T, size_t Capacity>
bool m2::pool_const_iterator<T,Capacity>::operator==(const pool_const_iterator<T,Capacity>& other) const {
    return id == other.id;
}

template <typename T, size_t Capacity>
m2::pool<T,Capacity>::pool() : items({}), size(0), next_key(1), highest_allocated_index(0), lowest_allocated_index(0), next_free_index(0) {
    shifted_pool_id = (static_cast<uint64_t>(g_pool_id++)) << 48;
    size_t i = 0;
    for (auto& item : items) {
        // Each item points to next item as free
        item.id = (i++ + 1) & 0xFFFF;
    }
}

template <typename T, size_t Capacity>
std::pair<T&, ID> m2::pool<T,Capacity>::alloc() {
    if (size < Capacity) {
        // Find the item that will be allocated
        const size_t index_to_alloc = next_free_index;
        pool_item& item = items[index_to_alloc];
        // Store next free index
        next_free_index = item.id & 0xFFFF;
        // Set id of the new item
        item.id = (static_cast<uint16_t>(next_key) << 16) | ( static_cast<uint16_t>(index_to_alloc) & 0xFFFF);
        // Adjust pool
        size++;
        next_key++;
        if (Capacity < next_key) {
            // Rewind key to beginning
            next_key = 1;
        }
        if (highest_allocated_index < index_to_alloc) {
            highest_allocated_index = index_to_alloc;
        }
        if (index_to_alloc < lowest_allocated_index) {
            lowest_allocated_index = index_to_alloc;
        }
        // Form ID
        const ID id = shifted_pool_id | static_cast<ID>(item.id);
        return std::make_pair<T&, ID>(item.data, id);
    } else {
        throw m2::error(M2ERR_LIMIT_EXCEEDED);
    }
}

template <typename T, size_t Capacity>
void m2::pool<T,Capacity>::free(ID id) {

}

template <typename T, size_t Capacity>
void m2::pool<T,Capacity>::free(const T* data) {

}

template <typename T, size_t Capacity>
void m2::pool<T,Capacity>::free_all() {

}

template <typename T, size_t Capacity>
bool m2::pool<T,Capacity>::contains(ID id) const {

}

template <typename T, size_t Capacity>
bool m2::pool<T,Capacity>::contains(const T* data) const {

}

template <typename T, size_t Capacity>
m2::pool_iterator<T,Capacity> m2::pool<T,Capacity>::begin() {

}

template <typename T, size_t Capacity>
m2::pool_const_iterator<T,Capacity> m2::pool<T,Capacity>::begin() const {

}

template <typename T, size_t Capacity>
m2::pool_iterator<T,Capacity> m2::pool<T,Capacity>::end() {

}

template <typename T, size_t Capacity>
m2::pool_const_iterator<T,Capacity> m2::pool<T,Capacity>::end() const {

}

template <typename T, size_t Capacity>
T* m2::pool<T,Capacity>::get(ID id) {

}

template <typename T, size_t Capacity>
const T* m2::pool<T,Capacity>::get(ID id) const {

}

template <typename T, size_t Capacity>
ID m2::pool<T,Capacity>::get_id(const T* data) const {

}
