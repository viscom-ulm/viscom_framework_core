/**
 * @file   ApplicationNodeInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the base application node base class.
 */

#include "ApplicationNodeInternal.h"
#include <imgui.h>
#include "core/open_gl.h"
#include "core/app/ApplicationNodeBase.h"
#include <openvr.h>
#include <iostream>
#include <fstream>

namespace viscom {

    enum class InternalTransferType : std::uint8_t {
        ResourceTransfer,
        ResourceReleaseTransfer,
        ResourceRequest
    };

    enum class InternalTransferTypeLarge : std::uint16_t {
        UserData = std::numeric_limits<std::uint16_t>::max()
    };

    ApplicationNodeInternal::ApplicationNodeInternal(FrameworkInternal& fwInternal) :
        tuio::TuioInputWrapper{ fwInternal.GetConfig().tuioPort_ },
        fwInternal_{ fwInternal },
        currentTime_{ 0.0 },
        elapsedTime_{ 0.0 }
    {
        if (fwInternal_.IsCoordinator()) appNodeImpl_ = fwInternal_.GetCoordinatorNodeFactory()(this);
        else appNodeImpl_ = fwInternal_.GetWorkerNodeFactory()(this);
#pragma warning( push )
#pragma warning( disable: 4996 )
        appNodeImpl_->PreWindow();
        appNodeImpl_->InitOpenGL();
#pragma warning( pop )
    }

    ApplicationNodeInternal::~ApplicationNodeInternal()
    {
#pragma warning( push )
#pragma warning( disable: 4996 )
        appNodeImpl_->CleanUp();
#pragma warning( pop )
        appNodeImpl_ = nullptr;
    }

    void ApplicationNodeInternal::PreWindow()
    {
        // nothing to do anymore.
    }

    void ApplicationNodeInternal::InitOpenGL()
    {
        // nothing to do anymore.
    }

    void ApplicationNodeInternal::PreSync()
    {
        ParseTrackingFrame();
        PollAndParseEvents();
        appNodeImpl_->PreSync();
    }

    void ApplicationNodeInternal::PostSync()
    {
        auto lastTime = currentTime_;
        currentTime_ = glfwGetTime();

        appNodeImpl_->UpdateSyncedInfo();

        elapsedTime_ = currentTime_ - lastTime;
        glfwPollEvents();

        appNodeImpl_->UpdateFrame(currentTime_, elapsedTime_);
    }

    void ApplicationNodeInternal::ClearBuffer(FrameBuffer& fbo)
    {
        appNodeImpl_->ClearBuffer(fbo);
    }

    void ApplicationNodeInternal::DrawFrame(FrameBuffer& fbo)
    {
        appNodeImpl_->DrawFrame(fbo);
    }

    void ApplicationNodeInternal::Draw2D(FrameBuffer& fbo)
    {
        appNodeImpl_->Draw2D(fbo);
    }

    void ApplicationNodeInternal::PostDraw()
    {
    }

    void ApplicationNodeInternal::CleanUp()
    {
    }

