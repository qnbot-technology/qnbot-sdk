#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        qnbot::Sdk sdk(
            example::composite_config(example::parse_port(argc, argv)));
        auto glove = sdk.glove();
        auto exo = sdk.exo();
        auto left_glove_device = glove.device(qnbot::Side::left);
        auto right_glove_device = glove.device(qnbot::Side::right);
        auto exo_device = exo.device();
        std::cout << "left glove source=" << left_glove_device.source_id()
                  << '\n';
        std::cout << "right glove source=" << right_glove_device.source_id()
                  << '\n';
        example::print_exo_info(exo_device.get_device_info());
        sdk.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
