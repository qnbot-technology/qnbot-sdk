#include "exo_example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv) {
    try {
        std::string port;
        std::string name = "primary";
        std::uint8_t left = 60;
        std::uint8_t right = 60;
        double hold = 1.0;
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            if (argument == "--port") {
                port = example::require_value(argc, argv, index, argument);
            } else if (argument == "--name") {
                name = example::require_value(argc, argv, index, argument);
            } else if (argument == "--left") {
                left = example::parse_strength(
                    example::require_value(argc, argv, index, argument),
                    argument);
            } else if (argument == "--right") {
                right = example::parse_strength(
                    example::require_value(argc, argv, index, argument),
                    argument);
            } else if (argument == "--hold") {
                hold = example::parse_seconds(
                    example::require_value(argc, argv, index, argument));
            } else {
                throw std::invalid_argument("unknown argument: " + argument);
            }
        }
        if (port.empty()) throw std::invalid_argument("--port is required");

        qnbot::Sdk sdk(example::serial_config(port, name));
        auto exo = sdk.exo();
        auto device = exo.device(name);
        if (!device.get_device_info().capabilities.haptics) {
            std::cout << "this device reports no haptics" << std::endl;
            exo.close();
            return EXIT_SUCCESS;
        }

        exo.start();
        auto haptics = device.haptics();
        qnbot::ExoHaptics value;
        value.left = left;
        value.right = right;
        haptics.set(value);
        std::cout << "set left=" << static_cast<int>(left)
                  << " right=" << static_cast<int>(right) << std::endl;
        std::this_thread::sleep_for(std::chrono::duration<double>(hold));

        haptics.clear();
        std::cout << "haptics cleared" << std::endl;
        exo.stop();
        exo.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
