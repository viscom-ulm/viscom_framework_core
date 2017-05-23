/**
 * @file   MeshManager.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Implementation of the texture manager.
 */

#include "MeshManager.h"

namespace viscom {

    /**
     * Constructor.
     * @param node the application object for resolving dependencies.
     */
    MeshManager::MeshManager(ApplicationNodeInternal* node) :
        ResourceManagerBase(node)
    {
    }

    /** Default copy constructor. */
    MeshManager::MeshManager(const MeshManager&) = default;
    /** Default copy assignment operator. */
    MeshManager& MeshManager::operator=(const MeshManager&) = default;

    /** Default move constructor. */
    MeshManager::MeshManager(MeshManager&& rhs) noexcept : ResourceManagerBase(std::move(rhs)) {}

    /** Default move assignment operator. */
    MeshManager& MeshManager::operator=(MeshManager&& rhs) noexcept
    {
        ResourceManagerBase* tResMan = this;
        *tResMan = static_cast<ResourceManagerBase&&>(std::move(rhs));
        return *this;
    }

    /** Default destructor. */
    MeshManager::~MeshManager() = default;
}
