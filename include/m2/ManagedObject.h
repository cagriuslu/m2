#pragma once
#include <functional>
#include <type_traits>

namespace m2 {
	/// \brief A utility for wrapping an object with a manager.
	/// \details Allows the contained object to be modified before writing, or side-effects to be incurred on each write.
	template <typename T>
	class ManagedObject final {
		static_assert(not std::is_reference_v<T>);

		T _obj;
		std::function<void(T&)> _writeHook;

	public:
		explicit ManagedObject(std::function<void(T&)> writeHook, T&& obj) : _obj(std::move(obj)), _writeHook(std::move(writeHook)) {}

		template <typename... TArgs>
		explicit ManagedObject(std::function<void(T&)> writeHook, TArgs... args) : _obj(args...), _writeHook(std::move(writeHook)) {}

		/// Access the object
		const T& Get() const { return _obj; }

		/// Replace the object and call the write-hook after
		void Set(T&& newValue) {
			_obj = std::move(newValue);
			if (_writeHook) {
				_writeHook(_obj);
			}
		}
		/// Replace the object and call the write-hook after
		template <typename... TArgs>
		void Emplace(TArgs... args) {
			_obj.~T();
			new (&_obj) T{std::forward<TArgs>(args)...};
			if (_writeHook) {
				_writeHook(_obj);
			}
		}
		/// Mutate the object and call the write-hook after
		void Mutate(const std::function<void(T&)>& mutator) {
			if (mutator) {
				mutator(_obj);
			}
			if (_writeHook) {
				_writeHook(_obj);
			}
		}
		/// Mutate the object and DON'T call the write-hook
		void MutateNoSideEffect(const std::function<void(T&)>& mutator) {
			if (mutator) {
				mutator(_obj);
			}
		}
	};
}
