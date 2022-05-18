/**
 * @file   ArcballCamera.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.06.07
 *
 * @brief  Declaration of an arcball camera.
 */

#pragma once

#include "core/CameraHelper.h"
#include "core/camera/Arcball.h"
#include "core/camera/CameraBase.h"
#include <glm/gtc/quaternion.hpp>

namespace viscom {

    /**
    * Represents a camera rotating around the origin.
    */
    class ArcballCameraOpenVR final : public CameraBase
    {
    public:
        ArcballCameraOpenVR(const glm::vec3& camPos, viscom::CameraHelper& cameraHelper) noexcept;
        virtual ~ArcballCameraOpenVR() override;

        virtual bool HandleMouse(int button, int action, float mouseWheelDelta, const ApplicationNodeBase* sender) override;
        virtual void UpdateCamera(double elapsedTime, const ApplicationNodeBase* sender) override;

        virtual glm::vec3 GetPosition() const noexcept override;

    private:
        virtual void SetCameraPosition(const glm::vec3& position) override;

        /** Holds the current camera position. */
        glm::vec3 baseCamPos_;
        /** Holds the mouse wheel delta. */
        float mouseWheelDelta_;
        /** Holds the arc-ball used for camera rotation. */
        Arcball camArcball_;
    };
}
