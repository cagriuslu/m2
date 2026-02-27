#pragma once
#include <m2/mt/ProtectedObject.h>
#include <m2/Meta.h>
#include <deque>
#include <algorithm>

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

        /// Peek the front-most message, if it exists.
        const T* PeekMessage() const {
            const T* frontMsg{};
            _protectedQueue.Read([&frontMsg](const std::deque<T>& queue) {
                if (not queue.empty()) {
                    frontMsg = &queue.front();
                }
            });
            return frontMsg;
        }
        /// Waits until the condition is true for any message in the queue. If timeout is given, and the condition
        /// doesn't hold until the given timeout, returns false. Otherwise, returns true if the a message matching the
        /// condition is encountered. Messages are never popped.
        bool WaitMessage(const std::function<bool(const T&)>& condition, const std::optional<Stopwatch::Duration> timeout = std::nullopt) const {
            return _protectedQueue.WaitUntilAndRead([&](const std::deque<T>& queue) -> bool {
                return std::ranges::any_of(queue, condition);
            }, [](const std::deque<T>&) {}, timeout);
        }
        /// Tries to pop one message from the queue. Does not block.
        bool TryPopMessage(std::optional<T>& out) {
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
        /// Handles up to nMaxMessages messages from the queue. Returns early if the queue becomes empty.
        void TryHandleMessages(const std::function<void(T&)>& handler, const int nMaxMessages = -1) {
            std::optional<T> msg;
            if (nMaxMessages < 0) {
                while (TryPopMessage(msg)) {
                    if (msg) {
                        handler(*msg);
                    }
                }
            } else {
                m2Repeat(nMaxMessages) {
                    if (TryPopMessage(msg)) {
                        if (msg) {
                            handler(*msg);
                        }
                    }
                }
            }
        }
        /// Handles up to nMaxMessages messages from the queue, as long as the handler keeps returning true. Returns
        /// early if the queue becomes empty.
        void TryHandleMessagesUntil(const std::function<bool(T&)>& handler, const int nMaxMessages = -1) {
            std::optional<T> msg;
            if (nMaxMessages < 0) {
                while (TryPopMessage(msg)) {
                    if (msg) {
                        if (not handler(*msg)) {
                            return;
                        }
                    }
                }
            } else {
                m2Repeat(nMaxMessages) {
                    if (TryPopMessage(msg)) {
                        if (msg) {
                            if (not handler(*msg)) {
                                return;
                            }
                        }
                    }
                }
            }
        }
    };

    /// Synchronously send a question, and wait for an answer
    template <typename TSend, typename TRecv>
    void SendQuestionReceiveAnswerSync(MessageBox<TSend>& sendBox, TSend&& msgToSend, MessageBox<TRecv>& recvBox,
            const std::function<bool(const TRecv&)>& isResponseInteresting,
            const std::function<void(const TRecv&)>& interestingResponseHandler,
            const std::function<void(const TRecv&)>& uninterestingResponseHandler) {
        // Send question
        sendBox.PushMessage(std::forward<TSend>(msgToSend));
        // Wait for answer that we're interested in
        recvBox.WaitMessage(isResponseInteresting);
        // Fetch returning messages
        recvBox.TryHandleMessagesUntil([&](const TRecv& msg) -> bool {
            if (isResponseInteresting(msg)) {
                interestingResponseHandler(msg);
                return false; // Answer is handled, stop
            }
            uninterestingResponseHandler(msg);
            return true; // Continue
        });
    }
}
