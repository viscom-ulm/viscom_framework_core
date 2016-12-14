/**
 * @file   ApplicationNodeImplementation.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Implementation of the application node class.
 */

#include "ApplicationNodeImplementation.h"
#include "Vertices.h"
#include <imgui.h>
#include "core/imgui/imgui_impl_glfw_gl3.h"

namespace viscom {

    ApplicationNodeImplementation::ApplicationNodeImplementation(ApplicationNode* appNode) :
        appNode_{ appNode }
    {
    }


    ApplicationNodeImplementation::~ApplicationNodeImplementation() = default;

    void ApplicationNodeImplementation::PreWindow()
    {
    }

    void ApplicationNodeImplementation::InitOpenGL()
    {
        const auto SHADER_PATH = GetConfig().baseDirectory_ + "/shader/";
        if (!sgct::ShaderManager::instance()->shaderProgramExists("backgroundGrid")) {
            sgct::ShaderManager::instance()->addShaderProgram(backgroundProgram_, "backgroundGrid", SHADER_PATH + "backgroundGrid.vert", SHADER_PATH + "backgroundGrid.frag");
            sgct::ShaderManager::instance()->bindShaderProgram(backgroundProgram_);
            sgct::ShaderManager::instance()->unBindShaderProgram();
        } else backgroundProgram_ = sgct::ShaderManager::instance()->getShaderProgram("backgroundGrid");

        backgroundMVPLoc_ = backgroundProgram_.getUniformLocation("MVP");


        if (!sgct::ShaderManager::instance()->shaderProgramExists("foregroundTriangle")) {
            sgct::ShaderManager::instance()->addShaderProgram(triangleProgram_, "foregroundTriangle", SHADER_PATH + "foregroundTriangle.vert", SHADER_PATH + "foregroundTriangle.frag");
            sgct::ShaderManager::instance()->bindShaderProgram(triangleProgram_);
            sgct::ShaderManager::instance()->unBindShaderProgram();
        } else triangleProgram_ = sgct::ShaderManager::instance()->getShaderProgram("foregroundTriangle");

        triangleMVPLoc_ = triangleProgram_.getUniformLocation("MVP");

        std::vector<GridVertex> gridVertices;

        auto delta = 0.125f;
        for (auto x = -1.0f; x < 1.0f; x += delta) {
            auto green = (x + 1.0f) / 2.0f;

            for (float y = -1.0; y < 1.0; y += delta) {
                auto red = (y + 1.0f) / 2.0f;

                auto dx = 0.004f;
                auto dy = 0.004f;

                gridVertices.emplace_back(glm::vec3(x + dx, y + dy, -1.0f), glm::vec4(red, green, 0.0f, 1.0f));//right top
                gridVertices.emplace_back(glm::vec3(x - dx + delta, y + dy, -1.0f), glm::vec4(red, green, 0.0f, 1.0f));//left top
                gridVertices.emplace_back(glm::vec3(x - dx + delta, y - dy + delta, -1.0f), glm::vec4(red, green, 0.0f, 1.0f));//left bottom

                gridVertices.emplace_back(glm::vec3(x - dx + delta, y - dy + delta, -1.0f), glm::vec4(red, green, 0.0f, 1.0f));//left bottom
                gridVertices.emplace_back(glm::vec3(x + dx, y - dy + delta, -1.0f), glm::vec4(red, green, 0.0f, 1.0f));//right bottom
                gridVertices.emplace_back(glm::vec3(x + dx, y + dy, -1.0f), glm::vec4(red, green, 0.0f, 1.0f));//right top
            }
        }

        numBackgroundVertices_ = static_cast<unsigned int>(gridVertices.size());

        gridVertices.emplace_back(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        gridVertices.emplace_back(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        gridVertices.emplace_back(glm::vec3(0.5f, -0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));


        glGenBuffers(1, &vboBackgroundGrid_);
        glBindBuffer(GL_ARRAY_BUFFER, vboBackgroundGrid_);
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(GridVertex), gridVertices.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &vaoBackgroundGrid_);
        glBindVertexArray(vaoBackgroundGrid_);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GridVertex), reinterpret_cast<GLvoid*>(offsetof(GridVertex, position_)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GridVertex), reinterpret_cast<GLvoid*>(offsetof(GridVertex, color_)));
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void ApplicationNodeImplementation::PreSync()
    {
    }

