#include "exo_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <utility>

int main() {
    try {
        qnbot::SdkConfig config;
        config.devices = {qnbot::ExoConfig{}};

        qnbot::Sdk sdk(std::move(config));
        auto exo = sdk.exo();
        auto device = exo.device();
        auto telemetry = device.telemetry();
        const auto subscription = telemetry.subscribe(
            [](const qnbot::Sample<qnbot::ExoTelemetry>& sample) {
                std::cout << "telemetry sequence=" << sample.sequence
                          << " left_arm=";
                example::print_values(
                    sample.value.payload.left_arm.joint_positions_rad);
                std::cout << '\n';
            });

        exo.start();
        std::cout << "running; press Ctrl+C to stop" << std::endl;
        exo.run_forever();
        static_cast<void>(subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
