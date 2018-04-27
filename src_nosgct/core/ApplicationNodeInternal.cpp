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
#include "core/open_gl.h"
#include <iostream>

namespace viscom {

    ApplicationNodeInternal::ApplicationNodeInternal(FWConfiguration&& config) :
        tuio::TuioInputWrapper{ config.tuioPort_ },
        config_{ std::move(config) },
        window_{ nullptr },
        camHelper_{ config_.nearPlaneSize_.x, config.nearPlaneSize_.y, glm::vec3(0.0f, 0.0f, 4.0f) },
        currentTime_{ 0.0 },
        elapsedTime_{ 0.0 },
        gpuProgramManager_{ this },
        textureManager_{ this },
        meshManager_{ this }
    {
        std::pair<int, int> oglVer = std::make_pair(3, 3);
        if (config_.openglProfile_ == "3.3") oglVer = std::make_pair(3, 3);
        else if (config_.openglProfile_ == "4.0") oglVer = std::make_pair(4, 0); //-V112
        else if (config_.openglProfile_ == "4.1") oglVer = std::make_pair(4, 1); //-V112
        else if (config_.openglProfile_ == "4.2") oglVer = std::make_pair(4, 2); //-V112
        else if (config_.openglProfile_ == "4.3") oglVer = std::make_pair(4, 3); //-V112
        else if (config_.openglProfile_ == "4.4") oglVer = std::make_pair(4, 4); //-V112
        else if (config_.openglProfile_ == "4.5") oglVer = std::make_pair(4, 5); //-V112

        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, oglVer.first);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, oglVer.second);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        if constexpr (DEBUG_MODE) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
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

        int fbWidth = 0;
        int fbHeight = 0;
        glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
        auto projectorSize = glm::ivec2(fbWidth, fbHeight);

        backBuffer_.Resize(projectorSize.x, projectorSize.y);
        viewportScreen_[0].position_ = glm::ivec2(0);
        viewportScreen_[0].size_ = projectorSize;
        viewportQuadSize_[0] = projectorSize;
        viewportScaling_[0] = glm::vec2(projectorSize) / config_.virtualScreenSize_;

        glm::vec2 relPosScale = 1.0f / glm::vec2(viewportQuadSize_[0]);
        glm::vec2 scaledRelPos = (glm::vec2(viewportScreen_[0].position_) / glm::vec2(viewportScreen_[0].size_)) * relPosScale;

        glm::mat4 glbToLcMatrix = glm::mat4{ 1.0f };
        // correct local matrix:
        // xlocal = xglobal*totalScreenSize - viewportScreen_[wId].position_
        glbToLcMatrix[0][0] = static_cast<float>(projectorSize.x);
        glbToLcMatrix[1][1] = static_cast<float>(projectorSize.y);
        glbToLcMatrix[3][0] = -static_cast<float>(viewportScreen_[0].position_.x);
        glbToLcMatrix[3][1] = -static_cast<float>(viewportScreen_[0].position_.y);
        camHelper_.SetLocalCoordMatrix(0, glbToLcMatrix, glm::vec2(projectorSize));

        ImGui_ImplGlfwGL3_Init(window_, false);

        FullscreenQuad::InitializeStatic();
        appNodeImpl_->InitOpenGL();
    }

    void ApplicationNodeInternal::PostSyncFunction()
    {
        auto lastTime = currentTime_;
        currentTime_ = glfwGetTime();

        glm::vec2 relProjectorPos = glm::vec2(viewportScreen_[0].position_) / glm::vec2(viewportScreen_[0].size_);
        glm::vec2 relQuadSize = glm::vec2(viewportQuadSize_[0]) / glm::vec2(viewportScreen_[0].size_);
        glm::vec2 relProjectorSize = 1.0f / relQuadSize;

        glm::mat4 pickMatrix = glm::mat4{ 1.0f };
        pickMatrix[0][0] = 2.0f * relProjectorSize.x;
        pickMatrix[1][1] = -2.0f * relProjectorSize.y;
        pickMatrix[3][0] = (-2.0f * relProjectorPos.x * relProjectorSize.x) - 1.0f;
        pickMatrix[3][1] = (-2.0f * relProjectorPos.y * relProjectorSize.y) + 1.0f;
        pickMatrix[3][2] = 1.0f;
        pickMatrix[3][3] = 1.0f;
        pickMatrix = glm::inverse(camHelper_.GetCentralViewPerspectiveMatrix()) * pickMatrix;
        camHelper_.SetPickMatrix(pickMatrix);

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

    bool ApplicationNodeInternal::IsMouseButtonPressed(int button) const noexcept
    {
        return glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    }

    bool ApplicationNodeInternal::IsKeyPressed(int key) const noexcept
    {
        return glfwGetKey(window_, key) == GLFW_PRESS;
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
            glfwSetWindowShouldClose(window_, 1);
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

        mousePosition_ = glm::vec2(mousePos.x, mousePos.y);
        mousePositionNormalized_.x = (2.0f * mousePosition_.x - 1.0f);
        mousePositionNormalized_.y = -(2.0f * mousePosition_.y - 1.0f);

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
        if constexpr (USE_TUIO) {
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->AddTuioCursor(tcur);
        }
    }

    void ApplicationNodeInternal::updateTuioCursor(TUIO::TuioCursor* tcur)
    {
        if constexpr (USE_TUIO) {
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->UpdateTuioCursor(tcur);
        }
    }

    void ApplicationNodeInternal::removeTuioCursor(TUIO::TuioCursor* tcur)
    {
        if constexpr (USE_TUIO) {
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->RemoveTuioCursor(tcur);
        }
    }

    void ApplicationNodeInternal::SetCursorInputMode(int mode)
    {
        glfwSetInputMode(window_, GLFW_CURSOR, mode);
    }

    glm::dvec2 ApplicationNodeInternal::ConvertInputCoordinates(double x, double y)
    {
        glm::dvec2 result{ x, static_cast<double>(viewportScreen_[0].size_.y) - y };
        result += viewportScreen_[0].position_;
        result /= viewportScreen_[0].size_;
        result.y = 1.0 - result.y;
        return result;
    }

    void ApplicationNodeInternal::Terminate() const
    {
        glfwSetWindowShouldClose(window_, 1);
    }

    std::vector<FrameBuffer> ApplicationNodeInternal::CreateOffscreenBuffers(const FrameBufferDescriptor & fboDesc, int sizeDivisor) const
    {
        std::vector<FrameBuffer> result;
        glm::ivec2 fboSize(config_.virtualScreenSize_.x / sizeDivisor, config_.virtualScreenSize_.y / sizeDivisor);
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
