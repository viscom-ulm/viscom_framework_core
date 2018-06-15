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
#include "core/TuioInputWrapper.h"

struct GLFWwindow;

namespace viscom {

    class ApplicationNodeBase;

    class ApplicationNodeInternal : public viscom::tuio::TuioInputWrapper
    {
    public:
        ApplicationNodeInternal(FWConfiguration&& config);
        ApplicationNodeInternal(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal(ApplicationNodeInternal&&) = delete;
        ApplicationNodeInternal& operator=(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal& operator=(ApplicationNodeInternal&&) = delete;
        virtual ~ApplicationNodeInternal() override;

        void Render();

        void BaseInitOpenGL();
        void PostSyncFunction();
        void BaseDrawFrame();
        void BaseDraw2D();
        void BaseCleanUp() const;

        bool IsMouseButtonPressed(int button) const noexcept;
        bool IsKeyPressed(int key) const noexcept;

        /** Returns the current mouse position. */
        const glm::vec2& GetMousePosition() const noexcept { return mousePosition_; }
        /** Return the current mouse position in normalized coordinates. */
        const glm::vec2& GetMousePositionNormalized() const noexcept { return mousePositionNormalized_; }

        static void ErrorCallbackStatic(int error, const char* description);
        static void BaseKeyboardCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void BaseCharCallbackStatic(GLFWwindow* window, unsigned int character);
        static void BaseMouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);
        static void BaseMousePosCallbackStatic(GLFWwindow* window, double x, double y);
        static void BaseMouseScrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);
        void BaseKeyboardCallback(int key, int scancode, int action, int mods);
        void BaseCharCallback(unsigned int character, int mods);
        void BaseMouseButtonCallback(int button, int action);
        void BaseMousePosCallback(double x, double y);
        void BaseMouseScrollCallback(double xoffset, double yoffset);

        virtual void addTuioCursor(TUIO::TuioCursor *tcur) override;
        virtual void updateTuioCursor(TUIO::TuioCursor *tcur) override;
        virtual void removeTuioCursor(TUIO::TuioCursor *tcur) override;

        void TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex);

        void TransferResource(std::string_view name, const void* data, std::size_t length, ResourceType type);
        void TransferReleaseResource(std::string_view name, ResourceType type);
        void WaitForResource(const std::string& name, ResourceType type);

        bool IsMaster() const { return true; }

        void SetCursorInputMode(int mode);

        const FWConfiguration& GetConfig() const { return config_; }
        FrameBuffer& GetFramebuffer(size_t windowId) { return backBuffer_; }

        const Viewport& GetViewportScreen(size_t windowId) const { return viewportScreen_[windowId]; }
        Viewport& GetViewportScreen(size_t windowId) { return viewportScreen_[windowId]; }
        const glm::ivec2& GetViewportQuadSize(size_t windowId) const { return viewportQuadSize_[windowId]; }
        glm::ivec2& GetViewportQuadSize(size_t windowId) { return viewportQuadSize_[windowId]; }
        const glm::vec2& GetViewportScaling(size_t windowId) const { return viewportScaling_[windowId]; }
        glm::vec2& GetViewportScaling(size_t windowId) { return viewportScaling_[windowId]; }

        double GetCurrentAppTime() const { return currentTime_; }
        double GetElapsedTime() const { return elapsedTime_; }
        void Terminate() const;

        CameraHelper* GetCamera() { return &camHelper_; }
        std::vector<FrameBuffer> CreateOffscreenBuffers(const FrameBufferDescriptor& fboDesc, int sizeDivisor = 1) const;
        const FrameBuffer* SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const;
        std::unique_ptr<FullscreenQuad> CreateFullscreenQuad(const std::string& fragmentShader);

        GPUProgramManager& GetGPUProgramManager() { return gpuProgramManager_; }
        TextureManager& GetTextureManager() { return textureManager_; }
        MeshManager& GetMeshManager() { return meshManager_; }

    private:
        glm::dvec2 ConvertInputCoordinates(double x, double y);

        /** Holds the applications configuration. */
        FWConfiguration config_;
        /** Holds the GLFW window. */
        GLFWwindow* window_;
        /** Holds the backbuffer frame-buffer. */
        FrameBuffer backBuffer_;
        /** Holds the application node implementation. */
        std::unique_ptr<ApplicationNodeBase> appNodeImpl_;

        /** Holds the viewport for rendering content to the total screen. */
        std::vector<Viewport> viewportScreen_;
        /** Holds the size of the viewport for each window quad. */
        std::vector<glm::ivec2> viewportQuadSize_;
        /** Holds the viewport scaling if one applies. */
        std::vector<glm::vec2> viewportScaling_;

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

        /** Holds the current mouse position. */
        glm::vec2 mousePosition_;
        /** Holds the current normalized mouse position. */
        glm::vec2 mousePositionNormalized_;
    };
}
