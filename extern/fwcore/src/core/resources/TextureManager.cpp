/**
 * @file   TextureManager.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Implementation of the texture manager.
 */

#include "TextureManager.h"

namespace viscom {

    /**
     * Constructor.
     * @param node the application object for resolving dependencies.
     */
    TextureManager::TextureManager(ApplicationNodeInternal* node) :
        ResourceManagerBase(node)
    {
    }

    /** Default copy constructor. */
    TextureManager::TextureManager(const TextureManager&) = default;
    /** Default copy assignment operator. */
    TextureManager& TextureManager::operator=(const TextureManager&) = default;

    /** Default move constructor. */
    TextureManager::TextureManager(TextureManager&& rhs) noexcept : ResourceManagerBase(std::move(rhs)) {}

    /** Default move assignment operator. */
    TextureManager& TextureManager::operator=(TextureManager&& rhs) noexcept
    {
        ResourceManagerBase* tResMan = this;
        *tResMan = static_cast<ResourceManagerBase&&>(std::move(rhs));
        return *this;
    }

    /** Default destructor. */
    TextureManager::~TextureManager() = default;
}
