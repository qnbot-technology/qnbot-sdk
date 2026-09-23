#include "exo_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    try {
        std::string port;
        std::string name = "primary";
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            if (argument == "--port") {
                port = example::require_value(argc, argv, index, argument);
            } else if (argument == "--name") {
                name = example::require_value(argc, argv, index, argument);
            } else {
                throw std::invalid_argument("unknown argument: " + argument);
            }
        }
        if (port.empty()) throw std::invalid_argument("--port is required");

        qnbot::Sdk sdk(example::serial_config(port, name));
        auto exo = sdk.exo();
        auto device = exo.device(name);
        auto telemetry = device.telemetry();
        auto status = device.status();
        const auto telemetry_subscription = telemetry.subscribe(
            [](const qnbot::Sample<qnbot::ExoTelemetry>& sample) {
                std::cout << "telemetry sequence=" << sample.sequence
                          << " left_arm=";
                example::print_encoder_counts(
                    sample.value.payload.left_arm.encoder_counts);
                std::cout << " right_arm=";
                example::print_encoder_counts(
                    sample.value.payload.right_arm.encoder_counts);
                std::cout << '\n';
            });
        const auto status_subscription = status.subscribe(
            [](const qnbot::Sample<qnbot::DeviceStatus>& sample) {
                std::cout << "status sequence=" << sample.sequence
                          << " connected=" << sample.value.connected
                          << " stale=" << sample.value.stale << '\n';
            });

        exo.start();
        std::cout << "running; press Ctrl+C to stop" << std::endl;
        exo.run_forever();
        static_cast<void>(telemetry_subscription);
        static_cast<void>(status_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
