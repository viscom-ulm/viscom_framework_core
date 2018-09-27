/**
 * @file   aabb.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.01.08
 *
 * @brief  Declaration of AABBs.
 */

#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vector>

namespace viscom::math {


    /** Class defining and handling all axis aligned bounding box operations. */
    template<typename real, int N, typename V> struct AABB {
        AABB() noexcept;
        AABB(const V& minValue, const V& maxValue);
        AABB(const std::vector<V>& points);
        AABB(const std::vector<AABB>& aabbs);

        /** Returns the dimensions of the bounding box. */
        V Size() const;

        /** Returns the minimum of the bounding box. */
        const V& GetMin() const;
        /** Returns the maximum of the bounding box. */
        const V& GetMax() const;

        /**
         *  Sets the minimum of the bounding box.
         *  @param v the new minimum.
         */
        void SetMin(const V&);
        /**
         *  Sets the maximum of the bounding box.
         *  @param v the new maximum.
         */
        void SetMax(const V&);

        /**
         *  Scales the bounding box.
         *  @param scale the scaling factor.
         */
        void Scale(float scale);
        /**
         *  Scales the bounding box.
         *  @param scale the scaling factor.
         */
        void Scale(const V& scale);
        /**
         *  Translates the bounding box.
         *  @param offset the translation offset.
         */
        void Offset(const V& offset);
        /**
         *  Performs a transformation on the bounding box.
         *  @param mat the transformation matrix.
         */
        void Transform(const glm::tmat4x4<real, glm::highp>&);
        /**
         *  Performs a transformation and yields a new bounding box.
         *  @param mat the transformation matrix.
         */
        AABB NewFromTransform(const glm::tmat4x4<real, glm::highp>&) const;

        /**
         *  Calculates the union of two bounding boxes.
         *  @param other the second bounding box.
         */
        AABB Union(const AABB& other) const;
        /**
         *  Calculates the intersection of two bounding boxes.
         *  @param other the second bounding box.
         */
        AABB Difference(const AABB& other) const;

        /**
         *  Checks if two bounding boxes intersect.
         *  @param other the second bounding box.
         */
        bool IsIntersecting(const AABB& other) const;
        /**
         *  Adds a point to the bounding box.
         *  @param point the point to add.
         */
        void AddPoint(const V&);

        /** Contains the minimum and maximum points of the box. */
        std::array<V, 2> minmax_;

    private:
        /**
         *  Defines new min and max values from a set of points.
         *  @param points the list of points to create the AABB with.
         */
        void FromPoints(const std::vector<V>&);
    };




    /** Constructor method. */
    template<typename real, int N, typename V>
    inline AABB<real, N, V>::AABB() noexcept :
        minmax_{ V(std::numeric_limits<real>::max()), V(std::numeric_limits<real>::lowest()) }
    {
    }

    /**
     *  Constructor method.
     *  @param minValue minimum corner for the AABB.
     *  @param maxValue maximum corner for the AABB.
     */
    template<typename real, int N, typename V>
    inline AABB<real, N, V>::AABB(const V& minValue, const V& maxValue) :
        minmax_{ minValue, maxValue }
    {
    }

    /**
     *  Constructor method.
     *  @param points list of points to retrieve bounding box information from.
     */
    template<typename real, int N, typename V>
    inline AABB<real, N, V>::AABB(const std::vector<V>& points) :
        minmax_{ V(std::numeric_limits<real>::max()), V(std::numeric_limits<real>::lowest()) }
    {
        FromPoints(points);
    }

    /**
     *  Constructor method.
     *  @param aabbs list of AABB to calculate new bounding box.
     */
    template<typename real, int N, typename V>
    inline AABB<real, N, V>::AABB(const std::vector<AABB>& aabbs) :
        minmax_{ V(std::numeric_limits<real>::max()), V(std::numeric_limits<real>::lowest()) }
    {
        if (aabbs.size() == 1) {
            minmax_[0] = aabbs[0].minmax_[0];
            minmax_[1] = aabbs[0].minmax_[1];
            return;
        }

        auto newBox = aabbs[0].Union(aabbs[1]);

        for (auto i = 2U; i < aabbs.size(); ++i) {
            newBox = newBox.Union(aabbs[i]);
        }

        minmax_[0] = newBox.minmax_[0];
        minmax_[1] = newBox.minmax_[1];
    }

    template<typename real, int N, typename V>
    inline V AABB<real, N, V>::Size() const
    {
        return glm::abs(minmax_[1] - minmax_[0]);
    }

    template<typename real, int N, typename V>
    inline const V& AABB<real, N, V>::GetMin() const
    {
        return minmax_[0];
    }

