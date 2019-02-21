/**
 * @file   OpenVRController.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.02.21
 *
 * @brief  Controller for all OpenVR related input.
 */

#pragma once

#include "OpenVRControllerIfc.h"

namespace viscom::ovr {

    class CalibrationController;

    class OpenVRController : public OpenVRControllerIfc
    {
    public:
        OpenVRController();
        ~OpenVRController();


        virtual bool InitialiseVR() override;
        virtual bool InitialiseDisplayVR() override;
        virtual bool CalibrateVR(CalibrateMethod method) override;
        virtual const std::vector<DeviceInfo>& GetConnectedDevices() const override;
        virtual const glm::vec3& GetControllerPosition(std::uint32_t trackedDeviceId) const override;
        virtual const glm::vec3& GetControllerDirection(std::uint32_t trackedDeviceId) const override;
        virtual const glm::quat& GetControllerOrientation(std::uint32_t trackedDeviceId) const override;
        virtual const glm::vec2& GetDisplayPointerPosition(std::uint32_t trackedDeviceId) const override;

        virtual void GetControllerButtonState(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2& axisvalues, ButtonState& buttonstate) const override;

        virtual std::vector<std::string> OutputDevices() const override;

    protected:
        /** Fills member variables with current data. */
        void ParseTrackingFrame();
        /** Polls and parses all upcoming events. */
        void PollAndParseEvents();

        void DisplayCalibrationGUI() const;
        bool IsCalibrating() const;

    private:
        bool InitCalibrationFromFile();
        void WriteCalibrationToFile();
        glm::vec2 GetDisplayPosition(const glm::vec3& position, const glm::vec3& direction) const;
        /** If using SGCT this method passes the tracker data as head tracked device to SGCT */
        void HandleSCGT(const glm::vec3& pos, const glm::quat& q) const;
        /** Processes a given vr event */
        bool ProcessVREvent(const vr::VREvent_t& event);

        std::unique_ptr<CalibrationController> calibration_;
        /** Polls and parses the next upcoming events. */
        // void PollAndParseNextEvent();

        
        // bool GetDisplayInitialised();
        // bool GetDisplayInitByFloor();


        /** Holds the IVRSystem pointer */
        vr::IVRSystem* pHMD_;
        /** Holds the controller states. */
        std::vector<ControllerState> controllerStates_;
        /** Holds the display position, where the controllers are pointing at. */
        std::vector<glm::vec2> controllerDisplayPositions_;

        DisplayPlane displayPlane_;

        // float midDisplayPos_[3] = { 0.0f,0.0f,0.0f };
        // float displayEdges_[3][3] = { { -1.7f, -0.2f, -3.0f },{ -1.7f, 1.5f, -3.0f },{ 1.8f, -0.28f, -3.0f } };
        // bool initDisplay_ = true;
        // bool displayllset_ = false;
        // bool displayulset_ = false;
        // bool displaylrset_ = false;
        // bool initfloor_ = true;
        // CalibrateMethod calibrationMethod_ = CalibrateMethod::CALIBRATE_BY_POINTING;
        // bool useleftcontroller_ = true;
        // bool calibrate_ = false;
        // std::vector<std::string> controller0buttons_;
        // std::vector<std::string> controller1buttons_;
        std::vector<DeviceInfo> connectedDevices_;
    };
}
