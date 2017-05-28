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
        userPosition_{ 0.0f, 0.0f, 4.0f },
        userView_{ glm::lookAt(userPosition_, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) },
        projection_{ glm::perspectiveFov(0.48995732625372830834416496242255f, 1.7778f, 1.0f, 0.1f, 100.0f) }
    {
    }

    glm::vec3 CameraHelper::GetUserPosition() const
    {
        return userPosition_;
    }

    glm::mat4 CameraHelper::GetViewPerspectiveMatrix() const
    {
        //4. transform user back to original position
        glm::mat4 result = glm::translate(glm::mat4(1.0f), userPosition_);
        //3. apply view rotation
        result *= glm::mat4_cast(glm::inverse(cameraOrientation_));
        //2. apply navigation translation
        result = glm::translate(result, -position_);
        //1. transform user to coordinate system origin
        result *= glm::translate(glm::mat4(1.0f), -userPosition_);

        return projection_ * userView_ * result;
    }

    glm::mat4 CameraHelper::GetCentralPerspectiveMatrix() const
    {
        return projection_;
    }
}
