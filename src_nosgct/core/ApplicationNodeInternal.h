/**
 * @file   ApplicationNodeInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declares a base class for all application nodes in the VISCOM lab cluster.
 */

#pragma once

#include "core/main.h"
#include "core/resources/GPUProgramManager.h"
#include "core/resources/TextureManager.h"
#include "core/resources/MeshManager.h"
#include "core/gfx/FrameBuffer.h"
#include "core/CameraHelper.h"
#include "core/gfx/FullscreenQuad.h"

namespace viscom {

    class ApplicationNodeBase;

    class ApplicationNodeInternal
    {
    public:
        ApplicationNodeInternal(FWConfiguration&& config);
        ApplicationNodeInternal(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal(ApplicationNodeInternal&&) = delete;
        ApplicationNodeInternal& operator=(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal& operator=(ApplicationNodeInternal&&) = delete;
        virtual ~ApplicationNodeInternal();

        void InitNode();
        void Render() const;

        void BasePreWindow();
        void BaseInitOpenGL();
        void BasePreSync();
        void PostSyncFunction();
        void BaseClearBuffer();
        void BaseDrawFrame();
        void BaseDraw2D();
        void BasePostDraw() const;
        void BaseCleanUp() const;

        void BaseKeyboardCallback(int key, int scancode, int action, int mods);
        void BaseCharCallback(unsigned int character, int mods);
        void BaseMouseButtonCallback(int button, int action);
        void BaseMousePosCallback(double x, double y);
        void BaseMouseScrollCallback(double xoffset, double yoffset);

        static void BaseEncodeDataStatic();
        static void BaseDecodeDataStatic();
        void BaseEncodeData();
        void BaseDecodeData();

        const FWConfiguration& GetConfig() const { return config_; }
        FrameBuffer& GetFramebuffer(size_t windowId) { return framebuffers_[windowId]; }

        const Viewport& GetViewportScreen(size_t windowId) const { return viewportScreen_[windowId]; }
        Viewport& GetViewportScreen(size_t windowId) { return viewportScreen_[windowId]; }
        const glm::ivec2& GetViewportQuadSize(size_t windowId) const { return viewportQuadSize_[windowId]; }
        glm::ivec2& GetViewportQuadSize(size_t windowId) { return viewportQuadSize_[windowId]; }
        const glm::vec2& GetViewportScaling(size_t windowId) const { return viewportScaling_[windowId]; }
        glm::vec2& GetViewportScaling(size_t windowId) { return viewportScaling_[windowId]; }

        double GetCurrentAppTime() const { return currentTime_; }
        double GetElapsedTime() const { return elapsedTime_; }

        CameraHelper* GetCamera() { return &camHelper_; }
        std::vector<FrameBuffer> CreateOffscreenBuffers(const FrameBufferDescriptor& fboDesc) const;
        const FrameBuffer* SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const;
        std::unique_ptr<FullscreenQuad> CreateFullscreenQuad(const std::string& fragmentShader);

        GPUProgramManager& GetGPUProgramManager() { return gpuProgramManager_; }
        TextureManager& GetTextureManager() { return textureManager_; }
        MeshManager& GetMeshManager() { return meshManager_; }

    private:
        /** Holds the applications configuration. */
        FWConfiguration config_;
        /** Holds the application node implementation. */
        std::unique_ptr<ApplicationNodeBase> appNodeImpl_;

        /** Holds the viewport for rendering content to the total screen. */
        std::vector<Viewport> viewportScreen_;
        /** Holds the size of the viewport for each window quad. */
        std::vector<glm::ivec2> viewportQuadSize_;
        /** Holds the viewport scaling if one applies. */
        std::vector<glm::vec2> viewportScaling_;
        /** Holds the frame buffer objects for each window. */
        std::vector<FrameBuffer> framebuffers_;

        /** The camera helper class. */
        CameraHelper camHelper_;

        /** Holds the current application time. */
        double currentTime_;
        /** Holds the time elapsed since the last frame. */
        double elapsedTime_;

        /** Holds the GPU program manager. */
        GPUProgramManager gpuProgramManager_;
        /** Holds the texture manager. */
        TextureManager textureManager_;
        /** Holds the mesh manager. */
        MeshManager meshManager_;
    };
}
