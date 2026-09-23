#include "composite_example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    try {
        qnbot::Sdk sdk(
            example::composite_config(example::parse_port(argc, argv)));
        auto exo_domain = sdk.exo();
        auto exo = exo_domain.device("exo");
        if (!exo.get_device_info().capabilities.haptics) {
            std::cout << "Exo device reports no haptics\n";
            exo_domain.close();
            return EXIT_SUCCESS;
        }

        exo_domain.start();
        qnbot::ExoHaptics haptics;
        haptics.left = 60;
        haptics.right = 60;
        auto output = exo.haptics();
        output.set(haptics);
        std::cout << "set Exo haptics left=60 right=60\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        output.clear();
        std::cout << "Exo haptics cleared\n";
        exo_domain.stop();
        exo_domain.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
