/**
 * @file   FrameworkInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the internal framework class for the VISCOM lab cluster.
 */

#include "FrameworkInternal.h"
#include <imgui.h>
#include "core/imgui/imgui_impl_glfw_gl3.h"
#include "core/open_gl.h"
#include <iostream>
#include "core/app_internal/CoordinatorNodeInternal.h"
#include "core/app_internal/WorkerNodeInternal.h"
#include "core/app/ApplicationNodeBase.h"

namespace viscom {

    enum class InternalTransferType : std::uint8_t {
        ResourceTransfer,
        ResourceReleaseTransfer,
        ResourceRequest
    };

    enum class InternalTransferTypeLarge : std::uint16_t {
        UserData = std::numeric_limits<std::uint16_t>::max()
    };

    FrameworkInternal::FrameworkInternal(FWConfiguration&& config,
        InitNodeFunc coordinatorNodeFactory, InitNodeFunc workerNodeFactory) :
        coordinatorNodeFactory_{ std::move(coordinatorNodeFactory) },
        workerNodeFactory_{ std::move(workerNodeFactory) },
        config_(std::move(config)),
        window_{ nullptr },
        camHelper_{ config_.nearPlaneSize_.x, config.nearPlaneSize_.y, glm::vec3(0.0f, 0.0f, 4.0f) },
        gpuProgramManager_{ this },
        textureManager_{ this },
        meshManager_{ this }
    {
        BasePreWindow();

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
        if constexpr (DEBUG_MODE)glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        glfwSetErrorCallback(FrameworkInternal::ErrorCallbackStatic);
        window_ = glfwCreateWindow(static_cast<int>(config_.virtualScreenSize_.x), static_cast<int>(config_.virtualScreenSize_.y), "VISCOM Framework", nullptr, nullptr);
        if (window_ == nullptr) {
            LOG(FATAL) << "Could not create window!";
            glfwTerminate();
            throw std::runtime_error("Could not create window!");
        }

        glfwSetWindowPos(window_, 0, 0);
        glfwSetWindowUserPointer(window_, this);
        glfwSetInputMode(window_, GLFW_STICKY_MOUSE_BUTTONS, 1);

        glfwSetKeyCallback(window_, FrameworkInternal::BaseKeyboardCallbackStatic);
        glfwSetCharCallback(window_, FrameworkInternal::BaseCharCallbackStatic);
        glfwSetMouseButtonCallback(window_, FrameworkInternal::BaseMouseButtonCallbackStatic);
        glfwSetCursorPosCallback(window_, FrameworkInternal::BaseMousePosCallbackStatic);
        glfwSetScrollCallback(window_, FrameworkInternal::BaseMouseScrollCallbackStatic);
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

    FrameworkInternal::~FrameworkInternal()
    {
        if (window_) glfwDestroyWindow(window_);
        glfwTerminate();
    }

    void FrameworkInternal::Render()
    {
        BaseInitOpenGL();

        while (!glfwWindowShouldClose(window_)) {
            appNodeInternal_->PreSync();
            PostSyncFunction();
            appNodeInternal_->ClearBuffer(backBuffer_);
            BaseDrawFrame();
            BaseDraw2D();
            appNodeInternal_->PostDraw();
            ImGui_ImplGlfwGL3_FinishAllFrames();
            glfwSwapBuffers(window_);
        }

        BaseCleanUp();
    }

    void FrameworkInternal::BasePreWindow()
    {
        appNodeInternal_ = std::make_unique<CoordinatorNodeInternal>(*this);
        appNodeInternal_->PreWindow();
    }

    void FrameworkInternal::BaseInitOpenGL()
    {
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

        // Setup ImGui binding
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui_ImplGlfwGL3_Init(window_, false);

        FullscreenQuad::InitializeStatic();
        appNodeInternal_->InitOpenGL();
    }

    void FrameworkInternal::PostSyncFunction()
    {
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

        appNodeInternal_->PostSync();
    }

    void FrameworkInternal::BaseDrawFrame()
    {
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        appNodeInternal_->DrawFrame(backBuffer_);
    }

    void FrameworkInternal::BaseDraw2D()
    {
        ImGui_ImplGlfwGL3_NewFrame(-GetViewportScreen(0).position_, GetViewportScreen(0).size_, GetViewportScreen(0).size_, GetViewportScaling(0),
            appNodeInternal_->GetCurrentAppTime(), appNodeInternal_->GetElapsedTime());

        appNodeInternal_->Draw2D(backBuffer_);

        backBuffer_.DrawToFBO([this]() {
            ImGui::Render();
            ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        });
    }

    void FrameworkInternal::BaseCleanUp()
    {
        ImGui_ImplGlfwGL3_Shutdown();
        ImGui::DestroyContext();
        appNodeInternal_->CleanUp();
    }

    bool FrameworkInternal::IsMouseButtonPressed(int button) const noexcept
    {
        return glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    }

    bool FrameworkInternal::IsKeyPressed(int key) const noexcept
    {
        return glfwGetKey(window_, key) == GLFW_PRESS;
    }

    void FrameworkInternal::ErrorCallbackStatic(int error, const char* description)
    {
        std::cerr << "An GLFW error occurred (" << error << "): " << std::endl;
        std::cerr << description << std::endl;
        LOG(WARNING) << "An GLFW error occurred (" << error << "): ";
        LOG(WARNING) << description << std::endl;
    }

    void FrameworkInternal::BaseKeyboardCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto app = reinterpret_cast<FrameworkInternal*>(glfwGetWindowUserPointer(window));
        app->BaseKeyboardCallback(key, scancode, action, mods);
    }

