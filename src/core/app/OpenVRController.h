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
        const glm::vec2& GetLeftControllerDisplayPosition() const { return controllerDisplayPositions_[leftControllerDeviceId_]; }
        const glm::vec2& GetRightControllerDisplayPosition() const { return controllerDisplayPositions_[rightControllerDeviceId_]; }

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

        /** Holds the IVRSystem pointer */
        vr::IVRSystem* pHMD_ = nullptr;
        /** Holds the controller states. */
        std::vector<ControllerState> controllerStates_;
        /** Holds the display position, where the controllers are pointing at. */
        std::vector<glm::vec2> controllerDisplayPositions_;
        /** The left controllers device id. */
        std::uint32_t leftControllerDeviceId_ = 0;
        /** The right controllers device id. */
        std::uint32_t rightControllerDeviceId_ = 0;

        DisplayPlane displayPlane_;

        std::vector<DeviceInfo> connectedDevices_;
    };
}
