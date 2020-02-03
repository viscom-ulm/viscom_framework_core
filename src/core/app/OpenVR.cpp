/**
 * @file   OpenVR.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.02.21
 *
 * @brief  Basic types for open vr interface.
 */

#include "OpenVR.h"
#include <openvr.h>

namespace viscom::ovr {

    TrackedDeviceRole GetTrackedDeviceRoleFromTrackedControllerRole(const vr::ETrackedControllerRole& role)
    {
        switch (role)
        {
        case vr::TrackedControllerRole_LeftHand: return TrackedDeviceRole::CONTROLLER_LEFT_HAND;
        case vr::TrackedControllerRole_RightHand: return TrackedDeviceRole::CONTROLLER_RIGHT_HAND;
        default: return TrackedDeviceRole::INVALID;
        }
    }

    glm::vec3 GetPositionFromHmdMatrix34(const vr::HmdMatrix34_t& m)
    {
        return glm::vec3{ m.m[0][3], m.m[1][3], m.m[2][3] };
    }

    glm::quat GetOrientationFromHmdMatrix34(const vr::HmdMatrix34_t& m)
    {
        glm::quat result{
            glm::sqrt(glm::max(0.0f, 1.0f + m.m[0][0] + m.m[1][1] + m.m[2][2])) / 2.0f,
            glm::sqrt(glm::max(0.0f, 1.0f + m.m[0][0] - m.m[1][1] - m.m[2][2])) / 2.0f,
            glm::sqrt(glm::max(0.0f, 1.0f - m.m[0][0] + m.m[1][1] - m.m[2][2])) / 2.0f,
            glm::sqrt(glm::max(0.0f, 1.0f - m.m[0][0] - m.m[1][1] + m.m[2][2])) / 2.0f
        };
        result.x = copysignf(result.x, static_cast<float>(m.m[2][1] - m.m[1][2]));
        result.y = copysignf(result.y, static_cast<float>(m.m[0][2] - m.m[2][0]));
        result.z = copysignf(result.z, static_cast<float>(m.m[1][0] - m.m[0][1]));
        return result;
    }

    glm::vec3 GetDirectionFromHmdMatrix34(const vr::HmdMatrix34_t& m)
    {
        return glm::vec3{ m.m[0][2], m.m[1][2], m.m[2][2] };
    }
}
