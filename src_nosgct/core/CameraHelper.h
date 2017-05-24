/**
 * @file   CameraHelper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.21
 *
 * @brief  Declaration of a helper class for camera movement.
 */

#pragma once

#include "main.h"
#include "core/open_gl.h"
#include <glm/gtc/quaternion.hpp>

namespace viscom {

    class CameraHelper
    {
    public:
        CameraHelper();

        glm::vec3 GetUserPosition() const;
        const glm::vec3& GetPosition() const { return position_; }
        const glm::quat& GetOrientation() const { return camera_orientation_; }
        void SetPosition(const glm::vec3& position) { position_ = position; }
        void SetOrientation(const glm::quat& orientation) { camera_orientation_ = orientation; }

        glm::mat4 GetViewPerspectiveMatrix() const;
        glm::mat4 GetCentralPerspectiveMatrix() const;

    private:
        /// Position of the camera.
        glm::vec3 position_;
        /// Orientation of the camera.
        glm::quat camera_orientation_;
    };
}
