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
     *  @param theCamPos the cameras initial position.
     *  @param cameraHelper the cameraHelper object holding all information about the view frustum.
     */
    CameraBase::CameraBase(const glm::vec3& theCamPos, viscom::CameraHelper& cameraHelper) noexcept :
        cameraHelper_{ cameraHelper }
    {
        cameraHelper_.SetPosition(theCamPos);
        cameraHelper_.SetOrientation(glm::quat());
    }

    CameraBase::~CameraBase() = default;

    void CameraBase::SetCameraOrientation(const glm::quat& orientation)
    {
        cameraHelper_.SetOrientation(orientation);
    }

    void CameraBase::SetCameraPosition(const glm::vec3& position)
    {
        cameraHelper_.SetPosition(position);
    }

}
