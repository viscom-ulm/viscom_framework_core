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
        FrameworkInternal(FWConfiguration&& config, std::unique_ptr<sgct::Engine> engine);
        FrameworkInternal(const FrameworkInternal&) = delete;
        FrameworkInternal(FrameworkInternal&&) = delete;
        FrameworkInternal& operator=(const FrameworkInternal&) = delete;
        FrameworkInternal& operator=(FrameworkInternal&&) = delete;
        ~FrameworkInternal();

        void InitNode(InitNodeFunc coordinatorNodeFactory, InitNodeFunc workerNodeFactory);
        void Render() const;

        bool IsMouseButtonPressed(int button) const noexcept;
        bool IsKeyPressed(int key) const noexcept;

        /** Returns the current mouse position. */
        const glm::vec2& GetMousePosition() const noexcept { return mousePosition_; }
        /** Return the current mouse position in normalized coordinates. */
        const glm::vec2& GetMousePositionNormalized() const noexcept { return mousePositionNormalized_; }

        void SetCursorInputMode(int mode);

        static void BaseEncodeDataStatic();
        static void BaseDecodeDataStatic();

        void TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex) const;
        void TransferData(const void* data, std::size_t length, std::uint16_t packageId) const;

        void TransferResource(std::string_view name, const void* data, std::size_t length, ResourceType type);
        void TransferResourceToNode(std::string_view name, const void* data, std::size_t length, ResourceType type, std::size_t nodeIndex);
        void TransferReleaseResource(std::string_view name, ResourceType type);
        void RequestSharedResources();
        void RequestSharedResource(std::string_view name, ResourceType type);
        void WaitForResource(const std::string& name, ResourceType type);

        bool IsCoordinator() const;

        sgct::Engine* GetEngine() const { return engine_.get(); }
        const FWConfiguration& GetConfig() const { return config_; }
        FrameBuffer& GetFramebuffer(std::size_t windowId) { return framebuffers_[windowId]; }
        std::size_t GetCurrentWindowID() const;
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

        bool IsInitialized() const { return initialized_; }
        InitNodeFunc& GetCoordinatorNodeFactory() { return coordinatorNodeFactory_; }
        InitNodeFunc& GetWorkerNodeFactory() { return workerNodeFactory_; }
        void SetApplicationHalted(bool halted) { applicationHalted_ = halted; }
        bool GetApplicationHalted() const { return applicationHalted_; }

        void CreateSynchronizedResources();

    private:
        void BasePreWindow();
        void BaseInitOpenGL();
        void BasePreSync();
        void PostSyncFunction();
        void BaseClearBuffer();
        void BaseDrawFrame();
        void BaseDraw2D();
        void BasePostDraw();
        void BaseCleanUp();
        void BaseDataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID);
        void BaseDataAcknowledgeCallback(int packageID, int clientID);
        void BaseDataTransferStatusCallback(bool connected, int clientID);

        void BaseKeyboardCallback(int key, int scancode, int action, int mods);
        void BaseCharCallback(unsigned int character, int mods);
        void BaseMouseButtonCallback(int button, int action);
        void BaseMousePosCallback(double x, double y);
        void BaseMouseScrollCallback(double xoffset, double yoffset);
        
        void BaseEncodeData();
        void BaseDecodeData();
    
    
        glm::dvec2 ConvertInputCoordinatesLocalToGlobal(const glm::dvec2& p);
        void ReleaseSynchronizedResource(ResourceType type, std::string_view name);
        void CreateSynchronizedResource(ResourceType type, const void* data, std::size_t length);
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
        glm::vec2 mousePosition_;
        /** Holds the current normalized mouse position. */
        glm::vec2 mousePositionNormalized_;
        /** Holds the current keyboard state. */
        std::vector<bool> keyPressedState_;
        /** Holds the current mouse button state. */
        std::vector<bool> mousePressedState_;
        /** Is the application currently halted. */
        bool applicationHalted_ = false;

        struct ResourceData {
            ResourceType type_ = ResourceType::All_Resources;
            std::string name_;
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
