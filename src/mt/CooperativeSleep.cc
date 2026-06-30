#include <m2/mt/CooperativeSleep.h>
#include <m2/mt/CooperativeActorScheduler.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif
#include <thread>

void m2::detail::StepActorsOnceAndWaitCooperatively(const int waitMs) {
#if defined(__EMSCRIPTEN__)
    // Under Asyncify emscripten_sleep suspends and later resumes the whole C++ stack. This is necessary to yield the
    // main thread back to the browser so that the graphics can be drawn.
    StepCooperativeActors();
    emscripten_sleep(static_cast<unsigned int>(waitMs < 0 ? 0 : waitMs));
#else
    // Native actors progress on their own, so we only sleep.
    std::this_thread::sleep_for(std::chrono::milliseconds{waitMs});
#endif
}