    void ApplicationNodeImplementation::UpdateSyncedInfo()
    {
    }

    void ApplicationNodeImplementation::UpdateFrame(double currentTime, double elapsedTime)
    {
        triangleModelMatrix_ = glm::rotate(glm::mat4(1.0f), static_cast<float>(currentTime), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void ApplicationNodeImplementation::ClearBuffer()
    {
        auto colorPtr = sgct::Engine::instance()->getClearColor();
        glClearColor(colorPtr[0], colorPtr[1], colorPtr[2], colorPtr[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void ApplicationNodeImplementation::DrawFrame()
    {
        glBindVertexArray(vaoBackgroundGrid_);
        glBindBuffer(GL_ARRAY_BUFFER, vboBackgroundGrid_);

        auto MVP = GetEngine()->getCurrentModelViewProjectionMatrix();
        {
            sgct::ShaderManager::instance()->bindShaderProgram(backgroundProgram_);
            glUniformMatrix4fv(backgroundMVPLoc_, 1, GL_FALSE, glm::value_ptr(MVP));
            glDrawArrays(GL_TRIANGLES, 0, numBackgroundVertices_);
        }

        {
            MVP *= triangleModelMatrix_;
            sgct::ShaderManager::instance()->bindShaderProgram(triangleProgram_);
            glUniformMatrix4fv(triangleMVPLoc_, 1, GL_FALSE, glm::value_ptr(MVP));
            glDrawArrays(GL_TRIANGLES, numBackgroundVertices_, 3);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        sgct::ShaderManager::instance()->unBindShaderProgram();
    }

    void ApplicationNodeImplementation::Draw2D()
    {
        auto window = GetEngine()->getCurrentWindowPtr();

#ifdef VISCOM_CLIENTGUI
        // ImGui_ImplGlfwGL3_NewFrame(-GetViewportOrigin(window->getId()), GetViewportSize(window->getId()), GetViewportScaling(window->getId()), GetCurrentAppTime(), GetElapsedTime());

        ImGui::ShowTestWindow();

        // ImGui::Render();
#endif
    }

    void ApplicationNodeImplementation::PostDraw()
    {
    }

    void ApplicationNodeImplementation::CleanUp()
    {
        if (vaoBackgroundGrid_ != 0) glDeleteVertexArrays(0, &vaoBackgroundGrid_);
        vaoBackgroundGrid_ = 0;
        if (vboBackgroundGrid_ != 0) glDeleteBuffers(0, &vboBackgroundGrid_);
        vboBackgroundGrid_ = 0;
    }

    void ApplicationNodeImplementation::KeyboardCallback(int key, int scancode, int action, int mods)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_KeyCallback(key, scancode, action, mods);
#endif
    }

    void ApplicationNodeImplementation::CharCallback(unsigned character, int mods)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_CharCallback(character);
#endif
    }

    void ApplicationNodeImplementation::MouseButtonCallback(int button, int action)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_MouseButtonCallback(button, action, 0);
#endif
    }

    void ApplicationNodeImplementation::MousePosCallback(double x, double y)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_MousePositionCallback(x, y);
#endif
    }

    void ApplicationNodeImplementation::MouseScrollCallback(double xoffset, double yoffset)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_ScrollCallback(xoffset, yoffset);
#endif
    }

    void ApplicationNodeImplementation::EncodeData()
    {
    }

    void ApplicationNodeImplementation::DecodeData()
    {
    }
}
