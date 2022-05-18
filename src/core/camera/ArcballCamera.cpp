/**
 * @file   ArcballCamera.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.06.07
 *
 * @brief  Implementation of an arcball camera.
 */

#include "ArcballCamera.h"
#include "core/app/ApplicationNodeBase.h"

#include "core/glfw.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

namespace viscom {

    /**
     *  Constructor.
     *  @param theCamPos the cameras initial position.
     *  @param cameraHelper the camera helper class.
     */
    ArcballCamera::ArcballCamera(const glm::vec3& theCamPos, viscom::CameraHelper& cameraHelper) noexcept
        : CameraBase(theCamPos, cameraHelper),
          baseCamPos_{glm::normalize(theCamPos)},
          mouseWheelDelta_{0.0f},
          mousePosOld_{0.0f},
          mouseDelta_{0.0f},
          eye_(theCamPos),
          lookAt_(GetUserPosition()),
          upVector_(glm::vec3(0.0, 1.0, 0.0)),
          sphereRadius_{glm::length(theCamPos)},
          sphereAngle_{0.0f}
          
    {
        UpdateViewMatrix();
    }

    void ArcballCamera::SetCameraView(glm::vec3 eye, glm::vec3 lookat, glm::vec3 up)
    {
        eye_ = std::move(eye);
        lookAt_ = std::move(lookat);
        upVector_ = std::move(up);
        UpdateViewMatrix();
    }

    void ArcballCamera::UpdateViewMatrix()
    {
        // Generate view matrix using the eye, lookAt and up vector
        viewMatrix_ = glm::lookAt(eye_, lookAt_, upVector_);
    }

    ArcballCamera::~ArcballCamera() = default;

    /**
     *  Updates the camera parameters using the internal arc-ball.
     */
    void ArcballCamera::UpdateCamera(double elapsedTime, const ApplicationNodeBase* sender)
    {
        const auto mouseWheelSpeed = 1.0f;
        //
        sphereRadius_ += mouseWheelDelta_ * mouseWheelSpeed;
        sphereRadius_ = glm::clamp(sphereRadius_, 1.0f, 20.0f);
        mouseWheelDelta_ = 0.0f;
        
        sphereAngle_ = mouseDelta_ * glm::vec2(2.0f, -1.5f);
        
             
        // reset to zero because handlemouse is not called every frame.
        mouseDelta_ = glm::vec2(0.0);

        
        // Get the homogenous position of the camera and pivot point
        glm::vec4 position(GetEye().x, GetEye().y, GetEye().z, 1);
        glm::vec4 pivot(GetLookAt().x, GetLookAt().y, GetLookAt().z, 1);
        
        // step 2 Rotate the camera around the view direction.
        //auto transformation = glm::translate(-glm::vec3(pivot));
        //transformation = glm::rotate(transformation, sphereAngle_.y, GetViewDir());
        //transformation = glm::rotate(transformation, sphereAngle_.x, GetUpVector());
        //transformation = glm::translate(transformation, glm::vec3(pivot));
        //const auto finalPosition = transformation * position;
        
        // step 2: Rotate the camera around the pivot point on the first axis.
        glm::mat4x4 rotationMatrixX(1.0f);
        rotationMatrixX = glm::rotate(rotationMatrixX, sphereAngle_.x, GetUpVector());
        position = (rotationMatrixX * (position - pivot)) + pivot;
        
        // step 3: Rotate the camera around the pivot point on the second axis.
        glm::mat4x4 rotationMatrixY(1.0f);
        rotationMatrixY = glm::rotate(rotationMatrixY, sphereAngle_.y, GetRightVector());
        glm::vec4 finalPosition = (rotationMatrixY * (position - pivot)) + pivot;


        // Update the camera view (we keep the same lookat and the same up vector)
        SetCameraView(finalPosition, GetLookAt(), GetUpVector());
        
                
        auto camOrient = glm::quatLookAt(GetViewDir(), GetUpVector());

        glm::mat3 matOrient{glm::mat3_cast(camOrient)};
        auto camPos = sphereRadius_ * (matOrient * baseCamPos_);
        //auto camPos = finalPosition;
        
        SetCameraPosition(camPos);
        SetCameraOrientation(glm::inverse(camOrient));
    }

    /**
     *  Handles the mouse events for the camera.
     *  @param button the mouse button the event belongs to.
     *  @param action the mouse buttons action.
     *  @param mouseWheelDelta the change in the mousewheel rotation.
     *  @param sender the application to supply normalized screen coordinates.
     */
    bool ArcballCamera::HandleMouse(int button, int action, float mouseWheelDelta, const ApplicationNodeBase* sender)
    {
        auto mousePos = sender->GetMousePositionNormalized();

        if (sender->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            mouseDelta_ = mousePosOld_ - mousePos;
        }

        mousePosOld_ = mousePos;

        auto handled = glm::length(mouseDelta_) > 0;
        if (glm::abs(mouseWheelDelta) > 0.5f) { //-V550
            mouseWheelDelta_ = mouseWheelDelta;
            handled = true;
        }

        return handled;
    }

    void ArcballCamera::SetCameraPosition(const glm::vec3& position)
    {
        CameraBase::SetCameraPosition(position - GetUserPosition());
    }

    glm::vec3 ArcballCamera::GetPosition() const noexcept { return CameraBase::GetPosition() + GetUserPosition(); }

} // namespace viscom
