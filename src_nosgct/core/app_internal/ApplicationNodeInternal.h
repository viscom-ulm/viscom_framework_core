/**
 * @file   ApplicationNodeInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declares a base class for all application nodes in the VISCOM lab cluster.
 */

#pragma once

#include "core/main.h"
#include "core/TuioInputWrapper.h"
#include "core/FrameworkInternal.h"

namespace vr {
    class IVRSystem;
    struct VREvent_t;
}

namespace viscom {
    enum class CalibrateMethod { CALIBRATE_BY_TOUCHING, CALIBRATE_BY_POINTING };
    enum class TrackedDeviceRole { CONTROLLER_LEFT_HAND, CONTROLLER_RIGHT_HAND, GENERIC_TRACKER, INVALID };
    enum class ButtonState { PRESSED, TOUCHED, RELEASED };
    enum class TrackedDeviceClass { INVALID, CONTROLLER, GENERIC_TRACKER, TRACKING_REFERENCE, DISPLAY_REDIRECT, HMD };

    struct DeviceInfo {
        size_t deviceId;
        TrackedDeviceRole deviceRole;
        TrackedDeviceClass deviceClass;
    };

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
        /**
        *  Constructor method.
        *  @param fwInternal the FrameworkInternal object to create the ApplicationNodeInternal with.
        */
        ApplicationNodeInternal(FrameworkInternal& fwInternal);
        ApplicationNodeInternal(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal(ApplicationNodeInternal&&) = delete;
        ApplicationNodeInternal& operator=(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal& operator=(ApplicationNodeInternal&&) = delete;
        virtual ~ApplicationNodeInternal() override;

        /** Called before a window is created. */
        [[deprecated("All initialization should be moved to the constructor in the future.")]]
        virtual void PreWindow();
        /** Called after the OpenGL context is created. Here OpenGL objects can be initialized. */
        [[deprecated("All initialization should be moved to the constructor in the future.")]]
        virtual void InitOpenGL();
        /** Called before each synchronization in each frame to prepare for it. */
        virtual void PreSync();
        /** Called after each synchronization in each frame to update local information based on the sync. */
        virtual void PostSync();
        /**
        *  This method is called once each frame to clear any frame buffer.
        *  @see ApplicationNodeBase::ClearBuffer.
        */
        virtual void ClearBuffer(FrameBuffer& fbo);
        /**
        *  This method is called once each frame to draw on any frame buffer.
        *  @see ApplicationNodeBase::DrawFrame.
        */
        virtual void DrawFrame(FrameBuffer& fbo);
        /**
        *  This method is called once each frame to render the GUI and 2D elements.
        *  @see ApplicationNodeBase::Draw2D.
        */
        virtual void Draw2D(FrameBuffer& fbo);
        /** Called each frame after everything has been drawn. */
        virtual void PostDraw();
        /**
         *  This method is called when exiting the application.
         *  @see ApplicationNodeBase::CleanUp.
         */
        [[deprecated("All initialization should be moved to the destructor in the future.")]]
        virtual void CleanUp();
        /**
         *  Called when receiving a message from another node.
         *  @see ApplicationNodeBase::DataTransferCallback.
         */
        void DataTransfer(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID);
        /**
         *  Called when successfully sending a message to another node.
         *  @see ApplicationNodeBase::DataAcknowledgeCallback.
         */
        void DataAcknowledge(std::uint16_t packageID, int clientID);
        /**
         *  Called when the connection status changes.
         *  @see ApplicationNodeBase::DataTransferStatusCallback.
         */
        void DataTransferStatus(bool connected, int clientID);

        /**
         *  This method is called to handle keyboard input.
         *  @see ApplicationNodeBase::KeyboardCallback.
         */
        virtual void KeyboardCallback(int key, int scancode, int action, int mods);
        /**
         *  This method is called to handle keyboard character input.
         *  @see ApplicationNodeBase::CharCallback.
         */
        virtual void CharCallback(unsigned int character, int mods);
        /**
         *  This method is called to handle mouse button input.
         *  @see ApplicationNodeBase::MouseButtonCallback.
         */
        virtual void MouseButtonCallback(int button, int action);
        /**
         *  This method is called to handle the cursor position.
         *  @see ApplicationNodeBase::MousePosCallback.
         */
        virtual void MousePosCallback(double x, double y);
        /**
         *  This method is called to handle mouse wheel scrolling.
         *  @see ApplicationNodeBase::MouseScrollCallback.
         */
        virtual void MouseScrollCallback(double xoffset, double yoffset);

        /**
         *  Called for touch screens to add a cursor.
         *  @see ApplicationNodeBase::AddTuioCursor.
         */
        void addTuioCursor(TUIO::TuioCursor *tcur) override;
        /**
         *  Called each frame for touch screens to update a cursor.
         *  @see ApplicationNodeBase::UpdateTuioCursor.
         */
        void updateTuioCursor(TUIO::TuioCursor *tcur) override;
        /**
         *  Called for touch screens to remove a cursor.
         *  @see ApplicationNodeBase::RemoveTuioCursor.
         */
        void removeTuioCursor(TUIO::TuioCursor *tcur) override;

        /** Returns the current application time. */
        double GetCurrentAppTime() const { return currentTime_; }
        /** Returns the time elapsed since the last frame. */
        double GetElapsedTime() const { return elapsedTime_; }

        /** Returns the internal framework. */
        FrameworkInternal & GetFramework() { return fwInternal_; }
        
        /** Initialises OpenVR for controller usage. */
        bool InitialiseVR();
        /** Initialises the Displayedges either by file or with default values if no displayEdges txt is found. */
        bool InitialiseDisplayVR();
        /** Calibrates the display edges by selected method. */
        bool CalibrateVR(CalibrateMethod method);
        /** Returns a DeviceInfo vector with the connected devices. */
        const std::vector<DeviceInfo>& GetConnectedDevices();
        /** Returns the position to a given tracked device id. */
        const glm::vec3& GetControllerPosition(size_t trackedDeviceId);
        /** Returns the z-vector to a given tracked device id. */
        const glm::vec3& GetControllerZVector(size_t trackedDeviceId);
        /** Returns the rotation to a given tracked device id. */
        const glm::quat& GetControllerRotation(size_t trackedDeviceId);
        /** Returns the display pointing position for a given tracked device id. */
        const glm::vec2& GetDisplayPointerPosition(size_t trackedDeviceId);

        void ControllerButtonPressedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues);
        void ControllerButtonTouchedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues);
        void ControllerButtonUnpressedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues);
        void ControllerButtonUntouchedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues);

        /** Fills buttonstate and axisvalues for a given tracked device and buttonid. */
        void GetControllerButtonState(size_t trackedDeviceId, size_t buttonid, glm::vec2& axisvalues, ButtonState& buttonstate);

        std::vector<std::string> OutputDevices();

    protected:
        /**
         * Sets the application node implementation class.
         * @param appNodeImpl the new application node implementation.
         */
        void SetApplicationNode(std::unique_ptr<ApplicationNodeBase> appNodeImpl) { appNodeImpl_ = std::move(appNodeImpl); }

    private:
        /** The internal framework class. */
        FrameworkInternal& fwInternal_;
        /** Holds the application node implementation. */
        std::unique_ptr<ApplicationNodeBase> appNodeImpl_;

        /** Holds the current application time. */
        double currentTime_;
        /** Holds the time elapsed since the last frame. */
        double elapsedTime_;
        
        /** Fills membervariables with current data. */
        void ParseTrackingFrame();
        /** Polls and parses the next upcoming events. */
        void PollAndParseNextEvent();
        /** Polls and parses all upcoming events. */
        void PollAndParseEvents();

        void InitialiseDisplay(bool useLeftController);
        bool GetDisplayInitialised();
        bool GetDisplayInitByFloor();

        float * GetPosition(const float hmdMatrix[3][4]);
        double * GetRotation(const float matrix[3][4]);
        float * GetZVector(const float matrix[3][4]);
        const glm::vec2& GetDisplayPosVector(const glm::vec3& pos, const glm::vec3& zvector);
        void InitDisplay(glm::vec3 dpos);
        void InitDisplayFloor(glm::vec3 cpos, glm::vec3 cz);
        void InitDisplayFromFile();
        void WriteInitDisplayToFile();
        /** Processes a given vr event */
        bool ProcessVREvent(const vr::VREvent_t & event);
        /** If using SGCT this method passes the tracker data as head tracked device to SGCT */
        void HandleSCGT(glm::vec3 pos, glm::quat q);

        /** Holds the IVRSystem pointer */
        vr::IVRSystem *m_pHMD_;
        /** Represents if the OpenVR initialisation was succesful. */
        bool vrInitSucc_ = false;
        /** Holds the left hand controller positon. */
        glm::vec3 controller0pos_;
        /** Holds the left hand controller z-vector. */
        glm::vec3 controller0zvec_;
        /** Holds the right hand controller positon. */
        glm::vec3 controller1pos_;
        /** Holds the right hand controller z-vector. */
        glm::vec3 controller1zvec_;
        /** Holds the tracker positon. */
        glm::vec3 trackerpos_;
        /** Holds the tracker z-vector.*/
        glm::vec3 trackerzvec_;
        /** Holds the left hand controller rotation. */
        glm::quat controller0rot_;
        /** Holds the right hand controller rotation. */
        glm::quat controller1rot_;
        /** Holds the tracker rotation. */
        glm::quat trackerrot_;
        /** Holds the display positon, where the left hand controller is pointing at */
        glm::vec2 controller0displaypos_;
        /** Holds the display positon, where the right hand controller is pointing at */
        glm::vec2 controller1displaypos_;
        float midDisplayPos_[3] = { 0.0f,0.0f,0.0f };
        float displayEdges_[3][3] = { { -1.7f, -0.2f, -3.0f },{ -1.7f, 1.5f, -3.0f },{ 1.8f, -0.28f, -3.0f } };
        bool initDisplay_ = true;
        bool displayllset_ = false;
        bool displayulset_ = false;
        bool displaylrset_ = false;
        bool initfloor_ = true;
        bool useleftcontroller_ = true;
        bool calibrate_ = false;
        std::vector<std::string> controller0buttons_;
        std::vector<std::string> controller1buttons_;
        std::vector<DeviceInfo> connectedDevices_;
    };
}
