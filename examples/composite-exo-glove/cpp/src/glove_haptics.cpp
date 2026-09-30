#include "composite_example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    try {
        qnbot::Sdk sdk(
            example::composite_config(example::parse_port(argc, argv)));
        auto glove = sdk.glove();
        auto left_device = glove.device(qnbot::Side::left);
        auto right_device = glove.device(qnbot::Side::right);
        sdk.start();

        const qnbot::GloveHaptics value{{
            {qnbot::GloveFinger::thumb, 80},
            {qnbot::GloveFinger::index, 40},
        }};
        auto left = left_device.haptics();
        auto right = right_device.haptics();
        left.set(value);
        right.set(value);
        std::cout << "set Glove haptics on left and right Glove\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        left.clear();
        right.clear();
        std::cout << "Glove haptics cleared\n";
        sdk.stop();
        sdk.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
