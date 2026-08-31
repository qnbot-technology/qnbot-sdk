#include "example_cleanup.hpp"

#include <qnbot/glove.hpp>

#include <algorithm>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr const char* operator_id = "operator-a";
constexpr const char* left_source = "primary-glove-left";
constexpr const char* right_source = "primary-glove-right";
constexpr const char* target_name = "selected-hand";

struct Options {
    std::vector<std::string> package_ids;
    bool force{false};
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--package-id") {
            if (++index >= argc)
                throw std::invalid_argument("--package-id requires a value");
            options.package_ids.emplace_back(argv[index]);
        } else if (argument == "--force") {
            options.force = true;
        } else {
            throw std::invalid_argument("unknown argument: " + argument);
        }
    }
    if (options.package_ids.empty()) {
        throw std::invalid_argument("--package-id is required");
    }
    return options;
}

const char* readiness_name(qnbot::CaptureReadinessState state) {
    switch (state) {
    case qnbot::CaptureReadinessState::not_required:
        return "not_required";
    case qnbot::CaptureReadinessState::ready:
        return "ready";
    case qnbot::CaptureReadinessState::needs_capture:
        return "needs_capture";
    }
    return "unknown";
}

const char* stage_readiness_name(qnbot::CaptureStageReadinessState state) {
    switch (state) {
    case qnbot::CaptureStageReadinessState::reusable:
        return "reusable";
    case qnbot::CaptureStageReadinessState::needs_capture:
        return "needs_capture";
    }
    return "unknown";
}

const char* session_state_name(qnbot::CaptureSessionState state) {
    switch (state) {
    case qnbot::CaptureSessionState::created:
        return "created";
    case qnbot::CaptureSessionState::awaiting_confirmation:
        return "awaiting_confirmation";
    case qnbot::CaptureSessionState::collecting:
        return "collecting";
    case qnbot::CaptureSessionState::completed:
        return "completed";
    case qnbot::CaptureSessionState::failed:
        return "failed";
    case qnbot::CaptureSessionState::cancelled:
        return "cancelled";
    }
    return "unknown";
}

const char* stage_state_name(qnbot::CaptureStageState state) {
    switch (state) {
    case qnbot::CaptureStageState::pending:
        return "pending";
    case qnbot::CaptureStageState::reused:
        return "reused";
    case qnbot::CaptureStageState::awaiting_confirmation:
        return "awaiting_confirmation";
    case qnbot::CaptureStageState::collecting:
        return "collecting";
    case qnbot::CaptureStageState::captured:
        return "captured";
    case qnbot::CaptureStageState::skipped:
        return "skipped";
    case qnbot::CaptureStageState::failed:
        return "failed";
    }
    return "unknown";
}

const char* stage_origin_name(qnbot::CaptureStageOrigin origin) {
    return origin == qnbot::CaptureStageOrigin::reused ? "reused"
                                                       : "new_capture";
}

const char* control_name(qnbot::CaptureControlAction action) {
    switch (action) {
    case qnbot::CaptureControlAction::confirm:
        return "confirm";
    case qnbot::CaptureControlAction::retry:
        return "retry";
    case qnbot::CaptureControlAction::skip:
        return "skip";
    case qnbot::CaptureControlAction::cancel:
        return "cancel";
    }
    return "unknown";
}

const char* calibration_state_name(qnbot::CalibrationJobState state) {
    switch (state) {
    case qnbot::CalibrationJobState::running:
        return "running";
    case qnbot::CalibrationJobState::saving:
        return "saving";
    case qnbot::CalibrationJobState::completed:
        return "completed";
    case qnbot::CalibrationJobState::failed:
        return "failed";
    }
    return "unknown";
}

bool allows(const qnbot::CaptureSessionSnapshot& snapshot,
            qnbot::CaptureControlAction action) {
    return std::find(snapshot.allowed_operations.begin(),
                     snapshot.allowed_operations.end(),
                     action) != snapshot.allowed_operations.end();
}

