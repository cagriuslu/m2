#pragma once
#include <m2/Chrono.h>
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

		template <typename ReaderF>
		void Read(ReaderF reader) const {
			std::shared_lock lock(_sharedMutex);
			reader(_obj);
		}

		/// Waits until the condition holds true and calls the reader with the object. If timeout is given, and the
		/// condition doesn't hold until the given timeout, returns false without calling the reader. If reader is
		/// called successfully, returns true.
		template <typename ConditionF, typename ReaderF>
		bool WaitUntilAndRead(ConditionF condition, ReaderF reader, const std::optional<Stopwatch::Duration> timeout = std::nullopt) const {
			std::shared_lock lock(_sharedMutex);
			while (condition(_obj) == false) {
				if (timeout) {
					if (_conditionVariable.wait_for(lock, *timeout) == std::cv_status::timeout) {
						return false;
					}
				} else {
					_conditionVariable.wait(lock);
				}
			}
			reader(_obj);
			return true;
		}

		template <typename WriterF>
		void Write(WriterF writer) {
			std::unique_lock lock(_sharedMutex);
			writer(_obj);
			_conditionVariable.notify_all();
		}

		/// Waits until the condition holds true and calls the writer with the object. If timeout is given, and the
		/// condition doesn't hold until the given timeout, returns false without calling the writer. If writer is
		/// called successfully, returns true.
		template <typename ConditionF, typename WriterF>
		bool WaitUntilAndWrite(ConditionF condition, WriterF writer, const std::optional<Stopwatch::Duration> timeout = std::nullopt) {
			std::unique_lock lock(_sharedMutex);
			while (condition(_obj) == false) {
				if (timeout) {
					if (_conditionVariable.wait_for(lock, *timeout) == std::cv_status::timeout) {
						return false;
					}
				} else {
					_conditionVariable.wait(lock);
				}
			}
			writer(_obj);
			_conditionVariable.notify_all();
			return true;
		}
	};
}
