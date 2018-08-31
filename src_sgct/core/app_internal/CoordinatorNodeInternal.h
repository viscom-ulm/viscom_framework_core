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
        
        virtual bool InitialiseVR() override;
        virtual bool CalibrateVR(CalibrateMethod method, TrackedDeviceIdentifier trackedDevice) override;
        virtual glm::vec3 GetControllerPosition(TrackedDeviceIdentifier trackedDevice) override;
        virtual glm::vec3 GetControllerZVector(TrackedDeviceIdentifier trackedDevice) override;
        virtual glm::quat GetControllerRotation(TrackedDeviceIdentifier trackedDevice) override;
        virtual glm::vec2 GetDisplayPosition(TrackedDeviceIdentifier trackedDevice) override;
        virtual void SetDisplayInitByFloor(bool b) override;

        virtual void ControllerButtonPressedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, float posx, float posy, glm::vec3 position, glm::vec3 zvector, glm::quat rotation) override;
        virtual void ControllerButtonTouchedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, float posx, float posy, glm::vec3 position, glm::vec3 zvector, glm::quat rotation) override;
        virtual void ControllerButtonUnpressedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, float posx, float posy, glm::vec3 position, glm::vec3 zvector, glm::quat rotation) override;
        virtual void ControllerButtonUntouchedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, float posx, float posy, glm::vec3 position, glm::vec3 zvector, glm::quat rotation) override;

        virtual void ParseTrackingFrame() override;
        virtual glm::vec3 GetController0Pos() override;
        virtual glm::vec3 GetController0Zvec() override;
        virtual glm::vec3 GetController1Pos() override;
        virtual glm::vec3 GetController1Zvec() override;
        virtual glm::vec3 GetTrackerPos() override;
        virtual glm::vec3 GetTrackerZvec() override;
        virtual glm::quat GetController0Rot() override;
        virtual glm::quat GetController1Rot() override;
        virtual glm::quat GetTrackerRot() override;
        virtual glm::vec2 GetDisplayPosition(bool useleftcontroller) override;
        virtual void InitialiseDisplay(bool useLeftController) override;
        virtual bool GetDisplayInitialised() override;
        virtual void SetDisplayNotInitialised() override;
        virtual bool GetDisplayInitByFloor() override;
        
        virtual void PollAndParseNextEvent() override;
        virtual void PollAndParseEvents() override;        
        //virtual float* GetDisplayEdges() override;
        //virtual bool GetVrInitSuccess() override;
        virtual std::vector<std::string> OutputDevices() override;
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

        float * GetPosition(const float hmdMatrix[3][4]);
        double * GetRotation(const float matrix[3][4]);
        float * GetZVector(const float matrix[3][4]);
        glm::vec2 GetDisplayPosVector(glm::vec3 pos, glm::vec3 zvector, const float display_lowerLeftCorner[3], const float display_upperLeftCorner[3], const float display_lowerRightCorner[3]);
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
        std::vector<std::string> controller0buttons;
        std::vector<std::string> controller1buttons;


    };
}
