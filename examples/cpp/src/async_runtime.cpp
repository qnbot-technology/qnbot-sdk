#include <qnbot/coro.hpp>

#include "example_coroutine_task.hpp"
#include "example_support.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <utility>

namespace {

struct Options {
    std::string port;
    std::optional<qnbot::Side> side;
    std::uint64_t samples{10};
    std::string target_name{example::default_target_name};
    std::string package_id;
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
        } else if (argument == "--samples") {
            options.samples = example::parse_positive_count(
                example::require_value(argc, argv, index, argument), argument);
        } else if (argument == "--target-name") {
            options.target_name =
                example::require_value(argc, argv, index, argument);
        } else if (argument == "--package-id") {
            options.package_id =
                example::require_value(argc, argv, index, argument);
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
    example::SerialOptions runtime;
    runtime.port = options.port;
    runtime.side = options.side;
    runtime.target_name = options.target_name;
    runtime.package_id = options.package_id;
    return example::runtime_config(runtime);
}

example::Task<qnbot::Sample<qnbot::HandJointCommand>> wait_for_output(
    qnbot::ReadChannel<qnbot::HandJointCommand> channel,
    std::shared_ptr<qnbot::coro::DeferredExecutor> executor,
    std::stop_token stop_token) {
    co_return co_await qnbot::coro::next(channel.next(), std::move(executor),
                                         stop_token);
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        qnbot::Sdk sdk(make_config(options));
        auto glove = sdk.glove();
        auto output = glove.device("primary").output(options.target_name);
        auto executor = std::make_shared<example::QueueExecutor>();

        glove.start();
        glove.run_background();
        for (std::uint64_t index = 0; index < options.samples; ++index) {
            auto pending = wait_for_output(output, executor, std::stop_token{});
            example::print_output(pending.get());
        }

        glove.request_stop();
        glove.join();
        glove.close();
        std::cout << "async runtime complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
