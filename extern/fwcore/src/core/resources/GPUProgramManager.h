/**
 * @file   GPUProgramManager.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Declaration of the GPU program manager.
 */

#pragma once

#include "ResourceManager.h"
#include "core/gfx/GPUProgram.h"

namespace viscom {

    class GPUProgramManager : public ResourceManager<GPUProgram>
    {
    public:
        explicit GPUProgramManager(ApplicationNodeInternal* node);
        GPUProgramManager(const GPUProgramManager&);
        GPUProgramManager& operator=(const GPUProgramManager&);
        GPUProgramManager(GPUProgramManager&&) noexcept;
        GPUProgramManager& operator=(GPUProgramManager&&) noexcept;
        virtual ~GPUProgramManager() override;

        void RecompileAll();
    };
}
