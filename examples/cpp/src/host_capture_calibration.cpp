#include <qnbot/glove.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr const char* operator_id = "default";
constexpr const char* left_source = "primary-glove-left";
constexpr const char* right_source = "primary-glove-right";
constexpr const char* target_name = "openxr_hand";
constexpr const char* builtin_skeleton_package_id =
    "qnbot_hand.dynamic_openxr_hand";
constexpr auto wait_timeout = std::chrono::minutes(5);

struct Options {
    std::vector<std::string> package_ids;
    bool force{false};
};

struct ConfiguredTarget {
    std::string package_id;
    std::string name;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--package-id") {
            if (++index >= argc)
                throw std::invalid_argument("--package-id requires a value");
            const std::string package_id = argv[index];
            if (std::find(options.package_ids.begin(),
                          options.package_ids.end(),
                          package_id) == options.package_ids.end()) {
                options.package_ids.push_back(package_id);
            }
        } else if (argument == "--force") {
            options.force = true;
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (options.package_ids.empty())
        throw std::invalid_argument("--package-id is required");
    return options;
}

std::vector<ConfiguredTarget>
configured_targets(const std::vector<std::string>& package_ids) {
    const auto ordinary_count = static_cast<std::size_t>(std::count_if(
        package_ids.begin(), package_ids.end(), [](const std::string& value) {
            return value != builtin_skeleton_package_id;
        }));
    std::size_t ordinary_index = 0;
    std::vector<ConfiguredTarget> targets;
    for (const auto& package_id : package_ids) {
        if (package_id == builtin_skeleton_package_id) {
            targets.push_back({package_id, "skeleton"});
            continue;
        }
        ++ordinary_index;
        targets.push_back(
            {package_id, ordinary_count == 1
                             ? target_name
                             : std::string(target_name) + "_" +
                                   std::to_string(ordinary_index)});
    }
    return targets;
}

qnbot::SdkConfig make_config(const std::vector<std::string>& package_ids) {
    qnbot::GloveConfig left;
    left.name = left_source;
    left.side = qnbot::Side::left;
    qnbot::GloveConfig right;
    right.name = right_source;
    right.side = qnbot::Side::right;

    qnbot::SdkConfig config;
    config.devices = {left, right};
    for (const auto& target : configured_targets(package_ids)) {
        if (target.package_id == builtin_skeleton_package_id) continue;
        for (const auto& side_and_source :
             {std::make_pair(qnbot::Side::left, left_source),
              std::make_pair(qnbot::Side::right, right_source)}) {
            qnbot::TargetConfig configured;
            configured.name = target.name;
            configured.side = side_and_source.first;
            configured.source = qnbot::DeviceSelector{
                "glove", std::string(side_and_source.second), std::nullopt};
            configured.algorithms = {qnbot::TargetAlgorithm{target.package_id}};
            config.targets.push_back(std::move(configured));
        }
    }
    return config;
}

std::string select_target(const std::vector<std::string>& package_ids) {
    const auto targets = configured_targets(package_ids);
    if (targets.size() == 1) return targets.front().name;
    for (std::size_t index = 0; index < targets.size(); ++index) {
        std::cout << index + 1 << ": " << targets[index].name << " ("
                  << targets[index].package_id << ")\n";
    }
    while (true) {
        std::cout << "target [1-" << targets.size() << "]: ";
        std::string answer;
        std::getline(std::cin, answer);
        try {
            const auto selected = static_cast<std::size_t>(std::stoul(answer));
            if (selected >= 1 && selected <= targets.size())
                return targets[selected - 1].name;
        } catch (const std::exception&) {
        }
        std::cout << "enter one of the listed target numbers\n";
    }
}

void update_or_wait(qnbot::Glove& glove) {
    const auto update = glove.update();
    if (update.has_next_task()) static_cast<void>(update.sleep());
}

