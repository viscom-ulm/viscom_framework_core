/**
 * @file   CoordinatorNodeInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.06.15
 *
 * @brief  Implementation of the internal coordinator node.
 */

#include "core/main.h"
#include <sgct.h>
#include "CoordinatorNodeInternal.h"
#include "core/imgui/imgui_impl_glfw.h"
#include "core/imgui/imgui_impl_opengl3.h"
#include <imgui.h>
#include "core/app/ApplicationNodeBase.h"
#include <iostream>
#include <fstream>

namespace viscom {

    CoordinatorNodeInternal::CoordinatorNodeInternal(FrameworkInternal& fwInternal) :
        ApplicationNodeInternal{ fwInternal }
    {
        vr::EVRInitError peError;
        // VRApplication_Scene (starts SteamVR no proper data) VRApplication_Overlay (starts SteamVR no SteamVRHome)  VRApplication_Background (doesn't start SteamVR uses SteamVRHome)
        m_pHMD = vr::VR_Init(&peError, vr::EVRApplicationType::VRApplication_Background);
        if (peError == vr::VRInitError_None) {
            vrInitSucc = true;
            //TODO fix output to imgui 
            //OutputDevices();
        }
    }

    //CoordinatorNodeInternal::~CoordinatorNodeInternal() = default;
    CoordinatorNodeInternal::~CoordinatorNodeInternal()
    {
        vr::VR_Shutdown();
    }

    void CoordinatorNodeInternal::PreWindow()
    {
        SetApplicationNode(GetFramework().GetCoordinatorNodeFactory()(this));
        ApplicationNodeInternal::PreWindow();
    }

    void CoordinatorNodeInternal::InitOpenGL()
    {
        // Setup ImGui binding
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui_ImplGlfw_InitForOpenGL(GetFramework().GetEngine()->getCurrentWindowPtr()->getWindowHandle(), !GetFramework().GetEngine()->isMaster() && SHOW_CLIENT_MOUSE_CURSOR);
        ImGui_ImplOpenGL3_Init();

        ImGui::StyleColorsDark();

        ApplicationNodeInternal::InitOpenGL();
    }

    void CoordinatorNodeInternal::PreSync()
    {
#ifdef VISCOM_SYNCINPUT
        {
            std::vector<KeyboardEvent> keybEvts;
            keybEvts.swap(keyboardEvents_);
            keyboardEventsSynced_.setVal(keybEvts);
        }

        {
            std::vector<CharEvent> charEvts;
            charEvts.swap(charEvents_);
            charEventsSynced_.setVal(charEvts);
        }

        {
            std::vector<MouseButtonEvent> mBtnEvts;
            mBtnEvts.swap(mouseButtonEvents_);
            mouseButtonEventsSynced_.setVal(mBtnEvts);
        }

        {
            std::vector<MousePosEvent> mPosEvts;
            mPosEvts.swap(mousePosEvents_);
            mousePosEventsSynced_.setVal(mPosEvts);
        }

        {
            std::vector<MouseScrollEvent> mScrlEvts;
            mScrlEvts.swap(mouseScrollEvents_);
            mouseScrollEventsSynced_.setVal(mScrlEvts);
        }
#endif
        syncInfoLocal_.currentTime_ = sgct::Engine::getTime();
        syncInfoLocal_.cameraPosition_ = GetFramework().GetCamera()->GetPosition();
        syncInfoLocal_.cameraOrientation_ = GetFramework().GetCamera()->GetOrientation();

        glm::vec2 relProjectorPos = glm::vec2(GetFramework().GetViewportScreen(0).position_) / glm::vec2(GetFramework().GetViewportScreen(0).size_);
        glm::vec2 relQuadSize = glm::vec2(GetFramework().GetViewportQuadSize(0)) / glm::vec2(GetFramework().GetViewportScreen(0).size_);
        glm::vec2 relProjectorSize = 1.0f / relQuadSize;

        syncInfoLocal_.pickMatrix_ = glm::mat4{ 1.0f };
        syncInfoLocal_.pickMatrix_[0][0] = 2.0f * relProjectorSize.x;
        syncInfoLocal_.pickMatrix_[1][1] = -2.0f * relProjectorSize.y;
        syncInfoLocal_.pickMatrix_[3][0] = (-2.0f * relProjectorPos.x * relProjectorSize.x) - 1.0f;
        syncInfoLocal_.pickMatrix_[3][1] = (-2.0f * relProjectorPos.y * relProjectorSize.y) + 1.0f;
        syncInfoLocal_.pickMatrix_[3][2] = 1.0f;
        syncInfoLocal_.pickMatrix_[3][3] = 1.0f;
        syncInfoLocal_.pickMatrix_ = glm::inverse(GetFramework().GetCamera()->GetCentralViewPerspectiveMatrix()) * syncInfoLocal_.pickMatrix_;

        syncInfoSynced_.setVal(syncInfoLocal_);

        ApplicationNodeInternal::PreSync();
    }

