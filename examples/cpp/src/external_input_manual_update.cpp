#include <qnbot/glove.hpp>

#include "external_input_support.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <unordered_set>

int main(int argc, char** argv) {
    try {
        const auto package_id =
            external_input_example::parse_package_id(argc, argv);
        qnbot::Sdk sdk(external_input_example::make_config(package_id));
        auto glove = sdk.glove();

        auto device = glove.device("external");
        auto pose = device.pose();
        auto output = device.output("hand");
        auto capture_progress = device.capture_progress();
        auto calibration_progress = device.calibration_progress("hand");
        auto capture_control = device.capture_control();
        std::unordered_set<std::string> confirmed_request_ids;
        device.start();

        const auto deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(60);
        std::uint64_t step = 1;
        while (!output.latest() &&
               std::chrono::steady_clock::now() < deadline) {
            const auto pushed =
                device.push_frame(external_input_example::frame(step));
            if (step == 1) {
                std::cout << "pushed pose sequence="
                          << pushed.meta.sequence.value_or(0) << '\n';
            }
            const auto update = glove.update();
            external_input_example::advance_capture(
                capture_progress, capture_control, confirmed_request_ids);
            external_input_example::check_calibration(calibration_progress);
            ++step;
            if (update.has_next_task())
                static_cast<void>(update.sleep());
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        const auto latest_pose = pose.latest();
        const auto latest_output = output.latest();
        if (!latest_pose || !latest_output) {
            throw std::runtime_error(
                "external input did not publish retargeting output");
        }
        std::cout << "latest pose sequence=" << latest_pose->sequence << '\n';
        example::print_output(*latest_output);
        glove.close();
        std::cout << "external manual input complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
