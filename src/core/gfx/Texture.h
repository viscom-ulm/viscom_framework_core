/**
 * @file   Texture.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Declaration of a 2D texture resource class.
 */

#pragma once

#include "core/main.h"
#include "core/open_gl_fwd.h"
#include "core/resources/Resource.h"

namespace viscom {

    class FrameworkInternal;

    /** Describes the format of a texture. */
    struct TextureDescriptor
    {
        /**
         *  Constructor.
         *  @param btsPP the bytes per pixel of the format.
         *  @param intFmt the internal format of the texture.
         *  @param fmt the format of the texture.
         *  @param tp the type of the texture.
         */
        TextureDescriptor(unsigned int btsPP, GLint intFmt, GLenum fmt, GLenum tp) noexcept : bytesPP_(btsPP), internalFormat_(intFmt), format_(fmt), type_(tp) {};

        /** Holds the bytes per pixel of the format. */
        unsigned int bytesPP_;
        /** Holds the internal format. */
        GLint internalFormat_;
        /** Holds the format. */
        GLenum format_;
        /** Holds the type. */
        GLenum type_;
    };

    /**
     * Helper class for loading an OpenGL texture from file.
     */
    class Texture final : public Resource
    {
    public:
        Texture(const std::string& texFilename, FrameworkInternal* node, bool synchronize = false);
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&&) noexcept = delete;
        Texture& operator=(Texture&&) noexcept = delete;
        virtual ~Texture() noexcept override;

        /**
         *  Initializes the Texture.
         *  @param useSRGB defines if the texture uses the standart RGB color space.
         */
        void Initialize(bool useSRGB = true);

        /** Returns the size of the texture. */
        glm::uvec2 getDimensions() const noexcept { return glm::uvec2(width_, height_); }
        /** Returns the OpenGL texture id. */
        GLuint getTextureId() const noexcept { return textureId_; }
        /** Returns the texture descriptor. */
        const TextureDescriptor& getDescriptor() const { return descriptor_; }

    protected:
        /**
         *  Loads the texture data from file.
         *  @param data vector for optional data.
         */
        virtual void Load(std::optional<std::vector<std::uint8_t>>& data) override;
        /**
         *  Loads the texture data from memory.
         *  @param data pointer to the texture data.
         *  @param size size of the texture data.
         */
        virtual void LoadFromMemory(const void* data, std::size_t size) override;

    private:
        /**
         *  Loads a low dynamic range image from file.
         *  @param filename the path to the image file.
         *  @param useSRGB defines if the texture uses the standart RGB color space.
         */
        std::pair<void*, std::size_t> LoadImageLDR(const std::string& filename, bool useSRGB);
        /**
         *  Loads a high dynamic range image from file.
         *  @param filename the path to the image file.
         */
        std::pair<void*, std::size_t> LoadImageHDR(const std::string& filename);
        /**
         *  Finds the appropriate format, internal format and number of bytes per pixel for a low dynamic range image.
         *  @param filename the path to the image file.
         *  @param imgChannels the number of channels the image uses.
         *  @param useSRGB defines if the texture uses the standart RGB color space.
         */
        std::tuple<unsigned int, int, int> FindFormatLDR(const std::string& filename, int imgChannels, bool useSRGB = false) const;
        /**
         *  Finds the appropriate format, internal format and number of bytes per pixel for a high dynamic range image.
         *  @param filename the path to the image file.
         *  @param imgChannels the number of channels the image uses.
         */
        std::tuple<unsigned int, int, int> FindFormatHDR(const std::string& filename, int imgChannels) const;

        /** Holds the OpenGL texture id. */
        GLuint textureId_;
        /** Holds the texture descriptor. */
        TextureDescriptor descriptor_;

        /** Holds the width. */
        unsigned int width_;
        /** Holds the height. */
        unsigned int height_;
        /** Is this an sRGB texture. */
        bool sRGB_;
    };
}
