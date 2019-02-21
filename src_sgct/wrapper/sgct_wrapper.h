/**
 * @file   sgct_wrapper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.01.28
 *
 * @brief  Simple wrapper for SGCT methods that use GLM.
 */

#pragma once

#include <array>
#include <sgct.h>

namespace sgct_wrapper {

    using wVec3 = std::array<float, 3>;

    wVec3 GetProjectionPlaneCoordinate(sgct::SGCTWindow* window, std::size_t viewportId, sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner planeCorner);
    wVec3 GetDefaultUserPosition();
}