qnbot::CaptureSession capture_side(qnbot::Glove& glove,
                                   const std::vector<std::string>& package_ids,
                                   const std::string& source, qnbot::Side side,
                                   bool force, bool start_runtime) {
    qnbot::CaptureStatusRequest status_request;
    status_request.package_ids = package_ids;
    status_request.operator_id = operator_id;
    status_request.source = source;
    status_request.side = side;
    const auto readiness = glove.capture_status(status_request);
    std::cout << (side == qnbot::Side::left ? "left" : "right")
              << ": capture readiness=" << static_cast<int>(readiness.state)
              << '\n';
    for (const auto& stage : readiness.stages)
        std::cout << "  " << stage.stage_id << '\n';

    qnbot::CaptureStartRequest request;
    request.package_ids = package_ids;
    request.operator_id = operator_id;
    request.source = source;
    request.side = side;
    request.force = force;
    auto session = glove.start_capture(request);
    if (start_runtime) glove.start();
    const auto deadline = std::chrono::steady_clock::now() + wait_timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        const auto snapshot = session.snapshot();
        if (snapshot.state == qnbot::CaptureSessionState::completed) {
            if (!session.result())
                throw std::runtime_error(
                    "completed capture session has no result");
            return session;
        }
        if (snapshot.state == qnbot::CaptureSessionState::failed ||
            snapshot.state == qnbot::CaptureSessionState::cancelled) {
            throw std::runtime_error(
                snapshot.failure ? snapshot.failure->message
                                 : std::string("capture session stopped"));
        }

        const auto current = std::find_if(
            snapshot.stage_progress.begin(), snapshot.stage_progress.end(),
            [&](const qnbot::CaptureStageProgress& stage) {
                return snapshot.current_stage &&
                       stage.stage_id == *snapshot.current_stage;
            });
        if (current != snapshot.stage_progress.end() &&
            current->state == qnbot::CaptureStageState::awaiting_confirmation) {
            if (!snapshot.request_id) {
                session.cancel();
                throw std::runtime_error(
                    "capture confirmation is missing its request token");
            }
            const auto plan_stage = std::find_if(
                snapshot.plan.stages.begin(), snapshot.plan.stages.end(),
                [&](const qnbot::CapturePlanStage& stage) {
                    return snapshot.current_stage &&
                           stage.stage_id == *snapshot.current_stage;
                });
            std::cout << (plan_stage == snapshot.plan.stages.end()
                              ? "Continue capture"
                              : plan_stage->prompt)
                      << " [Y/n]: ";
            std::string answer;
            std::getline(std::cin, answer);
            if (answer.empty() || answer == "confirm" || answer == "y" ||
                answer == "yes") {
                session.confirm(*snapshot.request_id);
            } else {
                session.cancel();
                throw std::runtime_error("capture cancelled by operator");
            }
            continue;
        }
        update_or_wait(glove);
    }
    session.cancel();
    throw std::runtime_error("capture timed out");
}

void calibrate(qnbot::Glove& glove,
               const qnbot::CaptureSession& capture_session,
               const std::vector<std::optional<std::string>>& targets) {
    auto calibration = glove.start_calibration(capture_session, targets);
    const auto deadline = std::chrono::steady_clock::now() + wait_timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        const auto snapshot = calibration.snapshot();
        if (snapshot.state == qnbot::CalibrationSessionState::completed) {
            const auto results = calibration.result();
            if (!results) {
                calibration.close();
                throw std::runtime_error(
                    "completed calibration session has no result");
            }
            for (const auto& result : *results)
                std::cout << "saved calibration record for " << result.target_id
                          << '\n';
            calibration.close();
            return;
        }
        const auto failed = std::find_if(
            snapshot.jobs.begin(), snapshot.jobs.end(),
            [](const qnbot::CalibrationJobProgress& job) {
                return job.state == qnbot::CalibrationJobState::failed;
            });
        if (failed != snapshot.jobs.end()) {
            const std::string message = failed->failure
                                            ? failed->failure->message
                                            : "calibration failed";
            calibration.close();
            throw std::runtime_error(message);
        }
        if (snapshot.state == qnbot::CalibrationSessionState::cancelled) {
            calibration.close();
            throw std::runtime_error("calibration cancelled");
        }
        update_or_wait(glove);
    }
    calibration.close();
    throw std::runtime_error("calibration timed out");
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        qnbot::Sdk sdk(make_config(options.package_ids));
        auto glove = sdk.glove();
        if (std::find(options.package_ids.begin(), options.package_ids.end(),
                      builtin_skeleton_package_id) !=
            options.package_ids.end()) {
            static_cast<void>(glove.device(left_source).skeleton());
            static_cast<void>(glove.device(right_source).skeleton());
        }

        auto left_session =
            capture_side(glove, options.package_ids, left_source,
                         qnbot::Side::left, options.force, true);
        auto right_session =
            capture_side(glove, options.package_ids, right_source,
                         qnbot::Side::right, options.force, false);
        const std::vector<std::optional<std::string>> targets{
            select_target(left_session.snapshot().plan.package_ids)};
        calibrate(glove, left_session, targets);
        calibrate(glove, right_session, targets);

        right_session.close();
        left_session.close();
        glove.close();
        std::cout << "Calibration results are saved and applied to retargeting "
                     "by the SDK.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "host capture and calibration failed: " << error.what()
                  << '\n';
        return 1;
    }
}
