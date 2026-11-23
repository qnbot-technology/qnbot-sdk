#include "exo_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <string>

namespace {

void print_imu(const std::string& label,
               const std::optional<qnbot::ExoImuState>& imu) {
    if (!imu) return;
    std::cout << label << " acceleration=";
    example::print_values(imu->acceleration_mps2);
    std::cout << " angular_velocity=";
    example::print_values(imu->angular_velocity_rad_s);
    std::cout << " orientation=";
    example::print_values(imu->orientation_xyzw);
    std::cout << '\n';
}

} // namespace

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
        if (!device.get_device_info().capabilities.imus) {
            std::cout << "this device reports no IMU" << std::endl;
            exo.close();
            return EXIT_SUCCESS;
        }

        const auto subscription = device.telemetry().subscribe(
            [](const qnbot::Sample<qnbot::ExoTelemetry>& sample) {
                print_imu("torso", sample.value.payload.torso_imu);
                print_imu("extra", sample.value.payload.extra_imu);
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
