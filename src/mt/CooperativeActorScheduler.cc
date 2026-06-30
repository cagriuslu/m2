#include <m2/mt/CooperativeActorScheduler.h>
#include <m2/common/Error.h>
#include <algorithm>

void m2::StepCooperativeActors() {
#if defined(__EMSCRIPTEN__)
    CooperativeActorScheduler::Instance().StepAll();
#endif
}

m2::CooperativeActorScheduler& m2::CooperativeActorScheduler::Instance() {
    // TODO move to Game or somewhere managed
    static CooperativeActorScheduler schedulerInstance;
    return schedulerInstance;
}

void m2::CooperativeActorScheduler::Register(CooperativelyExecutable* actor) {
#ifdef __EMSCRIPTEN__
    _registeredActors.emplace_back(actor);
#lese
    throw M2_ERROR("Actors in non-web builds shouldn't register themselves with the scheduler");
#endif
}
void m2::CooperativeActorScheduler::Unregister(CooperativelyExecutable* actor) {
#ifdef __EMSCRIPTEN__
    std::erase(_registeredActors, actor);
#lese
    throw M2_ERROR("Actors in non-web builds shouldn't unregister themselves from the scheduler");
#endif
}

void m2::CooperativeActorScheduler::StepAll() {
    // Iterate over a snapshot so that an actor's Step() may register new actors (e.g. a server spawning a connection
    // actor) without invalidating the iteration. Unregistering is still not allowed during iteration.
    for (const auto pumpablesSnapshot = _registeredActors; auto* pumpable : pumpablesSnapshot) {
        pumpable->Step();
    }
}
