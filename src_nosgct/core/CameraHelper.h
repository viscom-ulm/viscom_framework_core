/**
 * @file   CameraHelper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.21
 *
 * @brief  Declaration of a helper class for camera movement.
 */

#pragma once

#include "core/main.h"
#include "core/open_gl.h"
#include <glm/gtc/quaternion.hpp>

namespace viscom {

    class CameraHelper
    {
    public:
        CameraHelper(float width, float height, const glm::vec3& userPosition);

        glm::vec3 GetUserPosition() const;
        const glm::vec3& GetPosition() const { return position_; }
        const glm::quat& GetOrientation() const { return cameraOrientation_; }
        void SetPosition(const glm::vec3& position) { position_ = position; }
        void SetOrientation(const glm::quat& orientation) { cameraOrientation_ = orientation; }

        glm::mat4 GetViewPerspectiveMatrix() const;
        glm::mat4 GetCentralPerspectiveMatrix() const;

    private:
        /** Position of the camera. */
        glm::vec3 position_;
        /** Orientation of the camera. */ 
        glm::quat cameraOrientation_;

        /** The user position. */
        glm::vec3 userPosition_;
        /** The user view matrix. */
        glm::mat4 userView_;
        /** Standard projection matrix. */
        glm::mat4 projection_;
    };
}
