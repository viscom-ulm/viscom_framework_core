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

#ifdef VISCOM_CLIENTMOUSECURSOR
#define CLIENTMOUSE true
#else
#define CLIENTMOUSE false
#endif

namespace viscom {

    ApplicationNodeInternal* ApplicationNodeInternal::instance_{ nullptr };

    ApplicationNodeInternal::ApplicationNodeInternal(FWConfiguration&& config) :
        config_{ std::move(config) },
        currentTime_{ 0.0 },
        elapsedTime_{ 0.0 },
        gpuProgramManager_{ this },
        textureManager_{ this },
        meshManager_{ this }
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, PTRN_OPENGL_MAJOR_VERSION);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, PTRN_OPENGL_MINOR_VERSION);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, 0);
        if (config.backbufferBits == 32) {
            glfwWindowHint(GLFW_DEPTH_BITS, 32);
            glfwWindowHint(GLFW_STENCIL_BITS, 0);
        } else {
            glfwWindowHint(GLFW_DEPTH_BITS, config.backbufferBits);
        }
        glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_SAMPLES, 8);

        glfwSetErrorCallback(GLWindow::glfwErrorCallback);
        if (config.fullscreen) {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window_ = glfwCreateWindow(config.windowWidth, config.windowHeight, windowTitle.c_str(), glfwGetPrimaryMonitor(), nullptr);
            if (window_ == nullptr) {
                LOG(ERROR) << L"Could not create window!";
                glfwTerminate();
                throw std::runtime_error("Could not create window!");
            }
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
            window_ = glfwCreateWindow(config.windowWidth, config.windowHeight, windowTitle.c_str(), nullptr, nullptr);
            if (window_ == nullptr) {
                LOG(ERROR) << L"Could not create window!";
                glfwTerminate();
                throw std::runtime_error("Could not create window!");
            }
            glfwSetWindowPos(window_, config.windowLeft, config.windowTop);

        }
        glfwSetWindowUserPointer(window_, this);
        glfwSetInputMode(window_, GLFW_STICKY_MOUSE_BUTTONS, 1);
        glfwSetCursorPos(window_, 0.0, 0.0);

        glfwSetWindowPosCallback(window_, GLWindow::glfwWindowPosCallback);
        glfwSetWindowSizeCallback(window_, GLWindow::glfwWindowSizeCallback);
        glfwSetWindowFocusCallback(window_, GLWindow::glfwWindowFocusCallback);
        glfwSetWindowCloseCallback(window_, GLWindow::glfwWindowCloseCallback);
        glfwSetFramebufferSizeCallback(window_, GLWindow::glfwFramebufferSizeCallback);
        glfwSetWindowIconifyCallback(window_, GLWindow::glfwWindowIconifyCallback);


        glfwSetMouseButtonCallback(window_, GLWindow::glfwMouseButtonCallback);
        glfwSetCursorPosCallback(window_, GLWindow::glfwCursorPosCallback);
        glfwSetCursorEnterCallback(window_, GLWindow::glfwCursorEnterCallback);
        glfwSetScrollCallback(window_, GLWindow::glfwScrollCallback);
        glfwSetKeyCallback(window_, GLWindow::glfwKeyCallback);
        glfwSetCharCallback(window_, GLWindow::glfwCharCallback);
        glfwSetCharModsCallback(window_, GLWindow::glfwCharModsCallback);
        glfwSetDropCallback(window_, GLWindow::glfwDropCallback);

        // Check for Valid Context
        if (window_ == nullptr) {
            LOG(ERROR) << L"Could not create window!";
            glfwTerminate();
            throw std::runtime_error("Could not create window!");
        }

        LOG(DEBUG) << L"Window successfully initialized.";


        engine_->setPreWindowFunction([app = this]() { app->BasePreWindow(); });
        engine_->setInitOGLFunction([app = this]() { app->BaseInitOpenGL(); });
        engine_->setPreSyncFunction([app = this](){ app->BasePreSync(); });
        engine_->setPostSyncPreDrawFunction([app = this]() { app->PostSyncFunction(); });
        engine_->setClearBufferFunction([app = this]() { app->BaseClearBuffer(); });
        engine_->setDrawFunction([app = this]() { app->BaseDrawFrame(); });
        engine_->setDraw2DFunction([app = this]() { app->BaseDraw2D(); });
        engine_->setPostDrawFunction([app = this]() { app->BasePostDraw(); });
        engine_->setCleanUpFunction([app = this](){ app->BaseCleanUp(); });

        engine_->setKeyboardCallbackFunction([app = this](int key, int scancode, int action, int mods) { app->BaseKeyboardCallback(key, scancode, action, mods); });
        engine_->setCharCallbackFunction([app = this](unsigned int character, int mods) { app->BaseCharCallback(character, mods); });
        engine_->setMouseButtonCallbackFunction([app = this](int button, int action) { app->BaseMouseButtonCallback(button, action); });
        engine_->setMousePosCallbackFunction([app = this](double x, double y) { app->BaseMousePosCallback(x, y); });
        engine_->setMouseScrollCallbackFunction([app = this](double xoffset, double yoffset) { app->BaseMouseScrollCallback(xoffset, yoffset); });

        glewExperimental = GL_TRUE;
        /*
        void setDropCallbackFunction(sgct_cppxeleven::function<void(int, const char**)> fn); //arguments: int count, const char ** list of path strings

        void setExternalControlCallback(sgct_cppxeleven::function<void(const char *, int)> fn); //arguments: const char * buffer, int buffer length
        void setExternalControlStatusCallback(sgct_cppxeleven::function<void(bool)> fn); //arguments: const bool & connected
        void setContextCreationCallback(sgct_cppxeleven::function<void(GLFWwindow*)> fn); //arguments: glfw window share

        void setDataTransferCallback(sgct_cppxeleven::function<void(void *, int, int, int)> fn); //arguments: const char * buffer, int buffer length, int package id, int client
        void setDataTransferStatusCallback(sgct_cppxeleven::function<void(bool, int)> fn); //arguments: const bool & connected, int client
        void setDataAcknowledgeCallback(sgct_cppxeleven::function<void(int, int)> fn); //arguments: int package id, int client
        */
    }


    ApplicationNodeInternal::~ApplicationNodeInternal() = default;

    void ApplicationNodeInternal::InitNode()
    {
        if (!engine_->init(sgct::Engine::OpenGL_3_3_Core_Profile))
        {
            LOG(FATAL) << "Failed to create SGCT engine.";
            throw std::runtime_error("Failed to create SGCT engine.");
        }

        assert(instance_ == nullptr);
        instance_ = this;
        sgct::SharedData::instance()->setEncodeFunction(BaseEncodeDataStatic);
        sgct::SharedData::instance()->setDecodeFunction(BaseDecodeDataStatic);
    }

    void ApplicationNodeInternal::Render() const
    {
        engine_->render();
    }

    void ApplicationNodeInternal::BasePreWindow()
    {
        if (engine_->isMaster()) appNodeImpl_ = std::make_unique<MasterNode>(this);
        else appNodeImpl_ = std::make_unique<SlaveNode>(this);
    }

    void ApplicationNodeInternal::BaseInitOpenGL()
    {
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        auto numWindows = sgct_core::ClusterManager::instance()->getThisNodePtr()->getNumberOfWindows();
        viewportScreen_.resize(numWindows);
        viewportQuadSize_.resize(numWindows, glm::ivec2(0));
        viewportScaling_.resize(numWindows, glm::vec2(1.0f));

        for (size_t wId = 0; wId < numWindows; ++wId) {
            glm::ivec2 projectorSize;
            auto window = GetEngine()->getWindowPtr(wId);
            window->getFinalFBODimensions(projectorSize.x, projectorSize.y);
            framebuffers_.emplace_back();
            framebuffers_.back().Resize(projectorSize.x, projectorSize.y);
            viewportScreen_[wId].position_ = glm::ivec2(0);
            viewportScreen_[wId].size_ = projectorSize;
            viewportQuadSize_[wId] = projectorSize;
            viewportScaling_[wId] = glm::vec2(projectorSize) / config_.virtualScreenSize_;
        }

#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_Init(GetEngine()->getCurrentWindowPtr()->getWindowHandle(), !GetEngine()->isMaster() && CLIENTMOUSE);
#else
        if (GetEngine()->isMaster()) ImGui_ImplGlfwGL3_Init(GetEngine()->getCurrentWindowPtr()->getWindowHandle(), !GetEngine()->isMaster() && CLIENTMOUSE);
#endif

        appNodeImpl_->InitOpenGL();
    }

    void ApplicationNodeInternal::BasePreSync()
    {
        if (engine_->isMaster()) {

            currentTimeSynced_.setVal(sgct::Engine::getTime());
        }
        appNodeImpl_->PreSync();
    }

    void ApplicationNodeInternal::PostSyncFunction()
    {
        auto lastTime = currentTime_;
        currentTime_ = currentTimeSynced_.getVal();
        appNodeImpl_->UpdateSyncedInfo();

        elapsedTime_ = currentTimeSynced_ - lastTime;
        appNodeImpl_->UpdateFrame(currentTime_, elapsedTime_);
    }

    void ApplicationNodeInternal::BaseClearBuffer()
    {
        appNodeImpl_->ClearBuffer(framebuffers_[GetEngine()->getCurrentWindowIndex()]);
    }

    void ApplicationNodeInternal::BaseDrawFrame()
    {
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        appNodeImpl_->DrawFrame(framebuffers_[GetEngine()->getCurrentWindowIndex()]);
    }

    void ApplicationNodeInternal::BaseDraw2D()
    {
        auto window = GetEngine()->getCurrentWindowPtr();

        ImGui_ImplGlfwGL3_NewFrame(-GetViewportScreen(window->getId()).position_, GetViewportScreen(window->getId()).size_, GetViewportScaling(window->getId()), GetCurrentAppTime(), GetElapsedTime());

        auto& fbo = framebuffers_[GetEngine()->getCurrentWindowIndex()];
        appNodeImpl_->Draw2D(fbo);

        fbo.DrawToFBO([this]() {
            // ImGui::Render for slaves is called in SlaveNodeInternal...
            if (engine_->isMaster()) ImGui::Render();
        });
    }

    void ApplicationNodeInternal::BasePostDraw() const
    {
        appNodeImpl_->PostDraw();
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_FinishAllFrames();
#else
        if (engine_->isMaster()) ImGui_ImplGlfwGL3_FinishAllFrames();
#endif
    }

    void ApplicationNodeInternal::BaseCleanUp() const
    {
        std::lock_guard<std::mutex> lock{ instanceMutex_ };
        instance_ = nullptr;
#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_Shutdown();
#else
        if (GetEngine()->isMaster()) ImGui_ImplGlfwGL3_Shutdown();
#endif
        appNodeImpl_->CleanUp();
    }

    // ReSharper disable CppMemberFunctionMayBeConst
    void ApplicationNodeInternal::BaseKeyboardCallback(int key, int scancode, int action, int mods)
    {
        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            keyboardEvents_.emplace_back(key, scancode, action, mods);
#endif
            appNodeImpl_->KeyboardCallback(key, scancode, action, mods);
        }
    }

    void ApplicationNodeInternal::BaseCharCallback(unsigned int character, int mods)
    {
        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            charEvents_.emplace_back(character, mods);
#endif
            appNodeImpl_->CharCallback(character, mods);
        }
    }

    void ApplicationNodeInternal::BaseMouseButtonCallback(int button, int action)
    {
        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            mouseButtonEvents_.emplace_back(button, action);
#endif
            appNodeImpl_->MouseButtonCallback(button, action);
        }
    }

    void ApplicationNodeInternal::BaseMousePosCallback(double x, double y)
    {
        x /= static_cast<double>(viewportScreen_[0].size_.x);
        y /= static_cast<double>(viewportScreen_[0].size_.y);
        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            mousePosEvents_.emplace_back(x, y);
#endif
            appNodeImpl_->MousePosCallback(x, y);
        }
    }

    void ApplicationNodeInternal::BaseMouseScrollCallback(double xoffset, double yoffset)
    {
        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            mouseScrollEvents_.emplace_back(xoffset, yoffset);
#endif
            appNodeImpl_->MouseScrollCallback(xoffset, yoffset);
        }
    }
    // ReSharper restore CppMemberFunctionMayBeConst

    void ApplicationNodeInternal::BaseEncodeData()
    {
        sgct::SharedData::instance()->writeDouble(&currentTimeSynced_);
        appNodeImpl_->EncodeData();
    }

    void ApplicationNodeInternal::BaseDecodeData()
    {
        sgct::SharedData::instance()->readDouble(&currentTimeSynced_);
        appNodeImpl_->DecodeData();
    }

    std::vector<FrameBuffer> ApplicationNodeInternal::CreateOffscreenBuffers(const FrameBufferDescriptor & fboDesc) const
    {
        std::vector<FrameBuffer> result;
        auto numWindows = sgct_core::ClusterManager::instance()->getThisNodePtr()->getNumberOfWindows();
        for (const auto& fboSize : viewportQuadSize_) {
            result.emplace_back(fboSize.x, fboSize.y, fboDesc);
        }
        return result;
    }

    const FrameBuffer* ApplicationNodeInternal::SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const
    {
        return &offscreenBuffers[engine_->getCurrentWindowPtr()->getId()];
    }

    std::unique_ptr<FullscreenQuad> ApplicationNodeInternal::CreateFullscreenQuad(const std::string& fragmentShader)
    {
        return std::make_unique<FullscreenQuad>(fragmentShader, this);
    }
}
