/**
 * @file   Camera.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.06.07
 *
 * @brief  Implementation of an arcball camera.
 */

#include "Camera.h"

#define GLM_SWIZZLE
#include <core/open_gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>

namespace viscom {

    /**
     *  Constructor.
     *  @param theFovY the field of view in y direction.
     *  @param theAspectRatio the screens aspect ratio.
     *  @param theScreenSize the screen size.
     *  @param theNearZ the near z plane
     *  @param theFarZ the far z plane
     *  @param theCamPos the cameras initial position.
     */
    Camera::Camera(const glm::vec3& theCamPos, viscom::CameraHelper& cameraHelper) noexcept :
        CameraBase(theCamPos, cameraHelper),
        radius_{ 1.0f },
        baseCamPos_{ theCamPos },
        camArcball_{ GLFW_MOUSE_BUTTON_1 }
    {
    }

    Camera::~Camera() = default;

    /**
     *  Updates the camera parameters using the internal arc-ball.
     */
    void Camera::UpdateCamera(const ApplicationNodeBase*)
    {
        auto camOrient = glm::inverse(GetOrientation());
        glm::quat camOrientStep = camArcball_.GetWorldRotation(camOrient);
        camOrient = camOrientStep * camOrient;
        glm::mat3 matOrient{ glm::mat3_cast(camOrient) };
        auto camPos = radius_ * (matOrient * baseCamPos_);

        SetCameraPosition(camPos);
        SetCameraOrientation(glm::inverse(camOrient));
    }

    /**
     *  Handles the mouse events for the camera.
     *  @param button the mouse button the event belongs to.
     *  @param action the mouse buttons action.
     *  @param sender the application to supply normalized screen coordinates.
     */
    bool Camera::HandleMouse(int button, int action, float mouseWheelDelta, const ApplicationNodeBase* sender)
    {
        bool handled = camArcball_.HandleMouse(button, action, sender);

        if (mouseWheelDelta != 0) {
            radius_ -= mouseWheelDelta * 0.1f;
            radius_ = glm::clamp(radius_, 0.01f, 20.0f);
            handled = true;
        }

        return handled;
    }
}
