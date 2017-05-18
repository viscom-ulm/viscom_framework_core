/**
 * @file   FullscreenQuad.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.05.18
 *
 * @brief  Implementation of the renderable object for full screen quads.
 */

#include "FullscreenQuad.h"
#include "core/ApplicationNode.h"

namespace viscom {

    FullscreenQuad::FullscreenQuad(const std::string& fragmentProgram, ApplicationNode* appNode) :
        dummyVAO_{ 0 },
        gpuProgram_{ appNode->GetGPUProgramManager().GetResource("fullScreenQuad_" + fragmentProgram, std::initializer_list<std::string>{ "fullScreenQuad.vert", fragmentProgram }) }
    {
        glGenVertexArrays(1, &dummyVAO_);
    }

    FullscreenQuad::~FullscreenQuad()
    {
        if (dummyVAO_ != 0) glDeleteVertexArrays(1, &dummyVAO_);
        dummyVAO_ = 0;
    }

    void FullscreenQuad::Draw() const
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glBindVertexArray(dummyVAO_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }
}
