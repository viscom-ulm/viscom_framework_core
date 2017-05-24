/**
 * @file   CameraHelper.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.21
 *
 * @brief  Implementation of a helper class for camera movement.
 */

#include "CameraHelper.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace viscom {
    CameraHelper::CameraHelper(float width, float height) :
        projection_{ glm::perspectiveFov(60.0f, width, height, 1.0f, 100.0f) } // TODO: find better values. [5/24/2017 Sebastian Maisch]
    {
    }

    glm::vec3 CameraHelper::GetUserPosition() const
    {
        return glm::vec3(0.0f);
    }

    glm::mat4 CameraHelper::GetViewPerspectiveMatrix() const
    {
        auto result = glm::mat4_cast(glm::inverse(camera_orientation_));
        result = glm::translate(result, -position_);

        return projection_ * result;
    }

    glm::mat4 CameraHelper::GetCentralPerspectiveMatrix() const
    {
        return projection_;
    }
}
