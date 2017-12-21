/**
 * @file   CameraHelper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.21
 *
 * @brief  Declaration of a helper class for camera movement.
 */

#pragma once

#include "core/main.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include "core/math/primitives.h"

namespace viscom {

    class CameraHelper
    {
    public:
        CameraHelper(float width, float height, const glm::vec3& userPosition);

        glm::vec3 GetUserPosition() const;
        const glm::vec3& GetPosition() const { return position_; }
        const glm::quat& GetOrientation() const { return cameraOrientation_; }
        const glm::mat4& GetPickMatrix() const { return pickMatrix_; }
        void SetPosition(const glm::vec3& position) { position_ = position; }
        void SetOrientation(const glm::quat& orientation) { cameraOrientation_ = orientation; }
        void SetPickMatrix(const glm::mat4& pickMatrix) { pickMatrix_ = pickMatrix; }

        glm::mat4 GetViewPerspectiveMatrix() const;
        glm::mat4 GetCentralPerspectiveMatrix() const;
        glm::mat4 GetCentralViewPerspectiveMatrix() const;

        math::Line3<float> GetPickRay(const glm::vec2& globalScreenCoords) const;
        glm::vec3 GetPickPosition(const glm::vec2& globalScreenCoords) const;

        float GetNearPlane() const { return nearPlane_; }
        float GetFarPlane() const { return farPlane_; }
        void SetNearFarPlane(float near, float far);

    private:
        glm::mat4 CalculateViewUpdate() const;

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

        /** The matrix used for picking global coordinates. */
        glm::mat4 pickMatrix_;

        /** The camera view plane width. */
        float width_;
        /** The camera view plane height. */
        float height_;
        /** The cameras near plane. */
        float nearPlane_;
        /** The cameras far plane. */
        float farPlane_;
    };
}
