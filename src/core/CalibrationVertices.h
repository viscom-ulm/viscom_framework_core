/**
 * @file   CalibrationVertices.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.06
 *
 * @brief  Declaration of vertices used for calibration.
 */

#pragma once

#include <glm/glm.hpp>

namespace viscom {

    struct CalbrationProjectorQuadVertex
    {
        glm::vec3 position_;
        glm::vec3 texCoords_;

        CalbrationProjectorQuadVertex(const glm::vec3& pos, const glm::vec3& tex) : position_(pos), texCoords_(tex) {}
    };

}