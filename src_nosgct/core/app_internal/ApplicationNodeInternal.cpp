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
PUSH_DISABLE_DEPRECATED_WARNINGS
        appNodeImpl_->PreWindow();
        appNodeImpl_->InitOpenGL();
POP_WARNINGS
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

    bool ApplicationNodeInternal::ControllerButtonPressedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues)
    {
        return appNodeImpl_->ControllerButtonPressedCallback(trackedDeviceId, buttonid, axisvalues);
    }

    bool ApplicationNodeInternal::ControllerButtonTouchedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues)
    {
        return appNodeImpl_->ControllerButtonTouchedCallback(trackedDeviceId, buttonid, axisvalues);
    }

    bool ApplicationNodeInternal::ControllerButtonPressReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues)
    {
        return appNodeImpl_->ControllerButtonPressReleasedCallback(trackedDeviceId, buttonid, axisvalues);
    }

    bool ApplicationNodeInternal::ControllerButtonTouchReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues)
    {
        return appNodeImpl_->ControllerButtonTouchReleasedCallback(trackedDeviceId, buttonid, axisvalues);
    }

}
