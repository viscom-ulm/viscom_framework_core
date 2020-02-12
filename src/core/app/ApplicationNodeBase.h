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
#include "OpenVR.h"
#include "OpenVRControllerIfc.h"

struct GLFWwindow;

namespace viscom {

    class FrameworkInternal;
    class ApplicationNodeInternal;
    class FrameBuffer;

    /** Base class for the application node. */
    class ApplicationNodeBase : public ovr::OpenVRControllerIfc
    {
    public:
        /**
         *  Constructor method.
         *  @param appNode the ApplicationNodeInternal object to create the ApplicationNodeBase with.
         */
        explicit ApplicationNodeBase(ApplicationNodeInternal* appNode);
        ApplicationNodeBase(const ApplicationNodeBase&) = delete;
        ApplicationNodeBase(ApplicationNodeBase&&) = delete;
        ApplicationNodeBase& operator=(const ApplicationNodeBase&) = delete;
        ApplicationNodeBase& operator=(ApplicationNodeBase&&) = delete;
        virtual ~ApplicationNodeBase();

        /** Called before a window is created. */
        [[deprecated("All initialization should be moved to the constructor in the future.")]]
        virtual void PreWindow();
        /** Called after the OpenGL context is created. Here OpenGL objects can be initialized. */
        [[deprecated("All initialization should be moved to the constructor in the future.")]]
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
        /**
         *  This method is called once each frame to clear any frame buffer.
         *  @param fbo back buffer of the framework.
         */
        virtual void ClearBuffer(FrameBuffer& fbo);
        /**
         *  This method is called once each frame to draw on any frame buffer and render the scene.
         *  @param fbo back buffer of the framework.
         */
        virtual void DrawFrame(FrameBuffer& fbo);
        /**
         *  This method is called once each frame to render the GUI and 2D elements on the screen.
         *  @param fbo back buffer of the framework.
         */
        virtual void Draw2D(FrameBuffer& fbo);
        /** This method is called when exiting the application in order to delete vertex arrays and buffers. */
        [[deprecated("All initialization should be moved to the destructor in the future.")]]
        virtual void CleanUp();

        /**
         *  Called when receiving a message from another node.
         *  @param receivedData pointer to the received data.
         *  @param receivedLength length of the received data.
         *  @param packageID index of the received package.
         *  @param clientID index of the node sending the message.
         */
        virtual bool DataTransferCallback(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID);
        /**
         *  Called when successfully sending a message to another node.
         *  @param packageID index of the sent package.
         *  @param clientID index of the node receiving the message.
         */
        virtual bool DataAcknowledgeCallback(std::uint16_t packageID, int clientID);
        /**
         *  Called when the connection status changes.
         *  @param connected connection status.
         *  @param clientID index of the node connected or disconnected.
         */
        virtual bool DataTransferStatusCallback(bool connected, int clientID);

        /**
         *  This method is called to handle keyboard input.
         *  @param key GLFW key code.
         *  @param scancode platform specific scancode.
         *  @param action GLFW key and button action.
         *  @param mods modifier key flags.
         */
        virtual bool KeyboardCallback(int key, int scancode, int action, int mods);
        /**
         *  This method is called to handle keyboard character input.
         *  @param character key code of the character being pressed.
         *  @param mods modifier key flags.
         */
        virtual bool CharCallback(unsigned int character, int mods);
        /**
         *  This method is called to handle mouse button input.
         *  @param button GLFW mouse button code.
         *  @param action GLFW key and button action.
         */
        virtual bool MouseButtonCallback(int button, int action);
        /**
         *  This method is called to handle the cursor position.
         *  @param x horizontal position of the mouse cursor.
         *  @param y vertical position of the mouse cursor.
         */
        virtual bool MousePosCallback(double x, double y);
        /**
         *  This method is called to handle mouse wheel scrolling.
         *  @param xoffset horizontal movement of the mouse wheel.
         *  @param yoffset vertical movement of the mouse wheel.
         */
        virtual bool MouseScrollCallback(double xoffset, double yoffset);

        /**
         *  Called for touch screens to add a cursor.
         *  @param tcur cursor to be added.
         */
        virtual bool AddTuioCursor(TUIO::TuioCursor *tcur);
        /**
         *  Called each frame for touch screens to update a cursor.
         *  @param tcur cursor to be updated.
         */
        virtual bool UpdateTuioCursor(TUIO::TuioCursor *tcur);
        /**
         *  Called for touch screens to remove a cursor.
         *  @param tcur cursor to be removed.
         */
        virtual bool RemoveTuioCursor(TUIO::TuioCursor *tcur);

