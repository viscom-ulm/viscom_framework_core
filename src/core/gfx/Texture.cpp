/**
 * @file   Texture.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Implementation of the texture resource class.
 */

#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "core/ApplicationNode.h"
#include "core/resources/ResourceManager.h"

#undef min
#undef max

namespace viscom {

    /**
     * Constructor, creates a texture from file.
     * @param texFilename the filename of the texture file.
     */
    Texture::Texture(const std::string& texFilename, ApplicationNode* node) :
        Resource(texFilename, node),
        textureId_{ 0 },
        descriptor_{ 0, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE },
        width_{ 0 },
        height_{ 0 }
    {
        auto fullFilename = node->GetConfig().baseDirectory_ + resourceBasePath + texFilename;
        auto width = 0, height = 0, channels = 0;
        auto image = stbi_load(fullFilename.c_str(), &width, &height, &channels, 0);
        if (!image) {
            LOG(WARNING) << "Failed to load texture (" << fullFilename << ").";
            throw resource_loading_error(fullFilename, "Failed to load texture.");
        }

        // Set the Correct Channel Format
        switch (channels)
        {
        case 1:
            descriptor_.internalFormat_ = GL_R8;
            descriptor_.format_ = GL_RED;
            break;
        case 2:
            descriptor_.internalFormat_ = GL_RG8;
            descriptor_.format_ = GL_RG;
            break;
        case 3:
            descriptor_.internalFormat_ = GL_RGB8;
            descriptor_.format_ = GL_RGB;
            break;
        case 4:
            descriptor_.internalFormat_ = GL_RGBA8;
            descriptor_.format_ = GL_RGBA;
            break;
        default: break;
        }

        // Bind Texture and Set Filtering Levels
        glGenTextures(1, &textureId_);
        glBindTexture(GL_TEXTURE_2D, textureId_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, descriptor_.internalFormat_, width, height, 0, descriptor_.format_, descriptor_.type_, image);

        // Release Image Pointer and Store the Texture
        stbi_image_free(image);
    }

    /**
     *  Move-constructor.
     *  @param rhs the object to copy.
     */
    Texture::Texture(Texture&& rhs) noexcept :
        Resource(std::move(rhs)),
        textureId_{ std::move(rhs.textureId_) },
        descriptor_{ std::move(rhs.descriptor_) },
        width_{ std::move(rhs.width_) },
        height_{ std::move(rhs.height_) }
    {
        rhs.textureId_ = 0;
    }

    /**
     *  Move-assignment operator.
     *  @param rhs the object to assign.
     *  @return reference to this object.
     */
    Texture& Texture::operator=(Texture&& rhs) noexcept
    {
        if (this != &rhs) {
            this->~Texture();
            Resource* tRes = this;
            *tRes = static_cast<Resource&&>(std::move(rhs));
            textureId_ = std::move(rhs.textureId_);
            descriptor_ = std::move(rhs.descriptor_);
            width_ = std::move(rhs.width_);
            height_ = std::move(rhs.height_);
            rhs.textureId_ = 0;
        }
        return *this;
    }

    /** Destructor. */
    Texture::~Texture() noexcept
    {
        if (textureId_ != 0) {
            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(1, &textureId_);
            textureId_ = 0;
        }
    }
}
