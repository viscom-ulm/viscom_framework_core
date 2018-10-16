/**
 * @file   ApplicationNodeBase.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Declaration of the application node implementation common for master and slave nodes.
 */

#pragma once

#include "core/app_internal/ApplicationNodeInternal.h"
#include "core/FrameworkInternal.h"

namespace viscom {

    class FrameworkInternal;
    class ApplicationNodeInternal;
    class FrameBuffer;

    class ApplicationNodeBase
    {
    public:
        explicit ApplicationNodeBase(ApplicationNodeInternal* appNode);
        ApplicationNodeBase(const ApplicationNodeBase&) = delete;
        ApplicationNodeBase(ApplicationNodeBase&&) = delete;
        ApplicationNodeBase& operator=(const ApplicationNodeBase&) = delete;
        ApplicationNodeBase& operator=(ApplicationNodeBase&&) = delete;
        virtual ~ApplicationNodeBase();

        /** Called before a window is created. */
        virtual void PreWindow();
        /** Called after the OpenGL context is created. Here OpenGL objects can be initialized. */
        virtual void InitOpenGL();
        /** Called before each synchronization in each frame to prepare for it. */
        virtual void PreSync();
        /** Called after each synchronization in each frame to update local information based on the sync. */
        virtual void UpdateSyncedInfo();
        /**
         *  This method is called once each frame to step forward the simulation.
         *  @param currentTime the current time of the application.
         *  @param elapsedTime the time elapsed during the last frame.
         */
        virtual void UpdateFrame(double currentTime, double elapsedTime);
        virtual void ClearBuffer(FrameBuffer& fbo);
        virtual void DrawFrame(FrameBuffer& fbo);
        virtual void Draw2D(FrameBuffer& fbo);
        virtual void CleanUp();

        virtual bool DataTransferCallback(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID);
        virtual bool DataAcknowledgeCallback(std::uint16_t packageID, int clientID);
        virtual bool DataTransferStatusCallback(bool connected, int clientID);

        virtual bool KeyboardCallback(int key, int scancode, int action, int mods);
        virtual bool CharCallback(unsigned int character, int mods);
        virtual bool MouseButtonCallback(int button, int action);
        virtual bool MousePosCallback(double x, double y);
        virtual bool MouseScrollCallback(double xoffset, double yoffset);

        virtual bool AddTuioCursor(TUIO::TuioCursor *tcur);
        virtual bool UpdateTuioCursor(TUIO::TuioCursor *tcur);
        virtual bool RemoveTuioCursor(TUIO::TuioCursor *tcur);

        virtual void EncodeData();
        virtual void DecodeData();

        bool IsMouseButtonPressed(int button) const noexcept { return framework_->IsMouseButtonPressed(button); }
        bool IsKeyPressed(int key) const noexcept { return framework_->IsKeyPressed(key); }

        /** Returns the current mouse position. */
        const glm::vec2& GetMousePosition() const noexcept { return framework_->GetMousePosition(); }
        /** Return the current mouse position in normalized coordinates. */
        const glm::vec2& GetMousePositionNormalized() const noexcept { return framework_->GetMousePositionNormalized(); }

        void SetCursorInputMode(int mode) { framework_->SetCursorInputMode(mode); }

        GPUProgramManager& GetGPUProgramManager() { return framework_->GetGPUProgramManager(); }
        TextureManager& GetTextureManager() { return framework_->GetTextureManager(); }
        MeshManager& GetMeshManager() { return framework_->GetMeshManager(); }

        CameraHelper* GetCamera() { return framework_->GetCamera(); }
        std::vector<FrameBuffer> CreateOffscreenBuffers(const FrameBufferDescriptor& fboDesc, int sizeDivisor = 1) const { return framework_->CreateOffscreenBuffers(fboDesc, sizeDivisor); }
        const FrameBuffer* SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const { return framework_->SelectOffscreenBuffer(offscreenBuffers); }
        std::unique_ptr<FullscreenQuad> CreateFullscreenQuad(const std::string& fragmentShader) { return framework_->CreateFullscreenQuad(fragmentShader); }

        void TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex) const { framework_->TransferDataToNode(data, length, packageId, nodeIndex); }
        void TransferData(const void* data, std::size_t length, std::uint16_t packageId) const { framework_->TransferData(data, length, packageId); }

