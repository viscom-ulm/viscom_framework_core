/**
 * @file   ResourceManager.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.09.10
 *
 * @brief  Implementation of the ResourceManager class.
 */

#include "ResourceManager.h"
#include "core/FrameworkInternal.h"

namespace viscom {

    void BaseResourceManager::RequestSharedResource(std::string_view name, ResourceType type)
    {
        appNode_->RequestSharedResource(name, type);
    }

    void BaseResourceManager::TransferResourceToNode(std::string_view name, const void* data, std::size_t length, ResourceType type, std::size_t nodeIndex)
    {
        appNode_->TransferResourceToNode(name, data, length, type, nodeIndex);
    }
}
