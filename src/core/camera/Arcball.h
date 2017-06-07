/**
 * @file   Arcball.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.06.07
 *
 * @brief  Declaration of an arcball.
 */

#pragma once

#include <glm/glm.hpp>

namespace viscom {

    class ApplicationNodeBase;

    /**
     * Helper class for generic arc-balls.
     */
    class Arcball final
    {
    public:
        Arcball(int button) noexcept;

        bool HandleMouse(int button, int action, const ApplicationNodeBase* sender) noexcept;
        glm::quat GetWorldRotation(const glm::quat& camPosOrientation) noexcept;

    private:
        /** Holds the button to use. */
        const int button_;
        /** Holds whether the arc-ball is currently rotated. */
        bool arcballOn_;
        /** holds the current arc-ball position in normalized device coordinates. */
        glm::vec3 currentScreen_;
        /** holds the last arc-ball position in normalized device coordinates. */
        glm::vec3 lastScreen_;

    };
}
