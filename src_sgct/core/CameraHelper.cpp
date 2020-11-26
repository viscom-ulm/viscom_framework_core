/**
 * @file   CameraHelper.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.21
 *
 * @brief  Implementation of a helper class for camera movement.
 */

#include <sgct.h>
#include "CameraHelper.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <sgct_wrapper.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace viscom {
    CameraHelper::CameraHelper(sgct::Engine * engine) :
        engine_{ engine }
    {
    }

    glm::vec3 CameraHelper::GetUserPosition() const
    {
        auto userPosA = sgct_wrapper::GetDefaultUserPosition();
        return glm::vec3{ userPosA[0], userPosA[1], userPosA[2] };
    }

    void CameraHelper::SetLocalCoordMatrix(std::size_t windowID, const glm::mat4& localCoordMatrix, const glm::vec2& localScreenSize)
    {
        if (windowID >= localCoordsMatrices_.size()) {
            localCoordsMatrices_.resize(windowID + 1, std::make_pair(glm::mat4(), glm::vec2(0.0f)));
        }
        localCoordsMatrices_[windowID] = std::make_pair(localCoordMatrix, localScreenSize);
    }

    const glm::mat4& CameraHelper::GetPerspectiveMatrix() const
    {
        return engine_->getCurrentProjectionMatrix();
    }

    glm::mat4 CameraHelper::GetViewPerspectiveMatrix() const
    {
        auto result = CalculateViewUpdate();
        return engine_->getCurrentModelViewProjectionMatrix() * result;
    }

    glm::mat4 CameraHelper::GetCentralPerspectiveMatrix() const
    {
        return engine_->getWindowPtr(0)->getViewport(0)->getProjection(sgct_core::Frustum::MonoEye)->getProjectionMatrix();
    }

    glm::mat4 CameraHelper::GetCentralViewPerspectiveMatrix() const
    {
        auto result = CalculateViewUpdate();
        return engine_->getWindowPtr(0)->getViewport(0)->getProjection(sgct_core::Frustum::MonoEye)->getViewProjectionMatrix()
            * sgct_core::ClusterManager::instance()->getSceneTransform() * result;
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
        auto& localProps = localCoordsMatrices_[static_cast<std::size_t>(engine_->getCurrentWindowPtr()->getId())];
        glm::vec4 screenCoords = localProps.first * glm::vec4{ globalScreenCoords, 0.0f, 1.0 };
        glm::ivec2 iScreenCoords(screenCoords.x, screenCoords.y);
        screenCoords.x = screenCoords.x / localProps.second.x;
        screenCoords.y = 1.0f - screenCoords.y / localProps.second.y;

        glReadPixels(iScreenCoords.x, iScreenCoords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &screenCoords.z);
        screenCoords = glm::vec4((glm::vec3(screenCoords) * 2.0f) - 1.0f, screenCoords.w);
        spdlog::info("Picked position: ({})", glm::to_string(screenCoords));
        auto viewProjInv = glm::inverse(GetViewPerspectiveMatrix());

        auto postProjPos = viewProjInv * screenCoords;
        return glm::vec3(postProjPos) / postProjPos.w;
    }

    glm::mat4 CameraHelper::GetLocalCoordMatrix() const
    {
        // to local screen coordinates.
        auto& localProps = localCoordsMatrices_[static_cast<std::size_t>(engine_->getCurrentWindowPtr()->getId())];

        glm::mat4 localCoordMatrix = glm::mat4(1.0f);
        localCoordMatrix[0][0] = 0.5f;
        localCoordMatrix[1][1] = 0.5f;
        localCoordMatrix[3][0] = 0.5f;
        localCoordMatrix[3][1] = 0.5f;
        localCoordMatrix = localProps.first * localCoordMatrix;
        localCoordMatrix[0][0] = 2.0f * localCoordMatrix[0][0] / localProps.second.x;
        localCoordMatrix[3][0] = 2.0f * localCoordMatrix[3][0] / localProps.second.x - 1.0f;
        localCoordMatrix[1][1] = 2.0f * localCoordMatrix[1][1] / localProps.second.y;
        localCoordMatrix[3][1] = 2.0f * localCoordMatrix[3][1] / localProps.second.y - 1.0f;

        return localCoordMatrix;
    }

    glm::ivec2 CameraHelper::GetGlobalScreenSize() const
    {
        auto& localProps = localCoordsMatrices_[static_cast<std::size_t>(engine_->getCurrentWindowPtr()->getId())];
        return glm::ivec2(localProps.first[0][0], localProps.first[1][1]);
    }

    float CameraHelper::GetNearPlane() const
    {
        return engine_->getNearClippingPlane();
    }

    float CameraHelper::GetFarPlane() const
    {
        return engine_->getFarClippingPlane();
    }

    void CameraHelper::SetNearFarPlane(float nearPlane, float farPlane)
    {
        engine_->setNearAndFarClippingPlanes(nearPlane, farPlane);
    }

    glm::mat4 CameraHelper::CalculateViewUpdate() const
    {
        //4. transform user back to original position
        glm::mat4 result = glm::translate(glm::mat4(1.0f), GetUserPosition());
        //3. apply view rotation
        result *= glm::mat4_cast(camera_orientation_);
        //2. apply navigation translation
        result *= glm::translate(glm::mat4(1.0f), -position_);
        //1. transform user to coordinate system origin
        result *= glm::translate(glm::mat4(1.0f), -GetUserPosition());
        return result;
    }
}
