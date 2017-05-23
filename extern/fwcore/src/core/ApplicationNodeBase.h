/**
 * @file   ApplicationNodeBase.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Declaration of the application node implementation common for master and slave nodes.
 */

#pragma once

#include "core/ApplicationNodeInternal.h"

namespace viscom {

    class MeshRenderable;


#ifndef VISCOM_LOCAL_ONLY
    class SGCTEngineWrapper
    {
        friend class SlaveNodeInternal;

    public:
        SGCTEngineWrapper(sgct::Engine* engine) : engine_{ engine } {}

    private:
        int GetCurrentWindowId() const { return engine_->getCurrentWindowPtr()->getId(); }
        void UnbindCurrentWindowFBO() const { engine_->getCurrentWindowPtr()->getFBOPtr()->unBind(); }
        void SetProjectionPlaneCoordinate(std::size_t windowIdx, std::size_t vpIndex, std::size_t corner, glm::vec3 coordinate) const {
            engine_->getWindowPtr(windowIdx)->getViewport(vpIndex)->getProjectionPlane()->setCoordinate(corner, coordinate);
        }

        /** Holds the SGCT engine. */
        sgct::Engine* engine_;
    };
#endif

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
        virtual void PostDraw();
        virtual void CleanUp();

        virtual bool KeyboardCallback(int key, int scancode, int action, int mods);
        virtual bool CharCallback(unsigned int character, int mods);
        virtual bool MouseButtonCallback(int button, int action);
        virtual bool MousePosCallback(double x, double y);
        virtual bool MouseScrollCallback(double xoffset, double yoffset);

        virtual void EncodeData();
        virtual void DecodeData();

    protected:
        const FWConfiguration& GetConfig() const { return appNode_->GetConfig(); }
        ApplicationNodeInternal* GetApplication() const { return appNode_; }

        const Viewport& GetViewportScreen(size_t windowId) const { return appNode_->GetViewportScreen(windowId); }
        Viewport& GetViewportScreen(size_t windowId) { return appNode_->GetViewportScreen(windowId); }
        const glm::ivec2& GetViewportQuadSize(size_t windowId) const { return appNode_->GetViewportQuadSize(windowId); }
        glm::ivec2& GetViewportQuadSize(size_t windowId) { return appNode_->GetViewportQuadSize(windowId); }
        const glm::vec2& GetViewportScaling(size_t windowId) const { return appNode_->GetViewportScaling(windowId); }
        glm::vec2& GetViewportScaling(size_t windowId) { return appNode_->GetViewportScaling(windowId); }

        double GetCurrentAppTime() const { return appNode_->GetCurrentAppTime(); }
        double GetElapsedTime() const { return appNode_->GetElapsedTime(); }

        GPUProgramManager& GetGPUProgramManager() { return appNode_->GetGPUProgramManager(); }
        TextureManager& GetTextureManager() { return appNode_->GetTextureManager(); }
        MeshManager& GetMeshManager() { return appNode_->GetMeshManager(); }

        CameraHelper* GetCamera() { return appNode_->GetCamera(); }
        std::vector<FrameBuffer> CreateOffscreenBuffers(const FrameBufferDescriptor& fboDesc) const { return appNode_->CreateOffscreenBuffers(fboDesc); }
        const FrameBuffer* SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const { return appNode_->SelectOffscreenBuffer(offscreenBuffers); }
        std::unique_ptr<FullscreenQuad> CreateFullscreenQuad(const std::string& fragmentShader) { return appNode_->CreateFullscreenQuad(fragmentShader); }

    private:
        /** Holds the application node. */
        ApplicationNodeInternal* appNode_;

#ifndef VISCOM_LOCAL_ONLY
    protected:
        unsigned int GetGlobalProjectorId(int nodeId, int windowId) const { return appNode_->GetGlobalProjectorId(nodeId, windowId); }
        SGCTEngineWrapper GetEngine() const { return appNode_->GetEngine(); }
#endif
    };
}
