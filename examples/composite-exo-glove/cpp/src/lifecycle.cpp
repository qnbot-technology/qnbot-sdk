#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        qnbot::Sdk sdk(
            example::composite_config(example::parse_port(argc, argv)));
        auto glove = sdk.glove();
        auto exo = sdk.exo();
        auto left_glove_device = glove.device(qnbot::Side::left);
        auto right_glove_device = glove.device(qnbot::Side::right);
        auto exo_device = exo.device();
        sdk.start();
        static_cast<void>(sdk.update());
        const auto left_pose = left_glove_device.pose().latest();
        const auto right_pose = right_glove_device.pose().latest();
        const auto telemetry = exo_device.telemetry().latest();
        std::cout << "started composite device; glove_pose="
                  << (left_pose ? "available" : "waiting") << "/"
                  << (right_pose ? "available" : "waiting")
                  << " exo_telemetry=" << (telemetry ? "available" : "waiting")
                  << '\n';
        sdk.stop();
        std::cout << "stopped composite device\n";
        sdk.close();
        std::cout << "closed SDK\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
