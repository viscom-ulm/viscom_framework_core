/**
 * @file   FontManager.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2020.10.13
 *
 * @brief  Declaration of the font manager.
 */

#pragma once

#include "ResourceManager.h"
#include "core/gfx/fonts/Font.h"

namespace viscom {

    /** Manager for handling all mesh objects. */
    class FontManager final : public ResourceManager<Font>
    {
    public:
        explicit FontManager(FrameworkInternal* node);
        FontManager(const FontManager&);
        FontManager& operator=(const FontManager&);
        FontManager(FontManager&&) noexcept;
        FontManager& operator=(FontManager&&) noexcept;
        virtual ~FontManager() override;
    };
}
