#include "example_cleanup.hpp"
#include "example_support.hpp"

#include <pthread.h>
#include <signal.h>

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <system_error>
#include <thread>

namespace {

struct Options {
    std::string port;
    std::optional<qnbot::Side> side;
    std::string package_id;
    bool validate_only{false};
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--port") {
            options.port = example::require_value(argc, argv, index, argument);
        } else if (argument == "--side") {
            options.side = example::parse_side(
                example::require_value(argc, argv, index, argument));
        } else if (argument == "--package-id") {
            options.package_id =
                example::require_value(argc, argv, index, argument);
        } else if (argument == "--validate") {
            options.validate_only = true;
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (options.port.empty()) throw std::invalid_argument("--port is required");
    if (!options.side) {
        throw std::invalid_argument("--side is required with --port");
    }
    if (options.package_id.empty()) {
        throw std::invalid_argument("--package-id is required");
    }
    return options;
}

qnbot::SdkConfig make_config(const Options& options) {
    auto config = example::serial_config(options.port, *options.side);
    qnbot::TargetConfig target;
    target.type = qnbot::TargetType::hand;
    target.name = "primary";
    target.side = options.side;
    target.source =
        qnbot::DeviceSelector{"glove", std::string("primary"), std::nullopt};
    target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};
    config.targets.push_back(target);
    target.name = "backup";
    config.targets.push_back(std::move(target));
    return config;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
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

        qnbot::Sdk sdk(make_config(options));
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        std::mutex output_mutex;
        std::optional<qnbot::Subscription> primary_subscription;
        std::optional<qnbot::Subscription> backup_subscription;
        std::mutex error_mutex;
        std::exception_ptr stop_error;
        std::atomic<bool> runner_finished{false};
        std::thread coordinator;
        try {
            glove.emplace(sdk.glove());
            if (!options.validate_only) {
                glove->connect();
                auto device = glove->device("primary");
                auto primary = device.output("primary");
                auto backup = device.output("backup");
                primary_subscription.emplace(primary.subscribe(
                    [&output_mutex](
                        const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                        std::lock_guard<std::mutex> lock(output_mutex);
                        std::cout << "primary ";
                        example::print_output(sample);
                    }));
                backup_subscription.emplace(backup.subscribe(
                    [&output_mutex](
                        const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                        std::lock_guard<std::mutex> lock(output_mutex);
                        std::cout << "backup ";
                        example::print_output(sample);
                    }));
                glove->start();
                coordinator = std::thread([&] {
                    for (;;) {
                        int received = 0;
                        const int wait_error = sigwait(&wait_set, &received);
                        if (wait_error != 0) {
                            std::lock_guard<std::mutex> lock(error_mutex);
                            stop_error =
                                std::make_exception_ptr(std::system_error(
                                    wait_error, std::generic_category(),
                                    "failed to wait for stop signal"));
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
                std::cout << "ready; press Ctrl+C to stop" << std::endl;
                glove->run_forever();
                std::cout << "multiple outputs stopped\n";
            }
        } catch (...) {
            cleanup.capture_current("multiple outputs");
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
        if (options.validate_only) {
            std::cout << "multiple outputs configuration valid\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
