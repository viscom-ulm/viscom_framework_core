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
        
    /**
     *  Performs a transformation and yields a new bounding box.
     *  @param aabb the bounding box to be transformed.
     *  @param m the transformation matrix.
     */
    template<class T> AABB3<T> transformAABB(const AABB3<T>& aabb, const glm::mat4& m)
    {
        AABB3<T> result{ glm::vec3(std::numeric_limits<float>::infinity()), glm::vec3(-std::numeric_limits<float>::infinity()) };
        for (auto i = 0; i < 8; ++i) {
            glm::vec3 pt{ aabb.minmax_[(i & 0x4) == 0x4].x, aabb.minmax_[(i & 0x2) == 0x2].y, aabb.minmax_[i & 0x1].z };
            auto ptTransformed = glm::vec3(m * glm::vec4(pt, 1.0f));
            result.minmax_[0] = glm::min(result.minmax_[0], ptTransformed);
            result.minmax_[1] = glm::max(result.minmax_[1], ptTransformed);
        }
        return result;
    }
}}
