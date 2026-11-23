#include "exo_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    try {
        std::string port;
        for (int index = 1; index < argc; ++index) {
            const auto [option, value] =
                example::next_argument(argc, argv, index);
            if (option == "--port")
                port = value;
            else
                throw std::invalid_argument("unknown argument: " + option);
        }
        if (port.empty()) throw std::invalid_argument("--port is required");

        qnbot::Sdk sdk(example::serial_config(port));
        auto exo = sdk.exo();
        auto device = exo.device();
        const auto telemetry_subscription = device.telemetry().subscribe(
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
        const auto status_subscription = device.status().subscribe(
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
        exo.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