    void CoordinatorNodeInternal::Draw2D(FrameBuffer& fbo)
    {
        auto windowId = GetFramework().GetEngine()->getCurrentWindowPtr()->getId();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame(-GetFramework().GetViewportScreen(windowId).position_,
            GetFramework().GetViewportQuadSize(windowId), GetFramework().GetViewportScreen(windowId).size_,
            GetFramework().GetViewportScaling(windowId), GetCurrentAppTime(), GetElapsedTime());
        ImGui::NewFrame();

        ApplicationNodeInternal::Draw2D(fbo);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void CoordinatorNodeInternal::CleanUp()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        ApplicationNodeInternal::CleanUp();
    }

    void CoordinatorNodeInternal::KeyboardCallback(int key, int scancode, int action, int mods)
    {
#ifdef VISCOM_SYNCINPUT
        keyboardEvents_.emplace_back(key, scancode, action, mods);
#endif

        ImGui_ImplGlfw_KeyCallback(key, scancode, action, mods);
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        ApplicationNodeInternal::KeyboardCallback(key, scancode, action, mods);
    }

    void CoordinatorNodeInternal::CharCallback(unsigned int character, int mods)
    {
#ifdef VISCOM_SYNCINPUT
        charEvents_.emplace_back(character, mods);
#endif

        ImGui_ImplGlfw_CharCallback(character);
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        ApplicationNodeInternal::CharCallback(character, mods);
    }

    void CoordinatorNodeInternal::MouseButtonCallback(int button, int action)
    {
#ifdef VISCOM_SYNCINPUT
        mouseButtonEvents_.emplace_back(button, action);
#endif

        ImGui_ImplGlfw_MouseButtonCallback(button, action, 0);
        if (ImGui::GetIO().WantCaptureMouse) return;

        ApplicationNodeInternal::MouseButtonCallback(button, action);
    }

    void CoordinatorNodeInternal::MousePosCallback(double x, double y)
    {
#ifdef VISCOM_SYNCINPUT
        mousePosEvents_.emplace_back(x, y);
#endif

        ImGui_ImplGlfw_MousePositionCallback(x, y);
        if (ImGui::GetIO().WantCaptureMouse) return;

        ApplicationNodeInternal::MousePosCallback(x, y);
    }

    void CoordinatorNodeInternal::MouseScrollCallback(double xoffset, double yoffset)
    {
#ifdef VISCOM_SYNCINPUT
        mouseScrollEvents_.emplace_back(xoffset, yoffset);
#endif

        ImGui_ImplGlfw_ScrollCallback(xoffset, yoffset);
        if (ImGui::GetIO().WantCaptureMouse) return;

        ApplicationNodeInternal::MouseScrollCallback(xoffset, yoffset);
    }

    void CoordinatorNodeInternal::AddTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (!GetFramework().IsInitialized()) return;
#ifdef VISCOM_USE_TUIO
        // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
        ApplicationNodeInternal::AddTuioCursor(tcur);
#endif
    }

    void CoordinatorNodeInternal::UpdateTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (!GetFramework().IsInitialized()) return;
