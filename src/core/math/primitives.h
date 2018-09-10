/**
 * @file   primitives.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Declaration of some math primitives.
 */

#pragma once

#include "aabb.h"
#include <array>
#include <glm/glm.hpp>

#undef near
#undef far

namespace viscom::math {

    template<typename real> using Line2 = std::array<glm::tvec2<real, glm::highp>, 2>;
    template<typename real> using Line3 = std::array<glm::tvec3<real, glm::highp>, 2>;

    template<typename real> using Tri2 = std::array<glm::tvec2<real, glm::highp>, 3>;
    template<typename real> using Tri3 = std::array<glm::tvec3<real, glm::highp>, 3>;

	/** Holds the information defining a frustum as used for culling. */
    template<typename real> struct Frustum {
		/** The six planes defining the frustum. */
        std::array<glm::tvec4<real, glm::highp>, 6> planes;

		/** Returns the left plane of the frustum. */
        glm::tvec4<real, glm::highp>& left() { return planes[0]; }
		/** Returns the left plane of the frustum. */
        const glm::tvec4<real, glm::highp>& left() const { return planes[0]; }
		/** Returns the right plane of the frustum. */
        glm::tvec4<real, glm::highp>& right() { return planes[1]; }
		/** Returns the right plane of the frustum. */
        const glm::tvec4<real, glm::highp>& right() const { return planes[1]; }
		/** Returns the top plane of the frustum. */
        glm::tvec4<real, glm::highp>& top() { return planes[2]; }
		/** Returns the top plane of the frustum. */
        const glm::tvec4<real, glm::highp>& top() const { return planes[2]; }
		/** Returns the bottom plane of the frustum. */
        glm::tvec4<real, glm::highp>& bttm() { return planes[3]; }
		/** Returns the bottom plane of the frustum. */
        const glm::tvec4<real, glm::highp>& bttm() const { return planes[3]; }
		/** Returns the near plane of the frustum. */
        glm::tvec4<real, glm::highp>& near() { return planes[4]; }
		/** Returns the near plane of the frustum. */
        const glm::tvec4<real, glm::highp>& near() const { return planes[4]; }
		/** Returns the far plane of the frustum. */
        glm::tvec4<real, glm::highp>& far() { return planes[5]; }
		/** Returns the far plane of the frustum. */
        const glm::tvec4<real, glm::highp>& far() const { return planes[5]; }
    };
}
