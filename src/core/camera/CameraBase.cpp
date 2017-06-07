/**
 * @file   CameraBase.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.06.07
 *
 * @brief  Implementation of the camera base class.
 */

#include "CameraBase.h"

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
    CameraBase::CameraBase(const glm::vec3& theCamPos, viscom::CameraHelper& cameraHelper) noexcept :
        camPos_{ theCamPos },
        cameraHelper_{ cameraHelper }
    {
        cameraHelper_.SetPosition(camPos_);
    }

    CameraBase::~CameraBase() = default;

    void CameraBase::SetCameraPosition(const glm::vec3& position)
    {
        camPos_ = position;
        cameraHelper_.SetPosition(position);
    }

}
