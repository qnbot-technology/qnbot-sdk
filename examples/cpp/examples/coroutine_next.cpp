#include <qnbot/coro.hpp>

#include "example_cleanup.hpp"
#include "example_coroutine_task.hpp"
#include "example_support.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <stop_token>
#include <system_error>
#include <utility>

namespace {

qnbot::GloveNodePose node(double x) {
    return {{x, 0.0, 0.0}, {0.0, 0.0, 0.0, 1.0}};
}

qnbot::ExternalGloveFrame frame() {
    return {node(0.01), node(0.02), node(0.03), node(0.04), node(0.05), 0.01};
}

example::Task<qnbot::Sample<qnbot::GlovePose>>
wait_for_pose_impl(qnbot::ReadChannel<qnbot::GlovePose> channel,
                   std::shared_ptr<qnbot::coro::DeferredExecutor> executor,
                   std::stop_token stop_token = {}) {
    co_return co_await qnbot::coro::next(channel.next(), std::move(executor),
                                         stop_token);
}

example::Task<qnbot::Sample<qnbot::GlovePose>>
wait_for_pose(qnbot::ReadChannel<qnbot::GlovePose> channel,
              const std::shared_ptr<example::QueueExecutor>& executor,
              std::stop_token stop_token = {}) {
    const auto dispatch = example::make_task_dispatch(executor);
    return example::bind_task_dispatch(
        wait_for_pose_impl(std::move(channel), dispatch.executor, stop_token),
        dispatch);
}

qnbot::SdkConfig make_config() {
    qnbot::GloveConfig glove{qnbot::Side::right, qnbot::ExternalConnection{}};
    qnbot::SdkConfig config;
    config.devices.push_back(glove);
    return config;
}

} // namespace

int main(int argc, char** argv) {
    try {
        example::validate_no_options(argc, argv);
        qnbot::Sdk sdk(make_config());
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        std::shared_ptr<example::QueueExecutor> executor;
        std::stop_source pending_stop;
        std::optional<example::Task<qnbot::Sample<qnbot::GlovePose>>> pending;
        try {
            glove.emplace(sdk.glove());
            glove->connect();
            auto device = glove->device();
            auto pose = device.pose();
            executor = std::make_shared<example::QueueExecutor>();

            device.start();

            std::stop_source stop_source;
            auto cancelled =
                wait_for_pose(pose, executor, stop_source.get_token());
            stop_source.request_stop();
            try {
                static_cast<void>(cancelled.get());
                throw std::runtime_error(
                    "cancelled coroutine unexpectedly completed");
            } catch (const std::system_error& error) {
                if (error.code() != std::errc::operation_canceled) throw;
            }

            pending.emplace(
                wait_for_pose(pose, executor, pending_stop.get_token()));
            static_cast<void>(device.push_frame(frame()));
            const auto sample = pending->get();
            pending.reset();
            std::cout << "co_await pose sequence=" << sample.sequence << '\n';
        } catch (...) {
            cleanup.capture_current("coroutine next");
        }

        pending_stop.request_stop();
        cleanup.run("pending coroutine teardown", [&] { pending.reset(); });
        if (glove) {
            cleanup.run("glove.close()", [&] { glove->close(); });
        }
        cleanup.run("sdk.close()", [&] { sdk.close(); });
        cleanup.run("executor teardown", [&] { executor.reset(); });
        cleanup.rethrow_if_failed();
        std::cout << "coroutine next complete\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
