/**
 * @file   OpenVRController.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.02.21
 *
 * @brief  Implementation of the OpenVRController class.
 */

#include "OpenVRController.h"

#include <openvr.h>
#include <fstream>
#include <windows.h>

#include "DisplayPointerCalibrationController.h"

#ifdef VISCOM_USE_SGCT
#include <sgct_wrapper.h>
#endif

#include <iostream>

//#define MOCKING

namespace viscom::ovr {

#ifdef MOCKING
    static glm::vec2 value;
#endif

    OpenVRController::OpenVRController()
    {
        vr::EVRInitError peError;
        // VRApplication_Scene (starts SteamVR no proper data) VRApplication_Overlay (starts SteamVR no SteamVRHome)  VRApplication_Background (doesn't start SteamVR uses SteamVRHome)
        pHMD_ = vr::VR_Init(&peError, vr::EVRApplicationType::VRApplication_Background);
        if (peError != vr::VRInitError_None) {
            LOG(WARNING) << "Error while initializing OpenVR.";
            pHMD_ = nullptr;
        }
        LOG(INFO) << "OpenVR initialized.";
        controllerStates_.resize(vr::k_unMaxTrackedDeviceCount);
        controllerDisplayPositions_.resize(vr::k_unMaxTrackedDeviceCount);
    }

    OpenVRController::~OpenVRController()
    {
        vr::VR_Shutdown();
        pHMD_ = nullptr;
    }

    bool OpenVRController::InitialiseVR()
    {
        LOG(G3LOG_DEBUG) << "InitialiseVR called.";
        if (pHMD_ && InitCalibrationFromFile()) return true;
        return false;
    }

    bool OpenVRController::InitialiseDisplayVR()
    {
        if (!InitCalibrationFromFile()) {
            displayPlane_ = DisplayPlane{};
            return false;
        }
        return true;
    }

    bool OpenVRController::CalibrateVR(CalibrateMethod method)
    {
        LOG(G3LOG_DEBUG) << "CalibrateVR called.";
        calibration_ = std::make_unique<CalibrationController>(method);
        return true;
    }

    const std::vector<DeviceInfo>& OpenVRController::GetConnectedDevices() const
    {
        LOG(G3LOG_DEBUG) << "GetConnectedDevices called.";
        return connectedDevices_;
    }

