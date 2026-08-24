#include "example_support.hpp"
#include "example_cleanup.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::manual);
        qnbot::Sdk sdk(example::runtime_config(options));
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        try {
            glove.emplace(sdk.glove());
            if (!options.validate_only) {
                glove->connect();
                auto output =
                    glove->device("primary").output(options.target_name);
                glove->start();
                for (std::uint64_t index = 0; index < options.updates;
                     ++index) {
                    const auto update = glove->update();
                    std::cout << "tick=" << update.tick()
                              << " tasks=" << update.ran_task_count() << '\n';
                    if (update.has_next_task())
                        static_cast<void>(update.sleep());
                    if (const auto latest = output.latest()) {
                        example::print_output(*latest);
                    }
                }
                example::print_health(glove->health());
            }
        } catch (...) {
            cleanup.capture_current("manual runtime");
        }

        if (glove) {
            cleanup.run("glove.close()", [&] { glove->close(); });
        }
        cleanup.run("sdk.close()", [&] { sdk.close(); });
        cleanup.rethrow_if_failed();
        if (options.validate_only) {
            std::cout << "manual runtime configuration valid\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
