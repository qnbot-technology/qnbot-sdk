#include "example_support.hpp"

#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::foreground);
        qnbot::Sdk sdk(example::runtime_config(options));
        auto glove = sdk.glove();

        auto output = glove.device("primary").output(options.target_name);
        const auto subscription = output.subscribe(example::print_output);
        glove.start();
        example::print_health("running", glove.health());

        std::cout << "running; press Ctrl+C to stop" << std::endl;
        glove.run_forever();
        static_cast<void>(subscription);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