std::vector<std::string> retryable_stage_ids(
    const qnbot::CaptureSessionSnapshot& snapshot) {
    std::vector<std::string> stage_ids;
    for (const auto& stage : snapshot.stage_runs) {
        if (stage.state == qnbot::CaptureStageState::captured ||
            stage.state == qnbot::CaptureStageState::reused) {
            stage_ids.push_back(stage.stage_id);
        }
    }
    return stage_ids;
}

void render_capture_readiness(qnbot::Side side,
                              const qnbot::CaptureReadiness& readiness) {
    std::cout << (side == qnbot::Side::left ? "left" : "right")
              << ": capture readiness=" << readiness_name(readiness.state)
              << " plan=";
    for (std::size_t index = 0; index < readiness.plan.resolved_packages.size();
         ++index) {
        if (index != 0) std::cout << ',';
        const auto& package = readiness.plan.resolved_packages[index];
        std::cout << package.id << '@' << package.version;
    }
    std::cout << '\n';

    for (const auto& stage : readiness.plan.stages) {
        const auto current = std::find_if(
            readiness.stages.begin(), readiness.stages.end(),
            [&](const qnbot::CaptureStageReadiness& value) {
                return value.stage_id == stage.stage_id;
            });
        if (current == readiness.stages.end()) {
            throw std::runtime_error("capture readiness omitted a plan stage");
        }
        std::cout << "  " << stage.stage_id << ": "
                  << stage_readiness_name(current->state) << ", "
                  << (stage.optional ? "optional" : "required")
                  << ", samples=" << stage.required_sample_count
                  << ", prompt=" << stage.prompt << '\n';
    }
}

std::string format_session_snapshot(
    qnbot::Side side, const qnbot::CaptureSessionSnapshot& snapshot) {
    std::ostringstream output;
    output << (side == qnbot::Side::left ? "left" : "right")
           << ": session=" << session_state_name(snapshot.state)
           << " current=" << snapshot.current_stage.value_or("-")
           << " progress=" << snapshot.collected_sample_count << '/'
           << snapshot.required_sample_count << " next=" << snapshot.next_action
           << " allowed=";
    if (snapshot.allowed_operations.empty()) output << "none";
    for (std::size_t index = 0; index < snapshot.allowed_operations.size();
         ++index) {
        if (index != 0) output << ',';
        output << control_name(snapshot.allowed_operations[index]);
    }
    output << '\n';
    for (const auto& stage : snapshot.stage_runs) {
        output << "  " << stage.stage_id
               << ": state=" << stage_state_name(stage.state)
               << ", origin=" << stage_origin_name(stage.origin)
               << ", samples=" << stage.collected_sample_count << '/'
               << stage.required_sample_count << '\n';
    }
    return output.str();
}

void render_capture_set(qnbot::Side side,
                        const qnbot::CaptureSet& capture_set) {
    const auto snapshot = capture_set.snapshot();
    std::cout << (side == qnbot::Side::left ? "left" : "right")
              << ": completed CaptureSet source=" << snapshot.source_id
              << '\n';
    for (const auto& stage : snapshot.stage_samples) {
        std::cout << "  " << stage.stage_id
                  << ": saved samples=" << stage.frame_count << '\n';
    }
}

qnbot::SdkConfig make_config() {
    qnbot::GloveConfig left;
    left.name = left_source;
    left.side = qnbot::Side::left;

    qnbot::GloveConfig right;
    right.name = right_source;
    right.side = qnbot::Side::right;

    qnbot::TargetConfig left_target;
    left_target.type = qnbot::TargetType::hand;
    left_target.name = target_name;
    left_target.side = qnbot::Side::left;
    left_target.source =
        qnbot::DeviceSelector{"glove", std::string(left_source), std::nullopt};

    qnbot::TargetConfig right_target;
    right_target.type = qnbot::TargetType::hand;
    right_target.name = target_name;
    right_target.side = qnbot::Side::right;
    right_target.source =
        qnbot::DeviceSelector{"glove", std::string(right_source), std::nullopt};

    qnbot::SdkConfig config;
    config.devices = {left, right};
    config.targets = {left_target, right_target};
    return config;
}