#ifdef VISCOM_USE_TUIO
        // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
        ApplicationNodeInternal::UpdateTuioCursor(tcur);
#endif
    }

    void CoordinatorNodeInternal::RemoveTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (!GetFramework().IsInitialized()) return;
#ifdef VISCOM_USE_TUIO
        // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
        ApplicationNodeInternal::RemoveTuioCursor(tcur);
#endif
    }
    void CoordinatorNodeInternal::ParseTrackingFrame()
    {
        // Process SteamVR device states
        for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
            if (m_pHMD == NULL)
                continue;
            // if not connected just skip the rest of the routine
            if (!m_pHMD->IsTrackedDeviceConnected(unDevice))
                continue;

            vr::TrackedDevicePose_t trackedDevicePose;
            vr::TrackedDevicePose_t *devicePose = &trackedDevicePose;

            vr::VRControllerState_t controllerState;
            vr::VRControllerState_t *controllerState_ptr = &controllerState;

            vr::HmdQuaternion_t quaternion;

            if (!vr::VRSystem()->IsInputAvailable()) {
                //TODO Error Message no Input available
            }

            bool bPoseValid = trackedDevicePose.bPoseIsValid;
            vr::HmdVector3_t vVel;
            vr::HmdVector3_t vAngVel;
            vr::ETrackingResult eTrackingResult;

            // Get what type of device it is and work with its data
            vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
            switch (trackedDeviceClass) {
            case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
                // print stuff for the HMD here, see controller stuff in next case block

                // get pose relative to the safe bounds defined by the user (only if using TrackingUniverseStanding)
                vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1);

                // get the position and rotation
                position.v[0] = GetPosition(devicePose->mDeviceToAbsoluteTracking.m)[0];
                position.v[1] = GetPosition(devicePose->mDeviceToAbsoluteTracking.m)[1];
                position.v[2] = GetPosition(devicePose->mDeviceToAbsoluteTracking.m)[2];
                zvector.v[0] = GetZVector(devicePose->mDeviceToAbsoluteTracking.m)[0];
                zvector.v[1] = GetZVector(devicePose->mDeviceToAbsoluteTracking.m)[1];
                zvector.v[2] = GetZVector(devicePose->mDeviceToAbsoluteTracking.m)[2];
                quaternion.w = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[0];
                quaternion.x = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[1];
                quaternion.y = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[2];
                quaternion.z = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[3];
                

                    // get some data
                    vVel = trackedDevicePose.vVelocity;
                vAngVel = trackedDevicePose.vAngularVelocity;
                eTrackingResult = trackedDevicePose.eTrackingResult;
                bPoseValid = trackedDevicePose.bPoseIsValid;

                //TODO print the tracking data


                // and print some more info to the user about the state of the device/pose
                switch (eTrackingResult) {
                case vr::ETrackingResult::TrackingResult_Uninitialized:
                    //TODO Handle
                    break;
                case vr::ETrackingResult::TrackingResult_Calibrating_InProgress:
                    //TODO Handle
                    break;
                case vr::ETrackingResult::TrackingResult_Calibrating_OutOfRange:
                    //TODO Handle
                    break;
                case vr::ETrackingResult::TrackingResult_Running_OK:
                    //TODO Handle
                    break;
                case vr::ETrackingResult::TrackingResult_Running_OutOfRange:
                    //TODO Handle

                    break;
                default:
                    //TODO Handle
                    break;
                }
                //TODO check if pose is Valid
            case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
                // Simliar to the HMD case block above, please adapt as you like
                // to get away with code duplication and general confusion

                vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState, sizeof(controllerState), &trackedDevicePose);

                position.v[0] = GetPosition(devicePose->mDeviceToAbsoluteTracking.m)[0];
                position.v[1] = GetPosition(devicePose->mDeviceToAbsoluteTracking.m)[1];
                position.v[2] = GetPosition(devicePose->mDeviceToAbsoluteTracking.m)[2];
                zvector.v[0] = GetZVector(devicePose->mDeviceToAbsoluteTracking.m)[0];
                zvector.v[1] = GetZVector(devicePose->mDeviceToAbsoluteTracking.m)[1];
                zvector.v[2] = GetZVector(devicePose->mDeviceToAbsoluteTracking.m)[2];
                quaternion.w = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[0];
                quaternion.x = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[1];
                quaternion.y = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[2];
                quaternion.z = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[3];

                vVel = trackedDevicePose.vVelocity;
                vAngVel = trackedDevicePose.vAngularVelocity;
                eTrackingResult = trackedDevicePose.eTrackingResult;
                bPoseValid = trackedDevicePose.bPoseIsValid;
                switch (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice)) {
                case vr::TrackedControllerRole_Invalid:
                    // invalid hand...
                    break;


                case vr::TrackedControllerRole_LeftHand:

                    /*char buf[1024];

                    sprintf_s(buf, sizeof(buf), "\nLeft Controller\nx: %.2f y: %.2f z: %.2f\n", position.v[0], position.v[1], position.v[2]);
                    printf_s(buf);

                    sprintf_s(buf, sizeof(buf), "qw: %.2f qx: %.2f qy: %.2f qz: %.2f\n", quaternion.w, quaternion.x, quaternion.y, quaternion.z);
                    printf_s(buf);

                    switch (eTrackingResult) {
                    case vr::ETrackingResult::TrackingResult_Uninitialized:
                    sprintf_s(buf, sizeof(buf), "Invalid tracking result\n");
                    printf_s(buf);
                    break;
                    case vr::ETrackingResult::TrackingResult_Calibrating_InProgress:
                    sprintf_s(buf, sizeof(buf), "Calibrating in progress\n");
                    printf_s(buf);
                    break;
                    case vr::ETrackingResult::TrackingResult_Calibrating_OutOfRange:
                    sprintf_s(buf, sizeof(buf), "Calibrating Out of range\n");
                    printf_s(buf);
                    break;
                    case vr::ETrackingResult::TrackingResult_Running_OK:
                    sprintf_s(buf, sizeof(buf), "Running OK\n");
                    printf_s(buf);
                    break;
                    case vr::ETrackingResult::TrackingResult_Running_OutOfRange:
                    sprintf_s(buf, sizeof(buf), "WARNING: Running Out of Range\n");
                    printf_s(buf);

                    break;
                    default:
                    sprintf_s(buf, sizeof(buf), "Default\n");
                    printf_s(buf);
                    break;
                    }

                    if (bPoseValid)
                    sprintf_s(buf, sizeof(buf), "Valid pose\n");
                    else
                    sprintf_s(buf, sizeof(buf), "Invalid pose\n");
                    printf_s(buf);
                    */
                    break;

                case vr::TrackedControllerRole_RightHand:
                    char buf[1024];
                    /*
                    sprintf_s(buf, sizeof(buf), "\nRight Controller\nx: %.2f y: %.2f z: %.2f\n", position.v[0], position.v[1], position.v[2]);
                    printf_s(buf);

                    sprintf_s(buf, sizeof(buf), "z Vector: x: %.2f y: %.2f z: %.2f\n", zvector.v[0], zvector.v[1], zvector.v[2]);
                    printf_s(buf);

                    sprintf_s(buf, sizeof(buf), "qw: %.2f qx: %.2f qy: %.2f qz: %.2f\n", quaternion.w, quaternion.x, quaternion.y, quaternion.z);
                    printf_s(buf);

                    switch (eTrackingResult) {
                    case vr::ETrackingResult::TrackingResult_Uninitialized:
                    sprintf_s(buf, sizeof(buf), "Invalid tracking result\n");
                    printf_s(buf);
                    break;
                    case vr::ETrackingResult::TrackingResult_Calibrating_InProgress:
                    sprintf_s(buf, sizeof(buf), "Calibrating in progress\n");
                    printf_s(buf);
                    break;
                    case vr::ETrackingResult::TrackingResult_Calibrating_OutOfRange:
                    sprintf_s(buf, sizeof(buf), "Calibrating Out of range\n");
                    printf_s(buf);
                    break;
                    case vr::ETrackingResult::TrackingResult_Running_OK:
                    sprintf_s(buf, sizeof(buf), "Running OK\n");
                    printf_s(buf);
                    break;
                    case vr::ETrackingResult::TrackingResult_Running_OutOfRange:
                    sprintf_s(buf, sizeof(buf), "WARNING: Running Out of Range\n");
                    printf_s(buf);

                    break;
                    default:
                    sprintf_s(buf, sizeof(buf), "Default\n");
                    printf_s(buf);
                    break;
                    }

                    if (bPoseValid)
                    sprintf_s(buf, sizeof(buf), "Valid pose\n");
                    else
                    sprintf_s(buf, sizeof(buf), "Invalid pose\n");
                    printf_s(buf);
                    */
                    break;
                case vr::TrackedDeviceClass_TrackingReference:
                    // incomplete code, only here for switch reference
                    sprintf_s(buf, sizeof(buf), "Camera / Base Station");
                    printf_s(buf);
                    break;
                }

            case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
                vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState, sizeof(controllerState), &trackedDevicePose);

                trackerPos.v[0] = GetPosition(devicePose->mDeviceToAbsoluteTracking.m)[0];
                trackerPos.v[1] = GetPosition(devicePose->mDeviceToAbsoluteTracking.m)[1];
                trackerPos.v[2] = GetPosition(devicePose->mDeviceToAbsoluteTracking.m)[2];
                zvector.v[0] = GetZVector(devicePose->mDeviceToAbsoluteTracking.m)[0];
                zvector.v[1] = GetZVector(devicePose->mDeviceToAbsoluteTracking.m)[1];
                zvector.v[2] = GetZVector(devicePose->mDeviceToAbsoluteTracking.m)[2];
                quaternion.w = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[0];
                quaternion.x = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[1];
                quaternion.y = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[2];
                quaternion.z = GetRotation(devicePose->mDeviceToAbsoluteTracking.m)[3];

                vVel = trackedDevicePose.vVelocity;
                vAngVel = trackedDevicePose.vAngularVelocity;
                eTrackingResult = trackedDevicePose.eTrackingResult;
                bPoseValid = trackedDevicePose.bPoseIsValid;
                if (bPoseValid) {
                    //HandleSCGT(glm::vec3(trackerPos.v[0], trackerPos.v[1], trackerPos.v[2]), glm::dquat(quaternion.w, quaternion.x, quaternion.y, quaternion.z));
                }
                break;
            }
        }
    }
    float *CoordinatorNodeInternal::GetPosition(const float hmdMatrix[3][4])
    {
        float vector[3];
        vector[0] = hmdMatrix[0][3];
        vector[1] = hmdMatrix[1][3];
        vector[2] = hmdMatrix[2][3];
        return vector;
    }

    double *CoordinatorNodeInternal::GetRotation(const float matrix[3][4])
    {
        double q[4];

        q[0] = sqrt(fmax(0, 1 + matrix[0][0] + matrix[1][1] + matrix[2][2])) / 2;
        q[1] = sqrt(fmax(0, 1 + matrix[0][0] - matrix[1][1] - matrix[2][2])) / 2;
        q[2] = sqrt(fmax(0, 1 - matrix[0][0] + matrix[1][1] - matrix[2][2])) / 2;
        q[3] = sqrt(fmax(0, 1 - matrix[0][0] - matrix[1][1] + matrix[2][2])) / 2;
        q[1] = copysign(q[1], matrix[2][1] - matrix[1][2]);
        q[2] = copysign(q[2], matrix[0][2] - matrix[2][0]);
        q[3] = copysign(q[3], matrix[1][0] - matrix[0][1]);
        return q;
    }

    float *CoordinatorNodeInternal::GetZVector(const float matrix[3][4]) {
        float vector[3];
        vector[0] = matrix[0][2];
        vector[1] = matrix[1][2];
        vector[2] = matrix[2][2];
        return vector;
    }

    float *CoordinatorNodeInternal::GetDisplayPosVector(const float position[3], const float zvector[3], const float display_lowerLeftCorner[3], const float display_upperLeftCorner[3], const float display_lowerRightCorner[3]) {
        float d1[3] = { display_lowerLeftCorner[0], display_lowerLeftCorner[1],display_lowerLeftCorner[2] };
        float d2[3] = { display_upperLeftCorner[0] - display_lowerLeftCorner[0], display_upperLeftCorner[1] - display_lowerLeftCorner[1], display_upperLeftCorner[2] - display_lowerLeftCorner[2] };
        float d3[3] = { display_lowerRightCorner[0] - display_lowerLeftCorner[0], display_lowerRightCorner[1] - display_lowerLeftCorner[1], display_lowerRightCorner[2] - display_lowerLeftCorner[2] };
        float result[2];

        result[1] = (position[0] * zvector[1] * d3[0] * zvector[2] - position[0] * zvector[1] * d3[2] * zvector[0] - position[1] * zvector[0] * d3[0] * zvector[2] + position[1] * zvector[0] * d3[2] * zvector[0] - d1[0] * zvector[1] * d3[0] * zvector[2] + d1[0] * zvector[1] * d3[2] * zvector[0] + d1[1] * zvector[0] * d3[0] * zvector[2] - d1[1] * zvector[0] * d3[2] * zvector[0] - position[0] * zvector[2] * d3[0] * zvector[1] + position[0] * zvector[2] * d3[1] * zvector[0] + position[2] * zvector[0] * d3[0] * zvector[1] - position[2] * zvector[0] * d3[1] * zvector[0] + d1[0] * zvector[2] * d3[0] * zvector[1] - d1[0] * zvector[2] * d3[1] * zvector[0] - d1[2] * zvector[0] * d3[0] * zvector[1] + d1[2] * zvector[0] * d3[1] * zvector[0]) / (d2[0] * zvector[1] * d3[0] * zvector[2] - d2[0] * zvector[1] * d3[2] * zvector[0] - d2[1] * zvector[0] * d3[0] * zvector[2] + d2[1] * zvector[0] * d3[2] * zvector[0] - d2[0] * zvector[2] * d3[0] * zvector[1] + d2[0] * zvector[2] * d3[1] * zvector[0] + d2[2] * zvector[0] * d3[0] * zvector[1] - d2[2] * zvector[0] * d3[1] * zvector[0]);
        result[0] = (position[0] * zvector[1] * d2[0] * zvector[2] - position[0] * zvector[1] * d2[2] * zvector[0] - position[1] * zvector[0] * d2[0] * zvector[2] + position[1] * zvector[0] * d2[2] * zvector[0] - d1[0] * zvector[1] * d2[0] * zvector[2] + d1[0] * zvector[1] * d2[2] * zvector[0] + d1[1] * zvector[0] * d2[0] * zvector[2] - d1[1] * zvector[0] * d2[2] * zvector[0] - position[0] * zvector[2] * d2[0] * zvector[1] + position[0] * zvector[2] * d2[1] * zvector[0] + position[2] * zvector[0] * d2[0] * zvector[1] - position[2] * zvector[0] * d2[1] * zvector[0] + d1[0] * zvector[2] * d2[0] * zvector[1] - d1[0] * zvector[2] * d2[1] * zvector[0] - d1[2] * zvector[0] * d2[0] * zvector[1] + d1[2] * zvector[0] * d2[1] * zvector[0]) / (d3[0] * zvector[1] * d2[0] * zvector[2] - d3[0] * zvector[1] * d2[2] * zvector[0] - d3[1] * zvector[0] * d2[0] * zvector[2] + d3[1] * zvector[0] * d2[2] * zvector[0] - d3[0] * zvector[2] * d2[0] * zvector[1] + d3[0] * zvector[2] * d2[1] * zvector[0] + d3[2] * zvector[0] * d2[0] * zvector[1] - d3[2] * zvector[0] * d2[1] * zvector[0]);
        //TODO
        /*midDisplayPos[0] = d1[0] + 0.5f * d2[0] + 0.5f * d3[0];
        midDisplayPos[1] = d1[1] + 0.5f * d2[1] + 0.5f * d3[1];
        midDisplayPos[2] = d1[2] + 0.5f * d2[2] + 0.5f * d3[2];*/
        return result;
    }

    void CoordinatorNodeInternal::InitDisplay(float dpos[3]) {
        if (!displayllset) {
            displayEdges[0][0] = dpos[0];
            displayEdges[0][1] = dpos[1];
            displayEdges[0][2] = dpos[1];
            displayllset = true;
            return;
        }
        if (!displayulset) {
            displayEdges[1][0] = dpos[0];
            displayEdges[1][1] = dpos[1];
            displayEdges[1][2] = dpos[1];
            displayulset = true;
            return;
        }
        if (!displaylrset) {
            displayEdges[2][0] = dpos[0];
            displayEdges[2][1] = dpos[1];
            displayEdges[2][2] = dpos[1];
            displaylrset = true;
        }
        initDisplay = true;
    }

    void CoordinatorNodeInternal::InitDisplayFloor(float cpos[3], float cz[3]) {
        float t = (-cpos[1]) / cz[1];
        if (!displayllset) {
            displayEdges[0][0] = cpos[0] + t * cz[0];
            displayEdges[0][1] = 0.0f;
            displayEdges[0][2] = cpos[2] + t * cz[2];
            displayllset = true;
            return;
        }
        if (!displaylrset) {
            displayEdges[2][0] = cpos[0] + t * cz[0];
            displayEdges[2][1] = 0.0f;
            displayEdges[2][2] = cpos[2] + t * cz[2];
            displaylrset = true;
            return;
        }
        if (!displayulset) {
            float f = (displayEdges[0][0] * (displayEdges[2][2] - displayEdges[0][2]) - displayEdges[0][2] * (displayEdges[2][0] - displayEdges[0][0]) - cpos[0] * (displayEdges[2][2] - displayEdges[0][2]) + cpos[2] * (displayEdges[2][0] - displayEdges[0][0])) / (cz[0] * (displayEdges[2][2] - displayEdges[0][2]) - cz[2] * (displayEdges[2][0] - displayEdges[0][0]));
            displayEdges[1][0] = displayEdges[0][0]; //cpos[0] + f * cz[0];
            displayEdges[1][1] = cpos[1] + f * cz[1];
            displayEdges[1][2] = displayEdges[0][2]; //cpos[2] + f * cz[2];
            displayulset = true;
        }
        initDisplay = true;
    }

    void CoordinatorNodeInternal::InitDisplayFromFile() {
        std::ifstream myfile("displayEdges.txt");
        if (myfile.is_open()) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    myfile >> displayEdges[i][j];
                }
            }
        }
    }

    void CoordinatorNodeInternal::WriteInitDisplayToFile() {
        std::ofstream myfile;
        myfile.open("displayEdges.txt");
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                myfile << displayEdges[i][j] << " ";
            }
        }
        myfile.close();
    }

    void CoordinatorNodeInternal::HandleSCGT(glm::vec3 pos, glm::quat q) {
        pos.x -= midDisplayPos.v[0];
        pos.y -= midDisplayPos.v[1];
        pos.z -= midDisplayPos.v[2];
        sgctTrackerPos = { pos.x, pos.y, pos.z };
        GetFramework().GetEngine()->getDefaultUserPtr()->setPos(pos);
        GetFramework().GetEngine()->getDefaultUserPtr()->setOrientation(q);
    }
}
