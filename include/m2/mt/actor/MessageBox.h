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
        const T* PeekMessage() const {
            const T* frontMsg{};
            _protectedQueue.Read([&frontMsg](const std::deque<T>& queue) {
                if (not queue.empty()) {
                    frontMsg = &queue.front();
                }
            });
            return frontMsg;
        }
        /// Waits until the condition is true for any message in the queue. Messages aren't popped.
        void WaitMessage(const std::function<bool(const T&)>& condition) const {
            _protectedQueue.Read([](const std::deque<T>&) {}, [&](const std::deque<T>& queue) -> bool {
                return std::any_of(queue.begin(), queue.end(), condition);
            });
        }
        /// Tries to pop one message from the queue. Does not block.
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
        /// Pops up to nMaxMessages messages.
        void PopMessages(const std::function<void(const T&)>& handler, const int nMaxMessages = -1) {
            std::optional<T> msg;
            if (nMaxMessages < 0) {
                while (PopMessage(msg)) {
                    if (msg) {
                        handler(*msg);
                    }
                }
            } else {
                m2Repeat(nMaxMessages) {
                    if (PopMessage(msg)) {
                        if (msg) {
                            handler(*msg);
                        }
                    }
                }
            }
        }
        /// Pops up to nMaxMessages messages as long as the handler returns true.
        void PopMessagesUntil(const std::function<bool(const T&)>& handler, const int nMaxMessages = -1) {
            std::optional<T> msg;
            if (nMaxMessages < 0) {
                while (PopMessage(msg)) {
                    if (msg) {
                        if (not handler(*msg)) {
                            return;
                        }
                    }
                }
            } else {
                m2Repeat(nMaxMessages) {
                    if (PopMessage(msg)) {
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
        sendBox.PushMessage(std::move(msgToSend));
        // Wait for answer that we're interested in
        recvBox.WaitMessage(isResponseInteresting);
        // Fetch returning messages
        recvBox.PopMessagesUntil([&](const TRecv& msg) -> bool {
            if (isResponseInteresting(msg)) {
                interestingResponseHandler(msg);
                return false; // Stop
            }
            uninterestingResponseHandler(msg);
            return true; // Continue
        });
    }
}
