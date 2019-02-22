/**
 * @file   ApplicationNodeBase.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Implementation of the application node class.
 */

#include "ApplicationNodeBase.h"
#include <imgui.h>
#include "core/FrameworkInternal.h"
//#include <openvr.h>

namespace viscom {

    ApplicationNodeBase::ApplicationNodeBase(ApplicationNodeInternal* appNode) :
        appNode_{ appNode },
        framework_{ &appNode_->GetFramework() }
    {

    }

    ApplicationNodeBase::~ApplicationNodeBase() = default;
 
    void ApplicationNodeBase::PreWindow()
    {
    }

    void ApplicationNodeBase::InitOpenGL()
    {
    }

    void ApplicationNodeBase::PreSync()
    {
    }

    void ApplicationNodeBase::UpdateSyncedInfo()
    {
    }

    void ApplicationNodeBase::UpdateFrame(double, double)
    {
    }

    void ApplicationNodeBase::ClearBuffer(FrameBuffer&)
    {
    }

    void ApplicationNodeBase::DrawFrame(FrameBuffer&)
    {
    }

    void ApplicationNodeBase::Draw2D(FrameBuffer& fbo)
    {
    }

    void ApplicationNodeBase::CleanUp()
    {
    }

    bool ApplicationNodeBase::DataTransferCallback(void * receivedData, int receivedLength, std::uint16_t packageID, int clientID)
    {
        return false;
    }

    bool ApplicationNodeBase::DataAcknowledgeCallback(std::uint16_t packageID, int clientID)
    {
        return false;
    }

    bool ApplicationNodeBase::DataTransferStatusCallback(bool connected, int clientID)
    {
        return false;
    }

    bool ApplicationNodeBase::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        return false;
    }

    bool ApplicationNodeBase::CharCallback(unsigned int character, int mods)
    {
        return false;
    }

    bool ApplicationNodeBase::MouseButtonCallback(int button, int action)
    {
        return false;
    }

    bool ApplicationNodeBase::MousePosCallback(double x, double y)
    {
        return false;
    }

    bool ApplicationNodeBase::MouseScrollCallback(double xoffset, double yoffset)
    {
        return false;
    }

    bool ApplicationNodeBase::AddTuioCursor(TUIO::TuioCursor* tcur)
    {
        return false;
    }

    bool ApplicationNodeBase::UpdateTuioCursor(TUIO::TuioCursor* tcur)
    {
        return false;
    }

    bool ApplicationNodeBase::RemoveTuioCursor(TUIO::TuioCursor* tcur)
    {
        return false;
    }

    void ApplicationNodeBase::EncodeData()
    {
    }

    void ApplicationNodeBase::DecodeData()
    {
    }

    void ApplicationNodeBase::Terminate() const
    {
        framework_->Terminate();
    }
    
    bool ApplicationNodeBase::ControllerButtonPressedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues)
    {
        return false;
    }

    bool ApplicationNodeBase::ControllerButtonTouchedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues)
    {
        return false;
    }

    bool ApplicationNodeBase::ControllerButtonPressReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues)
    {
        return false;
    }

    bool ApplicationNodeBase::ControllerButtonTouchReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues)
    {
        return false;
    }

    void ApplicationNodeBase::GetControllerButtonState(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2& axisvalues, ovr::ButtonState& buttonstate) const
    {
        return appNode_->GetControllerButtonState(trackedDeviceId, buttonid, axisvalues, buttonstate);
    }
}
