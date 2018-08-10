/**
 * @file   FullscreenQuad.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.05.18
 *
 * @brief  Implementation of the renderable object for full screen quads.
 */

#include "FullscreenQuad.h"
#include "core/open_gl.h"
#include "core/app/ApplicationNodeBase.h"
#include "core/app_internal/ApplicationNodeInternal.h"

namespace viscom {

    StaticFullscreenQuad FullscreenQuad::staticQuad_;

    void StaticFullscreenQuad::Initialize()
    {
        glGenVertexArrays(1, &dummyVAO_);
    }

    StaticFullscreenQuad::StaticFullscreenQuad() :
        dummyVAO_{ 0 }
    {
    }

    StaticFullscreenQuad::~StaticFullscreenQuad()
    {
        if (dummyVAO_ != 0) glDeleteVertexArrays(1, &dummyVAO_);
        dummyVAO_ = 0;
    }

    FullscreenQuad::FullscreenQuad(const std::string& fragmentProgram, FrameworkInternal* appNode) :
        gpuProgram_{ appNode->GetGPUProgramManager().GetResource("fullScreenQuad_" + fragmentProgram, std::vector<std::string>{ "fullScreenQuad.vert", fragmentProgram }) }
    {
    }

    FullscreenQuad::FullscreenQuad(const std::string& fragmentProgram, ApplicationNodeBase* appNode) :
        gpuProgram_{ appNode->GetGPUProgramManager().GetResource("fullScreenQuad_" + fragmentProgram, std::vector<std::string>{ "fullScreenQuad.vert", fragmentProgram }) }
    {
    }

    FullscreenQuad::FullscreenQuad(const std::string& shaderName, const std::string& fragmentProgram,
        const std::vector<std::string>& defines, ApplicationNodeBase* appNode) :
        gpuProgram_{ appNode->GetGPUProgramManager().GetResource("fullScreenQuad_" + shaderName, std::vector<std::string>{ "fullScreenQuad.vert", fragmentProgram }, defines) }
    {
    }

    FullscreenQuad::~FullscreenQuad() = default;

    void FullscreenQuad::InitializeStatic()
    {
        staticQuad_.Initialize();
    }

    void FullscreenQuad::Draw() const
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glBindVertexArray(staticQuad_.dummyVAO_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }
}