    void ApplicationNodeInternal::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        appNodeImpl_->KeyboardCallback(key, scancode, action, mods);
    }

    void ApplicationNodeInternal::CharCallback(unsigned int character, int mods)
    {
        appNodeImpl_->CharCallback(character, mods);
    }

    void ApplicationNodeInternal::MouseButtonCallback(int button, int action)
    {
        appNodeImpl_->MouseButtonCallback(button, action);
    }

    void ApplicationNodeInternal::MousePosCallback(double x, double y)
    {
        appNodeImpl_->MousePosCallback(x, y);
    }

    void ApplicationNodeInternal::MouseScrollCallback(double xoffset, double yoffset)
    {
        appNodeImpl_->MouseScrollCallback(xoffset, yoffset);
    }

    void ApplicationNodeInternal::DataTransfer(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID)
    {
        appNodeImpl_->DataTransferCallback(receivedData, receivedLength, packageID, clientID);
    }

    void ApplicationNodeInternal::DataAcknowledge(std::uint16_t packageID, int clientID)
    {
        appNodeImpl_->DataAcknowledgeCallback(packageID, clientID);
    }

    void ApplicationNodeInternal::DataTransferStatus(bool connected, int clientID)
    {
        appNodeImpl_->DataTransferStatusCallback(connected, clientID);
    }

    void ApplicationNodeInternal::addTuioCursor(TUIO::TuioCursor* tcur)
    {
        if constexpr (USE_TUIO) {
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->AddTuioCursor(tcur);
        }
    }

    void ApplicationNodeInternal::updateTuioCursor(TUIO::TuioCursor* tcur)
    {
        if constexpr (USE_TUIO) {
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->UpdateTuioCursor(tcur);
        }
    }

    void ApplicationNodeInternal::removeTuioCursor(TUIO::TuioCursor* tcur)
    {
        if constexpr (USE_TUIO) {
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->RemoveTuioCursor(tcur);
        }
    }

    void ApplicationNodeInternal::ControllerButtonPressedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2 axisvalues)
    {
        LOG(G3LOG_DEBUG) << "ControllerButtonPressedCalb called on Coordinator.";
        ApplicationNodeInternal::ControllerButtonPressedCallback(trackedDeviceId, buttonid, axisvalues);
    }

    void ApplicationNodeInternal::ControllerButtonTouchedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2 axisvalues)
    {
        ApplicationNodeInternal::ControllerButtonTouchedCallback(trackedDeviceId, buttonid, axisvalues);
    }

    void ApplicationNodeInternal::ControllerButtonUnpressedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2 axisvalues)
    {
        ApplicationNodeInternal::ControllerButtonUnpressedCallback(trackedDeviceId, buttonid, axisvalues);
    }

    void ApplicationNodeInternal::ControllerButtonUntouchedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2 axisvalues)
    {
        ApplicationNodeInternal::ControllerButtonUntouchedCallback(trackedDeviceId, buttonid, axisvalues);
    }


    void ApplicationNodeInternal::GetControllerButtonState(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2& axisvalues, ButtonState& buttonstate)
    {
        LOG(G3LOG_DEBUG) << "GetContrButSta called on Coordinator.";
        uint64_t buttonspressed = 0;
        uint64_t buttonstouched = 0;
        for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
            if (m_pHMD_ == nullptr) return;
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
    

    /** Initializes the display position
    *   @param bool use left controller as pointing or touching device
    */
    void ApplicationNodeInternal::InitialiseDisplay(bool useLeftController)
    {
        if (initfloor_) {
            useLeftController ? InitDisplayFloor(controller0pos_, controller0zvec_) : InitDisplayFloor(controller1pos_, controller1zvec_);
        }
        else {
            useLeftController ? InitDisplay(controller0pos_) : InitDisplay(controller1pos_);
        }
    }
    /** Returns the display position as x and y.
    *   @param glm::vec3 position of the controller
    *   @param glm::vec3 z-vector of the used controller.
    *   @return glm::vec2 with x, y as display position.
    */
    const glm::vec2& ApplicationNodeInternal::GetDisplayPosVector(const glm::vec3& position, const glm::vec3& zvector) {
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
    void ApplicationNodeInternal::InitDisplay(glm::vec3 dpos) {
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
    void ApplicationNodeInternal::InitDisplayFloor(glm::vec3 cpos, glm::vec3 cz) {
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
    /** Reads displayEdges.txt and initializes the display with found values. */
    void ApplicationNodeInternal::InitDisplayFromFile() {
        std::ifstream myfile("displayEdges.txt");
        if (myfile.is_open()) {
            for (auto& displayEdge : displayEdges_) {
                for (float & j : displayEdge) {
                    myfile >> j;
                }
            }
        }
        displayllset_ = true;
        displayulset_ = true;
        displaylrset_ = true;
        initDisplay_ = true;
    }
    /** Writes the current display edges to displayEdges.txt */
    void ApplicationNodeInternal::WriteInitDisplayToFile() {
        std::ofstream myfile;
        myfile.open("displayEdges.txt");
        for (auto & displayEdge : displayEdges_) {
            for (float j : displayEdge) {
                myfile << j << " ";
            }
        }
        myfile.close();
    }

    /** Processes a given vr event currently only handling controller buttons.
    *   @return true if the end of the procedure was reached.
    */
    bool ApplicationNodeInternal::ProcessVREvent(const vr::VREvent_t & event) {
        LOG(INFO) << "ProcessVREvent called on Coordinator.";
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
            //TODO make uni-controller conform
            if (!initDisplay_ && event.data.controller.button == vr::k_EButton_SteamVR_Trigger && calibrate_) {
                ParseTrackingFrame();
                InitialiseDisplay(useleftcontroller_);
            }
            ControllerButtonPressedCallback(event.trackedDeviceIndex, event.data.controller.button, glm::vec2(event.data.dualAnalog.x, event.data.dualAnalog.y));
            switch (event.data.controller.button)
            {
            case vr::EVRButtonId::k_EButton_A:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("Button A");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("Button A");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_ApplicationMenu:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("AppMenu");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("AppMenu");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis0:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("Axis0");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("Axis0");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis1:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("Axis1");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("Axis1");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis2:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("Axis2");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("Axis2");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis3:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("Axis3");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("Axis3");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Axis4:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("Axis4");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("Axis4");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Down:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("DPadDown");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("DPadDown");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Left:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("DPadLeft");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("DPadLeft");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Right:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("DPadRight");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("DPadRight");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_DPad_Up:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("DPadUp");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("DPadUp");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Grip:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("Grip");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("Grip");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_Max:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("Max");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("Max");
                    }
                }
            }
            break;
            case vr::EVRButtonId::k_EButton_System:
            {
                if (vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_LeftHand) {
                        controller0buttons_.emplace_back("System");
                    }
                    if (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex) == vr::TrackedControllerRole_RightHand) {
                        controller1buttons_.emplace_back("System");
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
            ControllerButtonTouchedCallback(event.trackedDeviceIndex, event.data.controller.button, glm::vec2(event.data.dualAnalog.x, event.data.dualAnalog.y));
        }
        break;

        case vr::VREvent_ButtonUnpress:
        {
            ControllerButtonUnpressedCallback(event.trackedDeviceIndex, event.data.controller.button, glm::vec2(event.data.dualAnalog.x, event.data.dualAnalog.y));
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
            ControllerButtonUntouchedCallback(event.trackedDeviceIndex, event.data.controller.button, glm::vec2(event.data.dualAnalog.x, event.data.dualAnalog.y));
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

    /** Returns a bool which shows if the display was initialized.
    *   @return bool is display initializes.
    */
    bool ApplicationNodeInternal::GetDisplayInitialised() {
        return initDisplay_;
    }


    /** Returns true if the display is currently initialized by pointing at the display edges.
    *   @return bool is display initialized by pointing at the display corners.
    */
    bool ApplicationNodeInternal::GetDisplayInitByFloor() {
        return initfloor_;
    }

    /** Polls and parses the next vr event */
    void ApplicationNodeInternal::PollAndParseNextEvent()
    {
        vr::VREvent_t event;
        if (m_pHMD_ == nullptr) return;
        if (m_pHMD_->PollNextEvent(&event, sizeof(event))) {
            ProcessVREvent(event);
        }
    }
    /** Polls and parses all vr events in queue */
    void ApplicationNodeInternal::PollAndParseEvents()
    {
        vr::VREvent_t event;
        if (m_pHMD_ == nullptr) return;
        while (m_pHMD_->PollNextEvent(&event, sizeof(event)))
        {
            ProcessVREvent(event);
        }

    }
}
