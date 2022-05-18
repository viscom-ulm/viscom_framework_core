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
    class ArcballCamera final : public CameraBase
    {
    public:
        ArcballCamera(const glm::vec3& camPos, viscom::CameraHelper& cameraHelper) noexcept;
        virtual ~ArcballCamera() override;

        virtual bool HandleMouse(int button, int action, float mouseWheelDelta,
                                 const ApplicationNodeBase* sender) override;
        virtual void UpdateCamera(double elapsedTime, const ApplicationNodeBase* sender) override;

        virtual glm::vec3 GetPosition() const noexcept override;

    private:
        virtual void SetCameraPosition(const glm::vec3& position) override;

        /** Holds the current camera position. */
        glm::vec3 baseCamPos_;
        /** Holds the mouse wheel delta. */
        float mouseWheelDelta_;
        /** Holds the arc-ball used for camera rotation. */
        glm::vec2 mousePosOld_;
        
        float sphereRadius_;
        glm::vec2 sphereAngle_;
        
        glm::vec3 eye_;
        glm::vec3 lookAt_;
        glm::vec3 upVector_;
        glm::vec2 mouseDelta_;

        glm::mat4x4 viewMatrix_;

        glm::mat4x4 GetViewMatrix() const { return viewMatrix_; }
        glm::vec3 GetEye() const { return eye_; }
        glm::vec3 GetUpVector() const { return upVector_; }
        glm::vec3 GetLookAt() const { return lookAt_; }

        glm::vec3 GetViewDir() const { return -glm::transpose(viewMatrix_)[2]; }
        glm::vec3 GetRightVector() const { return glm::transpose(viewMatrix_)[0]; }

        void SetCameraView(glm::vec3 eye, glm::vec3 lookat, glm::vec3 up);
        void UpdateViewMatrix();
    };
} // namespace viscom
