/**
 * @file   Arcball.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.06.07
 *
 * @brief  Implementation of the arcball class.
 */

#include "Arcball.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <limits>
#include "core/ApplicationNodeBase.h"
#include <iostream>

#undef min
#undef max

namespace viscom {

    /**
     *  Constructor.
     *  @param theButton the mouse button to use.
     */
    Arcball::Arcball(int theButton) noexcept :
        button_{ theButton },
        arcballOn_{ false },
        currentScreen_{ 0.0f },
        lastScreen_{ 0.0f }
    {
    }

    /**
     *  Handles the mouse input to the arc-ball.
     *  @param button the mouse button the event belongs to.
     *  @param action the mouse buttons action.
     *  @param sender the application to supply normalized screen coordinates.
     */
    bool Arcball::HandleMouse(int theButton, int action, const ApplicationNodeBase* sender) noexcept
    {
        bool handled = false;
        if (button_ == theButton && action == GLFW_PRESS) {
            arcballOn_ = true;
            lastScreen_ = currentScreen_ = sender->GetMousePositionNormalized();
            handled = true;
        } else if (arcballOn_ && sender->IsMouseButtonPressed(button_)) {
            currentScreen_ = sender->GetMousePositionNormalized();
            handled = true;
        } else if (!sender->IsMouseButtonPressed(button_)) {
            handled = arcballOn_;
            arcballOn_ = false;
        }

        return handled;
    }

    /**
     *  Calculates the world rotation using a view matrix.
     *  @param view the view matrix.
     */
    glm::quat Arcball::GetWorldRotation(const glm::quat& camPosOrientation) noexcept
    {
        glm::quat result(1.0f, 0.0f, 0.0f, 0.0f);
        if (currentScreen_ != lastScreen_) {
            float angle = acos(glm::min(1.0f, glm::dot(lastScreen_, currentScreen_)));
            auto camAxis = glm::normalize(glm::cross(lastScreen_, currentScreen_));
            auto worldAxis = glm::normalize(glm::rotate(camPosOrientation, camAxis));
            result = glm::angleAxis(-2.0f * angle, worldAxis);
            lastScreen_ = currentScreen_;
        }
        return result;
    }
}
