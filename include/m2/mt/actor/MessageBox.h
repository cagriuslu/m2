#pragma once
#include <m2/mt/ProtectedObject.h>
#include <deque>

namespace m2 {
    /// \brief Synchronized message box used in communicating with an actor.
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
}
