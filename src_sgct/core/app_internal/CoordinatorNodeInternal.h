/**
 * @file   CoordinatorNodeInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.06.15
 *
 * @brief  Declaration of the internal coordinator node.
 */

#pragma once

#include "core/app_internal/ApplicationNodeInternal.h"
//#include <openvr.h>

namespace vr {
    class IVRSystem;
    struct VREvent_t;

}
namespace viscom {
    
    class CoordinatorNodeInternal : public ApplicationNodeInternal
    {
    public:
        CoordinatorNodeInternal(FrameworkInternal& fwInternal);
        virtual ~CoordinatorNodeInternal() override;

        virtual void PreWindow() override;
        virtual void InitOpenGL() override;
        virtual void PreSync() override;
        virtual void Draw2D(FrameBuffer& fbo) override;
        virtual void CleanUp() override;

        virtual void KeyboardCallback(int key, int scancode, int action, int mods) override;
        virtual void CharCallback(unsigned int character, int mods) override;
        virtual void MouseButtonCallback(int button, int action) override;
        virtual void MousePosCallback(double x, double y) override;
        virtual void MouseScrollCallback(double xoffset, double yoffset) override;

        virtual void AddTuioCursor(TUIO::TuioCursor* tcur) override;
        virtual void UpdateTuioCursor(TUIO::TuioCursor* tcur) override;
        virtual void RemoveTuioCursor(TUIO::TuioCursor* tcur) override;
        
        /** Initialises OpenVR for controller usage. */
        bool InitialiseVR() override;
        /** Calibrates the display edges by selected method. */
        bool CalibrateVR(CalibrateMethod method) override;
        /** Returns a DeviceInfo vector with the connected devices. */
        const std::vector<DeviceInfo>& GetConnectedDevices() override;
        /** Returns the position to a given tracked device id. */
        const glm::vec3& GetControllerPosition(size_t trackedDeviceId) override;
        /** Returns the z-vector to a given tracked device id. */
        const glm::vec3& GetControllerZVector(size_t trackedDeviceId) override;
        /** Returns the rotation to a given tracked device id. */
        const glm::quat& GetControllerRotation(size_t trackedDeviceId) override;
        /** Returns the display pointing position for a given tracked device id. */
        const glm::vec2& GetDisplayPosition(size_t trackedDeviceId) override;

        void ControllerButtonPressedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues) override;
        void ControllerButtonTouchedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues) override;
        void ControllerButtonUnpressedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues) override;
        void ControllerButtonUntouchedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues) override;
        
        /** Fills buttonstate and axisvalues for a given tracked device and buttonid. */
        void GetControllerButtonState(size_t trackedDeviceId, size_t buttonid, glm::vec2& axisvalues, ButtonState& buttonstate);

 
        virtual std::vector<std::string> OutputDevices();


    private:
#ifdef VISCOM_SYNCINPUT
        /** Holds the vector with keyboard events. */
        std::vector<KeyboardEvent> keyboardEvents_;
        /** Holds the vector with character events. */
        std::vector<CharEvent> charEvents_;
        /** Holds the vector with mouse button events. */
        std::vector<MouseButtonEvent> mouseButtonEvents_;
        /** Holds the vector with mouse position events. */
        std::vector<MousePosEvent> mousePosEvents_;
        /** Holds the vector with mouse scroll events. */
        std::vector<MouseScrollEvent> mouseScrollEvents_;
#endif
        /** Fills membervariables with current data. */
        void ParseTrackingFrame();
        /** Polls and parses the next upcoming events. */
        void PollAndParseNextEvent();
        /** Polls and parses all upcoming events. */
        void PollAndParseEvents();

        void InitialiseDisplay(bool useLeftController);
        bool GetDisplayInitialised();
        void SetDisplayNotInitialised();
        bool GetDisplayInitByFloor();

        float * GetPosition(const float hmdMatrix[3][4]);
        double * GetRotation(const float matrix[3][4]);
        float * GetZVector(const float matrix[3][4]);
        const glm::vec2& GetDisplayPosVector(const glm::vec3& pos,const glm::vec3& zvector);
        void InitDisplay(glm::vec3 dpos);
        void InitDisplayFloor(glm::vec3 cpos, glm::vec3 cz);
        void InitDisplayFromFile();
        void WriteInitDisplayToFile();
        bool ProcessVREvent(const vr::VREvent_t & event);
        void HandleSCGT(glm::vec3 pos, glm::quat q);

        vr::IVRSystem *m_pHMD_;
        bool vrInitSucc_ = false;
        glm::vec3 controller0pos_;
        glm::vec3 controller0zvec_;
        glm::vec3 controller1pos_;
        glm::vec3 controller1zvec_;
        glm::vec3 trackerpos_;
        glm::vec3 trackerzvec_;
        glm::quat controller0rot_;
        glm::quat controller1rot_;
        glm::quat trackerrot_;
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
