/**
 * @file   Camera.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.06.07
 *
 * @brief  Declaration of an arcball camera.
 */

#pragma once

#include "core/camera/CameraBase.h"
#include "core/CameraHelper.h"
#include "core/camera/Arcball.h"
#include <glm/gtc/quaternion.hpp>

namespace viscom {

    /**
    * Represents a camera rotating around the origin.
    */
    class Camera final : public CameraBase
    {
    public:
        Camera(const glm::vec3& camPos, viscom::CameraHelper& cameraHelper) noexcept;
        virtual ~Camera() override;

        virtual bool HandleMouse(int button, int action, float mouseWheelDelta, const ApplicationNodeBase* sender) override;
        virtual void UpdateCamera(const ApplicationNodeBase* sender) override;

    private:
        /** Holds the radius of the arcball. */
        float radius_;
        /** Holds the current camera position. */
        glm::vec3 baseCamPos_;
        /** Holds the arc-ball used for camera rotation. */
        Arcball camArcball_;
    };
}
