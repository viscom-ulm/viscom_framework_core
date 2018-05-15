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


    template<typename real, int N, typename V> struct AABB {
        AABB() noexcept;
        AABB(const V& minValue, const V& maxValue);
        AABB(const std::vector<V>& points);
        AABB(const std::vector<AABB>& aabbs);

        V Size() const;

        const V& GetMin() const;
        const V& GetMax() const;

        void SetMin(const V&);
        void SetMax(const V&);

        void Scale(float scale);
        void Scale(const V& scale);
        void Offset(const V& offset);
        void Transform(const glm::tmat4x4<real, glm::highp>&);
        AABB NewFromTransform(const glm::tmat4x4<real, glm::highp>&) const;

        AABB Union(const AABB& other) const;
        AABB Difference(const AABB& other) const;

        bool IsIntersecting(const AABB& other) const;
        void AddPoint(const V&);

        /** Contains the minimum and maximum points of the box. */
        std::array<V, 2> minmax_;

    private:
        void FromPoints(const std::vector<V>&);
    };





    template<typename real, int N, typename V>
    inline AABB<real, N, V>::AABB() noexcept :
        minmax_{ V(std::numeric_limits<real>::max()), V(std::numeric_limits<real>::lowest()) }
    {
    }

    template<typename real, int N, typename V>
    inline AABB<real, N, V>::AABB(const V& minValue, const V& maxValue) :
        minmax_{ minValue, maxValue }
    {
    }

    template<typename real, int N, typename V>
    inline AABB<real, N, V>::AABB(const std::vector<V>& points) :
        minmax_{ V(std::numeric_limits<real>::max()), V(std::numeric_limits<real>::lowest()) }
    {
        FromPoints(points);
    }

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
