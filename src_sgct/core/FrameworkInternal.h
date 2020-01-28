/**
 * @file   FrameworkInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declares the internal framework class for the VISCOM lab cluster.
 */

#pragma once

#include "core/main.h"
#ifdef VISCOM_SYNCINPUT
#include "core/InputWrapper.h"
#endif
#include <mutex>
#include "core/resources/GPUProgramManager.h"
#include "core/resources/TextureManager.h"
#include "core/resources/MeshManager.h"
#include "core/gfx/FrameBuffer.h"
#include "core/CameraHelper.h"
#include "core/gfx/FullscreenQuad.h"
#include "sgct/SharedDataTypes.h"

namespace viscom {

    class FrameworkInternal
    {
    public:
        /**
         * Constructor method, loads the framework configuration and the engine.
         * @param config defines the frameworks configuration.
         * @param engine the sgct engine.
         */
        FrameworkInternal(FWConfiguration&& config, std::unique_ptr<sgct::Engine> engine);
        FrameworkInternal(const FrameworkInternal&) = delete;
        FrameworkInternal(FrameworkInternal&&) = delete;
        FrameworkInternal& operator=(const FrameworkInternal&) = delete;
        FrameworkInternal& operator=(FrameworkInternal&&) = delete;
        ~FrameworkInternal();

        /**
         * Initializes the nodes by defining the functions creating the nodes.
         * @param coordinatorNodeFactory the function that will create the coordinator node.
         * @param workerNodeFactory the function that will create the worker node.
         */
        void InitNode(InitNodeFunc coordinatorNodeFactory, InitNodeFunc workerNodeFactory);
        /** Loop synchronizing and rendering with SGCT. */
        void Render() const;

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

        /** Static base function for synchronizing the framework by encoding and sending all synchronized data to the other nodes. */
        static void BaseEncodeDataStatic();
        /** Static base class for synchronizing the framework by receiving and decoding synchronized data from other nodes. */
        static void BaseDecodeDataStatic();

        /**
         *  Sends data to a specified node.
         *  @param data pointer to the data to be sent.
         *  @param length length of the data to be sent.
         *  @param packageId index of the package.
         *  @param nodeIndex index of the node the data is sent to.
         */
        void TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex) const;
        /**
         *  Sends data to all nodes.
         *  @param data pointer to the data to be sent.
         *  @param length length of the data to be sent.
         *  @param packageId index of the package.
         */
        void TransferData(const void* data, std::size_t length, std::uint16_t packageId) const;

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
         *  @param type the resource type
         *  @param nodeIndex the index of the node to transfer the resource to.
         */
        void TransferResourceToNode(std::string_view name, const void* data, std::size_t length, ResourceType type, std::size_t nodeIndex);
        void TransferReleaseResource(std::string_view name, ResourceType type);
        void RequestSharedResources();
        /**
         *  Requests a shared resource from the coordinator node.
         *  @param name the resource name.
         *  @param type the resource type.
         */
        void RequestSharedResource(std::string_view name, ResourceType type);
        void WaitForResource(const std::string& name, ResourceType type);

        /** Returns if the node is a coordinator. */
        bool IsCoordinator() const;

        /** Returns the applications SGCT engine. */
        sgct::Engine* GetEngine() const { return engine_.get(); }
        /** Returns the applications configuration. */
        const FWConfiguration& GetConfig() const { return config_; }
        /** Returns each windows frame buffer object. */
        FrameBuffer& GetFramebuffer(std::size_t windowId) { return framebuffers_[windowId]; }
        /** Returns the applications window id. */
        std::size_t GetCurrentWindowID() const;
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

        /** Returns the initialization state. */
        bool IsInitialized() const { return initialized_; }
        /** Returns the function that will create a coordinator node. */
        InitNodeFunc& GetCoordinatorNodeFactory() { return coordinatorNodeFactory_; }
        /** Returns the function that will create a coordinator node. */
        InitNodeFunc& GetWorkerNodeFactory() { return workerNodeFactory_; }
        /**
         *  Sets the application's halted state.
         *  @param halted the new halted value.
         */
        void SetApplicationHalted(bool halted) { applicationHalted_ = halted; }
        /** Returns if the application is currently halted. */
        bool GetApplicationHalted() const { return applicationHalted_; }

        /** Creates all synchronized resources from the creatable resources list. */
        void CreateSynchronizedResources();

