/**
 * @file   GPUProgramManager.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Implementation of the resource manager for GPU programs.
 */

#include "GPUProgramManager.h"

namespace viscom {

    /**
     * Constructor.
     * @param app the application node.
     */
    GPUProgramManager::GPUProgramManager(ApplicationNodeInternal* app) :
        ResourceManager(app)
    {
    }

    /** Default copy constructor. */
    GPUProgramManager::GPUProgramManager(const GPUProgramManager&) = default;
    /** Default copy assignment operator. */
    GPUProgramManager& GPUProgramManager::operator=(const GPUProgramManager&) = default;

    /** Default move constructor. */
    GPUProgramManager::GPUProgramManager(GPUProgramManager&& rhs) noexcept : ResourceManagerBase(std::move(rhs)) {}

    /** Default move assignment operator. */
    GPUProgramManager& GPUProgramManager::operator=(GPUProgramManager&& rhs) noexcept
    {
        ResourceManagerBase* tResMan = this;
        *tResMan = static_cast<ResourceManagerBase&&>(std::move(rhs));
        return *this;
    }

    GPUProgramManager::~GPUProgramManager() = default;

    /** Recompiles all GPU programs. */
    void GPUProgramManager::RecompileAll()
    {
        for (auto& program : resources_) {
            if (!program.second.expired()) program.second.lock()->recompileProgram();
        }
    }
}
