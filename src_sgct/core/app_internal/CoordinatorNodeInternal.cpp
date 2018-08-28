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
        m_pHMD = vr::VR_Init(&peError, vr::EVRApplicationType::VRApplication_Background);
        if (peError == vr::VRInitError_None) {
            vrInitSucc = true; 
            
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

    //TODO test
    /** Parses a OpenVR Tracking Frame by going through all connected devices. */
    void CoordinatorNodeInternal::ParseTrackingFrame()
    {
        vr::HmdVector3_t position;
        vr::HmdVector3_t zvector;
        vr::HmdQuaternion_t quaternion;
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
                continue;
            }

            bool bPoseValid = trackedDevicePose.bPoseIsValid;
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

                eTrackingResult = trackedDevicePose.eTrackingResult;
                bPoseValid = trackedDevicePose.bPoseIsValid;
                
                //TODO more info and print some more info to the user about the state of the device/pose
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

                eTrackingResult = trackedDevicePose.eTrackingResult;
                bPoseValid = trackedDevicePose.bPoseIsValid;
                switch (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice)) {
                case vr::TrackedControllerRole_Invalid:
                    // invalid hand...
                    break;


                case vr::TrackedControllerRole_LeftHand:
                    controller0pos = glm::vec3(position.v[0], position.v[1], position.v[2]);
                    controller0zvec = glm::vec3(zvector.v[0], zvector.v[1], zvector.v[2]);
                    controller0rot = glm::quat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
                    break;

                case vr::TrackedControllerRole_RightHand:
                    controller1pos = glm::vec3(position.v[0], position.v[1], position.v[2]);
                    controller1zvec = glm::vec3(zvector.v[0], zvector.v[1], zvector.v[2]);
                    controller1rot = glm::quat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
                    break;

                case vr::TrackedDeviceClass_TrackingReference:
                    //Handle
                    break;
                }
                break;

                case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
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
                trackerpos = glm::vec3(position.v[0], position.v[1], position.v[2]);
                trackerzvec = glm::vec3(zvector.v[0], zvector.v[1], zvector.v[2]);
                trackerrot = glm::quat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
                if (bPoseValid) {
                    HandleSCGT(trackerpos, trackerrot);
                }
                
                break;
            }
        }
    }
    /** Get the position of the left hand controller. 
    *   @return position of the left hand controller.^55
    */
    glm::vec3 CoordinatorNodeInternal::GetController0Pos()
    {
        return controller0pos;
    }
    /** Get the z-vector of the left hand controller. 
    *   @return zvector of the left hand controller.
    */
    glm::vec3 CoordinatorNodeInternal::GetController0Zvec()
    {
        return controller0zvec;
    }
    /** Get the position of the right hand controller. 
    *   @return position of the right hand controller.
    */
    glm::vec3 CoordinatorNodeInternal::GetController1Pos()
    {
        return controller1pos;
    }
    /** Get the z-vector of the right hand controller.
    *   @return z-vector of the right hand controller.
    */
    glm::vec3 CoordinatorNodeInternal::GetController1Zvec()
    {
        return controller1zvec;
    }
    /** Get the tracker position.
    *   @return tracker position.
    */
    glm::vec3 CoordinatorNodeInternal::GetTrackerPos()
    {
        return trackerpos;
    }
    /** Get the tracker z-vector.
    *   @return tracker z-vector.
    */
    glm::vec3 CoordinatorNodeInternal::GetTrackerZvec()
    {
        return trackerzvec;
    }
    /** Get the left hand controller rotation.
    *   @return left hand controller quaternion.
    */
    glm::quat CoordinatorNodeInternal::GetController0Rot()
    {
        return controller0rot;
    }
    /** Get the right hand controller rotation.
    *   @return right hand controller quaternion.
    */
    glm::quat CoordinatorNodeInternal::GetController1Rot()
    {
        return controller1rot;
    }
    /** Get the tracker rotation.
    *   @return tracker quaternion.
    */
    glm::quat CoordinatorNodeInternal::GetTrackerRot()
    {
        return trackerrot;
    }
    /** Get the left hand controller rotation.
    *   @param bool use the left controller as pointing device
    *   @return vec2 with display positon.
    */
    glm::vec2 CoordinatorNodeInternal::GetDisplayPosition(bool useleftcontroller)
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
        if (initfloor) {
            useLeftController ? InitDisplayFloor(controller0pos, controller0zvec) : InitDisplayFloor(controller1pos, controller1zvec);
        }
        else {
            useLeftController ? InitDisplay(controller0pos) : InitDisplay(controller1pos);
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
    *   @param float[3] containing the lower left display corner position.
    *   @param float[3] containing the upper left display corner position.
    *   @param float[3] containing the lower right display corner position.
    *   @return float[2] with x y as display position.
    */
    float *CoordinatorNodeInternal::GetDisplayPosVector(glm::vec3 position, glm::vec3 zvector, const float display_lowerLeftCorner[3], const float display_upperLeftCorner[3], const float display_lowerRightCorner[3]) {
        float d1[3] = { display_lowerLeftCorner[0], display_lowerLeftCorner[1],display_lowerLeftCorner[2] };
        float d2[3] = { display_upperLeftCorner[0] - display_lowerLeftCorner[0], display_upperLeftCorner[1] - display_lowerLeftCorner[1], display_upperLeftCorner[2] - display_lowerLeftCorner[2] };
        float d3[3] = { display_lowerRightCorner[0] - display_lowerLeftCorner[0], display_lowerRightCorner[1] - display_lowerLeftCorner[1], display_lowerRightCorner[2] - display_lowerLeftCorner[2] };
        float result[2];

        result[1] = (position[0] * zvector[1] * d3[0] * zvector[2] - position.x * zvector.y * d3[2] * zvector[0] - position[1] * zvector[0] * d3[0] * zvector[2] + position[1] * zvector[0] * d3[2] * zvector[0] - d1[0] * zvector[1] * d3[0] * zvector[2] + d1[0] * zvector[1] * d3[2] * zvector[0] + d1[1] * zvector[0] * d3[0] * zvector[2] - d1[1] * zvector[0] * d3[2] * zvector[0] - position[0] * zvector[2] * d3[0] * zvector[1] + position[0] * zvector[2] * d3[1] * zvector[0] + position[2] * zvector[0] * d3[0] * zvector[1] - position[2] * zvector[0] * d3[1] * zvector[0] + d1[0] * zvector[2] * d3[0] * zvector[1] - d1[0] * zvector[2] * d3[1] * zvector[0] - d1[2] * zvector[0] * d3[0] * zvector[1] + d1[2] * zvector[0] * d3[1] * zvector[0]) / (d2[0] * zvector[1] * d3[0] * zvector[2] - d2[0] * zvector[1] * d3[2] * zvector[0] - d2[1] * zvector[0] * d3[0] * zvector[2] + d2[1] * zvector[0] * d3[2] * zvector[0] - d2[0] * zvector[2] * d3[0] * zvector[1] + d2[0] * zvector[2] * d3[1] * zvector[0] + d2[2] * zvector[0] * d3[0] * zvector[1] - d2[2] * zvector[0] * d3[1] * zvector[0]);
        result[0] = (position[0] * zvector[1] * d2[0] * zvector[2] - position.x * zvector.y * d2[2] * zvector[0] - position[1] * zvector[0] * d2[0] * zvector[2] + position[1] * zvector[0] * d2[2] * zvector[0] - d1[0] * zvector[1] * d2[0] * zvector[2] + d1[0] * zvector[1] * d2[2] * zvector[0] + d1[1] * zvector[0] * d2[0] * zvector[2] - d1[1] * zvector[0] * d2[2] * zvector[0] - position[0] * zvector[2] * d2[0] * zvector[1] + position[0] * zvector[2] * d2[1] * zvector[0] + position[2] * zvector[0] * d2[0] * zvector[1] - position[2] * zvector[0] * d2[1] * zvector[0] + d1[0] * zvector[2] * d2[0] * zvector[1] - d1[0] * zvector[2] * d2[1] * zvector[0] - d1[2] * zvector[0] * d2[0] * zvector[1] + d1[2] * zvector[0] * d2[1] * zvector[0]) / (d3[0] * zvector[1] * d2[0] * zvector[2] - d3[0] * zvector[1] * d2[2] * zvector[0] - d3[1] * zvector[0] * d2[0] * zvector[2] + d3[1] * zvector[0] * d2[2] * zvector[0] - d3[0] * zvector[2] * d2[0] * zvector[1] + d3[0] * zvector[2] * d2[1] * zvector[0] + d3[2] * zvector[0] * d2[0] * zvector[1] - d3[2] * zvector[0] * d2[1] * zvector[0]);
        
        midDisplayPos[0] = d1[0] + 0.5f * d2[0] + 0.5f * d3[0];
        midDisplayPos[1] = d1[1] + 0.5f * d2[1] + 0.5f * d3[1];
        midDisplayPos[2] = d1[2] + 0.5f * d2[2] + 0.5f * d3[2];
        return result;
    }
    /** Sets the display to the given position vector.
    *   @param position of the controller.
    */
    void CoordinatorNodeInternal::InitDisplay(glm::vec3 dpos) {
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
    /** Sets a display to the point where a controller points.
    *   @param position of the controller.
    *   @param z-vector of the controller.
    */
    void CoordinatorNodeInternal::InitDisplayFloor(glm::vec3 cpos, glm::vec3 cz) {
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
    /** Reads displayEdges.txt and initialises the display with found values. */
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
    /** Writes the current display edges to displayEdges.txt */
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
    /** Passes the tracker position and rotation to sgct for head tracking 
    *   @param postion of the tracker
    *   @param tracker rotation
    */
    void CoordinatorNodeInternal::HandleSCGT(glm::vec3 pos, glm::quat q) {
        pos.x -= midDisplayPos[0];
        pos.y -= midDisplayPos[1];
        pos.z -= midDisplayPos[2];
        pos.z = pos.z * -1;
        GetFramework().GetEngine()->getDefaultUserPtr()->setPos(pos);
        GetFramework().GetEngine()->getDefaultUserPtr()->setOrientation(q);
    }
    /** Tests which devices are connected and returns them in a string vector.
    *   @return string vector containing the connected devices.
    */
    std::vector<std::string> CoordinatorNodeInternal::OutputDevices() {
        std::vector<std::string> devices;
        if (m_pHMD == nullptr) {
            return devices;
        }
        for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
            if (!m_pHMD->IsTrackedDeviceConnected(unDevice))
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
    /** Tests which buttons are pressed on the left hand controller and returns them in a string vector.
    *   @return string vector containing the pressed buttons on the left hand controller.
    */
    std::vector<std::string> CoordinatorNodeInternal::GetController0Buttons()
    {
        return controller0buttons;
    }
    /** Tests which buttons are pressed on the right hand controller and returns them in a string vector.
    *   @return string vector containing the pressed buttons on the right hand controller.
    */
    std::vector<std::string> CoordinatorNodeInternal::GetController1Buttons()
    {
        return controller1buttons;
    }
    //TODO use and test.
    /** Returns an 3x3 array containing the display edges in following order: lower left, upper left and lower right.
    *   @return 3x3 array containing the display edges
    */
    float * CoordinatorNodeInternal::GetDisplayEdges()
    {
        return *displayEdges;
    }
    /** Returns if the vr init was succesfull.
    *   @return bool if the vr initialisation was succesfull
    */
    bool CoordinatorNodeInternal::GetVrInitSuccess()
    {
        return vrInitSucc;
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
            //TODO test
            if (!initDisplay && event.data.controller.button == vr::k_EButton_SteamVR_Trigger) {
                ParseTrackingFrame();
                InitialiseDisplay(true);
            }
            
            switch (event.data.controller.button)
            {
            case vr::EVRButtonId::k_EButton_A:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("Button A");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("Button A");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_ApplicationMenu:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("AppMenu");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("AppMenu");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis0:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("Axis0");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("Axis0");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis1:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("Axis1");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("Axis1");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis2:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("Axis2");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("Axis2");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis3:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("Axis3");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("Axis3");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis4:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("Axis4");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("Axis4");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Down:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("DPadDown");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("DPadDown");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Left:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("DPadLeft");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("DPadLeft");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Right:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("DPadRight");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("DPadRight");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Up:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("DPadUp");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("DPadUp");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Grip:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("Grip");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("Grip");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Max:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("Max");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("Max");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_System:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.push_back("System");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.push_back("System");
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
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "Button A"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "Button A"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_ApplicationMenu:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "AppMenu"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "AppMenu"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis0:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "Axis0"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "Axis0"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis1:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "Axis1"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "Axis1"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis2:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "Axis2"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "Axis2"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis3:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "Axis3"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "Axis3"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis4:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "Axis4"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "Axis4"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Down:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "DPadDown"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "DPadDown"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Left:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "DPadLeft"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "DPadLeft"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Right:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "DPadRight"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "DPadRight"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Up:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "DPadUp"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "DPadUp"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Grip:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "Grip"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "Grip"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Max:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "Max"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "Max"));
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_System:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons.erase(std::find(controller0buttons.begin(), controller0buttons.end(), "System"));
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons.erase(std::find(controller1buttons.begin(), controller1buttons.end(), "System"));
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
        return initDisplay;
    }

    void CoordinatorNodeInternal::SetDisplayNotInitialised()
    {
        initDisplay = false;
        displayllset = false;
        displayulset = false;
        displaylrset = false;
    }
    /** Returns true if the display is currently initialised by pointing at the display edges.
    *   @return bool is display initialised by pointing at the display corners.
    */
    bool CoordinatorNodeInternal::GetDisplayInitByFloor() {
        return initfloor;
    }
    /** Sets if the display is initialised by pointing at the display corners or touching them.
    *   @param bool initialise display by pointing at the edges or touching them.
    */
    void CoordinatorNodeInternal::SetDisplayInitByFloor(bool b) {
        initfloor = b;
    }
    /** Polls and parses the next vr event */
    void CoordinatorNodeInternal::PollAndParseNextEvent()
    {
        vr::VREvent_t event;
        if (m_pHMD == nullptr) return;
        if (m_pHMD->PollNextEvent(&event, sizeof(event))) {
            ProcessVREvent(event);
        }
    }
    /** Polls and parses all vr events in queue */
    void CoordinatorNodeInternal::PollAndParseEvents()
    {
        vr::VREvent_t event;
        if (m_pHMD == nullptr) return;
        while (m_pHMD->PollNextEvent(&event, sizeof(event)))
        {
            ProcessVREvent(event);
        }
            
    }
}
