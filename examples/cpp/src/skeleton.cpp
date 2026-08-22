#include <qnbot/glove.hpp>

#include "example_cleanup.hpp"

#include <pthread.h>
#include <signal.h>

#include <atomic>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <mutex>
#include <optional>
#include <system_error>
#include <thread>

int main() {
    try {
        sigset_t wait_set;
        sigemptyset(&wait_set);
        sigaddset(&wait_set, SIGINT);
        sigaddset(&wait_set, SIGUSR1);
        if (pthread_sigmask(SIG_BLOCK, &wait_set, nullptr) != 0) {
            throw std::runtime_error("failed to block process stop signals");
        }

        qnbot::SdkConfig config;
        config.devices = {qnbot::GloveConfig{}};
        qnbot::Sdk sdk(config);
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        std::optional<qnbot::Subscription> skeleton_subscription;
        std::mutex error_mutex;
        std::exception_ptr stop_error;
        std::atomic<bool> runner_finished{false};
        std::thread coordinator;
        try {
            glove.emplace(sdk.glove());
            glove->connect();
            auto device = glove->device();
            auto skeleton = device.skeleton();
            skeleton_subscription.emplace(skeleton.subscribe(
                [](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                    std::cout << "skeleton sequence=" << sample.sequence
                              << " target=" << sample.value.target
                              << " joints={";
                    bool first = true;
                    for (const auto& joint : sample.value.joints) {
                        if (!first) std::cout << ", ";
                        std::cout << joint.first << ": " << joint.second;
                        first = false;
                    }
                    std::cout << "}\n";
                }));
            glove->start();

            coordinator = std::thread([&] {
                for (;;) {
                    int received = 0;
                    const int wait_error = sigwait(&wait_set, &received);
                    if (wait_error != 0) {
                        std::lock_guard<std::mutex> lock(error_mutex);
                        stop_error = std::make_exception_ptr(std::system_error(
                            wait_error, std::generic_category(),
                            "failed to wait for process stop signal"));
                        return;
                    }
                    if (received == SIGUSR1) {
                        if (runner_finished.load()) return;
                        continue;
                    }
                    if (received != SIGINT) continue;
                    try {
                        glove->request_stop();
                    } catch (...) {
                        std::lock_guard<std::mutex> lock(error_mutex);
                        stop_error = std::current_exception();
                    }
                    return;
                }
            });
            std::cout << "running; press Ctrl+C to stop" << std::endl;
            glove->run_forever();
        } catch (...) {
            cleanup.capture_current("skeleton");
        }

        runner_finished.store(true);
        if (coordinator.joinable()) {
            pthread_kill(coordinator.native_handle(), SIGUSR1);
            cleanup.run("signal coordinator join()",
                        [&] { coordinator.join(); });
            std::lock_guard<std::mutex> lock(error_mutex);
            cleanup.capture("glove.request_stop()", stop_error);
        }
        if (glove) cleanup.run("glove.close()", [&] { glove->close(); });
        cleanup.run("sdk.close()", [&] { sdk.close(); });
        cleanup.rethrow_if_failed();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "qnbot skeleton example failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
