/**
 * @file   ApplicationNodeBase.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Implementation of the application node class.
 */

#include "ApplicationNodeBase.h"
#include "core/imgui/imgui_impl_glfw_gl3.h"
#include "core/open_gl.h"
#include <imgui.h>

namespace viscom {

#ifndef VISCOM_LOCAL_ONLY

        int SGCTEngineWrapper::GetCurrentWindowId() const
        {
            return engine_->getCurrentWindowPtr()->getId();
        }

        void SGCTEngineWrapper::UnbindCurrentWindowFBO() const
        {
            engine_->getCurrentWindowPtr()->getFBOPtr()->unBind();
        }

        void SGCTEngineWrapper::SetProjectionPlaneCoordinate(std::size_t windowIdx, std::size_t vpIndex, std::size_t corner, glm::vec3 coordinate) const
        {
            engine_->getWindowPtr(windowIdx)->getViewport(vpIndex)->getProjectionPlane()->setCoordinate(corner, coordinate);
        }

#endif

    ApplicationNodeBase::ApplicationNodeBase(ApplicationNodeInternal* appNode) :
        appNode_{ appNode }
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

    void ApplicationNodeBase::PostDraw()
    {
    }

    void ApplicationNodeBase::CleanUp()
    {
    }

    bool ApplicationNodeBase::DataTransferCallback(void * receivedData, int receivedLength, int packageID, int clientID)
    {
        return false;
    }

    bool ApplicationNodeBase::DataAcknowledgeCallback(int packageID, int clientID)
    {
        return false;
    }

    bool ApplicationNodeBase::DataTransferStatusCallback(bool connected, int clientID)
    {
        return false;
    }

    // ReSharper disable CppParameterNeverUsed
    bool ApplicationNodeBase::KeyboardCallback(int key, int scancode, int action, int mods)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_KeyCallback(key, scancode, action, mods);
        if (ImGui::GetIO().WantCaptureKeyboard) return true;
#endif
        return false;
    }

    bool ApplicationNodeBase::CharCallback(unsigned int character, int mods)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_CharCallback(character);
        if (ImGui::GetIO().WantCaptureKeyboard) return true;
#endif
        return false;
    }

    bool ApplicationNodeBase::MouseButtonCallback(int button, int action)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_MouseButtonCallback(button, action, 0);
        if (ImGui::GetIO().WantCaptureMouse) return true;
#endif
        return false;
    }

    bool ApplicationNodeBase::MousePosCallback(double x, double y)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_MousePositionCallback(x, y);
        if (ImGui::GetIO().WantCaptureMouse) return true;
#endif
        return false;
    }

    bool ApplicationNodeBase::MouseScrollCallback(double xoffset, double yoffset)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_ScrollCallback(xoffset, yoffset);
        if (ImGui::GetIO().WantCaptureMouse) return true;
#endif
        return false;
    }

    // ReSharper restore CppParameterNeverUsed

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
        appNode_->Terminate();
    }
}
