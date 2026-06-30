#pragma once
#include <vector>

namespace m2 {
    /// Base class for actors that can be executed cooperatively on single-threaded (web) builds.
    class CooperativelyExecutable {
    public:
        virtual ~CooperativelyExecutable() = default;

        /// Executes exactly one iteration
        virtual void Step() {}
    };

    /// Advances every registered cooperative actor by one Step(). No-op on native builds.
    void StepCooperativeActors();

    /// Global registry of cooperatively executable actors
    class CooperativeActorScheduler {
        std::vector<CooperativelyExecutable*> _registeredActors;

        CooperativeActorScheduler() = default;

    public:
        static CooperativeActorScheduler& Instance();

        /// Cooperative actors must register themselves
        void Register(CooperativelyExecutable* actor);
        void Unregister(CooperativelyExecutable* actor);

    private:
        friend void StepCooperativeActors();
        void StepAll();
    };
}
