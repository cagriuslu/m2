#pragma once
#include <m2/LogHelpers.h>
#include "ActorLifetimeManager.h"
#include "ActorBase.h"
#include <thread>
#include <latch>
#include <type_traits>

namespace m2 {
    /// Base class for actor interfaces. The interface manages the communication with the Actor. As an implementation
    /// detail, the interface actually owns the Actor and its thread.
    template <typename Actor, typename ActorInputType, typename ActorOutputType>
    class ActorInterfaceBase : public ActorLifetimeManager {
        static_assert(std::is_base_of_v<ActorBase<ActorInputType, ActorOutputType>, Actor>, "Actor must be derived from ActorBase");

        MessageBox<ActorInputType> _actorInbox;
        MessageBox<ActorOutputType> _actorOutbox;

        std::latch _latch{1};
        Actor _actor;
        std::thread _thread;

    public:
        template <typename... TArgs>
        explicit ActorInterfaceBase(TArgs... args) : _actor(args...), _thread(ActorFunc, this) {
            _latch.count_down();
        }
        ~ActorInterfaceBase() override {
            RequestStop();
            if (_thread.joinable()) {
                // If the object is default created, thread may not be joinable
                _thread.join();
            }
        }

        [[nodiscard]] bool IsActorRunning() const { return IsRunning(); }

    protected:
        MessageBox<ActorInputType>& GetActorInbox() { return _actorInbox; }
        MessageBox<ActorOutputType>& GetActorOutbox() { return _actorOutbox; }

    private:
        static void ActorFunc(ActorInterfaceBase* baseActor) {
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
