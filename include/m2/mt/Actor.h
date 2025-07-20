#pragma once
#include <m2/Log.h>
#include "ProtectedObject.h"
#include <deque>
#include <functional>
#include <thread>
#include <latch>
#include <type_traits>

namespace m2 {
	/// A class that manages the lifetime of an actor.
	class ActorLifetimeManager {
		ProtectedObject<bool> _isRunning;

	public:
		/// The only purpose of the deconstructor is to signal the children class about the deconstruction.
		virtual ~ActorLifetimeManager() = default;

	protected:
		/// Only the children can instantiate this class.
		ActorLifetimeManager();

		/// Thread-safe getter to check if the actor is still running.
		bool IsRunning() const;

		/// Thread-safe method to request the actor to stop running.
		void RequestStop();

		/// If the thread stops running on its own, it can be marked as such.
		void SetAsStopped() { RequestStop(); }
	};

	template <typename T>
	class MessageBox {
		ProtectedObject<std::deque<T>> _protectedQueue;

	public:
		void PushMessage(T&& msg) {
			_protectedQueue.Write([&msg](std::deque<T>& queue) {
				queue.emplace_back(std::move(msg));
			});
		}
		const T* PeekMessage() const {
			const T* frontMsg{};
			_protectedQueue.Read([&frontMsg](const std::deque<T>& queue) {
				if (not queue.empty()) {
					frontMsg = &queue.front();
				}
			});
			return frontMsg;
		}
		/// Waits until the given condition is true for **any** message in the queue
		void WaitMessage(const std::function<bool(const T&)>& condition) const {
			_protectedQueue.Read([](const std::deque<T>&) {}, [&](const std::deque<T>& queue) -> bool {
				return std::any_of(queue.begin(), queue.end(), condition);
			});
		}
		bool PopMessage(std::optional<T>& out) {
			_protectedQueue.Write([&out](std::deque<T>& queue) {
				if (not queue.empty()) {
					out.emplace(std::move(queue.front()));
					queue.pop_front();
				} else {
					out.reset();
				}
			});
			return out.has_value();
		}
	};

	template <typename ActorInputType, typename ActorOutputType>
	class BaseActor {
	public:
		virtual ~BaseActor() = default;

		[[nodiscard]] virtual const char* ThreadNameForLogging() const = 0;

		virtual bool Initialize(MessageBox<ActorInputType>&, MessageBox<ActorOutputType>&) = 0;

		virtual bool operator()(MessageBox<ActorInputType>&, MessageBox<ActorOutputType>&) = 0;

		virtual void Deinitialize(MessageBox<ActorInputType>&, MessageBox<ActorOutputType>&) = 0;
	};

	/// Base class for actor interfaces. The interface manages the communication with the Actor. As an implementation
	/// detail, the interface actually owns the Actor and its thread.
	template <typename Actor, typename ActorInputType, typename ActorOutputType>
	class BaseActorInterface : public ActorLifetimeManager {
		static_assert(std::is_base_of_v<BaseActor<ActorInputType, ActorOutputType>, Actor>, "Actor must be derived from BaseActor");

		MessageBox<ActorInputType> _actorInbox;
		MessageBox<ActorOutputType> _actorOutbox;

		std::latch _latch{1};
		Actor _actor;
		std::thread _thread;

	public:
		template <typename... TArgs>
		explicit BaseActorInterface(TArgs... args) : _actor(args...), _thread(ActorFunc, this) {
			_latch.count_down();
		}
		~BaseActorInterface() override {
			RequestStop();
			if (_thread.joinable()) {
				// If the object is default created, thread may not be joinable
				_thread.join();
			}
		}

	protected:
		MessageBox<ActorInputType>& GetActorInbox() { return _actorInbox; }
		MessageBox<ActorOutputType>& GetActorOutbox() { return _actorOutbox; }

		// TODO we need a getter for the lifecycle manager

	private:
		static void ActorFunc(BaseActorInterface* baseActor) {
			baseActor->_latch.wait();

			SetThreadNameForLogging(baseActor->_actor.ThreadNameForLogging());

			if (baseActor->_actor.Initialize(baseActor->_actorInbox, baseActor->_actorOutbox)) {
				while (baseActor->IsRunning()) {
					if (not baseActor->_actor(baseActor->_actorInbox, baseActor->_actorOutbox)) {
						break;
					}
				}
			}

			baseActor->SetAsStopped();
			baseActor->_actor.Deinitialize(baseActor->_actorInbox, baseActor->_actorOutbox);
		}
	};


}
