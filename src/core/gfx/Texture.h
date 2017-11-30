/**
 * @file   Texture.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Declaration of a 2D texture resource class.
 */

#pragma once

#include "core/main.h"
#include "core/open_gl.h"
#include "core/resources/Resource.h"

namespace viscom {

    class ApplicationNodeInternal;

    /** Describes the format of a texture. */
    struct TextureDescriptor
    {
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

    struct TextureInfo
    {
        TextureInfo(): width(0), height(0), channels(0), internalFormat_(0), format_(0), type_(0)
        {
        }

        TextureInfo(int w, int h, int c, GLint iF, GLenum f, GLenum t) : width(w), height(h), channels(c), internalFormat_(iF), format_(f), type_(t)
        {
        }

        /** image width. */
        int width;
        /** image height. */
        int height;
        /** image channels. */
        int channels;
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
        Texture(const std::string& texFilename, ApplicationNodeInternal* node, bool useSRGB = true);
        Texture(const TextureInfo info, const std::vector<float> img_data, ApplicationNodeInternal* node);
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&&) noexcept;
        Texture& operator=(Texture&&) noexcept;
        virtual ~Texture() noexcept override;

        /** Returns the size of the texture. */
        glm::uvec2 getDimensions() const noexcept { return glm::uvec2(width_, height_); }
        /** Returns the OpenGL texture id. */
        GLuint getTextureId() const noexcept { return textureId_; }
        TextureInfo getInfo() const { return TextureInfo(width_, height_, channels_, descriptor_.internalFormat_, descriptor_.format_, descriptor_.type_); }
        std::vector<float> getImageData() const { return img_data_; }

    private:
        void LoadTextureLDR(const std::string& filename, bool useSRGB);
        void LoadTextureHDR(const std::string& filename);
        std::tuple<int, int> FindFormat(const std::string& filename, int imgChannels, bool useSRGB = false) const;

        /** Holds the OpenGL texture id. */
        GLuint textureId_;
        /** Holds the texture descriptor. */
        TextureDescriptor descriptor_;
        /** Holds info for serialization */
        TextureInfo info_;
        /** Holds the width. */
        int width_;
        /** Holds the height. */
        int height_;
        /** Holds the channels. */
        int channels_;
        /** Holds image buffer for serialization */
        std::vector<float> img_data_;
    };
}
