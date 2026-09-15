#include "example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::haptics);
        qnbot::Sdk sdk(example::serial_config(options.port, *options.side));
        auto glove = sdk.glove();
        glove.start();

        auto haptics = glove.device("primary").haptics();
        haptics.set(qnbot::Haptics{{
            {qnbot::GloveFinger::thumb, 80},
            {qnbot::GloveFinger::index, 40},
        }});
        if (const auto latest = haptics.latest()) {
            std::cout << "haptics sequence=" << latest->sequence << '\n';
        }
        std::this_thread::sleep_for(
            std::chrono::duration<double>(options.hold));
        haptics.clear();
        std::cout << "haptics cleared\n";
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
