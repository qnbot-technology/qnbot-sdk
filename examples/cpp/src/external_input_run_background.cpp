#include <qnbot/glove.hpp>

#include "example_cleanup.hpp"
#include "external_input_support.hpp"
#include "external_input_sync.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <thread>
#include <unordered_set>

int main(int argc, char** argv) {
    try {
        const auto package_id =
            external_input_example::parse_package_id(argc, argv);
        qnbot::Sdk sdk(external_input_example::make_config(package_id));
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        example::PoseCallbackState pose_callback;
        example::PoseCallbackState output_callback;
        std::optional<qnbot::Subscription> pose_subscription;
        std::optional<qnbot::Subscription> output_subscription;
        bool runner_started = false;
        try {
            glove.emplace(sdk.glove());
            glove->connect();
            auto device = glove->device("external");
            auto pose = device.pose();
            auto output = device.output("hand");
            auto calibration_progress = device.calibration_progress("hand");
            auto calibration_control = device.calibration_control("hand");
            std::unordered_set<std::string> confirmed_request_ids;

            pose_subscription.emplace(pose.subscribe(
                [&](const qnbot::Sample<qnbot::GlovePose>& sample) {
                    pose_callback.notify(sample.sequence);
                }));
            output_subscription.emplace(output.subscribe(
                [&](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                    example::print_output(sample);
                    output_callback.notify(sample.sequence);
                }));

            device.start();
            glove->run_background();
            runner_started = true;

            const auto deadline =
                std::chrono::steady_clock::now() + std::chrono::seconds(60);
            std::uint64_t step = 1;
            while (!output.latest() &&
                   std::chrono::steady_clock::now() < deadline) {
                const auto pushed =
                    device.push_frame(external_input_example::frame(step));
                if (step == 1)
                    std::cout << "pushed pose sequence="
                              << pushed.meta.sequence.value_or(0)
                              << '\n';
                external_input_example::confirm_calibration(
                    calibration_progress, calibration_control,
                    confirmed_request_ids);
                ++step;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            const auto latest_pose = pose.latest();
            const auto latest_output = output.latest();
            if (!latest_pose || !latest_output)
                throw std::runtime_error(
                    "external input did not publish retargeting output");
            if (!pose_callback.wait_until(deadline) ||
                !output_callback.wait_until(deadline))
                throw std::runtime_error(
                    "external input callbacks did not receive data");
            std::cout << "latest pose sequence=" << latest_pose->sequence
                      << " latest retargeting output sequence="
                      << latest_output->sequence << '\n';
            glove->request_stop();
            glove->join();
            runner_started = false;
        } catch (...) {
            cleanup.capture_current("external background input");
        }

        if (runner_started && glove) {
            cleanup.run("glove.request_stop()",
                        [&] { glove->request_stop(); });
            cleanup.run("glove.join()", [&] { glove->join(); });
        }
        if (glove)
            cleanup.run("glove.close()", [&] { glove->close(); });
        cleanup.run("sdk.close()", [&] { sdk.close(); });
        cleanup.rethrow_if_failed();
        std::cout << "external background input complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
