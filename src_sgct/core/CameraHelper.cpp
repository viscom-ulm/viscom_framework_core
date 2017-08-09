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
    CameraHelper::CameraHelper(sgct::Engine * engine) :
        engine_{ engine }
    {
    }

    glm::vec3 CameraHelper::GetUserPosition() const
    {
        return sgct::Engine::getDefaultUserPtr()->getPos();
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

    glm::mat4 CameraHelper::CalculateViewUpdate() const
    {
        //4. transform user back to original position
        glm::mat4 result = glm::translate(glm::mat4(1.0f), sgct::Engine::getDefaultUserPtr()->getPos());
        //3. apply view rotation
        result *= glm::mat4_cast(camera_orientation_);
        //2. apply navigation translation
        result *= glm::translate(glm::mat4(1.0f), -position_);
        //1. transform user to coordinate system origin
        result *= glm::translate(glm::mat4(1.0f), -sgct::Engine::getDefaultUserPtr()->getPos());
        return result;
    }
}
