/**
 * @file   ApplicationNodeInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the base application node base class.
 */

#include "ApplicationNodeInternal.h"
#include "app/MasterNode.h"
#include "app/SlaveNode.h"
#include <imgui.h>
#include "core/imgui/imgui_impl_glfw_gl3.h"
#include <GLFW/glfw3.h>
#include <iostream>

#ifdef VISCOM_CLIENTMOUSECURSOR
#define CLIENTMOUSE true
#else
#define CLIENTMOUSE false
#endif

namespace viscom {

    ApplicationNodeInternal::ApplicationNodeInternal(FWConfiguration&& config) :
        tuio::TuioInputWrapper{ config.tuioPort_ },
        config_{ std::move(config) },
        window_{ nullptr },
        camHelper_{ config_.virtualScreenSize_.x, config.virtualScreenSize_.y },
        currentTime_{ 0.0 },
        elapsedTime_{ 0.0 },
        gpuProgramManager_{ this },
        textureManager_{ this },
        meshManager_{ this }
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        glfwSetErrorCallback(ApplicationNodeInternal::ErrorCallbackStatic);
        window_ = glfwCreateWindow(static_cast<int>(config_.virtualScreenSize_.x), static_cast<int>(config_.virtualScreenSize_.y), "VISCOM Framework", nullptr, nullptr);
        if (window_ == nullptr) {
            LOG(FATAL) << "Could not create window!";
            glfwTerminate();
            throw std::runtime_error("Could not create window!");
        }

        glfwSetWindowPos(window_, 0, 0);
        glfwSetWindowUserPointer(window_, this);
        glfwSetInputMode(window_, GLFW_STICKY_MOUSE_BUTTONS, 1);

        glfwSetKeyCallback(window_, ApplicationNodeInternal::BaseKeyboardCallbackStatic);
        glfwSetCharCallback(window_, ApplicationNodeInternal::BaseCharCallbackStatic);
        glfwSetMouseButtonCallback(window_, ApplicationNodeInternal::BaseMouseButtonCallbackStatic);
        glfwSetCursorPosCallback(window_, ApplicationNodeInternal::BaseMousePosCallbackStatic);
        glfwSetScrollCallback(window_, ApplicationNodeInternal::BaseMouseScrollCallbackStatic);
        glfwMakeContextCurrent(window_);