qnbot::CaptureSet capture_side(qnbot::Glove& glove,
                               const std::vector<std::string>& candidates,
                               const std::string& source, qnbot::Side side,
                               bool force) {
    qnbot::CaptureStatusRequest status_request;
    status_request.candidate_ids = candidates;
    status_request.operator_id = operator_id;
    status_request.source = source;
    status_request.side = side;
    const auto readiness = glove.capture_status(status_request);
    render_capture_readiness(side, readiness);

    qnbot::CaptureStartRequest request;
    request.candidate_ids = candidates;
    request.operator_id = operator_id;
    request.source = source;
    request.side = side;
    request.force = force;
    auto session = glove.start_capture(request);

    example::Cleanup cleanup;
    std::optional<qnbot::CaptureSet> result;
    std::string shown_snapshot;
    try {
        while (!result) {
            const auto snapshot = session.snapshot();
            const auto current = format_session_snapshot(side, snapshot);
            if (current != shown_snapshot) {
                shown_snapshot = current;
                std::cout << current;
            }
            if (snapshot.state == qnbot::CaptureSessionState::completed) {
                result = session.result();
                if (!result) {
                    throw std::runtime_error(
                        "completed capture session has no result");
                }
                render_capture_set(side, *result);
                continue;
            }
            if (snapshot.state == qnbot::CaptureSessionState::failed ||
                snapshot.state == qnbot::CaptureSessionState::cancelled) {
                throw std::runtime_error(
                    snapshot.failure.value_or("capture session stopped"));
            }
            if (snapshot.state ==
                qnbot::CaptureSessionState::awaiting_confirmation) {
                if (!snapshot.request_id) {
                    throw std::runtime_error(
                        "capture confirmation is missing its request token");
                }
                const auto stage = std::find_if(
                    snapshot.plan.stages.begin(), snapshot.plan.stages.end(),
                    [&](const qnbot::CapturePlanStage& value) {
                        return snapshot.current_stage &&
                               value.stage_id == *snapshot.current_stage;
                    });
                const std::string prompt =
                    stage == snapshot.plan.stages.end()
                        ? snapshot.current_stage.value_or("current stage")
                        : stage->prompt;
                const auto retry_stages = retryable_stage_ids(snapshot);
                std::vector<std::string> choices;
                if (allows(snapshot, qnbot::CaptureControlAction::confirm))
                    choices.emplace_back("Enter=confirm");
                if (allows(snapshot, qnbot::CaptureControlAction::skip))
                    choices.emplace_back("skip=skip");
                if (allows(snapshot, qnbot::CaptureControlAction::retry)) {
                    std::ostringstream retry;
                    retry << "retry <stage_id> (";
                    for (std::size_t index = 0; index < retry_stages.size();
                         ++index) {
                        if (index != 0) retry << ", ";
                        retry << retry_stages[index];
                    }
                    retry << ')';
                    choices.push_back(retry.str());
                }
                if (allows(snapshot, qnbot::CaptureControlAction::cancel))
                    choices.emplace_back("cancel=cancel");
                std::cout << prompt << " [";
                for (std::size_t index = 0; index < choices.size(); ++index) {
                    if (index != 0) std::cout << ", ";
                    std::cout << choices[index];
                }
                std::cout << "]: ";
                std::string answer;
                std::getline(std::cin, answer);
                if (answer == "cancel" &&
                    allows(snapshot, qnbot::CaptureControlAction::cancel)) {
                    session.cancel();
                } else if (answer == "skip" &&
                           allows(snapshot,
                                  qnbot::CaptureControlAction::skip)) {
                    session.skip(*snapshot.request_id);
                } else if (answer.compare(0, 6, "retry ") == 0 &&
                           allows(snapshot,
                                  qnbot::CaptureControlAction::retry)) {
                    const std::string retry_stage_id = answer.substr(6);
                    if (std::find(retry_stages.begin(), retry_stages.end(),
                                  retry_stage_id) != retry_stages.end()) {
                        session.retry(retry_stage_id);
                    } else {
                        std::cout << "stage is not available to retry\n";
                    }
                } else if ((answer.empty() || answer == "confirm") &&
                           allows(snapshot,
                                  qnbot::CaptureControlAction::confirm)) {
                    session.confirm(*snapshot.request_id);
                } else {
                    std::cout
                        << "action is not allowed in the current session state\n";
                }
                continue;
            }
            const auto update = glove.update();
            if (update.has_next_task()) static_cast<void>(update.sleep());
        }
    } catch (...) {
        cleanup.capture_current("capture side");
    }
    cleanup.run("capture session close", [&] { session.close(); });
    cleanup.rethrow_if_failed();
    return *result;
}

