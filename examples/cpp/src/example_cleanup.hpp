#ifndef QNBOT_CPP_EXAMPLE_CLEANUP_HPP
#define QNBOT_CPP_EXAMPLE_CLEANUP_HPP

#include <exception>
#include <iostream>
#include <optional>
#include <cstddef>
#include <utility>

namespace example {

class SanitizedCleanupFailure final : public std::exception {
public:
    explicit SanitizedCleanupFailure(const char* operation) noexcept {
        append("qnbot example cleanup failed during ");
        append(operation != nullptr ? operation : "unknown operation");
        append(" (details redacted)");
    }

    const char* what() const noexcept override { return message_; }

private:
    void append(const char* text) noexcept {
        if (text == nullptr) return;
        while (*text != '\0' && size_ + 1 < sizeof(message_)) {
            message_[size_++] = *text++;
        }
        message_[size_] = '\0';
    }

    char message_[192]{};
    std::size_t size_{0};
};

class Cleanup {
public:
    void capture_current(const char* operation) noexcept {
        capture(operation, std::current_exception());
    }

    void capture(const char* operation, std::exception_ptr error) noexcept {
        if (!error) return;
        if (!failure_ && !redacted_failure_) {
            failure_ = std::move(error);
            return;
        }
        try {
            try {
                std::rethrow_exception(error);
            } catch (const std::exception& secondary) {
                std::cerr << "qnbot example secondary failure during "
                          << operation << ": " << secondary.what() << '\n';
            } catch (...) {
                std::cerr << "qnbot example secondary failure during "
                          << operation << ": unknown exception\n";
            }
        } catch (...) {
            // Secondary reporting must never interrupt the remaining cleanup.
        }
    }

    template <typename Action>
    void run(const char* operation, Action&& action) noexcept {
        try {
            action();
        } catch (...) {
            capture_current(operation);
        }
    }

    template <typename Action>
    bool run_redacted(const char* operation, Action&& action) noexcept {
        try {
            action();
            return true;
        } catch (...) {
            capture_redacted(operation, std::current_exception());
            return false;
        }
    }

    void rethrow_if_failed() const {
        if (failure_) std::rethrow_exception(failure_);
        if (redacted_failure_) throw *redacted_failure_;
    }

private:
    void capture_redacted(const char* operation,
                          std::exception_ptr error) noexcept {
        if (!error) return;
        if (!failure_ && !redacted_failure_) {
            redacted_failure_.emplace(operation);
            return;
        }
        try {
            std::cerr << "qnbot example secondary failure during " << operation
                      << ": redacted exception\n";
        } catch (...) {
            // Secondary reporting must never interrupt the remaining cleanup.
        }
    }

    std::exception_ptr failure_;
    std::optional<SanitizedCleanupFailure> redacted_failure_;
};

template <typename Sdk, typename OnPose, typename RunApplication>
void run_glove_application(Sdk& sdk, OnPose on_pose,
                           RunApplication run_application) {
    using Glove = decltype(sdk.glove());
    using PoseChannel =
        decltype(std::declval<Glove&>().device("primary").pose());
    using Subscription =
        decltype(std::declval<PoseChannel&>().subscribe(on_pose));

    Cleanup cleanup;
    std::optional<Glove> glove;
    std::optional<Subscription> subscription;
    try {
        glove.emplace(sdk.glove());
        glove->connect();
        auto device = glove->device("primary");
        auto pose = device.pose();
        subscription.emplace(pose.subscribe(std::move(on_pose)));
        glove->start();
        run_application(device);
    } catch (...) {
        cleanup.capture_current("application");
    }

    if (glove) {
        cleanup.run("glove.close()", [&] { glove->close(); });
    }
    cleanup.run("sdk.close()", [&] { sdk.close(); });
    cleanup.rethrow_if_failed();
}

} // namespace example

#endif
