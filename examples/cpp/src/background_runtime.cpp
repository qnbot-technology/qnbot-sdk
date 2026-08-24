#include "example_support.hpp"
#include "example_cleanup.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <thread>

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::background);
        qnbot::Sdk sdk(example::runtime_config(options));
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        std::optional<qnbot::Subscription> output_subscription;
        bool runner_started = false;
        try {
            glove.emplace(sdk.glove());
            if (!options.validate_only) {
                glove->connect();
                auto output =
                    glove->device("primary").output(options.target_name);
                output_subscription.emplace(
                    output.subscribe(example::print_output));
                glove->start();
                glove->run_background();
                runner_started = true;
                std::this_thread::sleep_for(
                    std::chrono::duration<double>(options.seconds));
                example::print_health("running", glove->health());
                glove->request_stop();
                glove->join();
                runner_started = false;
                example::print_health("stopped lifecycle", glove->health());
            }
        } catch (...) {
            cleanup.capture_current("background runtime");
        }

        if (runner_started && glove) {
            cleanup.run("glove.request_stop()", [&] { glove->request_stop(); });
            cleanup.run("glove.join()", [&] { glove->join(); });
        }
        if (glove) {
            cleanup.run("glove.close()", [&] { glove->close(); });
        }
        cleanup.run("sdk.close()", [&] { sdk.close(); });
        cleanup.rethrow_if_failed();
        if (options.validate_only) {
            std::cout << "background runtime configuration valid\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
