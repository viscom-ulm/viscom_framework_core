/**
 * @file   TextureManager.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Declaration of the texture manager.
 */

#pragma once

#include "ResourceManager.h"
#include "core/gfx/Texture.h"

namespace viscom {

    class TextureManager final : public ResourceManager<Texture>
    {
    public:
        explicit TextureManager(ApplicationNodeInternal* node);
        TextureManager(const TextureManager&);
        TextureManager& operator=(const TextureManager&);
        TextureManager(TextureManager&&) noexcept;
        TextureManager& operator=(TextureManager&&) noexcept;
        virtual ~TextureManager() override;
    };
}