    template<typename real, int N, typename V>
    inline const V& AABB<real, N, V>::GetMax() const
    {
        return minmax_[1];
    }

    template<typename real, int N, typename V>
    inline void AABB<real, N, V>::SetMin(const V& v)
    {
        minmax_[0] = v;
    }

    template<typename real, int N, typename V>
    inline void AABB<real, N, V>::SetMax(const V& v)
    {
        minmax_[1] = v;
    }

    template<typename real, int N, typename V>
    inline void AABB<real, N, V>::Scale(float scale)
    {
        minmax_[0] *= scale;
        minmax_[1] *= scale;
    }

    template<typename real, int N, typename V>
    inline void AABB<real, N, V>::Scale(const V& scale)
    {
        minmax_[0] *= scale;
        minmax_[1] *= scale;
    }

    template<typename real, int N, typename V>
    inline void AABB<real, N, V>::Offset(const V& offset)
    {
        minmax_[0] += offset;
        minmax_[1] += offset;
    }

    template<typename real, int N, typename V, int I>
    struct AABBInternal {
        static void permuteMultiply(std::vector<V>& result, const glm::tmat4x4<real, glm::highp>& mat, const glm::tvec4<real, glm::highp>& v, const AABB<real, N, V>& aabb)
        {
            auto v0 = v;
            v0[I] = aabb.minmax_[0][I];
            auto v1 = v;
            v1[I] = aabb.minmax_[1][I];
            AABBInternal<real, N, V, I - 1>::permuteMultiply(result, mat, v0, aabb);
            AABBInternal<real, N, V, I - 1>::permuteMultiply(result, mat, v1, aabb);
        }
    };

    template<typename real, int N, typename V>
    struct AABBInternal<real, N, V, 0> {
        static void permuteMultiply(std::vector<V>& result, const glm::tmat4x4<real, glm::highp>& mat, const glm::tvec4<real, glm::highp>& v, const AABB<real, N, V>& aabb)
        {
            auto v0 = v;
            v0[0] = aabb.minmax_[0][0];
            auto v1 = v;
            v1[0] = aabb.minmax_[1][0];
            result.emplace_back(mat * v0);
            result.emplace_back(mat * v1);
        }
    };

    template<typename real, int N, typename V>
    inline void AABB<real, N, V>::Transform(const glm::tmat4x4<real, glm::highp>& mat)
    {

        std::vector<V> newCorners;
        AABBInternal<real, N, V, N - 1>::permuteMultiply(newCorners, mat, glm::tvec4<real, glm::highp>{1.0f}, *this);

        FromPoints(newCorners);
    }

    template<typename real, int N, typename V>
    inline AABB<real, N, V> AABB<real, N, V>::NewFromTransform(const glm::tmat4x4<real, glm::highp>& mat) const
    {
        auto tmp = *this;
        tmp.Transform(mat);
        return tmp;
    }

    template<typename real, int N, typename V>
    inline AABB<real, N, V> AABB<real, N, V>::Union(const AABB& other) const
    {
        const auto newMin = glm::min(minmax_[0], other.minmax_[0]);
        const auto newMax = glm::max(minmax_[1], other.minmax_[1]);

        return AABB(newMin, newMax);
    }

    template<typename real, int N, typename V>
    inline AABB<real, N, V> AABB<real, N, V>::Difference(const AABB& other) const
    {
        const auto min = glm::max(minmax_[0], other.minmax_[0]);
        const auto max = glm::min(minmax_[1], other.minmax_[1]);

        return AABB(min, max);
    }

    template<typename real, int N, typename V>
    inline bool AABB<real, N, V>::IsIntersecting(const AABB& other) const
    {
        bool result = true;
        for (glm::length_t i = 0; i < N; ++i) result = result && minmax_[0][i] <= other.minmax_[1][i] && minmax_[1][i] >= other.minmax_[0][i];
        return result;
    }

    template<typename real, int N, typename V>
    inline void AABB<real, N, V>::AddPoint(const V& point)
    {
        minmax_[0] = glm::min(minmax_[0], point);
        minmax_[1] = glm::max(minmax_[1], point);
    }

    template<typename real, int N, typename V>
    inline void AABB<real, N, V>::FromPoints(const std::vector<V>& points)
    {
        if (points.size() <= 0) return;

        minmax_[0] = glm::vec3(std::numeric_limits<float>::max());
        minmax_[1] = glm::vec3(std::numeric_limits<float>::lowest());

        for (const auto& point : points) AddPoint(point);
    }


    template<typename real> using AABB2 = AABB<real, 2, glm::tvec2<real, glm::highp>>;
    template<typename real> using AABB3 = AABB<real, 3, glm::tvec3<real, glm::highp>>;
}
