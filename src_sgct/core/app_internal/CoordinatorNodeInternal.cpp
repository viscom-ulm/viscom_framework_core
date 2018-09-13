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
#include <openvr.h>

namespace viscom {

    CoordinatorNodeInternal::CoordinatorNodeInternal(FrameworkInternal& fwInternal) :
        ApplicationNodeInternal{ fwInternal }
    {
        vr::EVRInitError peError;
        // VRApplication_Scene (starts SteamVR no proper data) VRApplication_Overlay (starts SteamVR no SteamVRHome)  VRApplication_Background (doesn't start SteamVR uses SteamVRHome)
        m_pHMD_ = vr::VR_Init(&peError, vr::EVRApplicationType::VRApplication_Background);
        if (peError == vr::VRInitError_None) {
            vrInitSucc_ = true; 
            
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

        ParseTrackingFrame();
        PollAndParseEvents();

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

        if (calibrate_) {
            fbo.DrawToFBO([this]() {
                ImGui::ShowTestWindow();
                ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiSetCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiSetCond_FirstUseEver);
                ImGui::StyleColorsClassic();
                if (ImGui::Begin("Calibration Window", nullptr))
                {
                    ImGui::Text("Open VR Display Configuration:");
                    if (initfloor_) {
                        if (!displayllset_) ImGui::Text("Point at the lower left corner and press the trigger.");
                        if (displayllset_ && !displaylrset_) ImGui::Text("Pointa at the lower right corner and press the trigger.");
                        if (displayllset_ && displaylrset_ && !displayulset_) ImGui::Text("Point at the upper left corner and press the trigger.");
                    }
                    else {
                        if (!displayllset_) ImGui::Text("Touch the lower left corner and press the trigger.");
                        if (displayllset_ && !displayulset_) ImGui::Text("Touch the upper left corner and press the trigger.");
                        if (displayllset_ && displayulset_ && !displaylrset_) ImGui::Text("Touch the lower right corner and press the trigger.");
                    }
                }
                ImGui::End();
            });
        }

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

    //TODO check File
    bool CoordinatorNodeInternal::InitialiseVR()
    {
        InitDisplayFromFile();
        if (vrInitSucc_ && initDisplay_) {
            return true;
        }
        return false;
    }
    //TODO add CalibrationMode
    bool CoordinatorNodeInternal::CalibrateVR(CalibrateMethod method)
    {
        switch (method)
        {
        case viscom::CalibrateMethod::CALIBRATE_BY_TOUCHING:
            calibrate_ = true;
            initfloor_ = false;
            return true;
            break;
        case viscom::CalibrateMethod::CALIBRATE_BY_POINTING:
            calibrate_ = true;
            initfloor_ = true;
            return true;
            break;
        default:
            return false;
            break;
        }
    }

    const std::vector<DeviceInfo>& CoordinatorNodeInternal::GetConnectedDevices()
    {
        return connectedDevices_;
    }

    const glm::vec3& CoordinatorNodeInternal::GetControllerPosition(size_t trackedDeviceId)
    {
        if (m_pHMD_ == nullptr) return glm::vec3();
        vr::ETrackedDeviceClass deviceClass = m_pHMD_->GetTrackedDeviceClass(trackedDeviceId);
        vr::ETrackedControllerRole controllerRole = vr::ETrackedControllerRole::TrackedControllerRole_Invalid;
        switch (deviceClass)
        {
        case vr::TrackedDeviceClass_Controller:
            controllerRole = m_pHMD_->GetControllerRoleForTrackedDeviceIndex(trackedDeviceId);
            switch (controllerRole)
            {
            case vr::TrackedControllerRole_LeftHand:
                return controller0pos_;
                break;
            case vr::TrackedControllerRole_RightHand:
                return controller1pos_;
                break;
            case vr::TrackedControllerRole_OptOut:
                break;
            default:
                return glm::vec3();
                break;
            }
            break;
        case vr::TrackedDeviceClass_GenericTracker:
            return trackerpos_;
            break;
        default:
            return glm::vec3();
            break;
        }
    }

    const glm::vec3& CoordinatorNodeInternal::GetControllerZVector(size_t trackedDeviceId)
    {
        if (m_pHMD_ == nullptr) return glm::vec3();
        vr::ETrackedDeviceClass deviceClass = m_pHMD_->GetTrackedDeviceClass(trackedDeviceId);
        vr::ETrackedControllerRole controllerRole = vr::ETrackedControllerRole::TrackedControllerRole_Invalid;
        switch (deviceClass)
        {
        case vr::TrackedDeviceClass_Controller:
            controllerRole = m_pHMD_->GetControllerRoleForTrackedDeviceIndex(trackedDeviceId);
            switch (controllerRole)
            {
            case vr::TrackedControllerRole_LeftHand:
                return controller0zvec_;
                break;
            case vr::TrackedControllerRole_RightHand:
                return controller1zvec_;
                break;
            case vr::TrackedControllerRole_OptOut:
                break;
            default:
                return glm::vec3();
                break;
            }
            break;
        default:
            return glm::vec3();
            break;
        }
    }

    const glm::quat& CoordinatorNodeInternal::GetControllerRotation(size_t trackedDeviceId)
    {
        if (m_pHMD_ == nullptr) return glm::quat();
        vr::ETrackedDeviceClass deviceClass = m_pHMD_->GetTrackedDeviceClass(trackedDeviceId);
        vr::ETrackedControllerRole controllerRole = vr::ETrackedControllerRole::TrackedControllerRole_Invalid;
        switch (deviceClass)
        {
        case vr::TrackedDeviceClass_Controller:
            controllerRole = m_pHMD_->GetControllerRoleForTrackedDeviceIndex(trackedDeviceId);
            switch (controllerRole)
            {
            case vr::TrackedControllerRole_LeftHand:
                return controller0rot_;
                break;
            case vr::TrackedControllerRole_RightHand:
                return controller1rot_;
                break;
            case vr::TrackedControllerRole_OptOut:
                break;
            default:
                return glm::quat();
                break;
            }
            break;
        case vr::TrackedDeviceClass_GenericTracker:
            return trackerrot_;
            break;
        default:
            return glm::quat();
            break;
        }
    }

    const glm::vec2& CoordinatorNodeInternal::GetDisplayPosition(size_t trackedDeviceId)
    {
        if (m_pHMD_ == nullptr) return glm::vec2();
        vr::ETrackedDeviceClass deviceClass = m_pHMD_->GetTrackedDeviceClass(trackedDeviceId);
        vr::ETrackedControllerRole controllerRole = vr::ETrackedControllerRole::TrackedControllerRole_Invalid;
        switch (deviceClass)
        {
        case vr::TrackedDeviceClass_Controller:
            controllerRole = m_pHMD_->GetControllerRoleForTrackedDeviceIndex(trackedDeviceId);
            switch (controllerRole)
            {
            case vr::TrackedControllerRole_LeftHand:
                return GetDisplayPosVector(controller0pos_, controller0zvec_);
                break;
            case vr::TrackedControllerRole_RightHand:
                return GetDisplayPosVector(controller0pos_, controller0zvec_);
                break;
            case vr::TrackedControllerRole_OptOut:
                break;
            default:
                return glm::vec2();
                break;
            }
            break;
        default:
            return glm::vec2();
            break;
        }
    }

    void CoordinatorNodeInternal::ControllerButtonPressedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues)
    {
        ApplicationNodeInternal::ControllerButtonPressedCallback(trackedDeviceId, buttonid, axisvalues);
    }

