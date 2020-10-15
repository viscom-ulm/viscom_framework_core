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
#include "core/resources/FontManager.h"
#include "core/gfx/FrameBuffer.h"
#include "core/CameraHelper.h"
#include "core/gfx/FullscreenQuad.h"

struct GLFWwindow;

namespace viscom {

    class FrameworkInternal
    {
    public:
        /**
         *  Constructor method, loads the framework configuration and initializes GLFW and OpenGL.
         *  @param config defines the frameworks configuration.
         *  @param coordinatorNodeFactory the function that will create the coordinator node.
         *  @param workerNodeFactory the function that will create the worker node.
         */
        FrameworkInternal(FWConfiguration&& config, InitNodeFunc coordinatorNodeFactory, InitNodeFunc workerNodeFactory);
        FrameworkInternal(const FrameworkInternal&) = delete;
        FrameworkInternal(FrameworkInternal&&) = delete;
        FrameworkInternal& operator=(const FrameworkInternal&) = delete;
        FrameworkInternal& operator=(FrameworkInternal&&) = delete;
        ~FrameworkInternal();

        /** Loop synchronizing and rendering. */
        void Render();

        /**
         *  Checks if a mouse button is currently pressed.
         *  @param button GLFW mouse button code.
         */
        bool IsMouseButtonPressed(int button) const noexcept;
        /**
         *  Checks if a key is currently pressed.
         *  @param key GLFW key code.
         */
        bool IsKeyPressed(int key) const noexcept;

        /** Returns the current mouse position. */
        const glm::vec2& GetMousePosition() const noexcept { return mousePosition_; }
        /** Return the current mouse position in normalized coordinates. */
        const glm::vec2& GetMousePositionNormalized() const noexcept { return mousePositionNormalized_; }

        /**
        *  Sets the input mode for the mouse cursor.
        *  @param mode specifies the GLFW cursor input mode. Can be GLFW_CURSOR_NORMAL, GLFW_CURSOR_HIDDEN or GLFW_CURSOR_DISABLED.
        */
        void SetCursorInputMode(int mode);

        /**
         *  Sends data to a specified node.
         *  @param data pointer to the data to be sent.
         *  @param length length of the data to be sent.
         *  @param packageId index of the package.
         *  @param nodeIndex index of the node the data is sent to.
         */
        void TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex);
        /**
         *  Sends data to all nodes.
         *  @param data pointer to the data to be sent.
         *  @param length length of the data to be sent.
         *  @param packageId index of the package.
         */
        void TransferData(const void* data, std::size_t length, std::uint16_t packageId);
        /**
         *  Sends a shared resource to all nodes.
         *  @param name the resource name.
         *  @param data the resource data.
         *  @param length the length of the resource data.
         *  @param type the resource type.
         */
        void TransferResource(std::string_view name, const void* data, std::size_t length, ResourceType type);
        /**
         *  Sends a shared resource to a worker node.
         *  @param name the resource name.
         *  @param data the resource data.
         *  @param length the length of the resource data.
         *  @param type the resource type.
         *  @param nodeIndex the index of the node to transfer the resource to.
         */
        void TransferResourceToNode(std::string_view name, const void* data, std::size_t length, ResourceType type, std::size_t nodeIndex);
        void TransferReleaseResource(std::string_view name, ResourceType type);
        /**
         *  Requests a shared resource from the coordinator node.
         *  @param name the resource name.
         *  @param type the resource type.
         */
        void RequestSharedResource(std::string_view name, ResourceType type);
        void WaitForResource(const std::string& name, ResourceType type);

        /** Returns if the node is a coordinator. */
        inline bool IsCoordinator() const { return true; }

        /** Returns the applications configuration. */
        const FWConfiguration& GetConfig() const { return config_; }
        /** Returns the applications back buffer. */
        FrameBuffer& GetFramebuffer(std::size_t) { return backBuffer_; }
        /** Returns the applications window id. */
        std::size_t GetCurrentWindowID() const { return 0; }
        /** Returns the main window of the current node (for shared contexts). */
        GLFWwindow* GetCurrentNodeMainWindow() const { return window_; }
        /**
         *  Returns the viewport for the specified window.
         *  @param windowId the window id.
         */
        const Viewport& GetViewportScreen(std::size_t windowId) const { return viewportScreen_[windowId]; }
        /**
         *  Returns the viewport for the specified window.
         *  @param windowId the window id.
         */
        Viewport& GetViewportScreen(std::size_t windowId) { return viewportScreen_[windowId]; }
        /**
         *  Returns the size of the viewport for the specified window.
         *  @param windowId the window id.
         */
        const glm::ivec2& GetViewportQuadSize(std::size_t windowId) const { return viewportQuadSize_[windowId]; }
        /**
         *  Returns the size of the viewport for the specified window.
         *  @param windowId the window id.
         */
        glm::ivec2& GetViewportQuadSize(std::size_t windowId) { return viewportQuadSize_[windowId]; }
        /**
         *  Returns the viewport scaling for the specified window.
         *  @param windowId the window id.
         */
        const glm::vec2& GetViewportScaling(std::size_t windowId) const { return viewportScaling_[windowId]; }
        /**
         *  Returns the viewport scaling for the specified window.
         *  @param windowId the window id.
         */
        glm::vec2& GetViewportScaling(std::size_t windowId) { return viewportScaling_[windowId]; }

