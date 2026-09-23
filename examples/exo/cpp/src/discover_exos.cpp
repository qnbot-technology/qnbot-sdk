#include "exo_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
    try {
        for (const auto& device : qnbot::discover_exos()) {
            if (device.error) {
                std::cout << "port=" << device.port
                          << " error=" << device.error->message << '\n';
                continue;
            }
            if (!device.device_info) continue;
            const auto& info = *device.device_info;
            std::cout << "port=" << device.port
                      << " product=" << static_cast<int>(info.product)
                      << " protocol="
                      << static_cast<int>(info.protocol_version.major) << '.'
                      << static_cast<int>(info.protocol_version.minor);
            if (info.serial_number) std::cout << " sn=" << *info.serial_number;
            if (info.hardware_name)
                std::cout << " name=" << *info.hardware_name;
            std::cout << '\n';
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
