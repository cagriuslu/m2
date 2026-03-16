#pragma once
#include <m2/containers/Pool.h>

namespace m2 {
    /// Base class of all heap-allocated ObjectImpl
    class HeapObjectImpl {
    public:
        virtual ~HeapObjectImpl() = default;
    };

    /// Stores the details for a Pool-allocated ObjectImpl. This class handles the deallocation from the Pool when the
    /// object is eventually destroyed. This class is not meant to be derived from. Instead, the pointer to the pool and
    /// the ID of the item should be stored here.
    class PoolObjectImpl final {
        PoolBase* _poolBase;
        Id _id;

    public:
        explicit PoolObjectImpl(PoolBase& poolBase, const Id id) : _poolBase(&poolBase), _id(id) {}
        // Copy not allowed
        PoolObjectImpl(const PoolObjectImpl& other) = delete;
        PoolObjectImpl& operator=(const PoolObjectImpl& other) = delete;
        // Move not allowed
        PoolObjectImpl(PoolObjectImpl&& other) noexcept : _poolBase(other._poolBase), _id(other._id) { other._poolBase = nullptr; other._id = 0; }
        PoolObjectImpl& operator=(PoolObjectImpl&& other) noexcept { std::swap(_poolBase, other._poolBase); std::swap(_id, other._id); return *this; }
        // Destructor
        ~PoolObjectImpl() {
            if (_poolBase && _id) {
                _poolBase->Free(_id);
            }
        }

        [[nodiscard]] Id GetId() const { return _id; }
    };
}
