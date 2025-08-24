#pragma once
#include <shared_mutex>
#include <condition_variable>
#include <functional>

namespace m2 {
	/// Protects an object from race conditions. Only one write call is allowed at a time, whereas multiple read calls
	/// are allowed at the same time.
	template <typename T>
	class ProtectedObject {
		T _obj;
		mutable std::shared_mutex _sharedMutex;
		mutable std::condition_variable_any _conditionVariable;

	public:
		explicit ProtectedObject(T&& obj) : _obj(std::move(obj)) {}

		template <typename... TArgs>
		explicit ProtectedObject(TArgs... args) : _obj(args...) {}

		/// If condition is provided, blocks until the condition returns true.
		void Read(const std::function<void(const T&)>& reader, const std::function<bool(const T&)>& condition = {}) const {
			std::shared_lock lock(_sharedMutex);
			if (condition) {
				while (condition(_obj) == false) {
					_conditionVariable.wait(lock);
				}
			}
			reader(_obj);
		}

		/// If condition is provided, blocks until the condition returns true.
		void Write(const std::function<void(T&)>& writer, const std::function<bool(const T&)>& condition = {}) {
			std::unique_lock lock(_sharedMutex);
			if (condition) {
				while (condition(_obj) == false) {
					_conditionVariable.wait(lock);
				}
			}
			writer(_obj);
			_conditionVariable.notify_all();
		}
	};
}
