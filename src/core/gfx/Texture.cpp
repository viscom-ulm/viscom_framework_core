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
#include "core/ApplicationNodeInternal.h"
#include "core/resources/ResourceManager.h"

#undef min
#undef max

namespace viscom {

    /**
     * Constructor, creates a texture from file.
     * @param texFilename the filename of the texture file.
     */
    Texture::Texture(const std::string& texFilename, ApplicationNodeInternal* node, bool useSRGB) :
        Resource(texFilename, node),
        textureId_{ 0 },
        descriptor_{ 0, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE },
        width_{ 0 },
        height_{ 0 },
        channels_{ 0 }
    {
        auto fullFilename = FindResourceLocation(texFilename);

        stbi_set_flip_vertically_on_load(1);

        // Bind Texture and Set Filtering Levels
        glGenTextures(1, &textureId_);
        glBindTexture(GL_TEXTURE_2D, textureId_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (stbi_is_hdr(fullFilename.c_str()) != 0) LoadTextureHDR(fullFilename);
        else LoadTextureLDR(fullFilename, useSRGB);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::Texture(const TextureInfo info, std::vector<unsigned char> img_data, ApplicationNodeInternal* node) :
        Resource("", node), 
        textureId_(0),
        descriptor_{0, info.internalFormat_, info.format_, info.type_},
        width_(info.width),
        height_(info.height),
        channels_(info.channels),
        img_data_uc_(img_data)
    {
        glGenTextures(1, &textureId_);
        glBindTexture(GL_TEXTURE_2D, textureId_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, descriptor_.internalFormat_, width_, height_, 0, descriptor_.format_, descriptor_.type_, img_data.data());
        glBindTexture(GL_TEXTURE_2D, 0);
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
        height_{ std::move(rhs.height_) },
        channels_{ std::move(rhs.channels_) },
        info_{ std::move(rhs.info_) }
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
            info_ = std::move(rhs.info_);
            width_ = std::move(rhs.width_);
            height_ = std::move(rhs.height_);
            channels_ = std::move(rhs.channels_);
            info_ = std::move(rhs.info_);
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

    void Texture::LoadTextureLDR(const std::string& filename, bool useSRGB)
    {
        auto image = stbi_load(filename.c_str(), &width_, &height_, &channels_, 0);
        if (!image) {
            LOG(WARNING) << "Failed to load texture (" << filename << ").";
            throw resource_loading_error(filename, "Failed to load texture.");
        }
        const auto strsize = width_ * height_ * 3;
        img_data_uc_ = std::vector<unsigned char>(image, image + strsize);
        descriptor_.type_ = GL_UNSIGNED_BYTE;
        std::tie(descriptor_.internalFormat_, descriptor_.format_) = FindFormat(filename, channels_, useSRGB);
        glTexImage2D(GL_TEXTURE_2D, 0, descriptor_.internalFormat_, width_, height_, 0, descriptor_.format_, descriptor_.type_, image);

        stbi_image_free(image);
    }

    void Texture::LoadTextureHDR(const std::string& filename)
    {
        auto image = stbi_loadf(filename.c_str(), &width_, &height_, &channels_, 0);
        if (!image) {
            LOG(WARNING) << "Failed to load texture (" << filename << ").";
            throw resource_loading_error(filename, "Failed to load texture.");
        }
        const auto size = sizeof(image) / sizeof(image[0]);
        img_data_ = std::vector<float>(image, image + size);
        descriptor_.type_ = GL_FLOAT;
        std::tie(descriptor_.internalFormat_, descriptor_.format_) = FindFormat(filename, channels_);
        glTexImage2D(GL_TEXTURE_2D, 0, descriptor_.internalFormat_, width_, height_, 0, descriptor_.format_, descriptor_.type_, image);

        stbi_image_free(image);
    }

    std::tuple<int, int> Texture::FindFormat(const std::string& filename, int imgChannels, bool useSRGB) const
    {
        auto internalFmt = GL_RGBA8;
        auto fmt = GL_RGBA;
        switch (imgChannels) {
        case 1: internalFmt = GL_R8; fmt = GL_RED; break;
        case 2: internalFmt = GL_RG8; fmt = GL_RG; break;
        case 3: internalFmt = useSRGB ? GL_SRGB8 : GL_RGB8; fmt = GL_RGB; break;
        case 4: internalFmt = useSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8; fmt = GL_RGBA; break;
        default:
            LOG(WARNING) << L"Invalid number of texture channels (" << imgChannels << ").";
            throw resource_loading_error(filename, "Invalid number of texture channels.");
        }
        return std::make_tuple(internalFmt, fmt);
    }
}
