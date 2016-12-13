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
    }

    void MasterNode::PreSync()
    {
        ApplicationNodeImplementation::PreSync();
    }

    void MasterNode::DrawFrame()
    {
        ApplicationNodeImplementation::DrawFrame();
    }

    void MasterNode::Draw2D()
    {
        auto window = GetEngine()->getCurrentWindowPtr();

        ImGui_ImplGlfwGL3_NewFrame(-GetViewportOrigin(window->getId()), GetViewport(window->getId()).second, GetViewportScaling(window->getId()));

        ImGui::ShowTestWindow();

        ImGui::Render();

        ApplicationNodeImplementation::Draw2D();
    }

    void MasterNode::CleanUp()
    {
        ApplicationNodeImplementation::CleanUp();
    }

    void MasterNode::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        ApplicationNodeImplementation::KeyboardCallback(key, scancode, action, mods);
    }

    void MasterNode::CharCallback(unsigned character, int mods)
    {
        ApplicationNodeImplementation::CharCallback(character, mods);
    }

    void MasterNode::MouseButtonCallback(int button, int action)
    {
        ApplicationNodeImplementation::MouseButtonCallback(button, action);
    }

    void MasterNode::MouseScrollCallback(double xoffset, double yoffset)
    {
        ApplicationNodeImplementation::MouseScrollCallback(xoffset, yoffset);
    }

}
