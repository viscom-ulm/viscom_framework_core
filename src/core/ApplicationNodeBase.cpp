/**
 * @file   ApplicationNodeBase.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Implementation of the application node class.
 */

#include "ApplicationNodeBase.h"
#include <imgui.h>
#include "core/gfx/mesh/MeshRenderable.h"
#include "core/imgui/imgui_impl_glfw_gl3.h"

namespace viscom {

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
        keyPressedState_.resize(GLFW_KEY_LAST, false);
        mousePressedState_.resize(GLFW_MOUSE_BUTTON_LAST, false);
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

    bool ApplicationNodeBase::IsMouseButtonPressed(int button) const noexcept
    {
        return mousePressedState_[button];
    }

    bool ApplicationNodeBase::IsKeyPressed(int key) const noexcept
    {
        return keyPressedState_[key];
    }

    // ReSharper disable CppParameterNeverUsed
    bool ApplicationNodeBase::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        keyPressedState_[key] = (action == GLFW_RELEASE) ? false : true;

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
        mousePressedState_[button] = (action == GLFW_RELEASE) ? false : true;

#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_MouseButtonCallback(button, action, 0);
        if (ImGui::GetIO().WantCaptureMouse) return true;
#endif
        return false;
    }

    bool ApplicationNodeBase::MousePosCallback(double x, double y)
    {
        mousePosition_ = glm::vec2(x, y);
        mousePositionNormalized_.x = (2.0f * mousePosition_.x - 1.0f);
        mousePositionNormalized_.y = -(2.0f * mousePosition_.y - 1.0f);
        mousePositionNormalized_.z = 0.0f;
        mousePositionNormalized_ = glm::clamp(mousePositionNormalized_, glm::vec3(-1.0f), glm::vec3(1.0f));

        float length_squared = glm::dot(mousePositionNormalized_, mousePositionNormalized_);
        if (length_squared <= 1.0f) mousePositionNormalized_.z = sqrtf(1.0f - length_squared);
        else mousePositionNormalized_ = glm::normalize(mousePositionNormalized_);

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
}
