/**
 * @file   ApplicationNodeBase.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Declaration of the application node implementation common for master and slave nodes.
 */

#pragma once

// #include "core/app_internal/ApplicationNodeInternal.h"

namespace viscom {

    class FrameworkInternal;
    class ApplicationNodeInternal;

// #ifndef VISCOM_LOCAL_ONLY
//     class SGCTEngineWrapper
//     {
//         friend class SlaveNodeInternal;
// 
//     public:
//         SGCTEngineWrapper(sgct::Engine* engine) : engine_{ engine } {}
// 
//     private:
//         int GetCurrentWindowId() const;
//         void UnbindCurrentWindowFBO() const;
//         void SetProjectionPlaneCoordinate(std::size_t windowIdx, std::size_t vpIndex, std::size_t corner, glm::vec3 coordinate) const;
// 
//         /** Holds the SGCT engine. */
//         sgct::Engine* engine_;
//     };
// #endif

    class ApplicationNodeBase
    {
    public:
        explicit ApplicationNodeBase(ApplicationNodeInternal* appNode);
        ApplicationNodeBase(const ApplicationNodeBase&) = delete;
        ApplicationNodeBase(ApplicationNodeBase&&) = delete;
        ApplicationNodeBase& operator=(const ApplicationNodeBase&) = delete;
        ApplicationNodeBase& operator=(ApplicationNodeBase&&) = delete;
        virtual ~ApplicationNodeBase();

        virtual void PreWindow();
        virtual void InitOpenGL();
        virtual void PreSync();
        virtual void UpdateSyncedInfo();
        virtual void UpdateFrame(double currentTime, double elapsedTime);
        virtual void ClearBuffer(FrameBuffer& fbo);
        virtual void DrawFrame(FrameBuffer& fbo);
        virtual void Draw2D(FrameBuffer& fbo);
        virtual void CleanUp();

        virtual bool DataTransferCallback(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID);
        virtual bool DataAcknowledgeCallback(std::uint16_t packageID, int clientID);
        virtual bool DataTransferStatusCallback(bool connected, int clientID);

        // bool IsMouseButtonPressed(int button) const noexcept { return appNode_->IsMouseButtonPressed(button); }
        // bool IsKeyPressed(int key) const noexcept { return appNode_->IsKeyPressed(key); }
        // 
        // /** Returns the current mouse position. */
        // const glm::vec2& GetMousePosition() const noexcept { return appNode_->GetMousePosition(); }
        // /** Return the current mouse position in normalized coordinates. */
        // const glm::vec2& GetMousePositionNormalized() const noexcept { return appNode_->GetMousePositionNormalized(); }

        virtual bool KeyboardCallback(int key, int scancode, int action, int mods);
        virtual bool CharCallback(unsigned int character, int mods);
        virtual bool MouseButtonCallback(int button, int action);
        virtual bool MousePosCallback(double x, double y);
        virtual bool MouseScrollCallback(double xoffset, double yoffset);

        virtual bool AddTuioCursor(TUIO::TuioCursor *tcur);
        virtual bool UpdateTuioCursor(TUIO::TuioCursor *tcur);
        virtual bool RemoveTuioCursor(TUIO::TuioCursor *tcur);

        // void SetCursorInputMode(int mode) { appNode_->SetCursorInputMode(mode); }

        virtual void EncodeData();
        virtual void DecodeData();

        // GPUProgramManager& GetGPUProgramManager() { return appNode_->GetGPUProgramManager(); }
        // TextureManager& GetTextureManager() { return appNode_->GetTextureManager(); }
        // MeshManager& GetMeshManager() { return appNode_->GetMeshManager(); }

        // CameraHelper* GetCamera() { return appNode_->GetCamera(); }
        // std::vector<FrameBuffer> CreateOffscreenBuffers(const FrameBufferDescriptor& fboDesc, int sizeDivisor = 1) const { return appNode_->CreateOffscreenBuffers(fboDesc, sizeDivisor); }
        // const FrameBuffer* SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const { return appNode_->SelectOffscreenBuffer(offscreenBuffers); }
        // std::unique_ptr<FullscreenQuad> CreateFullscreenQuad(const std::string& fragmentShader) { return appNode_->CreateFullscreenQuad(fragmentShader); }

    protected:
        // const FWConfiguration& GetConfig() const { return appNode_->GetConfig(); }
        // ApplicationNodeInternal* GetApplication() const { return appNode_; }
        // 
        // const Viewport& GetViewportScreen(size_t windowId) const { return appNode_->GetViewportScreen(windowId); }
        // Viewport& GetViewportScreen(size_t windowId) { return appNode_->GetViewportScreen(windowId); }
        // const glm::ivec2& GetViewportQuadSize(size_t windowId) const { return appNode_->GetViewportQuadSize(windowId); }
        // glm::ivec2& GetViewportQuadSize(size_t windowId) { return appNode_->GetViewportQuadSize(windowId); }
        // const glm::vec2& GetViewportScaling(size_t windowId) const { return appNode_->GetViewportScaling(windowId); }
        // glm::vec2& GetViewportScaling(size_t windowId) { return appNode_->GetViewportScaling(windowId); }

        double GetCurrentAppTime() const { return appNode_->GetCurrentAppTime(); }
        double GetElapsedTime() const { return appNode_->GetElapsedTime(); }
        void Terminate() const;

    private:
        /** Holds the application node. */
        ApplicationNodeInternal* appNode_;
        /** Holds the framework. */
        FrameworkInternal* framework_;

// #ifndef VISCOM_LOCAL_ONLY
//     protected:
//         unsigned int GetGlobalProjectorId(int nodeId, int windowId) const { return appNode_->GetGlobalProjectorId(nodeId, windowId); }
//         SGCTEngineWrapper GetEngine() const { return appNode_->GetEngine(); }
// #endif
    };
}
