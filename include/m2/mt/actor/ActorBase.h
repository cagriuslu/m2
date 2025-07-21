#pragma once
#include "MessageBox.h"

namespace m2 {
    /// \brief Base class of all actors.
    /// \details The inherited class will execute the virtual methods in its own thread. Communication with the main
    /// thread (or other actors) should utilize the message boxes to ensure that the internal state of the actor is safe
    /// to access by the actor thread at all times.
    template <typename ActorInputT, typename ActorOutputT>
    class ActorBase {
    public:
        virtual ~ActorBase() = default;

        [[nodiscard]] virtual const char* ThreadNameForLogging() const = 0;

        virtual bool Initialize(MessageBox<ActorInputT>&, MessageBox<ActorOutputT>&) = 0;

        virtual bool operator()(MessageBox<ActorInputT>&, MessageBox<ActorOutputT>&) = 0;

        virtual void Deinitialize(MessageBox<ActorInputT>&, MessageBox<ActorOutputT>&) = 0;
    };
}
