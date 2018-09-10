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
        virtual void UpdateCamera(double elapsedTime, const ApplicationNodeBase* sender) = 0;

        /** Returns the cameras projection matrix. */
        glm::mat4 GetViewProjMatrix() const noexcept { return cameraHelper_.GetViewPerspectiveMatrix(); }
        /** Returns the cameras position. */
        virtual glm::vec3 GetPosition() const noexcept { return cameraHelper_.GetPosition(); }
        /** Returns the cameras orientation. */
        const glm::quat& GetOrientation() const noexcept { return cameraHelper_.GetOrientation(); }

    public:
		/**
		*  Sets the cameras orientation.
		*  @param orientation new camera orientation.
		*/
        void SetCameraOrientation(const glm::quat& orientation);
		/**
		*  Sets the cameras position.
		*  @param position new camera position.
		*/
        virtual void SetCameraPosition(const glm::vec3& position);
		/** Returns the user position. */
        glm::vec3 GetUserPosition() const noexcept { return cameraHelper_.GetUserPosition(); }

    public:
        /** Holds the framework camera helper. */
        viscom::CameraHelper& cameraHelper_;
    };
}
