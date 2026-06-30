#pragma once
#include <m2/LogHelpers.h>
#include "ActorLifetimeManager.h"
#include "ActorBase.h"
#include <m2/mt/CooperativeActorScheduler.h>
#include <thread>
#include <latch>
#include <type_traits>
#include <utility>

namespace m2 {
    /// Base class for actor interfaces. The interface not just manages the communication with the actor but also owns
    /// the instance of it. On web builds there are no OS threads. Instead of spawning an std::thread, the interface
    /// registers itself with the CooperativeActorScheduler, which executes the actor's step once per frame,
    /// cooperatively sharing the single thread with the main loop.
    template <typename Actor>
    class ActorInterfaceBase : public ActorLifetimeManager, public CooperativelyExecutable {
        static_assert(std::is_base_of_v<ActorBase<typename Actor::InputType, typename Actor::OutputType>, Actor>, "Actor must be derived from ActorBase");

        MessageBox<typename Actor::InputType> _actorInbox;
        MessageBox<typename Actor::OutputType> _actorOutbox;
        bool _hasBeenInitialized{}; // Unused in native builds
        bool _hasBeenDeinitialized{}; // Unused in native builds
        std::latch _latch{1}; // Unused in web builds
        Actor _actor;
        std::thread _thread; // Unused in web builds

    protected:
        template <typename... Args>
        requires std::is_constructible_v<Actor, Args...> // Make sure the constructor isn't greedy (ie. not acting like move or copy constructor)
        explicit ActorInterfaceBase(Args&&... args) : _actor(std::forward<Args>(args)...),
#ifndef __EMSCRIPTEN__
                _thread(ActorFunc, this)
#endif
        {
            CooperativeActorScheduler::Instance().Register(this);
#ifndef __EMSCRIPTEN__
            _latch.count_down();
#endif
        }

    public:
        ~ActorInterfaceBase() override {
            RequestStop();
            CooperativeActorScheduler::Instance().Unregister(this);
#ifdef __EMSCRIPTEN__
            // The actor may have been registered but never executed, or already deinitialized after Step.
            if (_hasBeenInitialized && not _hasBeenDeinitialized) {
                _actor.Deinitialize(_actorInbox, _actorOutbox);
            }
#else
            if (_thread.joinable()) {
                _thread.join(); // If the object is default created, thread may not be joinable
            }
#endif
        }

        [[nodiscard]] bool IsActorRunning() const { return IsRunning(); }

        // Unused in web builds
        void Step() override {
            if (_hasBeenDeinitialized) {
                return; // Already deinitialized
            }

            // Initialize lazily
            if (not _hasBeenInitialized) {
                _hasBeenInitialized = true;
                SetThreadNameForLogging(_actor.ThreadNameForLogging());

                if (not _actor.Initialize(_actorInbox, _actorOutbox)) {
                    // Initialization failed
                    SetAsStopped();
                    _actor.Deinitialize(_actorInbox, _actorOutbox);
                    _hasBeenDeinitialized = true;
                }
                return; // The first pump only initializes
            }

            // Step
            if (IsRunning() && _actor(_actorInbox, _actorOutbox)) {
                return;
            }
            // Must have been stopped externally, or the actor returned false.

            SetAsStopped();
            _actor.Deinitialize(_actorInbox, _actorOutbox);
            _hasBeenDeinitialized = true;
        }

    protected:
        MessageBox<typename Actor::InputType>& GetActorInbox() { return _actorInbox; }
        MessageBox<typename Actor::OutputType>& GetActorOutbox() { return _actorOutbox; }

    private:
        // Unused in web builds
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
