/**
 * @file   CoordinatorNodeInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.06.15
 *
 * @brief  Implementation of the internal coordinator node.
 */

#include "core/main.h"
#include <sgct.h>
#include "CoordinatorNodeInternal.h"
#include "core/imgui/imgui_impl_glfw_gl3.h"
#include <imgui.h>
#include "core/app/ApplicationNodeBase.h"

namespace viscom {

    CoordinatorNodeInternal::CoordinatorNodeInternal(FrameworkInternal& fwInternal) :
        ApplicationNodeInternal{ fwInternal }
    {
    }

    CoordinatorNodeInternal::~CoordinatorNodeInternal() = default;

    void CoordinatorNodeInternal::PreWindow()
    {
        SetApplicationNode(GetFramework().GetCoordinatorNodeFactory()(this));
        ApplicationNodeInternal::PreWindow();
    }

    void CoordinatorNodeInternal::InitOpenGL()
    {
        // Setup ImGui binding
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui_ImplGlfwGL3_Init(GetFramework().GetEngine()->getCurrentWindowPtr()->getWindowHandle(), !GetFramework().GetEngine()->isMaster() && SHOW_CLIENT_MOUSE_CURSOR);

        ApplicationNodeInternal::InitOpenGL();
    }

    void CoordinatorNodeInternal::PreSync()
    {
#ifdef VISCOM_SYNCINPUT
        {
            std::vector<KeyboardEvent> keybEvts;
            keybEvts.swap(keyboardEvents_);
            keyboardEventsSynced_.setVal(keybEvts);
        }

        {
            std::vector<CharEvent> charEvts;
            charEvts.swap(charEvents_);
            charEventsSynced_.setVal(charEvts);
        }

        {
            std::vector<MouseButtonEvent> mBtnEvts;
            mBtnEvts.swap(mouseButtonEvents_);
            mouseButtonEventsSynced_.setVal(mBtnEvts);
        }

        {
            std::vector<MousePosEvent> mPosEvts;
            mPosEvts.swap(mousePosEvents_);
            mousePosEventsSynced_.setVal(mPosEvts);
        }

        {
            std::vector<MouseScrollEvent> mScrlEvts;
            mScrlEvts.swap(mouseScrollEvents_);
            mouseScrollEventsSynced_.setVal(mScrlEvts);
        }
#endif
        syncInfoLocal_.currentTime_ = sgct::Engine::getTime();
        syncInfoLocal_.cameraPosition_ = GetFramework().GetCamera()->GetPosition();
        syncInfoLocal_.cameraOrientation_ = GetFramework().GetCamera()->GetOrientation();

        glm::vec2 relProjectorPos = glm::vec2(GetFramework().GetViewportScreen(0).position_) / glm::vec2(GetFramework().GetViewportScreen(0).size_);
        glm::vec2 relQuadSize = glm::vec2(GetFramework().GetViewportQuadSize(0)) / glm::vec2(GetFramework().GetViewportScreen(0).size_);
        glm::vec2 relProjectorSize = 1.0f / relQuadSize;

        syncInfoLocal_.pickMatrix_ = glm::mat4{ 1.0f };
        syncInfoLocal_.pickMatrix_[0][0] = 2.0f * relProjectorSize.x;
        syncInfoLocal_.pickMatrix_[1][1] = -2.0f * relProjectorSize.y;
        syncInfoLocal_.pickMatrix_[3][0] = (-2.0f * relProjectorPos.x * relProjectorSize.x) - 1.0f;
        syncInfoLocal_.pickMatrix_[3][1] = (-2.0f * relProjectorPos.y * relProjectorSize.y) + 1.0f;
        syncInfoLocal_.pickMatrix_[3][2] = 1.0f;
        syncInfoLocal_.pickMatrix_[3][3] = 1.0f;
        syncInfoLocal_.pickMatrix_ = glm::inverse(GetFramework().GetCamera()->GetCentralViewPerspectiveMatrix()) * syncInfoLocal_.pickMatrix_;

        syncInfoSynced_.setVal(syncInfoLocal_);

        ApplicationNodeInternal::PreSync();
    }

