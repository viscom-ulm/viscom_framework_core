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

    /** Helper class for the camera. */
    class CameraHelper
    {
    public:
        /**
        *  Constructor method.
        *  @param engine the sgct engine.
        */
        CameraHelper(sgct::Engine* engine);

        /** Returns the user position. */
        glm::vec3 GetUserPosition() const;
        /** Returns the position of the camera. */
        const glm::vec3& GetPosition() const { return position_; }
        /** Returns the orientation of the camera. */
        const glm::quat& GetOrientation() const { return camera_orientation_; }
        /** Returns the matrix used for picking global coordinates. */
        const glm::mat4& GetPickMatrix() const { return pickMatrix_; }
        /**
        *  Sets the position of the camera.
        *  @param position new camera position.
        */
        void SetPosition(const glm::vec3& position) { position_ = position; }
        /**
        *  Sets the orientation of the camera.
        *  @param orientation new camera orientation.
        */
        void SetOrientation(const glm::quat& orientation) { camera_orientation_ = orientation; }
        /**
        *  Sets the pick matrix of the camera.
        *  @param pickMatrix new pick matrix for the camera.
        */
        void SetPickMatrix(const glm::mat4& pickMatrix) { pickMatrix_ = pickMatrix; }
        /**
        *  Sets the matrix to calculate local coordinates from global ones.
        *  @param windowID index of the used window.
        *  @param localCoordMatrix new local coord matrix.
        *  @param localScreenSize size of the used screen.
        */
        void SetLocalCoordMatrix(std::size_t windowID, const glm::mat4& localCoordMatrix, const glm::vec2& localScreenSize);

        /** Returns the cameras eye dependent projection matrix. */
        const glm::mat4& GetPerspectiveMatrix() const;
        /** Returns the cameras eye dependent view-projection matrix. */
        glm::mat4 GetViewPerspectiveMatrix() const;

        /** Returns the cameras eye independent projection matrix. */
        glm::mat4 GetCentralPerspectiveMatrix() const;
        /** Returns the cameras eye independent view-projection matrix. */
        glm::mat4 GetCentralViewPerspectiveMatrix() const;

        /**
        *  Returns a ray for picking.
        *  @param globalScreenCoords global screen coordinates to pick.
        */
        math::Line3<float> GetPickRay(const glm::vec2& globalScreenCoords) const;
        /**
        *  Returns the picked position in world space.
        *  @param globalScreenCoords global screen coordinates to pick.
        */
        glm::vec3 GetPickPosition(const glm::vec2& globalScreenCoords) const;

        /** Returns the cameras near plane. */
        float GetNearPlane() const;
        /** Returns the cameras far plane. */
        float GetFarPlane() const;
        /**
        *  Sets the near and far plane values;
        *  @param nearPlane new near plane value.
        *  @param farPlane new far plane value.
        */
        void SetNearFarPlane(float nearPlane, float farPlane);

    private:
        /** Calculates and returns the cameras view matrix using user and camera position and camera orientation. */
        glm::mat4 CalculateViewUpdate() const;

        /** Holds the SGCT engine. */
        sgct::Engine* engine_;

        /** Position of the camera. */
        glm::vec3 position_;
        /** Orientation of the camera. */
        glm::quat camera_orientation_;
        /** The matrix used for picking global coordinates. */
        glm::mat4 pickMatrix_;
        /** The matrices to calculate local coordinates from global ones. */
        std::vector<std::pair<glm::mat4, glm::vec2>> localCoordsMatrices_;
    };
}
