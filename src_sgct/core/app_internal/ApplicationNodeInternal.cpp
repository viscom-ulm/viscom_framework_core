/**
 * @file   ApplicationNodeInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the base application node base class.
 */

#include "core/main.h"
#include <sgct.h>
#include "ApplicationNodeInternal.h"
#include "core/TuioInputWrapper.h"
#include <imgui.h>
#include "core/app/ApplicationNodeBase.h"
#include "core/utils/utils.h"
#include <sgct.h>

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
        fwInternal_{ fwInternal },
        tuio_{ std::make_unique<tuio::TuioInputWrapper>(fwInternal.GetConfig().tuioPort_)},
        elapsedTime_{ 0.0 }
    {
        tuio_->SetAddCursorCallback([this](TUIO::TuioCursor* tcur) { AddTuioCursor(tcur); });
        tuio_->SetUpdateCursorCallback([this](TUIO::TuioCursor* tcur) { UpdateTuioCursor(tcur); });
        tuio_->SetRemoveCursorCallback([this](TUIO::TuioCursor* tcur) { RemoveTuioCursor(tcur); });
    }

    ApplicationNodeInternal::~ApplicationNodeInternal() = default;

    void ApplicationNodeInternal::PreWindow()
    {
        lastFrameTime_ = sgct::Engine::getTime();
        syncInfoLocal_.currentTime_ = lastFrameTime_;

        appNodeImpl_->PreWindow();
    }

    void ApplicationNodeInternal::InitOpenGL()
    {
        appNodeImpl_->InitOpenGL();
    }

    void ApplicationNodeInternal::PreSync()
    {
        appNodeImpl_->PreSync();
    }

    void ApplicationNodeInternal::PostSync()
    {
        syncInfoLocal_ = syncInfoSynced_.getVal();
        appNodeImpl_->UpdateSyncedInfo();

        fwInternal_.GetCamera()->SetPosition(syncInfoLocal_.cameraPosition_);
        fwInternal_.GetCamera()->SetOrientation(syncInfoLocal_.cameraOrientation_);
        fwInternal_.GetCamera()->SetPickMatrix(syncInfoLocal_.pickMatrix_);

        elapsedTime_ = syncInfoLocal_.currentTime_ - lastFrameTime_;
        lastFrameTime_ = syncInfoLocal_.currentTime_;

        GetFramework().CreateSynchronizedResources();
        bool applicationHalted = false;
        applicationHalted = applicationHalted || fwInternal_.GetGPUProgramManager().ProcessResourceWaitList();
        applicationHalted = applicationHalted || fwInternal_.GetTextureManager().ProcessResourceWaitList();
        applicationHalted = applicationHalted || fwInternal_.GetMeshManager().ProcessResourceWaitList();
        GetFramework().SetApplicationHalted(applicationHalted);
        if (GetFramework().GetApplicationHalted()) return;
        appNodeImpl_->UpdateFrame(syncInfoLocal_.currentTime_, elapsedTime_);
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
        appNodeImpl_->CleanUp();
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

    void ApplicationNodeInternal::EncodeData()
    {
#ifdef VISCOM_SYNCINPUT
        sgct::SharedData::instance()->writeVector(&keyboardEventsSynced_);
        sgct::SharedData::instance()->writeVector(&charEventsSynced_);
        sgct::SharedData::instance()->writeVector(&mouseButtonEventsSynced_);
        sgct::SharedData::instance()->writeVector(&mousePosEventsSynced_);
        sgct::SharedData::instance()->writeVector(&mouseScrollEventsSynced_);
#endif
        sgct::SharedData::instance()->writeObj(&syncInfoSynced_);
        appNodeImpl_->EncodeData();
    }

    void ApplicationNodeInternal::DecodeData()
    {
#ifdef VISCOM_SYNCINPUT
        sgct::SharedData::instance()->readVector(&keyboardEventsSynced_);
        sgct::SharedData::instance()->readVector(&charEventsSynced_);
        sgct::SharedData::instance()->readVector(&mouseButtonEventsSynced_);
        sgct::SharedData::instance()->readVector(&mousePosEventsSynced_);
        sgct::SharedData::instance()->readVector(&mouseScrollEventsSynced_);
#endif
        sgct::SharedData::instance()->readObj(&syncInfoSynced_);
        appNodeImpl_->DecodeData();
    }

    void ApplicationNodeInternal::AddTuioCursor(TUIO::TuioCursor* tcur)
    {
        appNodeImpl_->AddTuioCursor(tcur);
    }

    void ApplicationNodeInternal::UpdateTuioCursor(TUIO::TuioCursor* tcur)
    {
        appNodeImpl_->UpdateTuioCursor(tcur);
    }

    void ApplicationNodeInternal::RemoveTuioCursor(TUIO::TuioCursor* tcur)
    {
        appNodeImpl_->RemoveTuioCursor(tcur);
    }
    bool ApplicationNodeInternal::InitialiseVR()
    {
        return false;
    }
    bool ApplicationNodeInternal::CalibrateVR(CalibrateMethod method, TrackedDeviceIdentifier trackedDevice)
    {
        return false;
    }
    const glm::vec3& ApplicationNodeInternal::GetControllerPosition(TrackedDeviceIdentifier trackedDevice)
    {
        return glm::vec3();
    }
    const glm::vec3& ApplicationNodeInternal::GetControllerZVector(TrackedDeviceIdentifier trackedDevice)
    {
        return glm::vec3();
    }
    const glm::quat& ApplicationNodeInternal::GetControllerRotation(TrackedDeviceIdentifier trackedDevice)
    {
        return glm::quat();
    }
    const glm::vec2& ApplicationNodeInternal::GetDisplayPosition(TrackedDeviceIdentifier trackedDevice)
    {
        return glm::vec2();
    }
   
    void ApplicationNodeInternal::ControllerButtonPressedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, glm::vec2 axisvalues)
    {
        appNodeImpl_->ControllerButtonPressedCallback(trackedDevice, buttonid, axisvalues);
    }
    void ApplicationNodeInternal::ControllerButtonTouchedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, glm::vec2 axisvalues)
    {
        appNodeImpl_->ControllerButtonTouchedCallback(trackedDevice, buttonid, axisvalues);
    }
    void ApplicationNodeInternal::ControllerButtonUnpressedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, glm::vec2 axisvalues)
    {
        appNodeImpl_->ControllerButtonUnpressedCallback(trackedDevice, buttonid, axisvalues);
    }
    void ApplicationNodeInternal::ControllerButtonUntouchedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, glm::vec2 axisvalues)
    {
        appNodeImpl_->ControllerButtonUntouchedCallback(trackedDevice, buttonid, axisvalues);
    }

    ControllerButtonState ApplicationNodeInternal::GetControllerButtonState(TrackedDeviceIdentifier trackedDevice)
    {
        return ControllerButtonState();
    }

    /*void ApplicationNodeInternal::ParseTrackingFrame()
    {
        appNodeImpl_->ParseTrackingFrame();
    }
    glm::vec3 ApplicationNodeInternal::GetController0Pos()
    {
        return appNodeImpl_->GetController0Pos();
    }
    glm::vec3 ApplicationNodeInternal::GetController0Zvec()
    {
        return appNodeImpl_->GetController0Zvec();
    }
    glm::vec3 ApplicationNodeInternal::GetController1Pos()
    {
        return appNodeImpl_->GetController1Pos();
    }
    glm::vec3 ApplicationNodeInternal::GetController1Zvec()
    {
        return appNodeImpl_->GetController1Zvec();
    }
    glm::vec3 ApplicationNodeInternal::GetTrackerPos()
    {
        return appNodeImpl_->GetTrackerPos();
    }
    glm::vec3 ApplicationNodeInternal::GetTrackerZvec()
    {
        return appNodeImpl_->GetTrackerZvec();
    }
    glm::quat ApplicationNodeInternal::GetController0Rot()
    {
        return appNodeImpl_->GetController0Rot();
    }
    glm::quat ApplicationNodeInternal::GetController1Rot()
    {
        return appNodeImpl_->GetController1Rot();
    }
    glm::quat ApplicationNodeInternal::GetTrackerRot()
    {
        return appNodeImpl_->GetTrackerRot();
    }
    glm::vec2 ApplicationNodeInternal::GetDisplayPosition(bool useleftcontroller)
    {
        return appNodeImpl_->GetDisplayPosition(useleftcontroller);
    }
    void ApplicationNodeInternal::InitialiseDisplay(bool useLeftController)
    {
        return appNodeImpl_->InitialiseDisplay(useLeftController);
    }
    bool ApplicationNodeInternal::GetDisplayInitialised()
    {
        return appNodeImpl_->GetDisplayInitialised();
    }
    void ApplicationNodeInternal::SetDisplayNotInitialised()
    {
        appNodeImpl_->SetDisplayNotInitialised();
    }
    bool ApplicationNodeInternal::GetDisplayInitByFloor()
    {
        return appNodeImpl_->GetDisplayInitByFloor();
    }
    void ApplicationNodeInternal::SetDisplayInitByFloor(bool b)
    {
        appNodeImpl_->SetDisplayInitByFloor(b);
    }
    void ApplicationNodeInternal::PollAndParseNextEvent()
    {
        appNodeImpl_->PollAndParseNextEvent();
    }
    void ApplicationNodeInternal::PollAndParseEvents()
    {
        appNodeImpl_->PollAndParseEvents();
    }
    std::vector<std::string> ApplicationNodeInternal::OutputDevices()
    {
        return appNodeImpl_->OutputDevices();
    }
    float * ApplicationNodeInternal::GetDisplayEdges()
    {
        return appNodeImpl_->GetDisplayEdges();
    }*/
    /*bool ApplicationNodeInternal::GetVrInitSuccess()
    {
        GetFramework()
    }*/
}