    /**
     *  Tests which devices are connected and returns them in a string vector.
     *  @return string vector containing the connected devices.
     */
    std::vector<std::string> OpenVRController::OutputDevices() const {
        std::vector<std::string> devices;
        if (!pHMD_) return devices;

        for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
            if (!pHMD_->IsTrackedDeviceConnected(unDevice))
                continue;
            vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
            switch (trackedDeviceClass) {
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_Controller):
                devices.emplace_back("Controller");
                break;
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker):
                devices.emplace_back("GenericTracker");
                break;
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_HMD):
                devices.emplace_back("HMD");
                break;
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference):
                devices.emplace_back("TrackingRef");
                break;
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_DisplayRedirect):
                devices.emplace_back("DisplayRedirect");
                break;
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid):
                devices.emplace_back("Invalid");
                break;
            }
        }
        return devices;
    }

    const glm::vec3& OpenVRController::GetControllerPosition(std::uint32_t trackedDeviceId) const
    {
        LOG(G3LOG_DEBUG) << "GetControllerPos called.";
        return controllerStates_[trackedDeviceId].position_;
    }

    const glm::vec3& OpenVRController::GetControllerDirection(std::uint32_t trackedDeviceId) const
    {
        return controllerStates_[trackedDeviceId].direction_;
    }

    const glm::quat& OpenVRController::GetControllerOrientation(std::uint32_t trackedDeviceId) const
    {
        return controllerStates_[trackedDeviceId].orientation_;
    }

    const glm::vec2& OpenVRController::GetDisplayPointerPosition(std::uint32_t trackedDeviceId) const
    {
        LOG(G3LOG_DEBUG) << "GetDisplayPointerPosition called.";

#ifndef MOCKING
        return controllerDisplayPositions_[trackedDeviceId];
#endif

#ifdef MOCKING
        double time = 0.001 * GetTickCount();
        value = glm::vec2(0.5 * sin(0.3 * time) + 0.5, 0.5 * cos(time) + 0.5);
    
        return value;
#endif
    }

    void OpenVRController::GetControllerButtonState(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2& axisvalues, ButtonState& buttonstate) const
    {
        LOG(G3LOG_DEBUG) << "GetControllerButtonState called.";
        if (!pHMD_) return;

        vr::VRControllerState_t controllerState;
        vr::VRSystem()->GetControllerState(trackedDeviceId, &controllerState, sizeof(vr::VRControllerState_t));

        auto pressed = controllerState.ulButtonPressed;
        auto touched = controllerState.ulButtonTouched;

        if (buttonid == 32) {
            axisvalues.x = controllerState.rAxis[0].x;
            axisvalues.y = controllerState.rAxis[0].y;
        }
        else if (buttonid == 33) {
            axisvalues.x = controllerState.rAxis[1].x;
            axisvalues.y = controllerState.rAxis[1].y;
        }
        else {
            axisvalues.x = 0.0f;
            axisvalues.y = 0.0f;
        }

        buttonstate = ButtonState::RELEASED;
        if (((touched >> buttonid) & 1) == 1) buttonstate = ButtonState::TOUCHED;
        if (((pressed >> buttonid) & 1) == 1) buttonstate = ButtonState::PRESSED;
    }

    /**
     * Returns the display position as x and y.
     *   @param glm::vec3 position of the controller
     *   @param glm::vec3 z-vector of the used controller.
     *   @return glm::vec2 with x, y as display position.
     */
    glm::vec2 OpenVRController::GetDisplayPosition(const glm::vec3& position, const glm::vec3& direction) const
    {
        glm::mat3 m{ 0.0f };
        m[0] = direction;
        m[1] = displayPlane_.right_; // GetSimPlane().right_ - GetSimPlane().position_;
        m[2] = displayPlane_.up_; // GetSimPlane().up_ - GetSimPlane().position_;

        auto intersection = glm::inverse(m) * (position - displayPlane_.mouseOrigin_);
        return glm::vec2{ intersection.y, intersection.z };
        // return glm::vec2(0.5f) + glm::vec2(intersection.y, intersection.z) / 2.0f;

        // TODO: clean up. [2/21/2019 Sebastian Maisch]
        // float d1[3] = { displayEdges_[0][0], displayEdges_[0][1],displayEdges_[0][2] }; // position lower left
        // float d2[3] = { displayEdges_[1][0] - displayEdges_[0][0], displayEdges_[1][1] - displayEdges_[0][1], displayEdges_[1][2] - displayEdges_[0][2] }; // up vector
        // float d3[3] = { displayEdges_[2][0] - displayEdges_[0][0], displayEdges_[2][1] - displayEdges_[0][1], displayEdges_[2][2] - displayEdges_[0][2] }; // right vector
        // glm::vec2 result;
        // 
        // result[1] = (position[0] * zvector[1] * d3[0] * zvector[2] - position.x * zvector.y * d3[2] * zvector[0] - position[1] * zvector[0] * d3[0] * zvector[2] + position[1] * zvector[0] * d3[2] * zvector[0] - d1[0] * zvector[1] * d3[0] * zvector[2] + d1[0] * zvector[1] * d3[2] * zvector[0] + d1[1] * zvector[0] * d3[0] * zvector[2] - d1[1] * zvector[0] * d3[2] * zvector[0] - position[0] * zvector[2] * d3[0] * zvector[1] + position[0] * zvector[2] * d3[1] * zvector[0] + position[2] * zvector[0] * d3[0] * zvector[1] - position[2] * zvector[0] * d3[1] * zvector[0] + d1[0] * zvector[2] * d3[0] * zvector[1] - d1[0] * zvector[2] * d3[1] * zvector[0] - d1[2] * zvector[0] * d3[0] * zvector[1] + d1[2] * zvector[0] * d3[1] * zvector[0]) / (d2[0] * zvector[1] * d3[0] * zvector[2] - d2[0] * zvector[1] * d3[2] * zvector[0] - d2[1] * zvector[0] * d3[0] * zvector[2] + d2[1] * zvector[0] * d3[2] * zvector[0] - d2[0] * zvector[2] * d3[0] * zvector[1] + d2[0] * zvector[2] * d3[1] * zvector[0] + d2[2] * zvector[0] * d3[0] * zvector[1] - d2[2] * zvector[0] * d3[1] * zvector[0]);
        // result[0] = (position[0] * zvector[1] * d2[0] * zvector[2] - position.x * zvector.y * d2[2] * zvector[0] - position[1] * zvector[0] * d2[0] * zvector[2] + position[1] * zvector[0] * d2[2] * zvector[0] - d1[0] * zvector[1] * d2[0] * zvector[2] + d1[0] * zvector[1] * d2[2] * zvector[0] + d1[1] * zvector[0] * d2[0] * zvector[2] - d1[1] * zvector[0] * d2[2] * zvector[0] - position[0] * zvector[2] * d2[0] * zvector[1] + position[0] * zvector[2] * d2[1] * zvector[0] + position[2] * zvector[0] * d2[0] * zvector[1] - position[2] * zvector[0] * d2[1] * zvector[0] + d1[0] * zvector[2] * d2[0] * zvector[1] - d1[0] * zvector[2] * d2[1] * zvector[0] - d1[2] * zvector[0] * d2[0] * zvector[1] + d1[2] * zvector[0] * d2[1] * zvector[0]) / (d3[0] * zvector[1] * d2[0] * zvector[2] - d3[0] * zvector[1] * d2[2] * zvector[0] - d3[1] * zvector[0] * d2[0] * zvector[2] + d3[1] * zvector[0] * d2[2] * zvector[0] - d3[0] * zvector[2] * d2[0] * zvector[1] + d3[0] * zvector[2] * d2[1] * zvector[0] + d3[2] * zvector[0] * d2[0] * zvector[1] - d3[2] * zvector[0] * d2[1] * zvector[0]);
        // 
        // midDisplayPos_[0] = d1[0] + 0.5f * d2[0] + 0.5f * d3[0];
        // midDisplayPos_[1] = d1[1] + 0.5f * d2[1] + 0.5f * d3[1];
        // midDisplayPos_[2] = d1[2] + 0.5f * d2[2] + 0.5f * d3[2];
        // return result;
    }

    /** Parses a OpenVR Tracking Frame by going through all connected devices. */
    void OpenVRController::ParseTrackingFrame()
    {
        LOG(G3LOG_DEBUG) << "ParseTrackingFrame called.";
        if (!pHMD_ || !vr::VRSystem()->IsInputAvailable()) return;

        connectedDevices_.clear();

        // Process SteamVR device states
        for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
            // if not connected just skip the rest of the routine
            if (!pHMD_->IsTrackedDeviceConnected(unDevice)) continue;

            DeviceInfo deviceInfo{ unDevice, TrackedDeviceRole::INVALID, TrackedDeviceClass::INVALID };

            vr::TrackedDevicePose_t trackedDevicePose;
            vr::VRControllerState_t controllerState;

            vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
            switch (trackedDeviceClass)
            {
            case vr::TrackedDeviceClass_HMD:
                deviceInfo.deviceClass_ = TrackedDeviceClass::HMD;
                vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1);
                break;
            case vr::TrackedDeviceClass_Controller:
                deviceInfo.deviceClass_ = TrackedDeviceClass::CONTROLLER;
                vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState, sizeof(controllerState), &trackedDevicePose);
                deviceInfo.deviceRole_ = GetTrackedDeviceRoleFromTrackedControllerRole(vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice));
                break;
            case vr::TrackedDeviceClass_GenericTracker:
                deviceInfo.deviceClass_ = TrackedDeviceClass::GENERIC_TRACKER;
                deviceInfo.deviceRole_ = TrackedDeviceRole::GENERIC_TRACKER;
                vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState, sizeof(controllerState), &trackedDevicePose);
                break;
            case vr::TrackedDeviceClass_TrackingReference:
                deviceInfo.deviceClass_ = TrackedDeviceClass::TRACKING_REFERENCE;
                break;
            case vr::TrackedDeviceClass_DisplayRedirect:
                deviceInfo.deviceClass_ = TrackedDeviceClass::DISPLAY_REDIRECT;
                break;
            default:
                break;
            }

            connectedDevices_.emplace_back(deviceInfo);

            controllerStates_[unDevice].position_ = GetPositionFromHmdMatrix34(trackedDevicePose.mDeviceToAbsoluteTracking);
            controllerStates_[unDevice].direction_ = GetDirectionFromHmdMatrix34(trackedDevicePose.mDeviceToAbsoluteTracking);
            controllerStates_[unDevice].orientation_ = GetOrientationFromHmdMatrix34(trackedDevicePose.mDeviceToAbsoluteTracking);
            controllerDisplayPositions_[unDevice] = GetDisplayPosition(controllerStates_[unDevice].position_, controllerStates_[unDevice].direction_);

            if (deviceInfo.deviceClass_ == TrackedDeviceClass::CONTROLLER) {
                if (deviceInfo.deviceRole_ == TrackedDeviceRole::CONTROLLER_LEFT_HAND) leftControllerDeviceId_ = unDevice;
                else if (deviceInfo.deviceRole_ == TrackedDeviceRole::CONTROLLER_RIGHT_HAND) rightControllerDeviceId_ = unDevice;
            }

            if (deviceInfo.deviceClass_ == TrackedDeviceClass::GENERIC_TRACKER && trackedDevicePose.bPoseIsValid) {
#ifdef VISCOM_USE_SGCT
                HandleSCGT(controllerStates_[unDevice].position_, controllerStates_[unDevice].orientation_);
#endif // VISCOM_USE_SGCT
            }
        }
    }

    /** Polls and parses all vr events in queue */
    void OpenVRController::PollAndParseEvents()
    {
        vr::VREvent_t event;
        if (!pHMD_) return;
        while (pHMD_->PollNextEvent(&event, sizeof(event))) ProcessVREvent(event);
    }

    void OpenVRController::DisplayCalibrationGUI() const
    {
        if (calibration_) calibration_->DisplayCalibrationGUI();
    }

    bool OpenVRController::IsCalibrating() const
    {
        if (calibration_) return !calibration_->IsFinished();
        return false;
    }


    /** Processes a given vr event currently only handling controller buttons.
    *   @return true if the end of the procedure was reached.
    */
    bool OpenVRController::ProcessVREvent(const vr::VREvent_t& event)
    {
        LOG(INFO) << "ProcessVREvent called on Coordinator.";
        switch (event.eventType)
        {
        case vr::VREvent_ButtonPress:
        {
            //TODO: make uni-controller conform -->?????
            if (calibration_ && event.data.controller.button == vr::k_EButton_SteamVR_Trigger) {
                calibration_->DoNextCalibrationStep(event.trackedDeviceIndex);
                if (calibration_->IsFinished()) {
                    displayPlane_ = calibration_->GetDisplayPlane();
                    calibration_ = nullptr;
                    WriteCalibrationToFile();
                }
            } else ControllerButtonPressedCallback(event.trackedDeviceIndex, event.data.controller.button, glm::vec2(event.data.dualAnalog.x, event.data.dualAnalog.y));
        }
        break;

        case vr::VREvent_ButtonTouch:
            ControllerButtonTouchedCallback(event.trackedDeviceIndex, event.data.controller.button, glm::vec2(event.data.dualAnalog.x, event.data.dualAnalog.y));
            break;

        case vr::VREvent_ButtonUnpress:
            ControllerButtonPressReleasedCallback(event.trackedDeviceIndex, event.data.controller.button, glm::vec2(event.data.dualAnalog.x, event.data.dualAnalog.y));
            break;

        case vr::VREvent_ButtonUntouch:
            ControllerButtonTouchReleasedCallback(event.trackedDeviceIndex, event.data.controller.button, glm::vec2(event.data.dualAnalog.x, event.data.dualAnalog.y));
            break;

        case (vr::VREvent_Quit):
        case (vr::VREvent_ProcessQuit):
        case (vr::VREvent_QuitAborted_UserPrompt):
        case (vr::VREvent_QuitAcknowledged):
            return false;

        default:
            break;
        }

        return true;
    }

    /** Passes the tracker position and rotation to sgct for head tracking
    *   @param pos position of the tracker
    *   @param q tracker orientation
    */
    void OpenVRController::HandleSCGT(const glm::vec3& pos, const glm::quat& q) const
    {
#ifdef VISCOM_USE_SGCT
        sgct_wrapper::wVec3 position;
        position[0] = pos.x - displayPlane_.center_.x;
        position[1] = pos.y - displayPlane_.center_.y;
        position[2] = -pos.z + displayPlane_.center_.z;

        sgct_wrapper::wQuat orientation{ q.w, q.x, q.y, q.z };
        sgct_wrapper::SetDefaultUserPosition(position);
        sgct_wrapper::SetDefaultUserOrientation(orientation);
#endif // VISCOM_USE_SGCT
    }

    /** Reads displayEdges.txt and initializes the display with found values. */
    bool OpenVRController::InitCalibrationFromFile()
    {
        std::ifstream calibFile(displayCalibrationFilename);
        if (calibFile.is_open()) {
            calibFile >> displayPlane_.mouseOrigin_.x >> displayPlane_.mouseOrigin_.y >> displayPlane_.mouseOrigin_.z;
            calibFile >> displayPlane_.center_.x >> displayPlane_.center_.y >> displayPlane_.center_.z;
            calibFile >> displayPlane_.right_.x >> displayPlane_.right_.y >> displayPlane_.right_.z;
            calibFile >> displayPlane_.up_.x >> displayPlane_.up_.y >> displayPlane_.up_.z;
        }
        return true;
    }

    /** Writes the current display edges to displayEdges.txt */
    void OpenVRController::WriteCalibrationToFile()
    {
        std::ofstream calibFile(displayCalibrationFilename);
        calibFile << displayPlane_.mouseOrigin_.x << " " << displayPlane_.mouseOrigin_.y << " " << displayPlane_.mouseOrigin_.z;
        calibFile << displayPlane_.center_.x << " " << displayPlane_.center_.y << " " << displayPlane_.center_.z;
        calibFile << displayPlane_.right_.x << " " << displayPlane_.right_.y << " " << displayPlane_.right_.z;
        calibFile << displayPlane_.up_.x << " " << displayPlane_.up_.y << " " << displayPlane_.up_.z;
    }
}
