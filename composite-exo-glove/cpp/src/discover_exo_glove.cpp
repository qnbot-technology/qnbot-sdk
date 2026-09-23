#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
    try {
        qnbot::Sdk sdk(example::composite_config());
        auto glove_domain = sdk.glove();
        auto exo_domain = sdk.exo();
        glove_domain.start();
        exo_domain.start();
        auto glove = glove_domain.device("glove");
        auto exo = exo_domain.device("exo");
        try {
            example::print_glove_info(glove.get_device_info());
        } catch (const std::exception& error) {
            std::cerr << "Glove discovery error: " << error.what() << '\n';
        }
        try {
            example::print_exo_info(exo.get_device_info());
        } catch (const std::exception& error) {
            std::cerr << "Exo discovery error: " << error.what() << '\n';
        }
        glove_domain.stop();
        exo_domain.stop();
        glove_domain.close();
        exo_domain.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
