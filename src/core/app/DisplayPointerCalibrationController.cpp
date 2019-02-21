/**
 * @file   DisplayPointerCalibrationController.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.02.21
 *
 * @brief  Calibration class for the display pointer.
 */

#include "DisplayPointerCalibrationController.h"

#include <openvr.h>
#include <imgui.h>

namespace viscom::ovr {

    void viscom::ovr::CalibrationController::DoNextCalibrationStep()
    {

        for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
            // if not connected just skip the rest of the routine
            if (!vr::VRSystem()->IsTrackedDeviceConnected(unDevice)) continue;

            vr::TrackedDevicePose_t trackedDevicePose;
            vr::VRControllerState_t controllerState;

            vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
            if (trackedDeviceClass != vr::TrackedDeviceClass_Controller) continue;

            auto controllerRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice);
            if (controllerRole != vr::TrackedControllerRole_LeftHand) continue;

            vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState, sizeof(controllerState), &trackedDevicePose);
            auto position = GetPositionFromHmdMatrix34(trackedDevicePose.mDeviceToAbsoluteTracking);
            auto direction = GetDirectionFromHmdMatrix34(trackedDevicePose.mDeviceToAbsoluteTracking);

            if (calibrationMethod_ == CalibrateMethod::CALIBRATE_BY_TOUCHING) {
                calibratedPositions_.emplace_back(position);
            }
            else {
                if (calibratedPositions_.size() < 2) {
                    float t = (-position.y) / direction.y;
                    calibratedPositions_.emplace_back(position + t * direction);
                }
                else {
                    glm::mat3 m{ 0.0f };
                    auto right = calibratedPositions_[1] - calibratedPositions_[0];
                    auto up = glm::vec3{ 0.0f, 1.0f, 0.0f };
                    m[0] = direction;
                    m[1] = right;
                    m[2] = up;

                    auto intersection = glm::inverse(m) * (position - calibratedPositions_[0]);

                    calibratedPositions_.emplace_back(calibratedPositions_[0] + intersection.y * right + intersection.z * up);
                }
            }

            return;
        }
    }

    void CalibrationController::DisplayCalibrationGUI() const
    {
        ImGui::ShowTestWindow();
        ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiSetCond_FirstUseEver);
        ImGui::StyleColorsClassic();
        if (ImGui::Begin("Calibration Window", nullptr))
        {
            ImGui::Text("Open VR Display Configuration:");
            if (calibrationMethod_ == CalibrateMethod::CALIBRATE_BY_POINTING) {
                if (calibratedPositions_.empty()) ImGui::Text("Point at the lower left corner and press the trigger.");
                if (calibratedPositions_.size() == 1) ImGui::Text("Point at the lower right corner and press the trigger.");
                if (calibratedPositions_.size() == 2) ImGui::Text("Point at the upper left corner and press the trigger.");
            } else  {
                if (calibratedPositions_.empty()) ImGui::Text("Touch the lower left corner and press the trigger.");
                if (calibratedPositions_.size() == 1) ImGui::Text("Touch the upper left corner and press the trigger.");
                if (calibratedPositions_.size() == 2) ImGui::Text("Touch the lower right corner and press the trigger.");
            }
        }
        ImGui::End();
    }

    DisplayPlane CalibrationController::GetDisplayPlane() const
    {
        if (calibratedPositions_.size() < 3) return DisplayPlane{};

        glm::vec3 up = calibratedPositions_[2] - calibratedPositions_[0];
        glm::vec3 right = calibratedPositions_[1] - calibratedPositions_[0];
        DisplayPlane result;
        result.mouseOrigin_ = calibratedPositions_[0];
        result.center_ = calibratedPositions_[0] + 0.5f * up + 0.5f * right;
        result.up_ = up;
        result.right_ = right;
        return result;
    }

}