std::string choose_package(const std::optional<std::string>& argument,
                           const std::vector<std::string>& candidates) {
    std::string selected;
    if (argument) {
        selected = *argument;
    } else {
        std::cout << "Choose package (";
        for (std::size_t index = 0; index < candidates.size(); ++index) {
            if (index != 0) std::cout << ", ";
            std::cout << candidates[index];
        }
        std::cout << "): ";
        std::getline(std::cin, selected);
    }
    if (std::find(candidates.begin(), candidates.end(), selected) ==
        candidates.end()) {
        throw std::invalid_argument(
            "selected package must be one of the captured candidates");
    }
    return selected;
}

void calibrate(qnbot::Glove& glove, const qnbot::CaptureSet& capture_set,
               const std::string& package_id) {
    auto job = glove.start_calibration(capture_set, package_id, target_name);
    example::Cleanup cleanup;
    std::optional<qnbot::CalibrationJobState> shown_state;
    try {
        while (true) {
            const auto snapshot = job.snapshot();
            if (!shown_state || *shown_state != snapshot.state) {
                std::cout << "calibration state="
                          << calibration_state_name(snapshot.state)
                          << " package=" << snapshot.package_id << '@'
                          << snapshot.package_version << '\n';
                shown_state = snapshot.state;
            }
            if (snapshot.state == qnbot::CalibrationJobState::completed) {
                const auto result = job.result();
                if (!result) {
                    throw std::runtime_error(
                        "completed calibration job has no result");
                }
                std::cout << "saved " << result->package_id << '@'
                          << result->package_version << " for "
                          << result->target_id << '\n';
                break;
            }
            if (snapshot.state == qnbot::CalibrationJobState::failed) {
                throw std::runtime_error(
                    snapshot.failure
                        ? snapshot.failure->message
                        : std::string("calibration failed"));
            }
            const auto update = glove.update();
            if (update.has_next_task()) static_cast<void>(update.sleep());
        }
    } catch (...) {
        cleanup.capture_current("calibration");
    }
    cleanup.run("calibration job close", [&] { job.close(); });
    cleanup.rethrow_if_failed();
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        qnbot::Sdk sdk(make_config());
        example::Cleanup cleanup;
        std::optional<qnbot::Glove> glove;
        std::optional<qnbot::CaptureSet> left_capture_set;
        std::optional<qnbot::CaptureSet> right_capture_set;
        try {
            glove.emplace(sdk.glove());
            glove->connect();
            glove->start();

            left_capture_set = capture_side(
                *glove, options.package_ids, left_source,
                qnbot::Side::left, options.force);
            right_capture_set = capture_side(
                *glove, options.package_ids, right_source,
                qnbot::Side::right, options.force);

            const auto selected_package_id =
                options.package_ids.size() == 1
                    ? options.package_ids.front()
                    : choose_package(std::nullopt, options.package_ids);
            calibrate(*glove, *left_capture_set, selected_package_id);
            calibrate(*glove, *right_capture_set, selected_package_id);
            std::cout << "Calibration results are saved and applied to "
                         "retargeting by the SDK.\n";
        } catch (...) {
            cleanup.capture_current("host capture and calibration");
        }

        if (right_capture_set)
            cleanup.run("right capture set close",
                        [&] { right_capture_set->close(); });
        if (left_capture_set)
            cleanup.run("left capture set close",
                        [&] { left_capture_set->close(); });
        if (glove) cleanup.run("glove.close()", [&] { glove->close(); });
        cleanup.run("sdk.close()", [&] { sdk.close(); });
        cleanup.rethrow_if_failed();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "host capture and calibration failed: " << error.what()
                  << '\n';
        return EXIT_FAILURE;
    }
}
