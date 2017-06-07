/**
 * @file   FreeCamera.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.06.07
 *
 * @brief  Implementation of the free flight camera.
 */

#include "FreeCamera.h"

#define GLM_SWIZZLE

#include <core/open_gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "core/ApplicationNodeBase.h"

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
    FreeCamera::FreeCamera(const glm::vec3& theCamPos, viscom::CameraHelper& cameraHelper) noexcept :
        CameraBase(theCamPos, cameraHelper)
    {
        // glm::scale(camOrient_, glm::vec3(1.0f, -1.0f, 1.0f));
        // view_ = glm::lookAt(camPos_, camPos_ - glm::vec3(0.0f, 0.0f, 1.0f), camUp_);
    }

    bool FreeCamera::HandleMouse(int button, int action, float mouseWheelDelta, const ApplicationNodeBase* sender)
    {
        glm::vec3 mouseVel = sender->GetMousePositionNormalized();
        const float speed = 1.0f;
        static float pitch, yaw;

        yaw = mouseVel.x * speed;
        pitch = mouseVel.y * speed;

        pitch = glm::clamp(pitch, -glm::half_pi<float>(), glm::half_pi<float>());

        if (yaw < -glm::two_pi<float>()) yaw += glm::two_pi<float>();
        if (yaw > glm::two_pi<float>()) yaw -= glm::two_pi<float>();

        glm::quat pitchQuat = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat yawQuat = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));


        SetCameraOrientation(yawQuat * pitchQuat);
        return true;
    }

    /**
     *  Updates the camera parameters using the internal arc-ball.
     */
    void FreeCamera::UpdateCamera(const ApplicationNodeBase* sender)
    {
        glm::vec3 camMove{ 0.0f };
        if (sender->IsKeyPressed(GLFW_KEY_W)) camMove += glm::vec3(0.0f, 0.0f, 0.04f);
        if (sender->IsKeyPressed(GLFW_KEY_A)) camMove += glm::vec3(0.04f, 0.0f, 0.0f);
        if (sender->IsKeyPressed(GLFW_KEY_S)) camMove -= glm::vec3(0.0f, 0.0f, 0.04f);
        if (sender->IsKeyPressed(GLFW_KEY_D)) camMove -= glm::vec3(0.04f, 0.0f, 0.0f);
        // TODO: More keys for y-movement? [1/13/2016 Sebastian Maisch]

        auto camPos = GetPosition() + glm::vec3(glm::inverse(GetOrientation()) * glm::vec4(camMove, 1.0f));

        SetCameraPosition(camPos);
    }
}
