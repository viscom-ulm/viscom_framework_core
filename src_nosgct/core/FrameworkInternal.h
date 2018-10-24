/**
 * @file   FrameworkInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declares the internal framework class for the VISCOM lab cluster.
 */

#pragma once

#include "core/main.h"
#include "core/resources/GPUProgramManager.h"
#include "core/resources/TextureManager.h"
#include "core/resources/MeshManager.h"
#include "core/gfx/FrameBuffer.h"
#include "core/CameraHelper.h"
#include "core/gfx/FullscreenQuad.h"

struct GLFWwindow;

namespace viscom {

    class FrameworkInternal
    {
    public:
        FrameworkInternal(FWConfiguration&& config, InitNodeFunc coordinatorNodeFactory, InitNodeFunc workerNodeFactory);
        FrameworkInternal(const FrameworkInternal&) = delete;
        FrameworkInternal(FrameworkInternal&&) = delete;
        FrameworkInternal& operator=(const FrameworkInternal&) = delete;
        FrameworkInternal& operator=(FrameworkInternal&&) = delete;
        ~FrameworkInternal();

        void Render();

        bool IsMouseButtonPressed(int button) const noexcept;
        bool IsKeyPressed(int key) const noexcept;

        /** Returns the current mouse position. */
        const glm::vec2& GetMousePosition() const noexcept { return mousePosition_; }
        /** Return the current mouse position in normalized coordinates. */
        const glm::vec2& GetMousePositionNormalized() const noexcept { return mousePositionNormalized_; }

        void SetCursorInputMode(int mode);

        void TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex);
        void TransferData(const void* data, std::size_t length, std::uint16_t packageId);
        void TransferResource(std::string_view name, const void* data, std::size_t length, ResourceType type);
        void TransferResourceToNode(std::string_view name, const void* data, std::size_t length, ResourceType type, std::size_t nodeIndex);
        void TransferReleaseResource(std::string_view name, ResourceType type);
        void RequestSharedResource(std::string_view name, ResourceType type);
        void WaitForResource(const std::string& name, ResourceType type);

        bool IsCoordinator() const { return true; }

        const FWConfiguration& GetConfig() const { return config_; }
        FrameBuffer& GetFramebuffer(std::size_t windowId) { return backBuffer_; }
        std::size_t GetCurrentWindowID() const { return 0; }
        const Viewport& GetViewportScreen(std::size_t windowId) const { return viewportScreen_[windowId]; }
        Viewport& GetViewportScreen(std::size_t windowId) { return viewportScreen_[windowId]; }
        const glm::ivec2& GetViewportQuadSize(std::size_t windowId) const { return viewportQuadSize_[windowId]; }
        glm::ivec2& GetViewportQuadSize(std::size_t windowId) { return viewportQuadSize_[windowId]; }
        const glm::vec2& GetViewportScaling(std::size_t windowId) const { return viewportScaling_[windowId]; }
        glm::vec2& GetViewportScaling(std::size_t windowId) { return viewportScaling_[windowId]; }

        void Terminate() const;

        CameraHelper* GetCamera() { return &camHelper_; }
        std::vector<FrameBuffer> CreateOffscreenBuffers(const FrameBufferDescriptor& fboDesc, int sizeDivisor = 1) const;
        const FrameBuffer* SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const;
        std::unique_ptr<FullscreenQuad> CreateFullscreenQuad(const std::string& fragmentShader);

        GPUProgramManager& GetGPUProgramManager() { return gpuProgramManager_; }
        TextureManager& GetTextureManager() { return textureManager_; }
        MeshManager& GetMeshManager() { return meshManager_; }

        InitNodeFunc& GetCoordinatorNodeFactory() { return coordinatorNodeFactory_; }
        InitNodeFunc& GetWorkerNodeFactory() { return workerNodeFactory_; }

        static void ErrorCallbackStatic(int error, const char* description);
        static void BaseKeyboardCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void BaseCharCallbackStatic(GLFWwindow* window, unsigned int character);
        static void BaseMouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);
        static void BaseMousePosCallbackStatic(GLFWwindow* window, double x, double y);
        static void BaseMouseScrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);

    private:
        void BasePreWindow();
        void BaseInitOpenGL();
        void PostSyncFunction();
        void BaseDrawFrame();
        void BaseDraw2D();
        void BaseCleanUp();

        void BaseKeyboardCallback(int key, int scancode, int action, int mods);
        void BaseCharCallback(unsigned int character, int mods);
        void BaseMouseButtonCallback(int button, int action);
        void BaseMousePosCallback(double x, double y);
        void BaseMouseScrollCallback(double xoffset, double yoffset);

        glm::dvec2 ConvertInputCoordinates(double x, double y);

        /** The function that will create a coordinator node. */
        InitNodeFunc coordinatorNodeFactory_;
        /** The function that will create a worker node. */
        InitNodeFunc workerNodeFactory_;

        /** Holds the applications configuration. */
        FWConfiguration config_;
        /** Holds the GLFW window. */
        GLFWwindow* window_;
        /** Holds the backbuffer frame-buffer. */
        FrameBuffer backBuffer_;
        /** Holds the application node implementation. */
        std::unique_ptr<ApplicationNodeInternal> appNodeInternal_;

        /** Holds the viewport for rendering content to the total screen. */
        std::vector<Viewport> viewportScreen_;
        /** Holds the size of the viewport for each window quad. */
        std::vector<glm::ivec2> viewportQuadSize_;
        /** Holds the viewport scaling if one applies. */
        std::vector<glm::vec2> viewportScaling_;

        /** The camera helper class. */
        CameraHelper camHelper_;

        /** Holds the GPU program manager. */
        GPUProgramManager gpuProgramManager_;
        /** Holds the texture manager. */
        TextureManager textureManager_;
        /** Holds the mesh manager. */
        MeshManager meshManager_;

        /** Holds the current mouse position. */
        glm::vec2 mousePosition_;
        /** Holds the current normalized mouse position. */
        glm::vec2 mousePositionNormalized_;
    };
}
