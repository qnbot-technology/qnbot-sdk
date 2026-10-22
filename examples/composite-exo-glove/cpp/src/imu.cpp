#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

namespace {

void print_exo_imu(const qnbot::ExoTelemetry& telemetry) {
    if (telemetry.payload.torso_imu) {
        std::cout << "exo torso acceleration=";
        example::print_values(telemetry.payload.torso_imu->acceleration_mps2);
        std::cout << '\n';
    }
    if (telemetry.payload.extra_imu) {
        std::cout << "exo extra acceleration=";
        example::print_values(telemetry.payload.extra_imu->acceleration_mps2);
        std::cout << '\n';
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        qnbot::Sdk sdk(
            example::composite_config(example::parse_port(argc, argv)));
        auto exo = sdk.exo();
        auto exo_device = exo.device();
        std::cout
            << "Glove raw IMU is unavailable on this shared-device connection; "
               "reading supported Exo IMU only\n";
        if (!exo_device.get_device_info().capabilities.imus) {
            std::cout << "this device reports no Exo IMU\n";
            sdk.close();
            return EXIT_SUCCESS;
        }
        const auto exo_subscription = exo_device.telemetry().subscribe(
            [](const qnbot::Sample<qnbot::ExoTelemetry>& sample) {
                print_exo_imu(sample.value);
            });

        sdk.start();
        std::cout << "reading Exo IMU data; press Ctrl+C to stop\n";
        // The root SDK lifecycle drives both member domains.
        sdk.run_forever();
        sdk.stop();
        sdk.close();
        static_cast<void>(exo_subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
