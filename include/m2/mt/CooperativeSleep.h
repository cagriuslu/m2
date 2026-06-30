#pragma once
#include <m2/common/Chrono.h>
#include <chrono>

namespace m2 {
    namespace detail {
        /// On the web build it executes every registered cooperative actor by one Step() and then yields to the browser
        /// with the given minimum delay. On native builds, only sleeps for the given duration as the Actors will have
        /// actual threads of execution.
        void StepActorsOnceAndWaitCooperatively(int waitMs);
    }

    /// Cooperatively waits until the predicate returns true
    template <typename Predicate>
    void CooperativeSleepUntil(Predicate predicate, const int pollIntervalMs = 25) {
        while (not predicate()) {
            detail::StepActorsOnceAndWaitCooperatively(pollIntervalMs);
        }
    }

    /// Like CooperativeSleepUntil, but gives up after `timeoutMs` milliseconds. Returns true if the predicate became
    /// true, or false if the timeout elapsed first.
    template <typename Predicate>
    bool CooperativeSleepUntilOrTimeout(Predicate predicate, const int timeoutMs, const int pollIntervalMs = 25) {
        const Stopwatch waitStartedAt;
        while (not predicate()) {
            if (waitStartedAt.HasTimePassed(std::chrono::milliseconds{timeoutMs})) {
                return false;
            }
            detail::StepActorsOnceAndWaitCooperatively(pollIntervalMs);
        }
        return true;
    }

    /// Cooperatively waits for a fixed duration
    inline void CooperativeSleepFor(const std::chrono::milliseconds duration) {
        detail::StepActorsOnceAndWaitCooperatively(static_cast<int>(duration.count()));
    }
}