    void CoordinatorNodeInternal::ControllerButtonTouchedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues)
    {
        ApplicationNodeInternal::ControllerButtonTouchedCallback(trackedDeviceId, buttonid, axisvalues);
    }

    void CoordinatorNodeInternal::ControllerButtonUnpressedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues)
    {
        ApplicationNodeInternal::ControllerButtonUnpressedCallback(trackedDeviceId, buttonid, axisvalues);
    }

    void CoordinatorNodeInternal::ControllerButtonUntouchedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues)
    {
        ApplicationNodeInternal::ControllerButtonUntouchedCallback(trackedDeviceId, buttonid, axisvalues);
    }


    void CoordinatorNodeInternal::GetControllerButtonState(size_t trackedDeviceId, size_t buttonid, glm::vec2& axisvalues, ButtonState& buttonstate)
    {
        uint64_t buttonspressed = 0;
        uint64_t buttonstouched = 0;
        for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
            if (m_pHMD_ == NULL) return;
            vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
            vr::VRControllerState_t controllerState;
            vr::VRControllerState_t *controllerState_ptr = &controllerState;

            if (trackedDeviceClass == vr::TrackedDeviceClass_Controller)
            {
                vr::VRSystem()->GetControllerState(unDevice, &controllerState, sizeof(controllerState));
                vr::ETrackedControllerRole controllerRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice);
                if ((controllerRole == vr::TrackedControllerRole_LeftHand || controllerRole == vr::TrackedControllerRole_RightHand) && trackedDeviceId == unDevice) {
                    buttonspressed = controllerState.ulButtonPressed;
                    buttonstouched = controllerState.ulButtonTouched;
                    axisvalues.x = controllerState.rAxis->x;
                    axisvalues.y = controllerState.rAxis->y;
                    if (1 == ((buttonspressed >> buttonid) & 1)) buttonstate = ButtonState::PRESSED;
                    if (1 == ((buttonstouched >> buttonid) & 1)) buttonstate = ButtonState::TOUCHED;
                    if (!(1 == ((buttonspressed >> buttonid) & 1) && !(1 == ((buttonstouched >> buttonid) & 1)))) buttonstate = ButtonState::RELEASED;
                }
            }
        }
    }
 
    //TODO test
    /** Parses a OpenVR Tracking Frame by going through all connected devices. */
    void CoordinatorNodeInternal::ParseTrackingFrame()
    {
        
        vr::HmdVector3_t position;
        vr::HmdVector3_t zvector;
        vr::HmdQuaternion_t quaternion;
        connectedDevices_.clear();
        // Process SteamVR device states
        for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
            if (m_pHMD_ == NULL)
                continue;
            // if not connected just skip the rest of the routine
            if (!m_pHMD_->IsTrackedDeviceConnected(unDevice))
                continue;
            
            
            vr::TrackedDevicePose_t trackedDevicePose;
            vr::TrackedDevicePose_t *devicePose = &trackedDevicePose;

            vr::VRControllerState_t controllerState;
            vr::VRControllerState_t *controllerState_ptr = &controllerState;

            vr::HmdQuaternion_t quaternion;
            DeviceInfo deviceInform;

            if (!vr::VRSystem()->IsInputAvailable()) {
                continue;
            }

            bool bPoseValid = trackedDevicePose.bPoseIsValid;
            vr::ETrackingResult eTrackingResult;

            deviceInform.deviceId = unDevice;

            // Get what type of device it is and work with its data
            vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
            switch (trackedDeviceClass) {
            case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
                deviceInform.deviceClass = TrackedDeviceClass::HMD;

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

                eTrackingResult = trackedDevicePose.eTrackingResult;
                bPoseValid = trackedDevicePose.bPoseIsValid;
                

                switch (eTrackingResult) {

                case vr::ETrackingResult::TrackingResult_Uninitialized:
                    break;
                case vr::ETrackingResult::TrackingResult_Calibrating_InProgress:
                    break;
                case vr::ETrackingResult::TrackingResult_Calibrating_OutOfRange:
                    break;
                case vr::ETrackingResult::TrackingResult_Running_OK:
                    break;
                case vr::ETrackingResult::TrackingResult_Running_OutOfRange:
                    break;
                default:
                    break;
                }

            case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
                deviceInform.deviceClass = TrackedDeviceClass::CONTROLLER;

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

                eTrackingResult = trackedDevicePose.eTrackingResult;
                bPoseValid = trackedDevicePose.bPoseIsValid;
                switch (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice)) {
                case vr::TrackedControllerRole_Invalid:
                    // invalid hand...
                    deviceInform.deviceRole = TrackedDeviceRole::INVALID;
                    break;


                case vr::TrackedControllerRole_LeftHand:
                    deviceInform.deviceRole = TrackedDeviceRole::CONTROLLER_LEFT_HAND;
                    controller0pos_ = glm::vec3(position.v[0], position.v[1], position.v[2]);
                    controller0zvec_ = glm::vec3(zvector.v[0], zvector.v[1], zvector.v[2]);
                    controller0rot_ = glm::quat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
                    break;

                case vr::TrackedControllerRole_RightHand:
                    deviceInform.deviceRole = TrackedDeviceRole::CONTROLLER_RIGHT_HAND;
                    controller1pos_ = glm::vec3(position.v[0], position.v[1], position.v[2]);
                    controller1zvec_ = glm::vec3(zvector.v[0], zvector.v[1], zvector.v[2]);
                    controller1rot_ = glm::quat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
                    break;

                case vr::TrackedDeviceClass_TrackingReference:
                    deviceInform.deviceClass = TrackedDeviceClass::TRACKING_REFERENCE;
                    break;
                }
                break;

                case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
                vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState, sizeof(controllerState), &trackedDevicePose);

                deviceInform.deviceClass = TrackedDeviceClass::GENERIC_TRACKER;
                deviceInform.deviceRole = TrackedDeviceRole::GENERIC_TRACKER;

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

                eTrackingResult = trackedDevicePose.eTrackingResult;
                bPoseValid = trackedDevicePose.bPoseIsValid;
                trackerpos_ = glm::vec3(position.v[0], position.v[1], position.v[2]);
                trackerzvec_ = glm::vec3(zvector.v[0], zvector.v[1], zvector.v[2]);
                trackerrot_ = glm::quat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
                if (bPoseValid) {
                    HandleSCGT(trackerpos_, trackerrot_);
                }                
                break;
            }

            
            if (m_pHMD_->IsTrackedDeviceConnected(unDevice)) {
                connectedDevices_.emplace_back(deviceInform);
            }
        }
    }

    /** Get the left hand controller rotation.
    *   @param bool use the left controller as pointing device
    *   @return vec2 with display positon.
    */
    /*glm::vec2 CoordinatorNodeInternal::GetDisplayPosition(bool useleftcontroller)
    {
        float *xydisplay;
        if (useleftcontroller) {
            xydisplay=GetDisplayPosVector(controller0pos , controller0zvec, displayEdges[0], displayEdges[1], displayEdges[2]);
        }
        else {
            xydisplay=GetDisplayPosVector(controller1pos, controller1zvec, displayEdges[0], displayEdges[1], displayEdges[2]);
        }
        return glm::vec2(xydisplay[0], xydisplay[1]);
    }
    /** Initialises the display position
    *   @param bool use left controller as pointing or touching device
    */
    void CoordinatorNodeInternal::InitialiseDisplay(bool useLeftController)
    {
        if (initfloor_) {
            useLeftController ? InitDisplayFloor(controller0pos_, controller0zvec_) : InitDisplayFloor(controller1pos_, controller1zvec_);
        }
        else {
            useLeftController ? InitDisplay(controller0pos_) : InitDisplay(controller1pos_);
        }
    }
    /** Returns the position from a given HMD matrix.
    *   @param hmdMatrix 3x4 containing.
    *   @return float[3] with x y z position data.
    */
    float *CoordinatorNodeInternal::GetPosition(const float hmdMatrix[3][4])
    {
        float vector[3];
        vector[0] = hmdMatrix[0][3];
        vector[1] = hmdMatrix[1][3];
        vector[2] = hmdMatrix[2][3];
        return vector;
    }
    /** Returns the rotation from a given HMD matrix.
    *   @return double[4] with q x y z quaternion.
    */
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
    /** Returns the z-vector from a given HMD matrix.
    *   @return float[3] with x y z as z-vector data.
    */
    float *CoordinatorNodeInternal::GetZVector(const float matrix[3][4]) {
        float vector[3];
        vector[0] = matrix[0][2];
        vector[1] = matrix[1][2];
        vector[2] = matrix[2][2];
        return vector;
    }
    /** Returns the display position as x and y.
    *   @param glm::vec3 position of the controller
    *   @param glm::vec3 z-vector of the used controller.
    *   @return glm::vec2 with x, y as display position.
    */
    const glm::vec2& CoordinatorNodeInternal::GetDisplayPosVector(const glm::vec3& position, const glm::vec3& zvector) {
        float d1[3] = { displayEdges_[0][0], displayEdges_[0][1],displayEdges_[0][2] };
        float d2[3] = { displayEdges_[1][0] - displayEdges_[0][0], displayEdges_[1][1] - displayEdges_[0][1], displayEdges_[1][2] - displayEdges_[0][2] };
        float d3[3] = { displayEdges_[2][0] - displayEdges_[0][0], displayEdges_[2][1] - displayEdges_[0][1], displayEdges_[2][2] - displayEdges_[0][2] };
        glm::vec2 result;

        result[1] = (position[0] * zvector[1] * d3[0] * zvector[2] - position.x * zvector.y * d3[2] * zvector[0] - position[1] * zvector[0] * d3[0] * zvector[2] + position[1] * zvector[0] * d3[2] * zvector[0] - d1[0] * zvector[1] * d3[0] * zvector[2] + d1[0] * zvector[1] * d3[2] * zvector[0] + d1[1] * zvector[0] * d3[0] * zvector[2] - d1[1] * zvector[0] * d3[2] * zvector[0] - position[0] * zvector[2] * d3[0] * zvector[1] + position[0] * zvector[2] * d3[1] * zvector[0] + position[2] * zvector[0] * d3[0] * zvector[1] - position[2] * zvector[0] * d3[1] * zvector[0] + d1[0] * zvector[2] * d3[0] * zvector[1] - d1[0] * zvector[2] * d3[1] * zvector[0] - d1[2] * zvector[0] * d3[0] * zvector[1] + d1[2] * zvector[0] * d3[1] * zvector[0]) / (d2[0] * zvector[1] * d3[0] * zvector[2] - d2[0] * zvector[1] * d3[2] * zvector[0] - d2[1] * zvector[0] * d3[0] * zvector[2] + d2[1] * zvector[0] * d3[2] * zvector[0] - d2[0] * zvector[2] * d3[0] * zvector[1] + d2[0] * zvector[2] * d3[1] * zvector[0] + d2[2] * zvector[0] * d3[0] * zvector[1] - d2[2] * zvector[0] * d3[1] * zvector[0]);
        result[0] = (position[0] * zvector[1] * d2[0] * zvector[2] - position.x * zvector.y * d2[2] * zvector[0] - position[1] * zvector[0] * d2[0] * zvector[2] + position[1] * zvector[0] * d2[2] * zvector[0] - d1[0] * zvector[1] * d2[0] * zvector[2] + d1[0] * zvector[1] * d2[2] * zvector[0] + d1[1] * zvector[0] * d2[0] * zvector[2] - d1[1] * zvector[0] * d2[2] * zvector[0] - position[0] * zvector[2] * d2[0] * zvector[1] + position[0] * zvector[2] * d2[1] * zvector[0] + position[2] * zvector[0] * d2[0] * zvector[1] - position[2] * zvector[0] * d2[1] * zvector[0] + d1[0] * zvector[2] * d2[0] * zvector[1] - d1[0] * zvector[2] * d2[1] * zvector[0] - d1[2] * zvector[0] * d2[0] * zvector[1] + d1[2] * zvector[0] * d2[1] * zvector[0]) / (d3[0] * zvector[1] * d2[0] * zvector[2] - d3[0] * zvector[1] * d2[2] * zvector[0] - d3[1] * zvector[0] * d2[0] * zvector[2] + d3[1] * zvector[0] * d2[2] * zvector[0] - d3[0] * zvector[2] * d2[0] * zvector[1] + d3[0] * zvector[2] * d2[1] * zvector[0] + d3[2] * zvector[0] * d2[0] * zvector[1] - d3[2] * zvector[0] * d2[1] * zvector[0]);
        
        midDisplayPos_[0] = d1[0] + 0.5f * d2[0] + 0.5f * d3[0];
        midDisplayPos_[1] = d1[1] + 0.5f * d2[1] + 0.5f * d3[1];
        midDisplayPos_[2] = d1[2] + 0.5f * d2[2] + 0.5f * d3[2];
        return result;
    }
    /** Sets the display to the given position vector.
    *   @param position of the controller.
    */
    void CoordinatorNodeInternal::InitDisplay(glm::vec3 dpos) {
        if (!displayllset_) {
            displayEdges_[0][0] = dpos[0];
            displayEdges_[0][1] = dpos[1];
            displayEdges_[0][2] = dpos[1];
            displayllset_ = true;
            return;
        }
        if (!displayulset_) {
            displayEdges_[1][0] = dpos[0];
            displayEdges_[1][1] = dpos[1];
            displayEdges_[1][2] = dpos[1];
            displayulset_ = true;
            return;
        }
        if (!displaylrset_) {
            displayEdges_[2][0] = dpos[0];
            displayEdges_[2][1] = dpos[1];
            displayEdges_[2][2] = dpos[1];
            displaylrset_ = true;
        }
        initDisplay_ = true;
        calibrate_ = false;
        WriteInitDisplayToFile();
    }
    /** Sets a display to the point where a controller points.
    *   @param position of the controller.
    *   @param z-vector of the controller.
    */
    void CoordinatorNodeInternal::InitDisplayFloor(glm::vec3 cpos, glm::vec3 cz) {
        float t = (-cpos[1]) / cz[1];
        if (!displayllset_) {
            displayEdges_[0][0] = cpos[0] + t * cz[0];
            displayEdges_[0][1] = 0.0f;
            displayEdges_[0][2] = cpos[2] + t * cz[2];
            displayllset_ = true;
            return;
        }
        if (!displaylrset_) {
            displayEdges_[2][0] = cpos[0] + t * cz[0];
            displayEdges_[2][1] = 0.0f;
            displayEdges_[2][2] = cpos[2] + t * cz[2];
            displaylrset_ = true;
            return;
        }
        if (!displayulset_) {
            float f = (displayEdges_[0][0] * (displayEdges_[2][2] - displayEdges_[0][2]) - displayEdges_[0][2] * (displayEdges_[2][0] - displayEdges_[0][0]) - cpos[0] * (displayEdges_[2][2] - displayEdges_[0][2]) + cpos[2] * (displayEdges_[2][0] - displayEdges_[0][0])) / (cz[0] * (displayEdges_[2][2] - displayEdges_[0][2]) - cz[2] * (displayEdges_[2][0] - displayEdges_[0][0]));
            displayEdges_[1][0] = displayEdges_[0][0]; //cpos[0] + f * cz[0];
            displayEdges_[1][1] = cpos[1] + f * cz[1];
            displayEdges_[1][2] = displayEdges_[0][2]; //cpos[2] + f * cz[2];
            displayulset_ = true;
        }
        initDisplay_ = true;
        calibrate_ = false;
        WriteInitDisplayToFile();
    }
    /** Reads displayEdges.txt and initialises the display with found values. */
    void CoordinatorNodeInternal::InitDisplayFromFile() {
        std::ifstream myfile("displayEdges.txt");
        if (myfile.is_open()) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    myfile >> displayEdges_[i][j];
                }
            }
        }
        displayllset_ = true;
        displayulset_ = true;
        displaylrset_ = true;
        initDisplay_ = true;
    }
    /** Writes the current display edges to displayEdges.txt */
    void CoordinatorNodeInternal::WriteInitDisplayToFile() {
        std::ofstream myfile;
        myfile.open("displayEdges.txt");
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                myfile << displayEdges_[i][j] << " ";
            }
        }
        myfile.close();
    }
    /** Passes the tracker position and rotation to sgct for head tracking 
    *   @param postion of the tracker
    *   @param tracker rotation
    */
    void CoordinatorNodeInternal::HandleSCGT(glm::vec3 pos, glm::quat q) {
        pos.x -= midDisplayPos_[0];
        pos.y -= midDisplayPos_[1];
        pos.z -= midDisplayPos_[2];
        pos.z = pos.z * -1;
        GetFramework().GetEngine()->getDefaultUserPtr()->setPos(pos);
        GetFramework().GetEngine()->getDefaultUserPtr()->setOrientation(q);
    }
    /** Tests which devices are connected and returns them in a string vector.
    *   @return string vector containing the connected devices.
    */
    std::vector<std::string> CoordinatorNodeInternal::OutputDevices() {
        std::vector<std::string> devices;
        if (m_pHMD_ == nullptr) {
            return devices;
        }
        for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
            if (!m_pHMD_->IsTrackedDeviceConnected(unDevice))
                continue;
            vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
            switch (trackedDeviceClass) {
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_Controller):
                devices.push_back("Controller");
                break;
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker):
                devices.push_back("GenericTracker");
                break;
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_HMD):
                devices.push_back("HMD");
                break;
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference):
                devices.push_back("TrackingRef");
                break;
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_DisplayRedirect):
                devices.push_back("DisplayRedirect");
                break;
            case(vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid):
                devices.push_back("Invalid");
                break;
            }
        }
        return devices;
    }

    /** Processes a given vr event currently only handling controller buttons.
    *   @return true if the end of the procedure was reached.
    */
    bool CoordinatorNodeInternal::ProcessVREvent(const vr::VREvent_t & event) {
        switch (event.eventType)
        {
        case vr::VREvent_None:
        {
            //process Event
        }
        case vr::VREvent_TrackedDeviceActivated:
        {
            //process Event
        }
        break;

        case vr::VREvent_TrackedDeviceDeactivated:
        {
            //process Event
        }
        break;

        case vr::VREvent_TrackedDeviceUpdated:
        {
            //process Event
        }
        break;

        case vr::VREvent_ButtonPress:
        {
            //TODO make unicontroller conform
            if (!initDisplay_ && event.data.controller.button == vr::k_EButton_SteamVR_Trigger && calibrate_) {
                ParseTrackingFrame();
                InitialiseDisplay(useleftcontroller_);
            }
            
            switch (event.data.controller.button)
            {
            case vr::EVRButtonId::k_EButton_A:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("Button A");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("Button A");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_ApplicationMenu:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("AppMenu");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("AppMenu");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis0:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("Axis0");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("Axis0");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis1:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("Axis1");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("Axis1");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis2:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("Axis2");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("Axis2");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis3:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("Axis3");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("Axis3");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis4:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("Axis4");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("Axis4");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Down:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("DPadDown");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("DPadDown");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Left:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("DPadLeft");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("DPadLeft");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Right:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("DPadRight");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("DPadRight");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Up:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("DPadUp");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("DPadUp");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Grip:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("Grip");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("Grip");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Max:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("Max");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("Max");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_System:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.push_back("System");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.push_back("System");
                    }
                }
            }
            break;

            default:
                break;
            }
        }
        break;

        case vr::VREvent_ButtonTouch:
        {
            //TODO add if needed
        }
        break;

        case vr::VREvent_ButtonUnpress:
        {
            switch (event.data.controller.button)
            {
            case vr::EVRButtonId::k_EButton_A:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "Button A"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "Button A"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_ApplicationMenu:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "AppMenu"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "AppMenu"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis0:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "Axis0"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "Axis0"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis1:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "Axis1"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "Axis1"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis2:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "Axis2"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "Axis2"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis3:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "Axis3"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "Axis3"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis4:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "Axis4"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "Axis4"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Down:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "DPadDown"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "DPadDown"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Left:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "DPadLeft"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "DPadLeft"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Right:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "DPadRight"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "DPadRight"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Up:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "DPadUp"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "DPadUp"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Grip:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "Grip"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "Grip"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Max:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "Max"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "Max"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_System:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.erase(std::find(controller0buttons_.begin(), controller0buttons_.end(), "System"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.erase(std::find(controller1buttons_.begin(), controller1buttons_.end(), "System"));
                    }
                }
            }
            break;

            default:
                break;
            }
        }
        break;

        case vr::VREvent_ButtonUntouch:
        {
            //process Event
        }
        break;

        case vr::VREvent_MouseMove:
        {
            //process Event
        }
        break;

        case vr::VREvent_MouseButtonDown:
        {
            //process Event
        }
        break;

        case vr::VREvent_MouseButtonUp:
        {
            //process Event
        }
        break;

        case vr::VREvent_InputFocusCaptured:
        {
            //process Event
        }
        break;

        case vr::VREvent_InputFocusReleased:
        {
            //process Event
        }
        break;

        case (vr::VREvent_ChaperoneDataHasChanged):
        {
            //process Event
        }
        break;

        case (vr::VREvent_ChaperoneSettingsHaveChanged):
        {
            //process Event
        }
        break;

        case (vr::VREvent_ChaperoneUniverseHasChanged):
        {
            //process Event
        }
        break;

        case (vr::VREvent_ApplicationTransitionStarted):
        {
            //process Event
        }
        break;

        case (vr::VREvent_ApplicationTransitionNewAppStarted):
        {
            //process Event
        }
        break;

        case (vr::VREvent_Quit):
        {
            //process Event
            return false;
        }
        break;

        case (vr::VREvent_ProcessQuit):
        {
            //process Event
            return false;
        }
        break;

        case (vr::VREvent_QuitAborted_UserPrompt):
        {
            //process Event
            return false;
        }
        break;

        case (vr::VREvent_QuitAcknowledged):
        {
            //process Event
            return false;
        }
        break;

        case (vr::VREvent_TrackedDeviceRoleChanged):
        {
            //process Event
            break;
        }

        case (vr::VREvent_TrackedDeviceUserInteractionStarted):
        {
            //process Event
            break;
        }

        default:
            //process Event
            break;
        }

        return true;
    }

    /** Returns a bool which shows if the display was initialised.
    *   @return bool is display initialises.
    */
    bool CoordinatorNodeInternal::GetDisplayInitialised() {
        return initDisplay_;
    }

    void CoordinatorNodeInternal::SetDisplayNotInitialised()
    {
        initDisplay_ = false;
        displayllset_ = false;
        displayulset_ = false;
        displaylrset_ = false;
    }
    /** Returns true if the display is currently initialised by pointing at the display edges.
    *   @return bool is display initialised by pointing at the display corners.
    */
    bool CoordinatorNodeInternal::GetDisplayInitByFloor() {
        return initfloor_;
    }

    /** Polls and parses the next vr event */
    void CoordinatorNodeInternal::PollAndParseNextEvent()
    {
        vr::VREvent_t event;
        if (m_pHMD_ == nullptr) return;
        if (m_pHMD_->PollNextEvent(&event, sizeof(event))) {
            ProcessVREvent(event);
        }
    }
    /** Polls and parses all vr events in queue */
    void CoordinatorNodeInternal::PollAndParseEvents()
    {
        vr::VREvent_t event;
        if (m_pHMD_ == nullptr) return;
        while (m_pHMD_->PollNextEvent(&event, sizeof(event)))
        {
            ProcessVREvent(event);
        }
            
    }
}
