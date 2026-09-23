#include "composite_example_support.hpp"

#include <cstdlib>
#include <exception>

int main(int argc, char** argv) {
    try {
        qnbot::Sdk sdk(
            example::composite_config(example::parse_port(argc, argv)));
        auto glove = sdk.glove();
        auto exo = sdk.exo();
        example::print_glove_info(glove.device("glove").get_device_info());
        example::print_exo_info(exo.device("exo").get_device_info());
        glove.close();
        exo.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
