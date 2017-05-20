/**
 * @file   FullscreenQuad.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2017.05.18
 *
 * @brief  Declaration of a renderable object for full screen quads.
 */

#pragma once

#include "main.h"
#include <sgct.h>
#include "GPUProgram.h"

namespace viscom {

    class FullscreenQuad
    {
    public:
        FullscreenQuad(const std::string& fragmentProgram, ApplicationNodeInternal* appNode);
        ~FullscreenQuad();

        void Draw() const;
        const GPUProgram* GetGPUProgram() const { return gpuProgram_.get(); }

    private:
        /** The dummy vertex array object needed for rendering. */
        GLuint dummyVAO_;
        /** The GPU program used for drawing. */
        std::shared_ptr<GPUProgram> gpuProgram_;
    };
}
