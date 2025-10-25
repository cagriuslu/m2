#pragma once

namespace m2 {
    // Forward declaration
    struct Object;

    class ObjectImpl {
        Object& _object;

    public:
        explicit ObjectImpl(Object& object) : _object(object) {}
        virtual ~ObjectImpl() = default;

        // Accessors

        const Object& GetObject() const { return _object; }

        // Modifiers

        Object& GetObject() { return _object; }
    };
}
