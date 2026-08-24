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
            argc, argv, example::SerialExample::haptics);
        qnbot::Sdk sdk(example::serial_config(options.port, *options.side));
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        std::optional<qnbot::StateChannel<qnbot::Haptics>> haptics;
        bool haptics_set = false;
        try {
            glove.emplace(sdk.glove());
            if (!options.validate_only) {
                glove->connect();
                haptics.emplace(glove->device("primary").haptics());
                glove->start();
                haptics->set(qnbot::Haptics{{
                    {qnbot::GloveFinger::thumb, 80},
                    {qnbot::GloveFinger::index, 40},
                }});
                haptics_set = true;
                if (const auto latest = haptics->latest()) {
                    std::cout << "haptics sequence=" << latest->sequence
                              << '\n';
                }
                std::this_thread::sleep_for(
                    std::chrono::duration<double>(options.hold));
                haptics->clear();
                haptics_set = false;
            }
        } catch (...) {
            cleanup.capture_current("haptics");
        }

        if (haptics_set && haptics) {
            cleanup.run("haptics.clear()", [&] { haptics->clear(); });
        }
        if (glove) {
            cleanup.run("glove.close()", [&] { glove->close(); });
        }
        cleanup.run("sdk.close()", [&] { sdk.close(); });
        cleanup.rethrow_if_failed();
        if (options.validate_only) {
            std::cout << "haptics arguments valid\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