    void FrameworkInternal::BaseCharCallbackStatic(GLFWwindow* window, unsigned int character)
    {
        auto app = reinterpret_cast<FrameworkInternal*>(glfwGetWindowUserPointer(window));
        app->BaseCharCallback(character, 0);
    }

    void FrameworkInternal::BaseMouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods)
    {
        auto app = reinterpret_cast<FrameworkInternal*>(glfwGetWindowUserPointer(window));
        app->BaseMouseButtonCallback(button, action);
    }

    void FrameworkInternal::BaseMousePosCallbackStatic(GLFWwindow* window, double x, double y)
    {
        auto app = reinterpret_cast<FrameworkInternal*>(glfwGetWindowUserPointer(window));
        app->BaseMousePosCallback(x, y);
    }

    void FrameworkInternal::BaseMouseScrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto app = reinterpret_cast<FrameworkInternal*>(glfwGetWindowUserPointer(window));
        app->BaseMouseScrollCallback(xoffset, yoffset);
    }

    void FrameworkInternal::BaseKeyboardCallback(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window_, 1);
            return;
        }

        ImGui_ImplGlfwGL3_KeyCallback(key, scancode, action, mods);
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        appNodeInternal_->KeyboardCallback(key, scancode, action, mods);
    }

    void FrameworkInternal::BaseCharCallback(unsigned int character, int mods)
    {
        ImGui_ImplGlfwGL3_CharCallback(character);
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        appNodeInternal_->CharCallback(character, mods);
    }

    void FrameworkInternal::BaseMouseButtonCallback(int button, int action)
    {
        ImGui_ImplGlfwGL3_MouseButtonCallback(button, action, 0);
        if (ImGui::GetIO().WantCaptureMouse) return;

        appNodeInternal_->MouseButtonCallback(button, action);
    }

    void FrameworkInternal::BaseMousePosCallback(double x, double y)
    {
        auto mousePos = ConvertInputCoordinates(x, y);

        mousePosition_ = glm::vec2(mousePos.x, mousePos.y);
        mousePositionNormalized_.x = (2.0f * mousePosition_.x - 1.0f);
        mousePositionNormalized_.y = -(2.0f * mousePosition_.y - 1.0f);

        ImGui_ImplGlfwGL3_MousePositionCallback(mousePos.x, mousePos.y);
        if (ImGui::GetIO().WantCaptureMouse) return;


        appNodeInternal_->MousePosCallback(mousePos.x, mousePos.y);
    }

    void FrameworkInternal::BaseMouseScrollCallback(double xoffset, double yoffset)
    {
        ImGui_ImplGlfwGL3_ScrollCallback(xoffset, yoffset);
        if (ImGui::GetIO().WantCaptureMouse) return;

        appNodeInternal_->MouseScrollCallback(xoffset, yoffset);
    }

    void FrameworkInternal::SetCursorInputMode(int mode)
    {
        glfwSetInputMode(window_, GLFW_CURSOR, mode);
    }

    void FrameworkInternal::TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex)
    {
        LOG(INFO) << "TransferDataToNode(...) not implemented in local mode.";
    }

    void FrameworkInternal::TransferResource(std::string_view name, const void* data, std::size_t length, ResourceType type)
    {
        LOG(INFO) << "TransferResource(...) not implemented in local mode.";
    }

    void FrameworkInternal::TransferReleaseResource(std::string_view name, ResourceType type)
    {
        LOG(INFO) << "TransferReleaseResource(...) not implemented in local mode.";
    }

    void FrameworkInternal::WaitForResource(const std::string& name, ResourceType type)
    {
        LOG(INFO) << "WaitForResource(...) not implemented in local mode.";
    }

    glm::dvec2 FrameworkInternal::ConvertInputCoordinates(double x, double y)
    {
        glm::dvec2 result{ x, static_cast<double>(viewportScreen_[0].size_.y) - y };
        result += viewportScreen_[0].position_;
        result /= viewportScreen_[0].size_;
        result.y = 1.0 - result.y;
        return result;
    }

    void FrameworkInternal::Terminate() const
    {
        glfwSetWindowShouldClose(window_, 1);
    }

    std::vector<FrameBuffer> FrameworkInternal::CreateOffscreenBuffers(const FrameBufferDescriptor & fboDesc, int sizeDivisor) const
    {
        std::vector<FrameBuffer> result;
        glm::ivec2 fboSize(config_.virtualScreenSize_.x / sizeDivisor, config_.virtualScreenSize_.y / sizeDivisor);
        result.emplace_back(fboSize.x, fboSize.y, fboDesc);
        return result;
    }

    const FrameBuffer* FrameworkInternal::SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const
    {
        return &offscreenBuffers[0];
    }

    std::unique_ptr<FullscreenQuad> FrameworkInternal::CreateFullscreenQuad(const std::string& fragmentShader)
    {
        return std::make_unique<FullscreenQuad>(fragmentShader, this);
    }
}
