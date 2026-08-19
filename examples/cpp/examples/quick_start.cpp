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

namespace {

constexpr const char* retargeting_algorithm_id =
    "qnbot.hand.glove_to_hand.retargeting";

} // namespace

int main() {
    try {
        sigset_t wait_set;
        sigemptyset(&wait_set);
        sigaddset(&wait_set, SIGINT);
        sigaddset(&wait_set, SIGUSR1);
        if (pthread_sigmask(SIG_BLOCK, &wait_set, nullptr) != 0) {
            throw std::runtime_error("failed to block process stop signals");
        }

        qnbot::TargetConfig target;
        target.algorithms = {qnbot::TargetAlgorithm{retargeting_algorithm_id}};

        qnbot::SdkConfig config;
        config.devices = {qnbot::GloveConfig{}};
        config.targets = {std::move(target)};
        qnbot::Sdk sdk(config);
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        std::optional<qnbot::Subscription> pose_subscription;
        std::optional<qnbot::Subscription> output_subscription;
        std::mutex error_mutex;
        std::exception_ptr stop_error;
        std::atomic<bool> runner_finished{false};
        std::thread coordinator;
        try {
            glove.emplace(sdk.glove());
            glove->connect();
            auto device = glove->device();
            auto pose = device.pose();
            auto output = device.output();
            pose_subscription.emplace(pose.subscribe(
                [](const qnbot::Sample<qnbot::GlovePose>& sample) {
                    std::cout
                        << "pose sequence=" << sample.sequence << " fingertips="
                        << sample.value.payload.fingertip_local.size() << '\n';
                }));
            output_subscription.emplace(output.subscribe(
                [](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                    std::cout << "retargeting sequence=" << sample.sequence
                              << " joints=" << sample.value.joints.size()
                              << '\n';
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
            cleanup.capture_current("quick start");
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
        std::cerr << "qnbot quick start failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
