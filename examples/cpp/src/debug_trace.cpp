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
    std::optional<std::string> log_dir;
    std::uint32_t sample_rate{10};
    qnbot::DebugDetail detail{qnbot::DebugDetail::summary};
    std::string target_name{example::default_target_name};
    std::string package_id;
    bool validate_only{false};
};

qnbot::DebugDetail parse_detail(const std::string& text) {
    if (text == "summary") return qnbot::DebugDetail::summary;
    if (text == "full") return qnbot::DebugDetail::full;
    if (text == "raw") return qnbot::DebugDetail::raw;
    throw std::invalid_argument("--detail must be summary, full, or raw");
}

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--port") {
            options.port = example::require_value(argc, argv, index, argument);
        } else if (argument == "--side") {
            options.side = example::parse_side(
                example::require_value(argc, argv, index, argument));
        } else if (argument == "--log-dir") {
            options.log_dir =
                example::require_value(argc, argv, index, argument);
        } else if (argument == "--sample-rate") {
            options.sample_rate =
                static_cast<std::uint32_t>(example::parse_positive_count(
                    example::require_value(argc, argv, index, argument),
                    argument));
        } else if (argument == "--detail") {
            options.detail = parse_detail(
                example::require_value(argc, argv, index, argument));
        } else if (argument == "--target-name") {
            options.target_name =
                example::require_value(argc, argv, index, argument);
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
    target.name = options.target_name;
    target.side = options.side;
    target.source =
        qnbot::DeviceSelector{"glove", std::string("primary"), std::nullopt};
    target.algorithms = {qnbot::TargetAlgorithm{options.package_id}};
    config.targets.push_back(std::move(target));
    config.debug = qnbot::DebugConfig{
        options.log_dir,
        {"serial", "glove", "calibration", "retargeting", "output", "buffer"},
        options.detail,
        options.sample_rate};
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
        std::optional<qnbot::Subscription> pose_subscription;
        std::optional<qnbot::Subscription> output_subscription;
        std::mutex error_mutex;
        std::exception_ptr stop_error;
        std::atomic<bool> runner_finished{false};
        std::thread coordinator;
        try {
            glove.emplace(sdk.glove());
            if (!options.validate_only) {
                glove->connect();
                auto device = glove->device("primary");
                auto pose = device.pose();
                auto output = device.output(options.target_name);
                pose_subscription.emplace(pose.subscribe(
                    [](const qnbot::Sample<qnbot::GlovePose>& sample) {
                        std::cout << "pose sequence=" << sample.sequence
                                  << '\n';
                    }));
                output_subscription.emplace(
                    output.subscribe(example::print_output));
                glove->start();
                std::cout << "debug traces: "
                          << (options.log_dir ? *options.log_dir
                                              : "platform default")
                          << '\n';
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
                if (const auto latest = pose.latest()) {
                    std::cout << "latest pose sequence=" << latest->sequence
                              << '\n';
                }
                auto& domain = *glove;
                example::print_health(domain.health());
            }
        } catch (...) {
            cleanup.capture_current("debug trace");
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
            std::cout << "debug trace configuration valid\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
