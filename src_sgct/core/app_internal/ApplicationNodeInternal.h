/**
 * @file   ApplicationNodeInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declares a base class for all application nodes in the VISCOM lab cluster.
 */

#pragma once

#include "core/main.h"
#ifdef VISCOM_SYNCINPUT
#include "core/InputWrapper.h"
#endif
#include "core/TuioInputWrapper.h"
#include "core/FrameworkInternal.h"
#include "sgct/SharedDataTypes.h"

namespace viscom {

    class ApplicationNodeBase;

    struct InternalSyncedInfo {
        double currentTime_ = 0.0;
        glm::vec3 cameraPosition_;
        glm::quat cameraOrientation_;
        glm::mat4 pickMatrix_;
    };

    class ApplicationNodeInternal : public viscom::tuio::TuioInputWrapper
    {
    public:
        ApplicationNodeInternal(FrameworkInternal& fwInternal);
        ApplicationNodeInternal(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal(ApplicationNodeInternal&&) = delete;
        ApplicationNodeInternal& operator=(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal& operator=(ApplicationNodeInternal&&) = delete;
        virtual ~ApplicationNodeInternal() override;

        virtual void PreWindow();
        virtual void InitOpenGL();
        virtual void PreSync();
        virtual void PostSync();
        virtual void ClearBuffer(FrameBuffer& fbo);
        virtual void DrawFrame(FrameBuffer& fbo);
        virtual void Draw2D(FrameBuffer& fbo);
        virtual void PostDraw();
        virtual void CleanUp();
        void DataTransfer(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID);
        void DataAcknowledge(std::uint16_t packageID, int clientID);
        void DataTransferStatus(bool connected, int clientID);

        // bool IsMouseButtonPressed(int button) const noexcept;
        // bool IsKeyPressed(int key) const noexcept;

        /** Returns the current mouse position. */
        // const glm::vec2& GetMousePosition() const noexcept { return mousePosition_; }
        /** Return the current mouse position in normalized coordinates. */
        // const glm::vec2& GetMousePositionNormalized() const noexcept { return mousePositionNormalized_; }

        virtual void KeyboardCallback(int key, int scancode, int action, int mods);
        virtual void CharCallback(unsigned int character, int mods);
        virtual void MouseButtonCallback(int button, int action);
        virtual void MousePosCallback(double x, double y);
        virtual void MouseScrollCallback(double xoffset, double yoffset);

        void addTuioCursor(TUIO::TuioCursor *tcur) override;
        void updateTuioCursor(TUIO::TuioCursor *tcur) override;
        void removeTuioCursor(TUIO::TuioCursor *tcur) override;

        // void SetCursorInputMode(int mode);

        // static void BaseEncodeDataStatic();
        // static void BaseDecodeDataStatic();
        void EncodeData();
        void DecodeData();

        // void TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex);
        // void TransferData(const void* data, std::size_t length, std::uint16_t packageId);

        // void TransferResource(std::string_view name, const void* data, std::size_t length, ResourceType type);
        // void TransferResourceToNode(std::string_view name, const void* data, std::size_t length, ResourceType type, std::size_t nodeIndex);
        // void TransferReleaseResource(std::string_view name, ResourceType type);
        // void RequestSharedResources();
        // void RequestSharedResource(std::string_view name, ResourceType type);
        // void WaitForResource(const std::string& name, ResourceType type);

        // bool IsMaster() const;

        // sgct::Engine* GetEngine() const { return engine_.get(); }
        // const FWConfiguration& GetConfig() const { return config_; }
        // FrameBuffer& GetFramebuffer(size_t windowId) { return framebuffers_[windowId]; }

        // const Viewport& GetViewportScreen(size_t windowId) const { return viewportScreen_[windowId]; }
        // Viewport& GetViewportScreen(size_t windowId) { return viewportScreen_[windowId]; }
        // const glm::ivec2& GetViewportQuadSize(size_t windowId) const { return viewportQuadSize_[windowId]; }
        // glm::ivec2& GetViewportQuadSize(size_t windowId) { return viewportQuadSize_[windowId]; }
        // const glm::vec2& GetViewportScaling(size_t windowId) const { return viewportScaling_[windowId]; }
        // glm::vec2& GetViewportScaling(size_t windowId) { return viewportScaling_[windowId]; }

        double GetCurrentAppTime() const { return syncInfoLocal_.currentTime_; }
        double GetElapsedTime() const { return elapsedTime_; }
        // void Terminate() const;

        // CameraHelper* GetCamera() { return &camHelper_; }
        // std::vector<FrameBuffer> CreateOffscreenBuffers(const FrameBufferDescriptor& fboDesc, int sizeDivisor = 1) const;
        // const FrameBuffer* SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const;
        // std::unique_ptr<FullscreenQuad> CreateFullscreenQuad(const std::string& fragmentShader);

        // GPUProgramManager& GetGPUProgramManager() { return gpuProgramManager_; }
        // TextureManager& GetTextureManager() { return textureManager_; }
        // MeshManager& GetMeshManager() { return meshManager_; }

    protected:
        FrameworkInternal & GetFramework() { return fwInternal_; }
        void SetApplicationNode(std::unique_ptr<ApplicationNodeBase> appNodeImpl) { appNodeImpl_ = std::move(appNodeImpl); }
        // ApplicationNodeBase* GetApplicationNode() { return appNodeImpl_.get(); }

        // virtual void NodeInitOpenGL();
        // virtual void NodeClearBuffer(FrameBuffer& fbo);
        // virtual void NodeDrawFrame(FrameBuffer& fbo);
        // virtual void NodeDraw2D(FrameBuffer& fbo);
        // virtual void NodeCleanUp();

        /** Holds the synchronized object (local). */
        InternalSyncedInfo syncInfoLocal_;
        /** Holds the synchronized object (synced). */
        sgct::SharedObject<InternalSyncedInfo> syncInfoSynced_;

#ifdef VISCOM_SYNCINPUT
        /** Holds the synchronized vector with keyboard events. */
        sgct::SharedVector<KeyboardEvent> keyboardEventsSynced_;
        /** Holds the synchronized vector with character events. */
        sgct::SharedVector<CharEvent> charEventsSynced_;
        /** Holds the synchronized vector with mouse button events. */
        sgct::SharedVector<MouseButtonEvent> mouseButtonEventsSynced_;
        /** Holds the synchronized vector with mouse position events. */
        sgct::SharedVector<MousePosEvent> mousePosEventsSynced_;
        /** Holds the synchronized vector with mouse scroll events. */
        sgct::SharedVector<MouseScrollEvent> mouseScrollEventsSynced_;
#endif

    private:
        // void ReleaseSynchronizedResource(ResourceType type, std::string_view name);
        // void CreateSynchronizedResource(ResourceType type, const void* data, std::size_t length);
        // void CreateSynchronizedResources();
        // void SendResourcesToNode(ResourceType type, const void* data, std::size_t length, int clientID);
        // static int MakePackageID(std::uint8_t internalType, std::uint8_t internalPID, std::uint16_t userPID);

        /** The internal framework class. */
        FrameworkInternal& fwInternal_;

        /** Holds the application node implementation. */
        std::unique_ptr<ApplicationNodeBase> appNodeImpl_;


        /** Holds the last frame time. */
        double lastFrameTime_ = 0.0;
        /** Holds the time elapsed since the last frame. */
        double elapsedTime_;

        // /** Holds the GPU program manager. */
        // GPUProgramManager gpuProgramManager_;
        // /** Holds the texture manager. */
        // TextureManager textureManager_;
        // /** Holds the mesh manager. */
        // MeshManager meshManager_;

        /** Holds the current mouse position. */
        // glm::vec2 mousePosition_;
        // /** Holds the current normalized mouse position. */
        // glm::vec2 mousePositionNormalized_;
        // /** Holds the current keyboard state. */
        // std::vector<bool> keyPressedState_;
        // /** Holds the current mouse button state. */
        // std::vector<bool> mousePressedState_;
        /** Is the application currently halted. */
        // bool applicationHalted_ = false;

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
    };
}