    void CoordinatorNodeInternal::Draw2D(FrameBuffer& fbo)
    {
        auto windowId = GetFramework().GetEngine()->getCurrentWindowPtr()->getId();
        ImGui_ImplGlfwGL3_NewFrame(-GetFramework().GetViewportScreen(windowId).position_,
            GetFramework().GetViewportQuadSize(windowId), GetFramework().GetViewportScreen(windowId).size_,
            GetFramework().GetViewportScaling(windowId), GetCurrentAppTime(), GetElapsedTime());

        ApplicationNodeInternal::Draw2D(fbo);

        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void CoordinatorNodeInternal::PostDraw()
    {
        ImGui_ImplGlfwGL3_FinishAllFrames();
        ApplicationNodeInternal::PostDraw();
    }

    void CoordinatorNodeInternal::CleanUp()
    {
        ImGui_ImplGlfwGL3_Shutdown();
        ImGui::DestroyContext();
        ApplicationNodeInternal::CleanUp();
    }

    void CoordinatorNodeInternal::KeyboardCallback(int key, int scancode, int action, int mods)
    {
#ifdef VISCOM_SYNCINPUT
        keyboardEvents_.emplace_back(key, scancode, action, mods);
#endif

        ImGui_ImplGlfwGL3_KeyCallback(key, scancode, action, mods);
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        ApplicationNodeInternal::KeyboardCallback(key, scancode, action, mods);
    }

    void CoordinatorNodeInternal::CharCallback(unsigned int character, int mods)
    {
#ifdef VISCOM_SYNCINPUT
        charEvents_.emplace_back(character, mods);
#endif

        ImGui_ImplGlfwGL3_CharCallback(character);
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        ApplicationNodeInternal::CharCallback(character, mods);
    }

    void CoordinatorNodeInternal::MouseButtonCallback(int button, int action)
    {
#ifdef VISCOM_SYNCINPUT
        mouseButtonEvents_.emplace_back(button, action);
#endif

        ImGui_ImplGlfwGL3_MouseButtonCallback(button, action, 0);
        if (ImGui::GetIO().WantCaptureMouse) return;

        ApplicationNodeInternal::MouseButtonCallback(button, action);
    }

    void CoordinatorNodeInternal::MousePosCallback(double x, double y)
    {
#ifdef VISCOM_SYNCINPUT
        mousePosEvents_.emplace_back(x, y);
#endif

        ImGui_ImplGlfwGL3_MousePositionCallback(x, y);
        if (ImGui::GetIO().WantCaptureMouse) return;

        ApplicationNodeInternal::MousePosCallback(x, y);
    }

    void CoordinatorNodeInternal::MouseScrollCallback(double xoffset, double yoffset)
    {
#ifdef VISCOM_SYNCINPUT
        mouseScrollEvents_.emplace_back(xoffset, yoffset);
#endif

        ImGui_ImplGlfwGL3_ScrollCallback(xoffset, yoffset);
        if (ImGui::GetIO().WantCaptureMouse) return;

        ApplicationNodeInternal::MouseScrollCallback(xoffset, yoffset);
    }

    void CoordinatorNodeInternal::AddTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (!GetFramework().IsInitialized()) return;
#ifdef VISCOM_USE_TUIO
        // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
        ApplicationNodeInternal::AddTuioCursor(tcur);
#endif
    }

    void CoordinatorNodeInternal::UpdateTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (!GetFramework().IsInitialized()) return;
#ifdef VISCOM_USE_TUIO
        // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
        ApplicationNodeInternal::UpdateTuioCursor(tcur);
#endif
    }

    void CoordinatorNodeInternal::RemoveTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (!GetFramework().IsInitialized()) return;
#ifdef VISCOM_USE_TUIO
        // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
        ApplicationNodeInternal::RemoveTuioCursor(tcur);
#endif
    }

}
