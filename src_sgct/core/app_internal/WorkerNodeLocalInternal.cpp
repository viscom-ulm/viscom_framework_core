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
#include <imgui.h>
#include "core/imgui/imgui_impl_glfw.h"
#include "core/imgui/imgui_impl_opengl3.h"
#include "core/open_gl.h"
#include "core/app/ApplicationNodeBase.h"

namespace viscom {

    WorkerNodeLocalInternal::WorkerNodeLocalInternal(FrameworkInternal& fwInternal) :
        ApplicationNodeInternal{ fwInternal }
    {
        if constexpr (SHOW_CLIENT_GUI) {
            // Setup ImGui binding
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui_ImplGlfw_InitForOpenGL(GetFramework().GetEngine()->getCurrentWindowPtr()->getWindowHandle(), !GetFramework().GetEngine()->isMaster() && SHOW_CLIENT_MOUSE_CURSOR);
            ImGui_ImplOpenGL3_Init();

            ImGui::StyleColorsDark();
        }
    }

    WorkerNodeLocalInternal::~WorkerNodeLocalInternal()
    {
        if constexpr (SHOW_CLIENT_GUI) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }

    void WorkerNodeLocalInternal::InitImplementation()
    {
        SetApplicationNode(GetFramework().GetWorkerNodeFactory()(this));
PUSH_DISABLE_DEPRECATED_WARNINGS
        GetApplicationNode()->PreWindow();
        GetApplicationNode()->InitOpenGL();
POP_WARNINGS
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
        if constexpr (SHOW_CLIENT_GUI) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame(-GetFramework().GetViewportScreen(windowId).position_,
                GetFramework().GetViewportQuadSize(windowId), GetFramework().GetViewportScreen(windowId).size_,
                GetFramework().GetViewportScaling(windowId), GetCurrentAppTime(), GetElapsedTime());
            ImGui::NewFrame();
        }

        ApplicationNodeInternal::Draw2D(fbo);

        if constexpr (SHOW_CLIENT_GUI) {
            fbo.DrawToFBO([this]() {
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            });
        }
    }

    void WorkerNodeLocalInternal::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfw_KeyCallback(key, scancode, action, mods);
    }

    void WorkerNodeLocalInternal::CharCallback(unsigned int character, int mods)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfw_CharCallback(character);
    }

    void WorkerNodeLocalInternal::MouseButtonCallback(int button, int action)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfw_MouseButtonCallback(button, action, 0);
    }

    void WorkerNodeLocalInternal::MousePosCallback(double x, double y)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfw_MousePositionCallback(x, y);
    }

    void WorkerNodeLocalInternal::MouseScrollCallback(double xoffset, double yoffset)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfw_ScrollCallback(xoffset, yoffset);
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
