/**
 * @file   WorkerNodeLocalInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.06.15
 *
 * @brief  Implementation of the ApplicationNodeInternal for general workers.
 */

#include "core/main.h"
#include <sgct.h>
#include "WorkerNodeLocalInternal.h"
#include "core/imgui/imgui_impl_glfw_gl3.h"
#include "core/open_gl.h"
#include "core/app/ApplicationNodeBase.h"

namespace viscom {

    WorkerNodeLocalInternal::WorkerNodeLocalInternal(FrameworkInternal& fwInternal) :
        ApplicationNodeInternal{ fwInternal }
    {
    }

    WorkerNodeLocalInternal::~WorkerNodeLocalInternal() = default;

    void WorkerNodeLocalInternal::PreWindow()
    {
        SetApplicationNode(GetFramework().GetWorkerNodeFactory()(this));
        ApplicationNodeInternal::PreWindow();
    }


    void WorkerNodeLocalInternal::InitOpenGL()
    {
        if constexpr (SHOW_CLIENT_GUI) {
            // Setup ImGui binding
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui_ImplGlfwGL3_Init(GetFramework().GetEngine()->getCurrentWindowPtr()->getWindowHandle(), !GetFramework().GetEngine()->isMaster() && SHOW_CLIENT_MOUSE_CURSOR);
        }

        ApplicationNodeInternal::InitOpenGL();
    }

    void WorkerNodeLocalInternal::PostSync()
    {
#ifdef VISCOM_SYNCINPUT
        {
            auto keybEvts = keyboardEventsSynced_.getVal();
            keyboardEventsSynced_.clear();
            for (const auto k : keybEvts) ApplicationNodeInternal::KeyboardCallback(k.key_, k.scancode_, k.action_, k.mods_);
        }
        {
            auto charEvts = charEventsSynced_.getVal();
            charEventsSynced_.clear();
            for (const auto c : charEvts) ApplicationNodeInternal::CharCallback(c.character_, c.mods_);
        }
        {
            auto mBtnEvts = mouseButtonEventsSynced_.getVal();
            mouseButtonEventsSynced_.clear();
            for (const auto b : mBtnEvts) ApplicationNodeInternal::MouseButtonCallback(b.button_, b.action_);
        }
        {
            auto mPosEvts = mousePosEventsSynced_.getVal();
            mousePosEventsSynced_.clear();
            for (const auto p : mPosEvts) ApplicationNodeInternal::MousePosCallback(p.x_, p.y_);
        }
        {
            auto mScrlEvts = mouseScrollEventsSynced_.getVal();
            mouseScrollEventsSynced_.clear();
            for (const auto s : mScrlEvts) ApplicationNodeInternal::MouseScrollCallback(s.xoffset_, s.yoffset_);
        }
#endif

        ApplicationNodeInternal::PostSync();
    }

    void WorkerNodeLocalInternal::Draw2D(FrameBuffer& fbo)
    {
        auto windowId = GetFramework().GetEngine()->getCurrentWindowPtr()->getId();
        if constexpr (SHOW_CLIENT_GUI)ImGui_ImplGlfwGL3_NewFrame(-GetFramework().GetViewportScreen(windowId).position_,
            GetFramework().GetViewportQuadSize(windowId), GetFramework().GetViewportScreen(windowId).size_,
            GetFramework().GetViewportScaling(windowId), GetCurrentAppTime(), GetElapsedTime());

        ApplicationNodeInternal::Draw2D(fbo);

        if constexpr (SHOW_CLIENT_GUI) {
            fbo.DrawToFBO([this]() {
                ImGui::Render();
                ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
            });
        }
    }

    void WorkerNodeLocalInternal::PostDraw()
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfwGL3_FinishAllFrames();
        ApplicationNodeInternal::PostDraw();
    }

    void WorkerNodeLocalInternal::CleanUp()
    {
        if constexpr (SHOW_CLIENT_GUI) {
            ImGui_ImplGlfwGL3_Shutdown();
            ImGui::DestroyContext();
        }
        ApplicationNodeInternal::CleanUp();
    }

    void WorkerNodeLocalInternal::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfwGL3_KeyCallback(key, scancode, action, mods);
    }

    void WorkerNodeLocalInternal::CharCallback(unsigned int character, int mods)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfwGL3_CharCallback(character);
    }

    void WorkerNodeLocalInternal::MouseButtonCallback(int button, int action)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfwGL3_MouseButtonCallback(button, action, 0);
    }

    void WorkerNodeLocalInternal::MousePosCallback(double x, double y)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfwGL3_MousePositionCallback(x, y);
    }

    void WorkerNodeLocalInternal::MouseScrollCallback(double xoffset, double yoffset)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfwGL3_ScrollCallback(xoffset, yoffset);
    }

    void WorkerNodeLocalInternal::AddTuioCursor(TUIO::TuioCursor* tcur)
    {
    }

    void WorkerNodeLocalInternal::UpdateTuioCursor(TUIO::TuioCursor* tcur)
    {
    }

    void WorkerNodeLocalInternal::RemoveTuioCursor(TUIO::TuioCursor* tcur)
    {
    }

}
