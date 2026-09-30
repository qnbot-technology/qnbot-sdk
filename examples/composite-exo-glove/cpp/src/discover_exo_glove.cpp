#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
    try {
        qnbot::Sdk sdk(example::composite_config());
        auto glove = sdk.glove();
        auto exo = sdk.exo();
        sdk.start();
        auto left_glove_device = glove.device(qnbot::Side::left);
        auto right_glove_device = glove.device(qnbot::Side::right);
        auto exo_device = exo.device();
        std::cout << "Left Glove discovered source="
                  << left_glove_device.source_id() << '\n';
        std::cout << "Right Glove discovered source="
                  << right_glove_device.source_id() << '\n';
        try {
            example::print_exo_info(exo_device.get_device_info());
        } catch (const std::exception& error) {
            std::cerr << "Exo discovery error: " << error.what() << '\n';
        }
        sdk.stop();
        sdk.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
