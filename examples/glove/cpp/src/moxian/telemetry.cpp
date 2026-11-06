#include "../example_support.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace {

template <typename T, std::size_t N>
void print_vector(std::ostream& out, const std::array<T, N>& values) {
    out << '[' << values[0];
    for (std::size_t index = 1; index < values.size(); ++index)
        out << ", " << values[index];
    out << ']';
}

void print_pose(const qnbot::Sample<qnbot::GlovePose>& sample) {
    const auto& tip =
        sample.value.payload.fingertip_local.at(qnbot::GloveFinger::index);
    std::ostringstream out;
    out << "pose sequence=" << sample.sequence << " index_position=";
    print_vector(out, tip.position);
    std::cout << out.str() + '\n';
}

void print_imu(const qnbot::Sample<qnbot::GloveImu>& sample) {
    const auto& imu = sample.value.payload;
    std::ostringstream out;
    out << "imu sequence=" << sample.sequence << " valid=" << std::boolalpha
        << imu.valid << " gyroscope_raw=";
    print_vector(out, imu.gyroscope_raw);
    out << " accelerometer_raw=";
    print_vector(out, imu.accelerometer_raw);
    std::cout << out.str() + '\n';
}

void print_joints(const qnbot::Sample<qnbot::HandJointCommand>& sample) {
    const auto& joints = sample.value.joints;
    const auto& [joint, angle] = *joints.begin();
    std::ostringstream out;
    out << "joint_angles sequence=" << sample.sequence
        << " joints=" << joints.size() << " first_joint=" << joint
        << " angle=" << angle << '\n';
    std::cout << out.str();
}

void print_skeleton_pose(const qnbot::Sample<qnbot::HandSkeletonPose>& sample) {
    const auto& pose = sample.value;
    const auto index_tip = static_cast<std::size_t>(
        std::find(pose.names.begin(), pose.names.end(), "index_tip") -
        pose.names.begin());
    std::ostringstream out;
    out << "skeleton_pose sequence=" << sample.sequence
        << " frame=" << pose.coordinate_frame << " node=index_tip position=";
    print_vector(out, pose.positions_local.at(index_tip));
    out << " quaternion_xyzw=";
    print_vector(out, pose.orientations_local_xyzw.at(index_tip));
    std::cout << out.str() + '\n';
}

void print_pressure(const qnbot::Sample<qnbot::GlovePressure>& sample) {
    const auto& readings = sample.value.readings;
    std::ostringstream out;
    out << "pressure sequence=" << sample.sequence
        << " points=" << readings.size()
        << " first_raw=" << static_cast<unsigned>(readings.front()) << '\n';
    std::cout << out.str();
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 3 || std::string(argv[1]) != "--port")
            throw std::invalid_argument("usage: moxian --port <port>");
        qnbot::GloveConfig glove_config;
        qnbot::SerialConnection connection;
        connection.port = argv[2];
        glove_config.connection = connection;
        qnbot::SdkConfig config;
        config.devices.push_back(glove_config);
        qnbot::Sdk sdk(config);
        auto glove = sdk.glove();
        auto device = glove.device();
        auto skeleton = device.skeleton();
        const auto display = qnbot::SubscriptionConfig::latest(1.0);
        const std::array subscriptions{
            device.pose().with_config(display).subscribe(print_pose),
            device.imu().with_config(display).subscribe(print_imu),
            skeleton.joint_angles().with_config(display).subscribe(
                print_joints),
            skeleton.pose().with_config(display).subscribe(print_skeleton_pose),
            device.pressure().with_config(display).subscribe(print_pressure),
        };

        glove.start();
        std::cout << "running; press Ctrl+C to stop" << std::endl;
        glove.run_forever();
        glove.close();
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        return example::report_error(error);
    }
}