        /** Initailises OpenVR */
        virtual bool InitialiseVR() { return appNode_->InitialiseVR(); }
        /** Calibrates OpenVR */
        virtual bool CalibrateVR(CalibrateMethod method) { return appNode_->CalibrateVR(method); }
        /** Returns a vector containing the connected Devices */
        virtual const std::vector<DeviceInfo>& GetConnectedDevices() { return appNode_->GetConnectedDevices(); }
        /**
        * Returns the current controller position for a given device id.
        * @param trackedDeviceId to select a device.
        * @return position of the selected controller.
        */
        virtual const glm::vec3& GetControllerPosition(size_t trackedDeviceId) { return appNode_->GetControllerPosition(trackedDeviceId); }
        /**
        * Returns the current z-vector for a given device id.
        * @param trackedDeviceId to select a device.
        * @return z-vector of the selected controller.
        */
        virtual const glm::vec3& GetControllerZVector(size_t trackedDeviceId) { return appNode_->GetControllerZVector(trackedDeviceId); }
        /**
        * Returns the current controller rotation for a given device id.
        * @param trackedDeviceId to select a device.
        * @return rotation quaternion for the selected controller.
        */
        virtual const glm::quat& GetControllerRotation(size_t trackedDeviceId) { return appNode_->GetControllerRotation(trackedDeviceId); }
        /**
        * Returns the display pointing position for a given device id.
        * @param trackedDeviceId to select a device.
        * @return position of the selected controller.
        */
        virtual const glm::vec2& GetDisplayPointerPosition(size_t trackedDeviceId) { return appNode_->GetDisplayPointerPosition(trackedDeviceId); }

        /** 
        * Callback for a button press on a controller.
        * @param trackedDeviceId identifies the device.
        * @param buttonid identifies the button ( using the OpenVR buttonids).
        * @param axisvalues for buttons having axis this will be filled with the axis values.
        */
        virtual bool ControllerButtonPressedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues);
        /**
        * Callback for a button touch on a controller.
        * @param trackedDeviceId identifies the device.
        * @param buttonid identifies the button ( using the OpenVR buttonids).
        * @param axisvalues for buttons having axis this will be filled with the axis values.
        */
        virtual bool ControllerButtonTouchedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues);
        /**
        * Callback if a button on a controller is not pressed anymore.
        * @param trackedDeviceId identifies the device.
        * @param buttonid identifies the button ( using the OpenVR buttonids).
        * @param axisvalues for buttons having axis this will be filled with the axis values.
        */
        virtual bool ControllerButtonUnpressedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues);
        /**
        * Callback if a button on a controller is not touched anymore.
        * @param trackedDeviceId identifies the device.
        * @param buttonid identifies the button ( using the OpenVR buttonids).
        * @param axisvalues for buttons having axis this will be filled with the axis values.
        */
        virtual bool ControllerButtonUntouchedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues);

        /**
        * Immediate method to get the buttonstate of a controller.
        * @param trackedDeviceId identifies the device.
        * @param buttonid identifies the button ( using the OpenVR buttonids).
        * @param axisvalues for buttons having axis this will be filled with the axis values.
        * @param buttonstate for identifying the button state ( pressed, touched, released)
        */
        virtual void GetControllerButtonState(size_t trackedDeviceId, size_t buttonid, glm::vec2& axisvalues, ButtonState& buttonstate);

    protected:
        const FWConfiguration& GetConfig() const { return framework_->GetConfig(); }
        ApplicationNodeInternal* GetApplication() const { return appNode_; }
        std::size_t GetCurrentWindowID() const { return framework_->GetCurrentWindowID(); }
        const Viewport& GetViewportScreen(size_t windowId) const { return framework_->GetViewportScreen(windowId); }
        Viewport& GetViewportScreen(size_t windowId) { return framework_->GetViewportScreen(windowId); }
        const glm::ivec2& GetViewportQuadSize(size_t windowId) const { return framework_->GetViewportQuadSize(windowId); }
        glm::ivec2& GetViewportQuadSize(size_t windowId) { return framework_->GetViewportQuadSize(windowId); }
        const glm::vec2& GetViewportScaling(size_t windowId) const { return framework_->GetViewportScaling(windowId); }
        glm::vec2& GetViewportScaling(size_t windowId) { return framework_->GetViewportScaling(windowId); }

        double GetCurrentAppTime() const { return appNode_->GetCurrentAppTime(); }
        double GetElapsedTime() const { return appNode_->GetElapsedTime(); }
        void Terminate() const;

    private:
        /** Holds the application node. */
        ApplicationNodeInternal* appNode_;
        /** Holds the framework. */
        FrameworkInternal* framework_;

    };
}
