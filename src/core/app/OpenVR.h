/**
 * @file   OpenVR.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.02.21
 *
 * @brief  Basic types for open vr interface.
 */

#pragma once

#include "core/main.h"
#include <glm/gtc/quaternion.hpp>
#include <openvr.h>

namespace vr {
    class IVRSystem;
    struct VREvent_t;
    struct HmdMatrix34_t;
    enum ETrackedControllerRole;
}

namespace viscom::ovr {

    constexpr auto displayCalibrationFilename = "constrollerDisplayCalibration.txt";

    enum class CalibrateMethod { CALIBRATE_BY_TOUCHING, CALIBRATE_BY_POINTING };
    enum class TrackedDeviceRole { CONTROLLER_LEFT_HAND, CONTROLLER_RIGHT_HAND, GENERIC_TRACKER, INVALID };
    enum class ButtonState { PRESSED, TOUCHED, RELEASED };
    enum class TrackedDeviceClass { INVALID, CONTROLLER, GENERIC_TRACKER, TRACKING_REFERENCE, DISPLAY_REDIRECT, HMD };

    struct DeviceInfo {
        std::uint32_t deviceId_ = 0;
        TrackedDeviceRole deviceRole_ = TrackedDeviceRole::INVALID;
        TrackedDeviceClass deviceClass_ = TrackedDeviceClass::INVALID;
    };

    struct ControllerState {
        /** Holds the controller position. */
        glm::vec3 position_ = glm::vec3{ 0.0f };
        /** Holds the controller direction (z-vector). */
        glm::vec3 direction_ = glm::vec3{ 0.0f };
        /** Holds the controller orientation. */
        glm::quat orientation_ = glm::quat{ 0.0f, 0.0f, 1.0f, 0.0f };
    };

    struct DisplayPlane {
        glm::vec3 mouseOrigin_ = glm::vec3{ -1.0f, -1.0f, -1.0f };
        glm::vec3 center_ = glm::vec3{ 0.0f, 0.0f, -1.0f };
        glm::vec3 right_ = glm::vec3{ 2.0f, 0.0f, 0.0f };
        glm::vec3 up_ = glm::vec3{ 0.0f, 2.0f, 0.0f };
    };

    TrackedDeviceRole GetTrackedDeviceRoleFromTrackedControllerRole(const vr::ETrackedControllerRole& role);
    glm::vec3 GetPositionFromHmdMatrix34(const vr::HmdMatrix34_t& m);
    glm::quat GetOrientationFromHmdMatrix34(const vr::HmdMatrix34_t& m);
    glm::vec3 GetDirectionFromHmdMatrix34(const vr::HmdMatrix34_t& m);
}
