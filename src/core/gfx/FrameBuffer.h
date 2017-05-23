/**
 * @file   FrameBuffer.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.21
 *
 * @brief  Definition of the frame buffer class.
 */

#pragma once

#include "main.h"
#include <sgct.h>

namespace viscom {

    /** Describes a render buffer. */
    struct RenderBufferDescriptor
    {
        // ReSharper disable once CppNonExplicitConvertingConstructor
        explicit RenderBufferDescriptor(GLenum internalFormat) : internalFormat_{ internalFormat } {}

        /** Holds the internal format of the render buffer. */
        GLenum internalFormat_;
    };

    /** Describes a texture render target for frame buffers. */
    struct FrameBufferTextureDescriptor
    {
        // ReSharper disable once CppNonExplicitConvertingConstructor
        FrameBufferTextureDescriptor(GLenum internalFormat, GLenum texType = GL_TEXTURE_2D) : internalFormat_{ internalFormat }, texType_{ texType } {}

        /** The texture descriptor. */
        GLenum internalFormat_;
        /** The texture type. */
        GLenum texType_;
    };

    /** Describes a frame buffer. */
    struct FrameBufferDescriptor
    {
        FrameBufferDescriptor() = default;
        FrameBufferDescriptor(const std::vector<FrameBufferTextureDescriptor>& tex, const std::vector<RenderBufferDescriptor>& rb) : texDesc_(tex), rbDesc_(rb), numSamples_(1) {}

        /** Holds descriptions for all textures used. */
        std::vector<FrameBufferTextureDescriptor> texDesc_;
        /** Holds descriptions for all render buffers used. */
        std::vector<RenderBufferDescriptor> rbDesc_;
        /** Holds the number of samples for the frame buffer. */
        unsigned int numSamples_ = 1;
    };

    struct Viewport
    {
        Viewport() : position_{ 0 }, size_{ 0 } {}
        Viewport(const glm::ivec2& position, const glm::uvec2& size) : position_{ position }, size_{ size } {}
        Viewport(int x, int y, unsigned int sizex, unsigned int sizey) : position_{ x, y }, size_{ sizex, sizey } {}

        /** Holds the viewport lower left position. */
        glm::ivec2 position_;
        /** Holds the viewport size. */
        glm::uvec2 size_;
    };

    /**
     * @brief  Represents frame buffers.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.17
     */
    class FrameBuffer
    {
    public:
        FrameBuffer();
        FrameBuffer(unsigned int fbWidth, unsigned int fbHeight, const FrameBufferDescriptor& desc);
        FrameBuffer(const FrameBuffer&);
        FrameBuffer(FrameBuffer&&) noexcept;
        FrameBuffer& operator=(const FrameBuffer&);
        FrameBuffer& operator=(FrameBuffer&&) noexcept;
        ~FrameBuffer();

        void UseAsRenderTarget() const;
        void UseAsRenderTarget(const std::vector<unsigned int> drawBufferIndices) const;

        void DrawToFBO(std::function<void()> drawFn) const;
        void DrawToFBO(const std::vector<unsigned int>& drawBufferIndices, std::function<void()> drawFn) const;

        void Resize(unsigned int fbWidth, unsigned int fbHeight);
        const std::vector<GLuint>& GetTextures() const { return textures_; }
        void SetStandardViewport(const Viewport& vp) { standardViewport_ = vp; }
        void SetStandardViewport(int x, int y, unsigned int sizex, unsigned int sizey) { standardViewport_.position_ = glm::ivec2(x, y); standardViewport_.size_ = glm::ivec2(sizex, sizey); }

        // void SetWidth(unsigned int width) { standardViewport_.size_.x = width; }
        // void SetHeight(unsigned int height) { standardViewport_.size_ = height; }
        unsigned int GetWidth() const { return width_; };
        unsigned int GetHeight() const { return height_; };

    private:
        static unsigned int findAttachment(GLenum internalFormat, unsigned int& colorAtt, std::vector<GLenum> &drawBuffers);

        /** holds the frame buffers OpenGL name. */
        GLuint fbo_;
        /** Holds whether this represents the back buffer or not. */
        bool isBackbuffer_;
        /** Holds the description for the frame buffer object. */
        FrameBufferDescriptor desc_;
        /** holds the frame buffers textures to render to. */
        std::vector<GLuint> textures_;
        /** Holds the a list of color attachments for the textures. */
        std::vector<GLenum> drawBuffers_;
        /** holds the frame buffers render buffers to render to. */
        std::vector<GLuint> renderBuffers_;
        /** Holds the standard viewport to be used with the frame-buffer. */
        Viewport standardViewport_;
        /** holds the frame buffers width. */
        unsigned int width_;
        /** holds the frame buffers height. */
        unsigned int height_;
    };
}
