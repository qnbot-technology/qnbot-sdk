#ifndef QNBOT_CPP_DEVICE_AUTHORIZATION_CLEANUP_HPP
#define QNBOT_CPP_DEVICE_AUTHORIZATION_CLEANUP_HPP

#include "example_cleanup.hpp"

#include <utility>

namespace example {

struct AuthorizationSessionState {
    bool started;
    bool factory_active;
    bool connected;
};

template <typename Device, typename CloseGlove, typename CloseSdk>
AuthorizationSessionState cleanup_authorization_session(
    Cleanup& cleanup, Device* device, AuthorizationSessionState state,
    CloseGlove&& close_glove, CloseSdk&& close_sdk) noexcept {
    if (state.started && device != nullptr &&
        cleanup.run_redacted("device.stop()", [&] { device->stop(); })) {
        state.started = false;
    }
    if (!state.started && state.factory_active && device != nullptr &&
        cleanup.run_redacted("device.exit_factory_calibration()", [&] {
            static_cast<void>(device->exit_factory_calibration());
        })) {
        state.factory_active = false;
    }
    if (!state.started && !state.factory_active && state.connected &&
        device != nullptr && cleanup.run_redacted("device.disconnect()", [&] {
            device->disconnect();
        })) {
        state.connected = false;
    }
    if (!state.started && !state.factory_active && !state.connected &&
        cleanup.run_redacted("glove.close()",
                             std::forward<CloseGlove>(close_glove))) {
        cleanup.run_redacted("sdk.close()", std::forward<CloseSdk>(close_sdk));
    }
    return state;
}

} // namespace example

#endif
