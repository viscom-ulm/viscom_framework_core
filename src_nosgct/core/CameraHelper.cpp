/**
 * @file   CameraHelper.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.21
 *
 * @brief  Implementation of a helper class for camera movement.
 */

#include "CameraHelper.h"
#include "core/open_gl.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace viscom {
    CameraHelper::CameraHelper(float width, float height, const glm::vec3& userPosition) :
        position_{ 0.0f },
        cameraOrientation_{ 0.0f, 0.0f, 1.0f, 0.0f },
        userPosition_{ userPosition },
        userView_{ glm::lookAt(userPosition_, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) },
        projection_{ glm::perspectiveFov(2.0f * glm::atan(height, userPosition_.z), width, height, 0.1f, 100.0f) },
        pickMatrix_{ 1.0f },
        width_{ width },
        height_{ height },
        nearPlane_{ 0.1f },
        farPlane_{ 100.0f }
    {
    }

    glm::vec3 CameraHelper::GetUserPosition() const
    {
        return userPosition_;
    }

    void CameraHelper::SetLocalCoordMatrix(std::size_t windowID, const glm::mat4& localCoordMatrix, const glm::vec2& localScreenSize)
    {
        localCoordsMatrix_.first = localCoordMatrix;
        localCoordsMatrix_.second = localScreenSize;
    }

    glm::mat4 CameraHelper::GetViewPerspectiveMatrix() const
    {
        auto result = CalculateViewUpdate();
        return projection_ * userView_ * result;
    }

    glm::mat4 CameraHelper::GetCentralPerspectiveMatrix() const
    {
        return projection_;
    }

    glm::mat4 CameraHelper::GetCentralViewPerspectiveMatrix() const
    {
        return GetViewPerspectiveMatrix();
    }

    math::Line3<float> CameraHelper::GetPickRay(const glm::vec2& globalScreenCoords) const
    {
        math::Line3<float> result;
        result[0] = GetPosition() + GetUserPosition();
        auto pickResult = pickMatrix_ * glm::vec4(globalScreenCoords.x, globalScreenCoords.y, -1., 1.0f);
        pickResult /= pickResult.w;

        result[1] = glm::vec3(pickResult);

        return result;
    }

    glm::vec3 CameraHelper::GetPickPosition(const glm::vec2& globalScreenCoords) const
    {
        // to local screen coordinates.
        glm::vec4 screenCoords = localCoordsMatrix_.first * glm::vec4{globalScreenCoords, 0.0f, 1.0};
        glm::ivec2 iScreenCoords(screenCoords.x, screenCoords.y);
        screenCoords.x = screenCoords.x / localCoordsMatrix_.second.x;
        screenCoords.y = 1.0f - screenCoords.y / localCoordsMatrix_.second.y;

        glReadPixels(iScreenCoords.x, iScreenCoords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &screenCoords.z);
        screenCoords = glm::vec4((glm::vec3(screenCoords) * 2.0f) - 1.0f, screenCoords.w);
        LOG(INFO) << "Picked position: (" << screenCoords.x << ", " << screenCoords.y << ", " << screenCoords.z << ")";
        auto viewProjInv = glm::inverse(GetViewPerspectiveMatrix());

        auto postProjPos = viewProjInv * screenCoords;
        return glm::vec3(postProjPos) / postProjPos.w;
    }

#undef near
#undef far

    void CameraHelper::SetNearFarPlane(float near, float far)
    {
        nearPlane_ = near;
        farPlane_ = far;
        projection_ = glm::perspectiveFov(2.0f * glm::atan(height_, userPosition_.z), width_, height_, nearPlane_, farPlane_);
    }

    glm::mat4 CameraHelper::CalculateViewUpdate() const
    {
        //4. transform user back to original position
        glm::mat4 result = glm::translate(glm::mat4(1.0f), userPosition_);
        //3. apply view rotation
        result *= glm::mat4_cast(cameraOrientation_);
        //2. apply navigation translation
        result *= glm::translate(glm::mat4(1.0f), -position_);
        //1. transform user to coordinate system origin
        result *= glm::translate(glm::mat4(1.0f), -userPosition_);
        return result;
    }
}
