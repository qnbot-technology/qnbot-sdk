#include "exo_example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

int main(int argc, char** argv) {
    try {
        std::string first_port;
        std::string second_port;
        double seconds = 3.0;
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            if (argument == "--first-port") {
                first_port =
                    example::require_value(argc, argv, index, argument);
            } else if (argument == "--second-port") {
                second_port =
                    example::require_value(argc, argv, index, argument);
            } else if (argument == "--seconds") {
                seconds = example::parse_seconds(
                    example::require_value(argc, argv, index, argument));
            } else {
                throw std::invalid_argument("unknown argument: " + argument);
            }
        }
        if (first_port.empty() || second_port.empty()) {
            throw std::invalid_argument(
                "--first-port and --second-port are required");
        }

        qnbot::ExoConfig first_config;
        first_config.connection.port = first_port;
        first_config.name = "first";
        qnbot::ExoConfig second_config;
        second_config.connection.port = second_port;
        second_config.name = "second";
        qnbot::SdkConfig config;
        config.devices = {first_config, second_config};

        qnbot::Sdk sdk(std::move(config));
        auto exo = sdk.exo();
        auto first = exo.device("first");
        auto second = exo.device("second");
        first.start();
        second.start();
        example::print_health("first", first.health());
        example::print_health("second", second.health());
        std::this_thread::sleep_for(std::chrono::duration<double>(seconds));

        second.stop();
        std::cout << "second device stopped; first remains active" << std::endl;
        example::print_health("first", first.health());
        const auto latest = first.telemetry().latest();
        if (latest) {
            std::cout << "first telemetry sequence=" << latest->sequence
                      << " left_arm=";
            example::print_encoder_counts(
                latest->value.payload.left_arm.encoder_counts);
            std::cout << '\n';
        }

        first.stop();
        first.close();
        second.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
