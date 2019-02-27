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

    void ApplicationNodeBase::Draw2D(FrameBuffer&)
    {
    }

    void ApplicationNodeBase::CleanUp()
    {
    }

    bool ApplicationNodeBase::DataTransferCallback(void*, int, std::uint16_t, int)
    {
        return false;
    }

    bool ApplicationNodeBase::DataAcknowledgeCallback(std::uint16_t, int)
    {
        return false;
    }

    bool ApplicationNodeBase::DataTransferStatusCallback(bool, int)
    {
        return false;
    }

    bool ApplicationNodeBase::KeyboardCallback(int, int, int, int)
    {
        return false;
    }

    bool ApplicationNodeBase::CharCallback(unsigned int, int)
    {
        return false;
    }

    bool ApplicationNodeBase::MouseButtonCallback(int, int)
    {
        return false;
    }

    bool ApplicationNodeBase::MousePosCallback(double, double)
    {
        return false;
    }

    bool ApplicationNodeBase::MouseScrollCallback(double, double)
    {
        return false;
    }

    bool ApplicationNodeBase::AddTuioCursor(TUIO::TuioCursor*)
    {
        return false;
    }

    bool ApplicationNodeBase::UpdateTuioCursor(TUIO::TuioCursor*)
    {
        return false;
    }

    bool ApplicationNodeBase::RemoveTuioCursor(TUIO::TuioCursor*)
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
}
