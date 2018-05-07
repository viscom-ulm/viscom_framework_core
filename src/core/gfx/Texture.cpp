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
#include "core/open_gl.h"

namespace viscom {

    /**
     * Constructor, creates a texture from file.
     * @param texFilename the filename of the texture file.
     */
    Texture::Texture(const std::string& texFilename, ApplicationNodeInternal* node, bool synchronize) :
        Resource(texFilename, ResourceType::Texture, node, synchronize),
        textureId_{ 0 },
        descriptor_{ 3, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE },
        width_{ 0 },
        height_{ 0 },
        sRGB_{ true }
    {
        // Bind Texture and Set Filtering Levels
        glGenTextures(1, &textureId_);
        auto e = glGetError();
        glBindTexture(GL_TEXTURE_2D, textureId_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
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

    void Texture::Initialize(bool useSRGB)
    {
        sRGB_ = useSRGB;
        InitializeFinished();
    }

    void Texture::Load(std::optional<std::vector<std::uint8_t>>& data)
    {
        auto fullFilename = FindResourceLocation(GetId());

        stbi_set_flip_vertically_on_load(1);

        std::pair<void*, std::size_t> image = std::make_pair(nullptr, 0);
        if (stbi_is_hdr(fullFilename.c_str()) != 0) image = LoadImageHDR(fullFilename);
        else image = LoadImageLDR(fullFilename, sRGB_);

        glBindTexture(GL_TEXTURE_2D, textureId_);
        glTexImage2D(GL_TEXTURE_2D, 0, descriptor_.internalFormat_, width_, height_, 0, descriptor_.format_, descriptor_.type_, image.first);

        if (data.has_value()) {
            data->clear();
            data->resize(sizeof(TextureDescriptor) + 2 * sizeof(unsigned int) + sizeof(bool) + image.second);
            auto dataptr = data->data();
            memcpy(dataptr, &descriptor_, sizeof(TextureDescriptor));
            dataptr += sizeof(TextureDescriptor);
            memcpy(dataptr, &width_, sizeof(unsigned int));
            dataptr += sizeof(unsigned int);
            memcpy(dataptr, &height_, sizeof(unsigned int));
            dataptr += sizeof(unsigned int);
            memcpy(dataptr, &sRGB_, sizeof(bool));
            dataptr += sizeof(bool);
            utils::memcpyfaster(dataptr, image.first, image.second);
        }

        stbi_image_free(image.first);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::LoadFromMemory(const void* data, std::size_t size)
    {
        auto dataptr = reinterpret_cast<const std::uint8_t*>(data);
        descriptor_ = *reinterpret_cast<const TextureDescriptor*>(dataptr);
        dataptr += sizeof(TextureDescriptor);
        width_ = *reinterpret_cast<const unsigned int*>(dataptr);
        dataptr += sizeof(unsigned int);
        height_ = *reinterpret_cast<const unsigned int*>(dataptr);
        dataptr += sizeof(unsigned int);
        sRGB_ = *reinterpret_cast<const bool*>(dataptr);
        dataptr += sizeof(bool);

        glBindTexture(GL_TEXTURE_2D, textureId_);
        glTexImage2D(GL_TEXTURE_2D, 0, descriptor_.internalFormat_, width_, height_, 0, descriptor_.format_, descriptor_.type_, dataptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    std::pair<void*, std::size_t> Texture::LoadImageLDR(const std::string& filename, bool useSRGB)
    {
        auto imgWidth = 0, imgHeight = 0, imgChannels = 0;
        auto image = stbi_load(filename.c_str(), &imgWidth, &imgHeight, &imgChannels, 0);
        if (!image) {
            LOG(WARNING) << "Failed to load texture (" << filename << ").";
            throw resource_loading_error(filename, "Failed to load texture.");
        }

        width_ = imgWidth;
        height_ = imgHeight;
        descriptor_.type_ = GL_UNSIGNED_BYTE;
        std::tie(descriptor_.bytesPP_, descriptor_.internalFormat_, descriptor_.format_) = FindFormatLDR(filename, imgChannels, useSRGB);
        return std::make_pair(image, imgWidth * imgHeight * imgChannels);
    }

    std::pair<void*, std::size_t> Texture::LoadImageHDR(const std::string& filename)
    {
        auto imgWidth = 0, imgHeight = 0, imgChannels = 0;
        auto image = stbi_loadf(filename.c_str(), &imgWidth, &imgHeight, &imgChannels, 0);
        if (!image) {
            LOG(WARNING) << "Failed to load texture (" << filename << ").";
            throw resource_loading_error(filename, "Failed to load texture.");
        }

        width_ = imgWidth;
        height_ = imgHeight;
        descriptor_.type_ = GL_FLOAT;
        std::tie(descriptor_.bytesPP_, descriptor_.internalFormat_, descriptor_.format_) = FindFormatHDR(filename, imgChannels);
        return std::make_pair(image, imgWidth * imgHeight * imgChannels);
    }

    std::tuple<unsigned int, int, int> Texture::FindFormatLDR(const std::string& filename, int imgChannels, bool useSRGB) const
    {
        auto bytesPP = 4U;
        auto internalFmt = GL_RGBA8;
        auto fmt = GL_RGBA;
        switch (imgChannels) {
        case 1: bytesPP = 1U; internalFmt = GL_R8; fmt = GL_RED; break;
        case 2: bytesPP = 2U; internalFmt = GL_RG8; fmt = GL_RG; break;
        case 3: bytesPP = 3U; internalFmt = useSRGB ? GL_SRGB8 : GL_RGB8; fmt = GL_RGB; break;
        case 4: bytesPP = 4U; internalFmt = useSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8; fmt = GL_RGBA; break;
        default:
            LOG(WARNING) << L"Invalid number of texture channels (" << imgChannels << ").";
            throw resource_loading_error(filename, "Invalid number of texture channels.");
        }
        return std::make_tuple(bytesPP, internalFmt, fmt);
    }

    std::tuple<unsigned int, int, int> Texture::FindFormatHDR(const std::string& filename, int imgChannels) const
    {
        auto bytesPP = 16U;
        auto internalFmt = GL_RGBA32F;
        auto fmt = GL_RGBA;
        switch (imgChannels) {
        case 1: bytesPP = 4U; internalFmt = GL_R32F; fmt = GL_RED; break;
        case 2: bytesPP = 8U; internalFmt = GL_RG32F; fmt = GL_RG; break;
        case 3: bytesPP = 12U; internalFmt = GL_RGB32F; fmt = GL_RGB; break;
        case 4: bytesPP = 16U; internalFmt = GL_RGBA32F; fmt = GL_RGBA; break;
        default:
            LOG(WARNING) << L"Invalid number of texture channels (" << imgChannels << ").";
            throw resource_loading_error(filename, "Invalid number of texture channels.");
        }
        return std::make_tuple(bytesPP, internalFmt, fmt);
    }

}
