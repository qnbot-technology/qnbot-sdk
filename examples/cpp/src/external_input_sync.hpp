#ifndef QNBOT_CPP_EXTERNAL_INPUT_SYNC_HPP
#define QNBOT_CPP_EXTERNAL_INPUT_SYNC_HPP

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace example {

class PoseCallbackState {
public:
    void notify(std::uint64_t sequence) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            sequence_ = sequence;
        }
        ready_.notify_all();
    }

    bool wait_until(std::chrono::steady_clock::time_point deadline) {
        std::unique_lock<std::mutex> lock(mutex_);
        return ready_.wait_until(lock, deadline,
                                 [this] { return sequence_ != 0; });
    }

    std::uint64_t sequence() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return sequence_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable ready_;
    std::uint64_t sequence_{0};
};

} // namespace example

#endif
