/**
 * @file   CameraBase.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.06.07
 *
 * @brief  Declaration of the camera base class.
 */

#pragma once

#include "core/CameraHelper.h"

namespace viscom {

    class ApplicationNodeBase;

    /**
    * Represents a camera rotating around the origin.
    */
    class CameraBase
    {
    public:
        CameraBase(const glm::vec3& camPos, viscom::CameraHelper& cameraHelper) noexcept;
        virtual ~CameraBase();

        virtual bool HandleMouse(int button, int action, float mouseWheelDelta, const ApplicationNodeBase* sender) = 0;
        virtual void UpdateCamera(const ApplicationNodeBase* sender) = 0;

        /** Returns the cameras projection matrix. */
        glm::mat4 GetViewProjMatrix() const noexcept { return cameraHelper_.GetViewPerspectiveMatrix(); }
        /** Returns the cameras position. */
        const glm::vec3& GetPosition() const noexcept { return camPos_; }

    protected:
        void SetCameraOrientation(const glm::quat& orientation) { cameraHelper_.SetOrientation(orientation); }
        void SetCameraPosition(const glm::vec3& position);

    private:
        /** Holds the current camera position. */
        glm::vec3 camPos_;
        /** Holds the framework camera helper. */
        viscom::CameraHelper& cameraHelper_;
    };
}