        /** Synchronizes the framework by encoding and sending all synchronized data to the other nodes. */
        virtual void EncodeData();
        /** Synchronizes the framework by receiving and decoding synchronized data from other nodes. */
        virtual void DecodeData();

        /**
         *  Checks if a mouse button is currently pressed.
         *  @param button GLFW mouse button code.
         */
        bool IsMouseButtonPressed(int button) const noexcept { return framework_->IsMouseButtonPressed(button); }
        /**
         *  Checks if a key is currently pressed.
         *  @param key GLFW key code.
         */
        bool IsKeyPressed(int key) const noexcept { return framework_->IsKeyPressed(key); }

        /** Returns the current mouse position. */
        const glm::vec2& GetMousePosition() const noexcept { return framework_->GetMousePosition(); }
        /** Return the current mouse position in normalized coordinates. */
        const glm::vec2& GetMousePositionNormalized() const noexcept { return framework_->GetMousePositionNormalized(); }

        /**
         *  Sets the input mode for the mouse cursor.
         *  @param mode specifies the GLFW cursor input mode. Can be GLFW_CURSOR_NORMAL, GLFW_CURSOR_HIDDEN or GLFW_CURSOR_DISABLED.
         */
        void SetCursorInputMode(int mode) { framework_->SetCursorInputMode(mode); }

        /** Returns the GPU program manager for shader resource management. */
        GPUProgramManager& GetGPUProgramManager() { return framework_->GetGPUProgramManager(); }
        /** Returns the texture manager for texture resource management. */
        TextureManager& GetTextureManager() { return framework_->GetTextureManager(); }
        /** Returns the mesh manager for mesh resource management. */
        MeshManager& GetMeshManager() { return framework_->GetMeshManager(); }

        /** Return the camera of the scene. */
        CameraHelper* GetCamera() { return framework_->GetCamera(); }
        /**
         *  Creates frame buffers, appropriate textures and render buffers for offscreen rendering.
         *  @param fboDesc descriptor holding information about the number and type of textures and render buffers.
         *  @param sizeDivisor size of the frame buffers in inverse proportion to the size of the back buffer.
         */
        std::vector<FrameBuffer> CreateOffscreenBuffers(const FrameBufferDescriptor& fboDesc, int sizeDivisor = 1) const { return framework_->CreateOffscreenBuffers(fboDesc, sizeDivisor); }
        /**
         *  Returns a frame buffer from previously created offscreen buffers.
         *  @param offscreenBuffers list of frame buffers created for offscreen rendering.
         */
        const FrameBuffer* SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const { return framework_->SelectOffscreenBuffer(offscreenBuffers); }
        /**
         *  Creates a fullscreen quad for shading.
         *  @param fragmentShader fragment shader to be used by the fullscreen quad.
         */
        std::unique_ptr<FullscreenQuad> CreateFullscreenQuad(const std::string& fragmentShader) { return framework_->CreateFullscreenQuad(fragmentShader); }

        /**
         *  Sends data to a specified node.
         *  @param data pointer to the data to be sent.
         *  @param length length of the data to be sent.
         *  @param packageId index of the package.
         *  @param nodeIndex index of the node the data is sent to.
         */
        void TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex) const { framework_->TransferDataToNode(data, length, packageId, nodeIndex); }
        /**
         *  Sends data to all nodes.
         *  @param data pointer to the data to be sent.
         *  @param length length of the data to be sent.
         *  @param packageId index of the package.
         */
        void TransferData(const void* data, std::size_t length, std::uint16_t packageId) const { framework_->TransferData(data, length, packageId); }

        /** Initializes OpenVR */
        virtual bool InitialiseVR() override { return appNode_->InitialiseVR(); }
        /** Initializes the display edges either by file or with default values if no displayEdges txt is found */
        virtual bool InitialiseDisplayVR() override { return appNode_->InitialiseDisplayVR(); }
        /** Calibrates OpenVR */
        virtual bool CalibrateVR(ovr::CalibrateMethod method) override { return appNode_->CalibrateVR(method); }
        /** Returns a vector containing the connected Devices */
        virtual const std::vector<ovr::DeviceInfo>& GetConnectedDevices() const override { return appNode_->GetConnectedDevices(); }
        /**
        * Returns the current controller position for a given device id.
        * @param trackedDeviceId to select a device.
        * @return position of the selected controller.
        */
        virtual const glm::vec3& GetControllerPosition(std::uint32_t trackedDeviceId) const override { return appNode_->GetControllerPosition(trackedDeviceId); }
        /**
        * Returns the current z-vector for a given device id.
        * @param trackedDeviceId to select a device.
        * @return z-vector of the selected controller.
        */
        virtual const glm::vec3& GetControllerDirection(std::uint32_t trackedDeviceId) const override { return appNode_->GetControllerDirection(trackedDeviceId); }
        /**
        * Returns the current controller rotation for a given device id.
        * @param trackedDeviceId to select a device.
        * @return rotation quaternion for the selected controller.
        */
        virtual const glm::quat& GetControllerOrientation(std::uint32_t trackedDeviceId) const override { return appNode_->GetControllerOrientation(trackedDeviceId); }
        /**
        * Returns the display pointing position for a given device id.
        * @param trackedDeviceId to select a device.
        * @return position of the selected controller.
        */
        virtual const glm::vec2& GetDisplayPointerPosition(std::uint32_t trackedDeviceId) const override { return appNode_->GetDisplayPointerPosition(trackedDeviceId); }

