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
#include "core/app/TuioInputWrapper.h"
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
        lastFrameTime_ = sgct::Engine::getTime();
        syncInfoLocal_.currentTime_ = lastFrameTime_;
    }

    ApplicationNodeInternal::~ApplicationNodeInternal()
    {
PUSH_DISABLE_DEPRECATED_WARNINGS
        appNodeImpl_->CleanUp();
POP_WARNINGS

        appNodeImpl_ = nullptr;
    }

    void ApplicationNodeInternal::PreWindow()
    {
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
        if (appNodeImpl_) appNodeImpl_->DataTransferCallback(receivedData, receivedLength, packageID, clientID);
    }

    void ApplicationNodeInternal::DataAcknowledge(std::uint16_t packageID, int clientID)
    {
        if (appNodeImpl_) appNodeImpl_->DataAcknowledgeCallback(packageID, clientID);
    }

    void ApplicationNodeInternal::DataTransferStatus(bool connected, int clientID)
    {
        if (appNodeImpl_) appNodeImpl_->DataTransferStatusCallback(connected, clientID);
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
    bool ApplicationNodeInternal::InitialiseDisplayVR()
    {
        return false;
    }
    bool ApplicationNodeInternal::CalibrateVR(ovr::CalibrateMethod)
    {
        return false;
    }
    const std::vector<ovr::DeviceInfo>& ApplicationNodeInternal::GetConnectedDevices() const
    {
        return noDeviceInfo_;
    }
    const glm::vec3& ApplicationNodeInternal::GetControllerPosition(std::uint32_t ) const
    {
        return zeroVec3_;
    }
    const glm::vec3& ApplicationNodeInternal::GetControllerDirection(std::uint32_t) const
    {
        return zeroVec3_;
    }
    const glm::quat& ApplicationNodeInternal::GetControllerOrientation(std::uint32_t) const
    {
        return identityQuat_;
    }
    const glm::vec2& ApplicationNodeInternal::GetDisplayPointerPosition(std::uint32_t) const
    {
        return zeroVec2_;
    }

    bool ApplicationNodeInternal::ControllerButtonPressedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid)
    {
        return appNodeImpl_->ControllerButtonPressedCallback(trackedDeviceId, buttonid);
    }

    bool ApplicationNodeInternal::ControllerButtonTouchedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid)
    {
        return appNodeImpl_->ControllerButtonTouchedCallback(trackedDeviceId, buttonid);
    }

    bool ApplicationNodeInternal::ControllerButtonPressReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid)
    {
        return appNodeImpl_->ControllerButtonPressReleasedCallback(trackedDeviceId, buttonid);
    }

    bool ApplicationNodeInternal::ControllerButtonTouchReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid)
    {
        return appNodeImpl_->ControllerButtonTouchReleasedCallback(trackedDeviceId, buttonid);
    }

    void ApplicationNodeInternal::GetControllerButtonState(std::uint32_t, std::size_t, glm::vec2&, ovr::ButtonState&) const
    {
    }

    std::vector<std::string> ApplicationNodeInternal::OutputDevices() const
    {
        return std::vector<std::string>();
    }

}
