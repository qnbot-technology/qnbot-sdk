#include "example_support.hpp"
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

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::foreground);
        if (options.validate_only) {
            std::cout << "foreground runtime configuration valid\n";
            return EXIT_SUCCESS;
        }
        sigset_t wait_set;
        if (!options.validate_only) {
            sigemptyset(&wait_set);
            sigaddset(&wait_set, SIGINT);
            sigaddset(&wait_set, SIGUSR1);
            if (pthread_sigmask(SIG_BLOCK, &wait_set, nullptr) != 0) {
                throw std::runtime_error(
                    "failed to block process stop signals");
            }
        }

        qnbot::Sdk sdk(example::runtime_config(options));
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        std::optional<qnbot::Subscription> output_subscription;
        std::mutex error_mutex;
        std::exception_ptr stop_error;
        std::atomic<bool> runner_finished{false};
        std::thread coordinator;
        try {
            glove.emplace(sdk.glove());
            if (!options.validate_only) {
                glove->connect();
                auto output =
                    glove->device("primary").output(options.target_name);
                output_subscription.emplace(
                    output.subscribe(example::print_output));
                glove->start();
                example::print_health("running", glove->health());

                coordinator = std::thread([&] {
                    for (;;) {
                        int received = 0;
                        const int wait_error = sigwait(&wait_set, &received);
                        if (wait_error != 0) {
                            std::lock_guard<std::mutex> lock(error_mutex);
                            stop_error =
                                std::make_exception_ptr(std::system_error(
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
                example::print_health("stopped lifecycle", glove->health());
            }
        } catch (...) {
            cleanup.capture_current("foreground runtime");
        }

        runner_finished.store(true);
        if (coordinator.joinable()) {
            pthread_kill(coordinator.native_handle(), SIGUSR1);
            cleanup.run("signal coordinator join()",
                        [&] { coordinator.join(); });
            {
                std::lock_guard<std::mutex> lock(error_mutex);
                cleanup.capture("glove.request_stop()", stop_error);
            }
        }
        if (glove) {
            cleanup.run("glove.close()", [&] { glove->close(); });
        }
        cleanup.run("sdk.close()", [&] { sdk.close(); });
        cleanup.rethrow_if_failed();

        if (options.validate_only) {
            std::cout << "foreground runtime configuration valid\n";
        } else {
            std::cout << "foreground runtime complete\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