        /** 
        * Callback for a button press on a controller.
        * @param trackedDeviceId identifies the device.
        * @param buttonid identifies the button ( using the OpenVR button ids).
        */
        virtual bool ControllerButtonPressedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override;
        /**
        * Callback for a button touch on a controller.
        * @param trackedDeviceId identifies the device.
        * @param buttonid identifies the button ( using the OpenVR button ids).
        */
        virtual bool ControllerButtonTouchedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override;
        /**
        * Callback if a button on a controller is not pressed anymore.
        * @param trackedDeviceId identifies the device.
        * @param buttonid identifies the button ( using the OpenVR button ids).
        */
        virtual bool ControllerButtonPressReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override;
        /**
        * Callback if a button on a controller is not touched anymore.
        * @param trackedDeviceId identifies the device.
        * @param buttonid identifies the button ( using the OpenVR button ids).
        */
        virtual bool ControllerButtonTouchReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override;

        /**
        * Immediate method to get the buttonstate of a controller.
        * @param trackedDeviceId identifies the device.
        * @param buttonid identifies the button ( using the OpenVR button ids).
        * @param axisvalues for buttons having axis this will be filled with the axis values.
        * @param buttonstate for identifying the button state ( pressed, touched, released)
        */
        virtual void GetControllerButtonState(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2& axisvalues, ovr::ButtonState& buttonstate) const override;
        virtual std::vector<std::string> OutputDevices() const override { return appNode_->OutputDevices(); }

    protected:
        /** Returns the applications configuration. */
        const FWConfiguration& GetConfig() const { return framework_->GetConfig(); }
        /** Returns the application node. */
        ApplicationNodeInternal* GetApplication() const { return appNode_; }
        /** Returns the current window id. */
        std::size_t GetCurrentWindowID() const { return framework_->GetCurrentWindowID(); }
        /** Returns the main window of the current node (for shared contexts). */
        GLFWwindow* GetCurrentNodeMainWindow() const { return framework_->GetCurrentNodeMainWindow(); }
        /**
         *  Returns the viewport for the specified window.
         *  @param windowId the window id.
         */
        const Viewport& GetViewportScreen(std::size_t windowId) const { return framework_->GetViewportScreen(windowId); }
        /**
         *  Returns the viewport for the specified window.
         *  @param windowId the window id.
         */
        Viewport& GetViewportScreen(std::size_t windowId) { return framework_->GetViewportScreen(windowId); }
        /**
         *  Returns the size of the viewport for the specified window.
         *  @param windowId the window id.
         */
        const glm::ivec2& GetViewportQuadSize(std::size_t windowId) const { return framework_->GetViewportQuadSize(windowId); }
        /**
         *  Returns the size of the viewport for the specified window.
         *  @param windowId the window id.
         */
        glm::ivec2& GetViewportQuadSize(std::size_t windowId) { return framework_->GetViewportQuadSize(windowId); }
        /**
         *  Returns the viewport scaling for the specified window.
         *  @param windowId the window id.
         */
        const glm::vec2& GetViewportScaling(std::size_t windowId) const { return framework_->GetViewportScaling(windowId); }
        /**
         *  Returns the viewport scaling for the specified window.
         *  @param windowId the window id.
         */
        glm::vec2& GetViewportScaling(std::size_t windowId) { return framework_->GetViewportScaling(windowId); }

        /** Returns the current application time. */
        double GetCurrentAppTime() const { return appNode_->GetCurrentAppTime(); }
        /** Returns the time elapsed since the last frame. */
        double GetElapsedTime() const { return appNode_->GetElapsedTime(); }
        /** Commands the GLFW window to close. */
        void Terminate() const;

    private:
        /** Holds the application node. */
        ApplicationNodeInternal* appNode_;
        /** Holds the framework. */
        FrameworkInternal* framework_;

    };
}