        /** Closes the GLFW window and terminates the application. */
        void Terminate() const;

        /** Returns the camera helper. */
        CameraHelper* GetCamera() { return &camHelper_; }
        /**
         *  Creates frame buffers, appropriate textures and render buffers for offscreen rendering.
         *  @param fboDesc descriptor holding information about the number and type of textures and render buffers.
         *  @param sizeDivisor size of the frame buffers in inverse proportion to the size of the back buffer.
         */
        std::vector<FrameBuffer> CreateOffscreenBuffers(const FrameBufferDescriptor& fboDesc, int sizeDivisor = 1) const;
        /**
         *  Returns a frame buffer from previously created offscreen buffers.
         *  @param offscreenBuffers list of frame buffers created for offscreen rendering.
         */
        const FrameBuffer* SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const;
        /**
         *  Creates a fullscreen quad for shading.
         *  @param fragmentShader fragment shader to be used by the fullscreen quad.
         */
        std::unique_ptr<FullscreenQuad> CreateFullscreenQuad(const std::string& fragmentShader);

        /** Returns the GPU program manager. */
        GPUProgramManager& GetGPUProgramManager() { return gpuProgramManager_; }
        /** Returns the texture manager. */
        TextureManager& GetTextureManager() { return textureManager_; }
        /** Returns the mesh manager. */
        MeshManager& GetMeshManager() { return meshManager_; }
        /** Returns the font manager. */
        FontManager& GetFontManager() { return fontManager_; }

        /** Returns the function that will create a coordinator node. */
        InitNodeFunc& GetCoordinatorNodeFactory() { return coordinatorNodeFactory_; }
        /** Returns the function that will create a worker node. */
        InitNodeFunc& GetWorkerNodeFactory() { return workerNodeFactory_; }

        /**
         *  This method is called every time an error occurs.
         *  @param error the error code.
         *  @param description a description of the error.
         */
        static void ErrorCallbackStatic(int error, const char* description);
        /**
         *  The static base function to handle keyboard input for a specified GLFW window.
         *  @param window the window to retrieve input from.
         *  @param key GLFW key code.
         *  @param scancode platform specific scancode.
         *  @param action GLFW key and button action.
         *  @param mods modifier key flags.
         */
        static void BaseKeyboardCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
        /**
         *  The static base function to handle keyboard character input.
         *  @param window the window to retrieve input from.
         *  @param character key code of the character being pressed.
         *  @param mods modifier key flags.
         */
        static void BaseCharCallbackStatic(GLFWwindow* window, unsigned int character);
        /**
         *  The static base function to handle mouse button input.
         *  @param window the window to retrieve input from.
         *  @param button GLFW mouse button code.
         *  @param action GLFW key and button action.
         */
        static void BaseMouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);
        /**
         *  The static base function to handle cursor position input.
         *  @param window the window to retrieve input from.
         *  @param x horizontal position of the mouse cursor.
         *  @param y vertical position of the mouse cursor.
         */
        static void BaseMousePosCallbackStatic(GLFWwindow* window, double x, double y);
        /**
         *  The static base function to handle mouse wheel scrolling.
         *  @param window the window to retrieve input from.
         *  @param xoffset horizontal movement of the mouse wheel.
         *  @param yoffset vertical movement of the mouse wheel.
         */
        static void BaseMouseScrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);

    private:
        /** The base pre window function. */
        void BasePreWindow();
        /** The base function to initialize OpenGL. */
        void BaseInitOpenGL();
        /** The base post synchronization function. */
        void PostSyncFunction();
        /** The base function for rendering. */
        void BaseDrawFrame();
        /** The base function for rendering 2D. */
        void BaseDraw2D();
        /** The base clean up function. */
        void BaseCleanUp();

        /**
         *  The base function to handle keyboard input.
         *  @param key GLFW key code.
         *  @param scancode platform specific scancode.
         *  @param action GLFW key and button action.
         *  @param mods modifier key flags.
         */
        void BaseKeyboardCallback(int key, int scancode, int action, int mods);
        /**
         *  The base function to handle keyboard character input.
         *  @param character key code of the character being pressed.
         *  @param mods modifier key flags.
         */
        void BaseCharCallback(unsigned int character, int mods);
        /**
         *  The base function to handle mouse button input.
         *  @param button GLFW mouse button code.
         *  @param action GLFW key and button action.
         */
        void BaseMouseButtonCallback(int button, int action);
        /**
         *  The base function to handle cursor position input.
         *  @param x horizontal position of the mouse cursor.
         *  @param y vertical position of the mouse cursor.
         */
        void BaseMousePosCallback(double x, double y);
        /**
         *  The base function to handle mouse wheel scrolling.
         *  @param xoffset horizontal movement of the mouse wheel.
         *  @param yoffset vertical movement of the mouse wheel.
         */
        void BaseMouseScrollCallback(double xoffset, double yoffset);

        /**
         *  Converts local screen coordinates to global coordinates.
         *  @param x horizontal position in local coordinates to convert.
         *  @param y vertical position in local coordinates to convert.
         */
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
        /** Holds the font manager. */
        FontManager fontManager_;

        /** Holds the current mouse position. */
        glm::vec2 mousePosition_ = glm::vec2{ 0.0f };
        /** Holds the current normalized mouse position. */
        glm::vec2 mousePositionNormalized_ = glm::vec2{ 0.0f };
    };
}
