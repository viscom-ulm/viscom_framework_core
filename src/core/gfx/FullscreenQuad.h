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
    class FrameworkInternal;

    /** Static helper class for having only a single VAO for all fullscreen quads. */
    class StaticFullscreenQuad
    {
    public:

    private:
        friend class FullscreenQuad;

        StaticFullscreenQuad();
        ~StaticFullscreenQuad();

		/** Generates the dummy vertex array object. */
        void Initialize();

        /** The dummy vertex array object needed for rendering. */
        GLuint dummyVAO_;
    };

	/** Fullscreen quad for rendering as used in deferred shading or various post effects. */
    class FullscreenQuad
    {
    public:
        FullscreenQuad(const std::string& fragmentProgram, FrameworkInternal* appNode);
        FullscreenQuad(const std::string& fragmentProgram, ApplicationNodeBase* appNode);
        FullscreenQuad(const std::string& shaderName, const std::string& fragmentProgram,
            const std::vector<std::string>& defines, ApplicationNodeBase* appNode);
        ~FullscreenQuad();

		/** Generates the static dummy vertex array object. */
        static void InitializeStatic();

		/** Renders using the fullscreen quad. */
        void Draw() const;
		/** Returns the GPU program of the fullscreen quad. */
        const GPUProgram* GetGPUProgram() const { return gpuProgram_.get(); }

    private:
        /** The static part of the FullscreenQuad. */
        static StaticFullscreenQuad staticQuad_;
        /** The GPU program used for drawing. */
        std::shared_ptr<GPUProgram> gpuProgram_;
    };
}