    private:
        /** The base pre window function. */
        void BasePreWindow();
        /** The base function to initialize OpenGL. */
        void BaseInitOpenGL();
        /** The base pre synchronization function. */
        void BasePreSync();
        /** The base post synchronization function. */
        void PostSyncFunction();
        /** The base function for clearing buffers. */
        void BaseClearBuffer();
        /** The base function for rendering. */
        void BaseDrawFrame();
        /** The base function for rendering 2D. */
        void BaseDraw2D();
        /** The base post draw function. */
        void BasePostDraw();
        /** The base clean up function. */
        void BaseCleanUp();
        /**
         *  The base function for receiving a message from another node.
         *  @param receivedData pointer to the received data.
         *  @param receivedLength length of the received data.
         *  @param packageID index of the received package.
         *  @param clientID index of the node sending the message.
         */
        void BaseDataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID);
        /**
         *  The base function for sending a message to another node.
         *  @param packageID index of the sent package.
         *  @param clientID index of the node receiving the message.
         */
        void BaseDataAcknowledgeCallback(int packageID, int clientID);
        /**
         *  The base function called when the connection status changes.
         *  @param connected connection status.
         *  @param clientID index of the node connected or disconnected.
         */
        void BaseDataTransferStatusCallback(bool connected, int clientID);

        /**
         *  The base function to handle keyboard input for a specified GLFW window.
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
         *  The base function to handle changes in cursor position.
         *  @param xoffset horizontal movement of the mouse cursor.
         *  @param yoffset vertical movement of the mouse cursor.
         */
        void BaseMouseScrollCallback(double xoffset, double yoffset);

        /** Base function for synchronizing the framework by encoding and sending all synchronized data to the other nodes. */
        void BaseEncodeData();
        /** Base class for synchronizing the framework by receiving and decoding synchronized data from other nodes. */
        void BaseDecodeData();


        /**
         *  Converts local screen coordinates to global coordinates.
         *  @param p point in local coordinates to convert.
         */
        glm::dvec2 ConvertInputCoordinatesLocalToGlobal(const glm::dvec2& p);
        /**
         *  Releases a shared resource.
         *  @param type the resource type.
         *  @param name the resource name.
         */
        void ReleaseSynchronizedResource(ResourceType type, std::string_view name);
        /**
         *  Loads a synchronized resource and stores it in the creatable resources list for later creation.
         *  @param type the resource type.
         *  @param data the resource data pointer.
         *  @param length the resource data length.
         */
        void CreateSynchronizedResource(ResourceType type, const void* data, std::size_t length);
        /**
         *  Sends a resource to a specific node.
         *  @param type the resource type.
         *  @param data the resource data pointer.
         *  @param length the resource data length.
         *  @param clientID index of the client to send the resource to.
         */
        void SendResourcesToNode(ResourceType type, const void* data, std::size_t length, int clientID);
        static int MakePackageID(std::uint8_t internalType, std::uint8_t internalPID, std::uint16_t userPID);

        /** The function the will create a coordinator node. */
        InitNodeFunc coordinatorNodeFactory_;
        /** The function the will create a worker node. */
        InitNodeFunc workerNodeFactory_;

        /** Holds a static pointer to an object to this class making it singleton in a way. */
        // TODO: This is only a workaround and should be fixed in the future. [12/5/2016 Sebastian Maisch]
        static FrameworkInternal* instance_;
        /** Holds the mutex for the instance pointer. */
        static std::mutex instanceMutex_;
        /** Holds the initialization state of this object. */
        bool initialized_ = false;

        /** Holds the applications configuration. */
        FWConfiguration config_;
        /** Holds the application node implementation. */
        std::unique_ptr<ApplicationNodeInternal> appNodeInternal_;
        /** Holds the SGCT engine. */
        std::unique_ptr<sgct::Engine> engine_;

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

        /** Holds the GPU program manager. */
        GPUProgramManager gpuProgramManager_;
        /** Holds the texture manager. */
        TextureManager textureManager_;
        /** Holds the mesh manager. */
        MeshManager meshManager_;

        /** Holds the current mouse position. */
        glm::vec2 mousePosition_ = glm::vec2{0.0f, 0.0f};
        /** Holds the current normalized mouse position. */
        glm::vec2 mousePositionNormalized_ = glm::vec2{0.0f, 0.0f};
        /** Holds the current keyboard state. */
        std::vector<bool> keyPressedState_;
        /** Holds the current mouse button state. */
        std::vector<bool> mousePressedState_;
        /** Is the application currently halted. */
        bool applicationHalted_ = false;

        /** Holds all data of a resource. */
        struct ResourceData {
            /** Holds the resource type. */
            ResourceType type_ = ResourceType::All_Resources;
            /** Holds the resource name. */
            std::string name_;
            /** Holds the resource data. */
            std::vector<std::uint8_t> data_;

            bool operator==(const ResourceData& other) const { return type_ == other.type_ && name_ == other.name_; }
        };

        /** Synchronized resources to be created at next possible time. */
        std::vector<ResourceData> creatableResources_;
        /** The mutex for creatable resources. */
        std::mutex creatableResourceMutex_;

#ifndef VISCOM_LOCAL_ONLY
    public:
        unsigned int GetGlobalProjectorId(int nodeId, int windowId) const;

    private:
        void loadProperties();

        /** Holds the start node used for slaves. */
        unsigned int startNode_ = 0;
#endif
    };
}
