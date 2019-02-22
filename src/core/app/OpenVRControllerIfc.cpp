/**
 * @file   OpenVRControllerIfc.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.02.22
 *
 * @brief  Definition of the dummy OpenVR controllers static variables.
 */

#include "OpenVRControllerIfc.h"

namespace viscom::ovr {

    const std::vector<DeviceInfo> OpenVRControllerIfc::noDeviceInfo_ = std::vector<DeviceInfo>{ };
    const glm::vec3 OpenVRControllerIfc::zeroVec3_ = glm::vec3{ 0.0f };
    const glm::vec2 OpenVRControllerIfc::zeroVec2_ = glm::vec2{ 0.0f };
    const glm::quat OpenVRControllerIfc::identityQuat_ = glm::quat{ 0.0f, 0.0f, 1.0f, 0.0f };

}
