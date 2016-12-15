/**
 * @file   transforms.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Definition of math helper functions for transforming primitives.
 */

#pragma once

#include "primitives.h"

namespace viscom {
    namespace math {

    template<class T> AABB3<T> transformAABB(const AABB3<T>& aabb, const glm::mat4& m)
    {
        AABB3<T> result{ { { glm::vec3(std::numeric_limits<float>::infinity()), glm::vec3(-std::numeric_limits<float>::infinity()) } } };
        for (auto i = 0; i < 8; ++i) {
            glm::vec3 pt{ aabb.minmax[(i & 0x4) == 0x4].x, aabb.minmax[(i & 0x2) == 0x2].y, aabb.minmax[i & 0x1].z };
            auto ptTransformed = glm::vec3(m * glm::vec4(pt, 1.0f));
            result.minmax[0] = glm::min(result.minmax[0], ptTransformed);
            result.minmax[1] = glm::max(result.minmax[1], ptTransformed);
        }
        return result;
    }
}}
