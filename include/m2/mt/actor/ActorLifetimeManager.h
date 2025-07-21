#pragma once
#include <m2/mt/ProtectedObject.h>

namespace m2 {
    /// \brief A class that manages the lifetime of an actor.
    /// \details This class is parent to all actors, thus a pointer to this class can be used to stop the actor during
    /// shutdown if necessary.
    class ActorLifetimeManager {
        ProtectedObject<bool> _isRunning;

    public:
        /// Signal the children class about the deconstruction.
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
}
