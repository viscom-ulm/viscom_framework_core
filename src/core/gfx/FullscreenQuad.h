/**
 * @file   FullscreenQuad.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.05.18
 *
 * @brief  Declaration of a renderable object for full screen quads.
 */

#pragma once

#include "core/main.h"
#include "core/open_gl_fwd.h"
#include "GPUProgram.h"

namespace viscom {

    class ApplicationNodeBase;

    class StaticFullscreenQuad
    {
    private:
        friend class FullscreenQuad;

        StaticFullscreenQuad();
        ~StaticFullscreenQuad();

        /** The dummy vertex array object needed for rendering. */
        GLuint dummyVAO_;
    };

    class FullscreenQuad
    {
    public:
        FullscreenQuad(const std::string& fragmentProgram, ApplicationNodeInternal* appNode);
        FullscreenQuad(const std::string& fragmentProgram, ApplicationNodeBase* appNode);
        FullscreenQuad(const std::string& shaderName, const std::string& fragmentProgram,
            const std::vector<std::string>& defines, ApplicationNodeBase* appNode);
        ~FullscreenQuad();

        void Draw() const;
        const GPUProgram* GetGPUProgram() const { return gpuProgram_.get(); }

    private:
        /** The static part of the FullscreenQuad. */
        static StaticFullscreenQuad staticQuad_;
        /** The GPU program used for drawing. */
        std::shared_ptr<GPUProgram> gpuProgram_;
    };
}
