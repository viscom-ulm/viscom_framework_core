/**
 * @file   MasterNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the master application node.
 */

#include "MasterNode.h"
#include <imgui.h>
#include "core/imgui/imgui_impl_glfw_gl3.h"

namespace viscom {

    MasterNode::MasterNode(ApplicationNode* appNode) :
        ApplicationNodeImplementation{ appNode }
    {
    }


    MasterNode::~MasterNode() = default;


    void MasterNode::InitOpenGL()
    {
        ApplicationNodeImplementation::InitOpenGL();

        ImGui_ImplGlfwGL3_Init(GetEngine()->getCurrentWindowPtr()->getWindowHandle(), false);

    }

    void MasterNode::PreSync()
    {
        ApplicationNodeImplementation::PreSync();
    }

    void MasterNode::DrawFrame()
    {
        ApplicationNodeImplementation::DrawFrame();

        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::ShowTestWindow();

        ImGui::Render();
    }

    void MasterNode::CleanUp()
    {
        ImGui_ImplGlfwGL3_Shutdown();
        ApplicationNodeImplementation::CleanUp();
    }

    void MasterNode::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        ImGui_ImplGlfwGL3_KeyCallback(GetEngine()->getCurrentWindowPtr()->getWindowHandle(), key, scancode, action, mods);
    }

    void MasterNode::CharCallback(unsigned character, int mods)
    {
        ImGui_ImplGlfwGL3_CharCallback(GetEngine()->getCurrentWindowPtr()->getWindowHandle(), character);
    }

    void MasterNode::MouseButtonCallback(int button, int action)
    {
        ImGui_ImplGlfwGL3_MouseButtonCallback(GetEngine()->getCurrentWindowPtr()->getWindowHandle(), button, action, 0);
    }

    void MasterNode::MouseScrollCallback(double xoffset, double yoffset)
    {
        ImGui_ImplGlfwGL3_ScrollCallback(GetEngine()->getCurrentWindowPtr()->getWindowHandle(), xoffset, yoffset);
    }

}
