#pragma once

namespace m2 {
	template <typename T>
	class PropagateConstPtr {
		T* _ptr;

	public:
		explicit PropagateConstPtr(T* ptr) : _ptr(ptr) {}

		[[nodiscard]] explicit operator bool() const { return static_cast<bool>(_ptr); }

		// Non-const access returns non-const reference

		T& operator*() { return *_ptr; }
		T* operator->() { return _ptr; }

		// Const access returns const reference

		const T& operator*() const { return *_ptr; }
		const T* operator->() const { return _ptr; }
	};

	template <typename T>
	class PropagateConstRef {
		T& _ref;

	public:
		explicit PropagateConstRef(T& ref) : _ref(ref) {}

		[[nodiscard]] explicit operator bool() const { return true; }

		// Non-const access returns non-const reference

		T& operator*() { return _ref; }
		T* operator->() { return &_ref; }

		// Const access returns const reference
		
		const T& operator*() const { return _ref; }
		const T* operator->() const { return &_ref; }
	};
}
