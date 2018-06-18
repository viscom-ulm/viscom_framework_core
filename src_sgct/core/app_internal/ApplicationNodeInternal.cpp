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
#include "core/imgui/imgui_impl_glfw_gl3.h"
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
}
