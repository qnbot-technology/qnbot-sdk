#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        qnbot::Sdk sdk(
            example::composite_config(example::parse_port(argc, argv)));
        auto glove_domain = sdk.glove();
        auto exo_domain = sdk.exo();
        auto glove = glove_domain.device("glove");
        auto exo = exo_domain.device("exo");
        glove_domain.start();
        exo_domain.start();
        static_cast<void>(glove_domain.update());
        static_cast<void>(exo_domain.update());
        const auto pose = glove.pose().latest();
        const auto telemetry = exo.telemetry().latest();
        std::cout << "started composite device; glove_pose="
                  << (pose ? "available" : "waiting")
                  << " exo_telemetry=" << (telemetry ? "available" : "waiting")
                  << '\n';
        glove_domain.stop();
        exo_domain.stop();
        std::cout << "stopped composite device\n";
        glove_domain.close();
        exo_domain.close();
        std::cout << "closed member domains\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
