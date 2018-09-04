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
        
        bool InitialiseVR() override;
        bool CalibrateVR(CalibrateMethod method, TrackedDeviceIdentifier trackedDevice) override;
        const glm::vec3& GetControllerPosition(TrackedDeviceIdentifier trackedDevice) override;
        const glm::vec3& GetControllerZVector(TrackedDeviceIdentifier trackedDevice) override;
        const glm::quat& GetControllerRotation(TrackedDeviceIdentifier trackedDevice) override;
        const glm::vec2& GetDisplayPosition(TrackedDeviceIdentifier trackedDevice) override;        

        void ControllerButtonPressedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, glm::vec2 axisvalues) override;
        void ControllerButtonTouchedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, glm::vec2 axisvalues) override;
        void ControllerButtonUnpressedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, glm::vec2 axisvalues) override;
        void ControllerButtonUntouchedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, glm::vec2 axisvalues) override;
        
        ControllerButtonState GetControllerButtonState(TrackedDeviceIdentifier trackedDevice);

              
        //virtual float* GetDisplayEdges() override;
        //virtual bool GetVrInitSuccess() override;
        virtual std::vector<std::string> OutputDevices();
        //virtual std::vector<std::string> GetController0Buttons() override;
        //virtual std::vector<std::string> GetController1Buttons() override;

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

        void ParseTrackingFrame();
        void PollAndParseNextEvent();
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

        vr::IVRSystem *m_pHMD;
        bool vrInitSucc = false;
        glm::vec3 controller0pos;
        glm::vec3 controller0zvec;
        glm::vec3 controller1pos;
        glm::vec3 controller1zvec;
        glm::vec3 trackerpos;
        glm::vec3 trackerzvec;
        glm::quat controller0rot;
        glm::quat controller1rot;
        glm::quat trackerrot;
        float midDisplayPos[3] = { 0.0f,0.0f,0.0f };
        float displayEdges[3][3] = { { -1.7f, -0.2f, -3.0f },{ -1.7f, 1.5f, -3.0f },{ 1.8f, -0.28f, -3.0f } };
        bool initDisplay = true;
        bool displayllset = false;
        bool displayulset = false;
        bool displaylrset = false;
        bool initfloor = true;
        bool useleftcontroller;
        bool calibrate = false;
        std::vector<std::string> controller0buttons;
        std::vector<std::string> controller1buttons;


    };
}
