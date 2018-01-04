/**
 * @file   CameraHelper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.21
 *
 * @brief  Declaration of a helper class for camera movement.
 */

#pragma once

#include "core/main.h"
#include "core/math/primitives.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

namespace sgct {
    class Engine;
}

namespace viscom {

    class CameraHelper
    {
    public:
        CameraHelper(sgct::Engine* engine);

        glm::vec3 GetUserPosition() const;
        const glm::vec3& GetPosition() const { return position_; }
        const glm::quat& GetOrientation() const { return camera_orientation_; }
        const glm::mat4& GetPickMatrix() const { return pickMatrix_; }
        void SetPosition(const glm::vec3& position) { position_ = position; }
        void SetOrientation(const glm::quat& orientation) { camera_orientation_ = orientation; }
        void SetPickMatrix(const glm::mat4& pickMatrix) { pickMatrix_ = pickMatrix; }
        void SetLocalCoordMatrix(std::size_t windowID, const glm::mat4& localCoordMatrix);

        /** Get camera matrices (eye dependent). */
        const glm::mat4& GetPerspectiveMatrix() const;
        glm::mat4 GetViewPerspectiveMatrix() const;

        /** Get camera matrices (eye independent). */
        glm::mat4 GetCentralPerspectiveMatrix() const;
        glm::mat4 GetCentralViewPerspectiveMatrix() const;

        math::Line3<float> GetPickRay(const glm::vec2& globalScreenCoords) const;
        glm::vec3 GetPickPosition(const glm::vec2& globalScreenCoords) const;

        float GetNearPlane() const;
        float GetFarPlane() const;
        void SetNearFarPlane(float nearPlane, float farPlane);

    private:
        glm::mat4 CalculateViewUpdate() const;

        /** Holds the SGCT engine. */
        sgct::Engine* engine_;

        /// Position of the camera.
        glm::vec3 position_;
        /// Orientation of the camera.
        glm::quat camera_orientation_;
        /** The matrix used for picking global coordinates. */
        glm::mat4 pickMatrix_;
        /** The matrices to calculate local coordinates from global ones. */
        std::vector<glm::mat4> localCoordsMatrices_;
    };
}