        LOG(INFO) << "Window successfully initialized.";

        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (GLEW_OK != err) {
            LOG(FATAL) << "Could not load OpenGL: " << glewGetErrorString(err);
            glfwTerminate();
            throw std::runtime_error("Could not load OpenGL.");
        }
    }


    ApplicationNodeInternal::~ApplicationNodeInternal()
    {
        if (window_) glfwDestroyWindow(window_);
        glfwTerminate();
    }

    void ApplicationNodeInternal::Render()
    {
        BaseInitOpenGL();

        while (!glfwWindowShouldClose(window_)) {
            appNodeImpl_->PreSync();
            PostSyncFunction();
            appNodeImpl_->ClearBuffer(backBuffer_);
            BaseDrawFrame();
            BaseDraw2D();
            appNodeImpl_->PostDraw();
            ImGui_ImplGlfwGL3_FinishAllFrames();
            glfwSwapBuffers(window_);
        }

        BaseCleanUp();
    }

    void ApplicationNodeInternal::BaseInitOpenGL()
    {
        appNodeImpl_ = std::make_unique<MasterNode>(this);

        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        viewportScreen_.resize(1);
        viewportQuadSize_.resize(1, glm::ivec2(0));
        viewportScaling_.resize(1, glm::vec2(1.0f));

        
        auto projectorSize = glm::ivec2(config_.virtualScreenSize_);
        backBuffer_.Resize(projectorSize.x, projectorSize.y);
        viewportScreen_[0].position_ = glm::ivec2(0);
        viewportScreen_[0].size_ = projectorSize;
        viewportQuadSize_[0] = projectorSize;
        viewportScaling_[0] = glm::vec2(projectorSize) / config_.virtualScreenSize_;

        ImGui_ImplGlfwGL3_Init(window_, false);

        appNodeImpl_->InitOpenGL();
    }

    void ApplicationNodeInternal::PostSyncFunction()
    {
        auto lastTime = currentTime_;
        currentTime_ = glfwGetTime();
        appNodeImpl_->UpdateSyncedInfo();

        elapsedTime_ = currentTime_ - lastTime;
        glfwPollEvents();
        appNodeImpl_->UpdateFrame(currentTime_, elapsedTime_);
    }

    void ApplicationNodeInternal::BaseDrawFrame()
    {
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        appNodeImpl_->DrawFrame(backBuffer_);
    }

    void ApplicationNodeInternal::BaseDraw2D()
    {
        ImGui_ImplGlfwGL3_NewFrame(-GetViewportScreen(0).position_, GetViewportScreen(0).size_, GetViewportScaling(0), GetCurrentAppTime(), GetElapsedTime());

        appNodeImpl_->Draw2D(backBuffer_);

        backBuffer_.DrawToFBO([this]() {
            ImGui::Render();
        });
    }

    void ApplicationNodeInternal::BaseCleanUp() const
    {
        ImGui_ImplGlfwGL3_Shutdown();
        appNodeImpl_->CleanUp();
    }

    void ApplicationNodeInternal::ErrorCallbackStatic(int error, const char* description)
    {
        std::cerr << "An GLFW error occurred (" << error << "): " << std::endl;
        std::cerr << description << std::endl;
        LOG(WARNING) << "An GLFW error occurred (" << error << "): ";
        LOG(WARNING) << description << std::endl;
    }

    void ApplicationNodeInternal::BaseKeyboardCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto app = reinterpret_cast<ApplicationNodeInternal*>(glfwGetWindowUserPointer(window));
        app->BaseKeyboardCallback(key, scancode, action, mods);
    }

    void ApplicationNodeInternal::BaseCharCallbackStatic(GLFWwindow* window, unsigned int character)
    {
        auto app = reinterpret_cast<ApplicationNodeInternal*>(glfwGetWindowUserPointer(window));
        app->BaseCharCallback(character, 0);
    }

    void ApplicationNodeInternal::BaseMouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods)
    {
        auto app = reinterpret_cast<ApplicationNodeInternal*>(glfwGetWindowUserPointer(window));
        app->BaseMouseButtonCallback(button, action);
    }

    void ApplicationNodeInternal::BaseMousePosCallbackStatic(GLFWwindow* window, double x, double y)
    {
        auto app = reinterpret_cast<ApplicationNodeInternal*>(glfwGetWindowUserPointer(window));
        app->BaseMousePosCallback(x, y);
    }

    void ApplicationNodeInternal::BaseMouseScrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto app = reinterpret_cast<ApplicationNodeInternal*>(glfwGetWindowUserPointer(window));
        app->BaseMouseScrollCallback(xoffset, yoffset);
    }

    // ReSharper disable CppMemberFunctionMayBeConst
    void ApplicationNodeInternal::BaseKeyboardCallback(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window_, true);
            return;
        }

        ImGui_ImplGlfwGL3_KeyCallback(key, scancode, action, mods);
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        appNodeImpl_->KeyboardCallback(key, scancode, action, mods);
    }

    void ApplicationNodeInternal::BaseCharCallback(unsigned int character, int mods)
    {
        ImGui_ImplGlfwGL3_CharCallback(character);
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        appNodeImpl_->CharCallback(character, mods);
    }

    void ApplicationNodeInternal::BaseMouseButtonCallback(int button, int action)
    {
        ImGui_ImplGlfwGL3_MouseButtonCallback(button, action, 0);
        if (ImGui::GetIO().WantCaptureMouse) return;

        appNodeImpl_->MouseButtonCallback(button, action);
    }

    void ApplicationNodeInternal::BaseMousePosCallback(double x, double y)
    {
        auto mousePos = ConvertInputCoordinates(x, y);

        ImGui_ImplGlfwGL3_MousePositionCallback(mousePos.x, mousePos.y);
        if (ImGui::GetIO().WantCaptureMouse) return;


        appNodeImpl_->MousePosCallback(mousePos.x, mousePos.y);
    }

    void ApplicationNodeInternal::BaseMouseScrollCallback(double xoffset, double yoffset)
    {
        ImGui_ImplGlfwGL3_ScrollCallback(xoffset, yoffset);
        if (ImGui::GetIO().WantCaptureMouse) return;

        appNodeImpl_->MouseScrollCallback(xoffset, yoffset);
    }

    void ApplicationNodeInternal::addTuioCursor(TUIO::TuioCursor* tcur)
    {
#ifdef WITH_TUIO
        // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
        appNodeImpl_->AddTuioCursor(tcur);
#endif
    }

    void ApplicationNodeInternal::updateTuioCursor(TUIO::TuioCursor* tcur)
    {
#ifdef WITH_TUIO
        // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
        appNodeImpl_->UpdateTuioCursor(tcur);
#endif
    }

    void ApplicationNodeInternal::removeTuioCursor(TUIO::TuioCursor* tcur)
    {
#ifdef WITH_TUIO
        // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
        appNodeImpl_->RemoveTuioCursor(tcur);
#endif
    }

    glm::dvec2 ApplicationNodeInternal::ConvertInputCoordinates(double x, double y)
    {
        glm::dvec2 result{ x, static_cast<double>(viewportScreen_[0].size_.y) - y };
        result += viewportScreen_[0].position_;
        result /= viewportScreen_[0].size_;
        result.y = 1.0 - result.y;
        return result;
    }

    std::vector<FrameBuffer> ApplicationNodeInternal::CreateOffscreenBuffers(const FrameBufferDescriptor & fboDesc) const
    {
        std::vector<FrameBuffer> result;
        glm::ivec2 fboSize(config_.virtualScreenSize_.x, config_.virtualScreenSize_.y);
        result.emplace_back(fboSize.x, fboSize.y, fboDesc);
        return result;
    }

    const FrameBuffer* ApplicationNodeInternal::SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const
    {
        return &offscreenBuffers[0];
    }

    std::unique_ptr<FullscreenQuad> ApplicationNodeInternal::CreateFullscreenQuad(const std::string& fragmentShader)
    {
        return std::make_unique<FullscreenQuad>(fragmentShader, this);
    }
}
