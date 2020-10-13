/**
 * @file   FontManager.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2020.10.13
 *
 * @brief  Implementation of the font manager.
 */

#include "FontManager.h"

namespace viscom {

    /**
     * Constructor.
     * @param node the application object for resolving dependencies.
     */
    FontManager::FontManager(FrameworkInternal* node) :
        ResourceManagerBase(node)
    {
    }

    /** Default copy constructor. */
    FontManager::FontManager(const FontManager&) = default;
    /** Default copy assignment operator. */
    FontManager& FontManager::operator=(const FontManager&) = default;

    /** Default move constructor. */
    FontManager::FontManager(FontManager&& rhs) noexcept : ResourceManagerBase(std::move(rhs)) {}

    /** Default move assignment operator. */
    FontManager& FontManager::operator=(FontManager&& rhs) noexcept
    {
        ResourceManagerBase* tResMan = this;
        *tResMan = static_cast<ResourceManagerBase&&>(std::move(rhs));
        return *this;
    }

    /** Default destructor. */
    FontManager::~FontManager() = default;
}
