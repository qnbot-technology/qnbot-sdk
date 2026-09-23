#include "exo_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

namespace {

void print_info(const qnbot::ExoDeviceInfo& info) {
    std::cout << "product=" << static_cast<int>(info.product)
              << " protocol=" << static_cast<int>(info.protocol_version.major)
              << '.' << static_cast<int>(info.protocol_version.minor)
              << " firmware=" << static_cast<int>(info.firmware.platform) << '.'
              << static_cast<int>(info.firmware.revision) << '.'
              << static_cast<int>(info.firmware.feature) << '.'
              << static_cast<int>(info.firmware.build);
    if (info.serial_number) std::cout << " sn=" << *info.serial_number;
    if (info.hardware_name) std::cout << " name=" << *info.hardware_name;
    std::cout << '\n';

    const auto& components = info.components;
    std::cout << "components encoders="
              << static_cast<int>(components.encoders.available) << '/'
              << static_cast<int>(components.encoders.total_slots)
              << " handsets=" << static_cast<int>(components.handsets.available)
              << '/' << static_cast<int>(components.handsets.total_slots)
              << " imus=" << static_cast<int>(components.imus.available) << '/'
              << static_cast<int>(components.imus.total_slots) << '\n';

    const auto& capabilities = info.capabilities;
    std::cout << "capabilities telemetry=" << capabilities.telemetry
              << " encoders=" << capabilities.encoders
              << " handsets=" << capabilities.handsets
              << " imus=" << capabilities.imus
              << " haptics=" << capabilities.haptics
              << " wireless=" << capabilities.wireless << '\n';
}

} // namespace

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
        print_info(exo.device(name).get_device_info());
        exo.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
