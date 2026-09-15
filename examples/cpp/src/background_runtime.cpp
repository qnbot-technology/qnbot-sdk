#include "example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    try {
        const auto options = example::parse_serial_options(
            argc, argv, example::SerialExample::background);
        qnbot::Sdk sdk(example::runtime_config(options));
        auto glove = sdk.glove();

        auto output = glove.device("primary").output(options.target_name);
        const auto subscription = output.subscribe(example::print_output);
        glove.start();
        glove.run_background();
        std::this_thread::sleep_for(
            std::chrono::duration<double>(options.seconds));
        example::print_health("running", glove.health());

        glove.request_stop();
        glove.join();
        glove.close();
        static_cast<void>(subscription);
        std::cout << "background runtime complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
