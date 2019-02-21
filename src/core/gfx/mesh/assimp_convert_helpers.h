/**
 *  @file   assimp_convert_helpers.h
 *  @author Christian van Onzenoodt
 *  @date   09.05.2017
 *  @brief  Helper functions for converting assimp data types into glm data types.
 */
#pragma once

#include <assimp/matrix4x4.h>
#include <glm/mat4x4.hpp>

namespace viscom {

    /**
     *  Converts a assimp aiMatrix4x4 into a glm::mat4.
     * 
     *  @param from Matrix to convert.
     * 
     *  @return Converted Matrix.
     */
    static glm::mat4 AiMatrixToGLM(const aiMatrix4x4& from)
    {
        glm::mat4 to;

        to[0][0] = from.a1;
        to[1][0] = from.a2;
        to[2][0] = from.a3;
        to[3][0] = from.a4;
        to[0][1] = from.b1;
        to[1][1] = from.b2;
        to[2][1] = from.b3;
        to[3][1] = from.b4;
        to[0][2] = from.c1;
        to[1][2] = from.c2;
        to[2][2] = from.c3;
        to[3][2] = from.c4;
        to[0][3] = from.d1;
        to[1][3] = from.d2;
        to[2][3] = from.d3;
        to[3][3] = from.d4;

        return to;
    }

    /**
     *  Converts a glm::mat4 into a assimp aiMatrix4x4.
     * 
     *  @param from Matrix to convert.
     * 
     *  @return Converted Matrix.
     */
    static aiMatrix4x4 GLMToAiMatrix(const glm::mat4& from)
    {
        aiMatrix4x4 to;

        to.a1 = from[0][0];
        to.a2 = from[1][0];
        to.a3 = from[2][0];
        to.a4 = from[3][0];
        to.b1 = from[0][1];
        to.b2 = from[1][1];
        to.b3 = from[2][1];
        to.b4 = from[3][1];
        to.c1 = from[0][2];
        to.c2 = from[1][2];
        to.c3 = from[2][2];
        to.c4 = from[3][2];
        to.d1 = from[0][3];
        to.d2 = from[1][3];
        to.d3 = from[2][3];
        to.d4 = from[3][3];

        return to;
    }

} // namespace get
