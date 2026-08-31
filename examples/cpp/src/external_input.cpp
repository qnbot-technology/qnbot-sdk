#include <qnbot/glove.hpp>

#include "example_cleanup.hpp"
#include "example_support.hpp"
#include "external_input_sync.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <system_error>

namespace {

qnbot::GloveNodePose node(double x, double y, double z) {
    return {{x, y, z}, {0.0, 0.0, 0.0, 1.0}};
}

qnbot::ExternalGloveFrame frame(std::uint64_t step) {
    const double offset = static_cast<double>(step) * 0.001;
    return {
        node(0.01 + offset, 0.02, 0.03), node(0.02 + offset, 0.03, 0.04),
        node(0.03 + offset, 0.04, 0.05), node(0.04 + offset, 0.05, 0.06),
        node(0.05 + offset, 0.06, 0.07), static_cast<double>(step) * 0.01,
    };
}

qnbot::SdkConfig make_config() {
    qnbot::GloveConfig glove{qnbot::Side::right, qnbot::ExternalConnection{}};
    glove.name = "external";

    qnbot::TargetConfig target{
        qnbot::TargetType::hand,
        qnbot::DeviceSelector{"glove", std::string("external"), std::nullopt}};
    target.name = "hand";
    target.side = qnbot::Side::right;

    qnbot::SdkConfig config;
    config.devices.push_back(glove);
    config.targets.push_back(target);
    return config;
}

} // namespace

int main(int argc, char** argv) {
    try {
        example::validate_no_options(argc, argv);
        qnbot::Sdk sdk(make_config());
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        example::PoseCallbackState pose_callback;
        example::PoseCallbackState output_callback;
        std::optional<qnbot::Subscription> pose_subscription;
        std::optional<qnbot::Subscription> output_subscription;
        std::optional<qnbot::Subscription> status_subscription;
        bool runner_started = false;
        try {
            glove.emplace(sdk.glove());
            glove->connect();
            auto device = glove->device("external");
            auto pose = device.pose();
            auto status = device.status();
            auto output = device.output("hand");
            auto calibration = device.calibration_progress("hand");

            pose_subscription.emplace(pose.subscribe(
                [&](const qnbot::Sample<qnbot::GlovePose>& sample) {
                    pose_callback.notify(sample.sequence);
                }));
            output_subscription.emplace(output.subscribe(
                [&](const qnbot::Sample<qnbot::HandJointCommand>& sample) {
                    output_callback.notify(sample.sequence);
                }));
            status_subscription.emplace(status.subscribe(
                [](const qnbot::Sample<qnbot::GloveStatus>& sample) {
                    std::cout << "status connected=" << sample.value.connected
                              << " stale=" << sample.value.stale << '\n';
                }));

            device.start();
            glove->run_background();
            runner_started = true;

            auto next_pose = pose.next();
            const auto pushed = device.push_frame(frame(1));
            const auto received = next_pose.get();
            if (received.value.meta.sequence != pushed.meta.sequence) {
                throw std::runtime_error("next() returned a different pose");
            }

            auto cancelled = status.next();
            cancelled.cancel();
            try {
                static_cast<void>(cancelled.get());
                throw std::runtime_error(
                    "cancelled next() unexpectedly completed");
            } catch (const std::system_error& error) {
                if (error.code() != std::errc::operation_canceled) throw;
            }

            for (std::uint64_t step = 2; step <= 4; ++step) {
                static_cast<void>(device.push_frame(frame(step)));
            }

            const auto deadline =
                std::chrono::steady_clock::now() + std::chrono::seconds(2);
            if (!pose_callback.wait_until(deadline)) {
                throw std::runtime_error(
                    "pose subscription did not receive data");
            }
            if (!output_callback.wait_until(deadline)) {
                throw std::runtime_error(
                    "output subscription did not receive data");
            }

            const auto latest_pose = pose.latest();
            const auto latest_status = status.latest();
            const auto latest_output = output.latest();
            if (!latest_pose || !latest_status || !latest_output) {
                throw std::runtime_error(
                    "external input did not publish all expected data");
            }
            const auto callback_sequence = pose_callback.sequence();
            if (callback_sequence == 0) {
                throw std::runtime_error(
                    "pose subscription did not receive data");
            }
            if (calibration.latest()) {
                throw std::runtime_error(
                    "pass-through route reported calibration progress");
            }

            try {
                device.haptics().set(
                    qnbot::Haptics{{{qnbot::GloveFinger::thumb, 20}}});
                throw std::runtime_error(
                    "external input unexpectedly accepted haptics");
            } catch (const qnbot::QnBotError&) {
            }

            const auto health = glove->health();
            std::cout << "pose sequence=" << latest_pose->sequence
                      << " output sequence=" << latest_output->sequence
                      << " callback sequence=" << callback_sequence
                      << " health=" << health.ok << '\n';
            glove->request_stop();
            glove->join();
            runner_started = false;
        } catch (...) {
            cleanup.capture_current("external input");
        }

        if (runner_started && glove) {
            cleanup.run("glove.request_stop()",
                        [&] { glove->request_stop(); });
            cleanup.run("glove.join()", [&] { glove->join(); });
        }
        if (glove) {
            cleanup.run("glove.close()", [&] { glove->close(); });
        }
        cleanup.run("sdk.close()", [&] { sdk.close(); });
        cleanup.rethrow_if_failed();
        std::cout << "external input complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
