/**
 * @file   math.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Declaration of math primitives and methods.
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include "primitives.h"

// ReSharper disable CppDoxygenUnresolvedReference

namespace viscom { namespace math {

    /**
     *  2D cross product.
     *  @param real the floating point type used.
     *  @param v0 first vector
     *  @param v1 second vector
     *  @return the 2D cross product.
     */
    template<typename real>
    real crossz(const glm::tvec2<real, glm::highp>& v0, const glm::tvec2<real, glm::highp>& v1) {
        return (v0.x * v1.y) - (v0.y * v1.x);
    }

    /**
     *  Checks if two segments intersect.
     *  @param real the floating point type used.
     *  @param seg0 first segment.
     *  @param seg1 second segment.
     *  @return <code>true</code> if an intersection occurs.
     */
    template<typename real>
    bool segmentsIntersect(const Line2<real>& seg0, const Line2<real>& seg1) {
        // see http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

        glm::tvec2<real, glm::highp> r = seg0[1] - seg0[0];
        glm::tvec2<real, glm::highp> s = seg1[1] - seg1[0];

        real rxs = crossz(r, s);
        glm::tvec2<real, glm::highp> diff0 = seg1[0] - seg0[0];
        if (glm::abs(rxs) < glm::epsilon<real>()) {
            return false;
        }
        real t = crossz(diff0, s) / rxs;
        real u = crossz(diff0, r) / rxs;
        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) return true;
        return false;
    }

    /**
     *  Tests if a point is inside a triangle.
     *  @param real the floating point type used.
     *  @param tri the triangle to test against.
     *  @param p the point to test.
     *  @param testVal some value for debugging (see implementation).
     */
    template<typename real>
    bool pointInTriangleTest(const Tri3<real>& tri, const glm::tvec3<real, glm::highp>& p, real* testVal) {
        using vec3 = glm::tvec3<real, glm::highp>;

        vec3 ptest = p - tri[0];
        vec3 tv0 = tri[1] - tri[0];
        vec3 tv1 = tri[2] - tri[0];
        vec3 tn = glm::cross(tv0, tv1);
        if (glm::abs(glm::dot(ptest, tn)) < glm::epsilon<real>()) {
            real A2 = glm::dot(tn, tn) / 4.0f;
            vec3 tp0 = tri[0] - p;
            vec3 tp1 = tri[1] - p;
            vec3 tp2 = tri[2] - p;
            vec3 tn01 = glm::cross(tp0, tp1);
            vec3 tn12 = glm::cross(tp1, tp2);
            vec3 tn20 = glm::cross(tp2, tp0);
            real A01_2 = glm::dot(tn01, tn01) / 4.0f;
            real A12_2 = glm::dot(tn12, tn12) / 4.0f;
            real A20_2 = glm::dot(tn20, tn20) / 4.0f;

            real a = glm::sqrt(A01_2 / A2);
            real b = glm::sqrt(A12_2 / A2);
            real c = glm::sqrt(A20_2 / A2);

            if (testVal) *testVal = a + b + c;
            if (glm::abs(a + b + c - 1.0f) < glm::epsilon<real>()) return true;
        }
        return false;
    }

    /**
     *  Tests if a point is inside an AABB2.
     *  @param real the floating point type used.
     *  @param b the AABB2.
     *  @param p the point.
     */
    template<typename real> bool pointInAABB2Test(const AABB2<real>& b, const glm::tvec2<real, glm::highp>& p) {
        return (p.x >= b.minmax[0].x && p.y >= b.minmax[0].y && p.x <= b.minmax[1].x && p.y <= b.minmax[1].y);
    }

    /**
     *  Tests if a point is inside an AABB3.
     *  @param real the floating point type used.
     *  @param b the AABB3.
     *  @param p the point.
     */
    template<typename real> bool pointInAABB3Test(const AABB3<real>& b, const glm::tvec3<real, glm::highp>& p) {
        return (p.x >= b.minmax[0].x && p.y >= b.minmax[0].y && p.z >= b.minmax[0].z
            && p.x <= b.minmax[1].x && p.y <= b.minmax[1].y && p.z <= b.minmax[1].z);
    }

    /**
     *  Tests if two AABB2 overlap.
     *  @param real the floating point type used.
     *  @param b0 the first box.
     *  @param b1 the second box.
     */
    template<typename real> bool overlapAABB2Test(const AABB2<real>& b0, const AABB2<real>& b1) {
        return (pointInAABB2Test(b0, b1.minmax[0]) || pointInAABB2Test(b0, b1.minmax[1]));
    }

    /**
     *  Tests if two AABB3 overlap.
     *  @param real the floating point type used.
     *  @param b0 the first box.
     *  @param b1 the second box.
     */
    template<typename real> bool overlapAABB3Test(const AABB3<real>& b0, const AABB3<real>& b1) {
        return (pointInAABB3Test(b0, b1.minmax[0]) || pointInAABB3Test(b0, b1.minmax[1]));
    }

    /**
     *  Tests if one AABB2 (b0) is completely inside the other (b1).
     *  @param real the floating point type used.
     *  @param b0 the first box.
     *  @param b1 the second box.
     */
    template<typename real> bool containAABB2Test(const AABB2<real>& b0, const AABB2<real>& b1) {
        return (pointInAABB2Test(b0, b1.minmax[0]) && pointInAABB2Test(b0, b1.minmax[1]));
    }

    /**
     *  Tests if one AABB3 (b0) is completely inside the other (b1).
     *  @param real the floating point type used.
     *  @param b0 the first box.
     *  @param b1 the second box.
     */
    template<typename real> bool containAABB3Test(const AABB3<real>& b0, const AABB3<real>& b1) {
        return (pointInAABB3Test(b0, b1.minmax[0]) && pointInAABB3Test(b0, b1.minmax[1]));
    }

    /**
     *  Tests if a AABB3 is inside or intersected by a Frustum (culling test).
     *  @param real the floating point type used.
     *  @param f the frustum.
     *  @param b the box.
     */
    template<typename real> bool AABBInFrustumTest(const Frustum<real>& f, const AABB3<real>& b) {
        for (unsigned int i = 0; i < 6; ++i) {
            glm::vec3 p{ b.minmax[0] };
            if (f.planes[i].x >= 0) p.x = b.minmax[1].x;
            if (f.planes[i].y >= 0) p.y = b.minmax[1].y;
            if (f.planes[i].z >= 0) p.z = b.minmax[1].z;

            // is the positive vertex outside?
            if ((glm::dot(glm::vec3(f.planes[i]), p) + f.planes[i].w) < 0) return false;
        }
        return true;
    }
}}

// ReSharper restore CppDoxygenUnresolvedReference
