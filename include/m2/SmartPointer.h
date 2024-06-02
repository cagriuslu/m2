#pragma once
#include "M2.h"
#include <variant>
#include <memory>

namespace m2 {
	/// Smart pointer can hold nothing, or pointer to an object with a static duration, or unique_ptr to an object.
	/// It's useful when the user of the object doesn't need to know the origins of the object.
	template <typename T> class SmartPointer final {
		std::variant<std::monostate, T*, std::unique_ptr<T>> _ptr;

	public:
		SmartPointer() = default;
		SmartPointer(SmartPointer&& other) noexcept = default;
		SmartPointer& operator=(SmartPointer&& other) noexcept = default;
		explicit SmartPointer(T* ptr) : _ptr(ptr) {}
		explicit SmartPointer(std::unique_ptr<T>&& ptr) : _ptr(std::move(ptr)) {}
		~SmartPointer() = default;

		inline explicit operator bool() const {
			return std::visit(m2::overloaded {
					[](MAYBE std::add_const_t<T>* v) -> bool { return v; },
					[](MAYBE const std::unique_ptr<T>& v) -> bool { return (bool)v; },
					[](MAYBE const std::monostate& v) -> bool { return false; }
			}, _ptr);
		}

		T* get() {
			return std::visit(m2::overloaded {
					[](MAYBE T* v) -> T* { return v; },
					[](MAYBE std::unique_ptr<T>& v) -> T* { return v.get(); },
					[](MAYBE std::monostate& v) -> T* { return nullptr; }
			}, _ptr);
		}
		std::add_const_t<T>* get() const {
			return std::visit(m2::overloaded {
					[](MAYBE std::add_const_t<T>* v) -> std::add_const_t<T>* { return v; },
					[](MAYBE const std::unique_ptr<T>& v) -> std::add_const_t<T>* { return v.get(); },
					[](MAYBE const std::monostate& v) -> std::add_const_t<T>* { return nullptr; }
			}, _ptr);
		}
		T* operator->() {
			return get();
		}
		const T* operator->() const {
			return get();
		}
		T& operator*() {
			return *get();
		}
		const T& operator*() const {
			return *get();
		}
	};

	template <typename T> SmartPointer<T> make_static(T* t) {
		return SmartPointer<T>{t};
	}
	template <typename T, typename ...Args> SmartPointer<T> make_dynamic(Args ...args) {
		return SmartPointer<T>{std::unique_ptr<T>{args...}};
	}
}
