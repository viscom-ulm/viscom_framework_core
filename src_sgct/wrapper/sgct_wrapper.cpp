/**
 * @file   sgct_wrapper.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.01.28
 *
 * @brief  Simple wrapper for SGCT methods that use GLM.
 */

#include "sgct_wrapper.h"

namespace sgct_wrapper {

    wVec3 GetProjectionPlaneCoordinate(sgct::SGCTWindow* window, std::size_t viewportId, sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner planeCorner)
    {
        auto corner = window->getViewport(viewportId)->getProjectionPlane()->getCoordinate(planeCorner);
        return wVec3{ corner.x, corner.y, corner.z };
    }

    wVec3 GetDefaultUserPosition()
    {
        auto position = sgct::Engine::getDefaultUserPtr()->getPos();
        return wVec3{ position.x, position.y, position.z };
    }
}
