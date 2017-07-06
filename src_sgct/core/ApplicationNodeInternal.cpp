/**
 * @file   ApplicationNodeInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the base application node base class.
 */

#include "ApplicationNodeInternal.h"
#include "external/tinyxml2.h"
#include "app/MasterNode.h"
#include "app/SlaveNode.h"
#include <imgui.h>
#include "core/imgui/imgui_impl_glfw_gl3.h"

#ifndef VISCOM_LOCAL_ONLY
#include "core/OpenCVParserHelper.h"
#endif

#ifdef VISCOM_CLIENTMOUSECURSOR
#define CLIENTMOUSE true
#else
#define CLIENTMOUSE false
#endif

namespace viscom {

    ApplicationNodeInternal* ApplicationNodeInternal::instance_{ nullptr };
    std::mutex ApplicationNodeInternal::instanceMutex_{ };

    ApplicationNodeInternal::ApplicationNodeInternal(FWConfiguration&& config, std::unique_ptr<sgct::Engine> engine) :
        tuio::TuioInputWrapper{ config.tuioPort_ },
        config_( std::move(config) ),
        engine_{ std::move(engine) },
        camHelper_{ engine_.get() },
        elapsedTime_{ 0.0 },
        gpuProgramManager_{ this },
        textureManager_{ this },
        meshManager_{ this }
    {
#ifndef VISCOM_LOCAL_ONLY
        loadProperties();
#endif
        engine_->setPreWindowFunction([app = this]() { app->BasePreWindow(); });
        engine_->setInitOGLFunction([app = this]() { app->BaseInitOpenGL(); });
        engine_->setPreSyncFunction([app = this](){ app->BasePreSync(); });
        engine_->setPostSyncPreDrawFunction([app = this]() { app->PostSyncFunction(); });
        engine_->setClearBufferFunction([app = this]() { app->BaseClearBuffer(); });
        engine_->setDrawFunction([app = this]() { app->BaseDrawFrame(); });
        engine_->setDraw2DFunction([app = this]() { app->BaseDraw2D(); });
        engine_->setPostDrawFunction([app = this]() { app->BasePostDraw(); });
        engine_->setCleanUpFunction([app = this]() { app->BaseCleanUp(); });
        engine_->setDataTransferCallback([app = this](void* receivedData, int receivedLength, int packageID, int clientID) { app->BaseDataTransferCallback(receivedData, receivedLength, packageID, clientID); });
        engine_->setDataAcknowledgeCallback([app = this](int packageID, int clientID) { app->BaseDataAcknowledgeCallback(packageID, clientID); });
        engine_->setDataTransferStatusCallback([app = this](bool connected, int clientID) { app->BaseDataTransferStatusCallback(connected, clientID); });

        engine_->setKeyboardCallbackFunction([app = this](int key, int scancode, int action, int mods) { app->BaseKeyboardCallback(key, scancode, action, mods); });
        engine_->setCharCallbackFunction([app = this](unsigned int character, int mods) { app->BaseCharCallback(character, mods); });
        engine_->setMouseButtonCallbackFunction([app = this](int button, int action) { app->BaseMouseButtonCallback(button, action); });
        engine_->setMousePosCallbackFunction([app = this](double x, double y) { app->BaseMousePosCallback(x, y); });
        engine_->setMouseScrollCallbackFunction([app = this](double xoffset, double yoffset) { app->BaseMouseScrollCallback(xoffset, yoffset); });

        /*
        void setDropCallbackFunction(sgct_cppxeleven::function<void(int, const char**)> fn); //arguments: int count, const char ** list of path strings

        void setExternalControlCallback(sgct_cppxeleven::function<void(const char *, int)> fn); //arguments: const char * buffer, int buffer length
        void setExternalControlStatusCallback(sgct_cppxeleven::function<void(bool)> fn); //arguments: const bool & connected
        void setContextCreationCallback(sgct_cppxeleven::function<void(GLFWwindow*)> fn); //arguments: glfw window share
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
        lastFrameTime_ = sgct::Engine::getTime();
        syncInfoLocal_.currentTime_ = lastFrameTime_;
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
        keyPressedState_.resize(GLFW_KEY_LAST, false);
        mousePressedState_.resize(GLFW_MOUSE_BUTTON_LAST, false);

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

            glm::vec2 vpLocalLowerLeft = glm::vec2(window->getViewport(0)->getProjectionPlane()->getCoordinate(sgct_core::SGCTProjectionPlane::LowerLeft));
            glm::vec2 vpLocalUpperLeft = glm::vec2(window->getViewport(0)->getProjectionPlane()->getCoordinate(sgct_core::SGCTProjectionPlane::UpperLeft));
            glm::vec2 vpLocalUpperRight = glm::vec2(window->getViewport(0)->getProjectionPlane()->getCoordinate(sgct_core::SGCTProjectionPlane::UpperRight));
            glm::vec2 vpLocalSize = vpLocalUpperRight - vpLocalLowerLeft;
            glm::vec2 vpTotalSize = 2.0f * GetConfig().nearPlaneSize_;

            glm::vec2 totalScreenSize = (vpTotalSize / vpLocalSize) * glm::vec2(projectorSize);

            viewportScreen_[wId].position_ = ((vpLocalLowerLeft + GetConfig().nearPlaneSize_) / vpTotalSize) * totalScreenSize;
            viewportScreen_[wId].size_ = glm::ivec2(totalScreenSize);
            viewportQuadSize_[wId] = projectorSize;
            viewportScaling_[wId] = totalScreenSize / config_.virtualScreenSize_;
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
        lastFrameTime_ = syncInfoLocal_.currentTime_;

        if (engine_->isMaster()) {
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
            syncInfoLocal_.cameraPosition_ = camHelper_.GetPosition();
            syncInfoLocal_.cameraOrientation_ = camHelper_.GetOrientation();
            syncInfoSynced_.setVal(syncInfoLocal_);
        }
        appNodeImpl_->PreSync();
    }

    void ApplicationNodeInternal::PostSyncFunction()
    {
#ifdef VISCOM_SYNCINPUT
        if (!engine_->isMaster()) {
            {
                auto keybEvts = keyboardEventsSynced_.getVal();
                keyboardEventsSynced_.clear();
                for (const auto k : keybEvts) appNodeImpl_->KeyboardCallback(k.key_, k.scancode_, k.action_, k.mods_);
            }
            {
                auto charEvts = charEventsSynced_.getVal();
                charEventsSynced_.clear();
                for (const auto c : charEvts) appNodeImpl_->CharCallback(c.character_, c.mods_);
            }
            {
                auto mBtnEvts = mouseButtonEventsSynced_.getVal();
                mouseButtonEventsSynced_.clear();
                for (const auto b : mBtnEvts) appNodeImpl_->MouseButtonCallback(b.button_, b.action_);
            }
            {
                auto mPosEvts = mousePosEventsSynced_.getVal();
                mousePosEventsSynced_.clear();
                for (const auto p : mPosEvts) appNodeImpl_->MousePosCallback(p.x_, p.y_);
            }
            {
                auto mScrlEvts = mouseScrollEventsSynced_.getVal();
                mouseScrollEventsSynced_.clear();
                for (const auto s : mScrlEvts) appNodeImpl_->MouseScrollCallback(s.xoffset_, s.yoffset_);
            }
        }
#endif

        // auto lastTime = syncInfoLocal_.currentTime_;
        syncInfoLocal_ = syncInfoSynced_.getVal();
        appNodeImpl_->UpdateSyncedInfo();

        camHelper_.SetPosition(syncInfoLocal_.cameraPosition_);
        camHelper_.SetOrientation(syncInfoLocal_.cameraOrientation_);
        elapsedTime_ = syncInfoLocal_.currentTime_ - lastFrameTime_;
        appNodeImpl_->UpdateFrame(syncInfoLocal_.currentTime_, elapsedTime_);
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

#ifdef VISCOM_CLIENTGUI
        ImGui_ImplGlfwGL3_NewFrame(-GetViewportScreen(window->getId()).position_, GetViewportScreen(window->getId()).size_, GetViewportScaling(window->getId()), GetCurrentAppTime(), GetElapsedTime());
#else
        if (engine_->isMaster()) ImGui_ImplGlfwGL3_NewFrame(-GetViewportScreen(window->getId()).position_, GetViewportScreen(window->getId()).size_, GetViewportScaling(window->getId()), GetCurrentAppTime(), GetElapsedTime());
#endif
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

    bool ApplicationNodeInternal::IsMouseButtonPressed(int button) const noexcept
    {
        return mousePressedState_[button];
    }

    bool ApplicationNodeInternal::IsKeyPressed(int key) const noexcept
    {
        return keyPressedState_[key];
    }

    // ReSharper disable CppMemberFunctionMayBeConst
    void ApplicationNodeInternal::BaseKeyboardCallback(int key, int scancode, int action, int mods)
    {
        keyPressedState_[key] = (action == GLFW_RELEASE) ? false : true;

        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            keyboardEvents_.emplace_back(key, scancode, action, mods);
#endif

#ifndef VISCOM_CLIENTGUI
            ImGui_ImplGlfwGL3_KeyCallback(key, scancode, action, mods);
            if (ImGui::GetIO().WantCaptureKeyboard) return;
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

#ifndef VISCOM_CLIENTGUI
            ImGui_ImplGlfwGL3_CharCallback(character);
            if (ImGui::GetIO().WantCaptureKeyboard) return;
#endif

            appNodeImpl_->CharCallback(character, mods);
        }
    }

    void ApplicationNodeInternal::BaseMouseButtonCallback(int button, int action)
    {
        mousePressedState_[button] = (action == GLFW_RELEASE) ? false : true;

        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            mouseButtonEvents_.emplace_back(button, action);
#endif

#ifndef VISCOM_CLIENTGUI
            ImGui_ImplGlfwGL3_MouseButtonCallback(button, action, 0);
            if (ImGui::GetIO().WantCaptureMouse) return;
#endif

            appNodeImpl_->MouseButtonCallback(button, action);
        }
    }

    void ApplicationNodeInternal::BaseMousePosCallback(double x, double y)
    {
        auto mousePos = glm::dvec2(x, y);
        if (engine_->isMaster()) {
            mousePos = ConvertInputCoordinates(x, y);

            /*auto mode = glfwGetInputMode(engine_->getWindowPtr(0)->getWindowHandle(), GLFW_CURSOR);
            if (mode == GLFW_CURSOR_DISABLED) {
                glfwSetCursorPos(engine_->getWindowPtr(0)->getWindowHandle(), 500, 500);
                mousePos += mousePosition_;
            }*/
        }

        mousePosition_ = glm::vec2(mousePos.x, mousePos.y);
        mousePositionNormalized_.x = (2.0f * mousePosition_.x - 1.0f);
        mousePositionNormalized_.y = -(2.0f * mousePosition_.y - 1.0f);

        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            mousePosEvents_.emplace_back(mousePos.x, mousePos.y);
#endif

#ifndef VISCOM_CLIENTGUI
            ImGui_ImplGlfwGL3_MousePositionCallback(mousePos.x, mousePos.y);
            if (ImGui::GetIO().WantCaptureMouse) return;
#endif

            appNodeImpl_->MousePosCallback(mousePos.x, mousePos.y);
        }
    }

    void ApplicationNodeInternal::BaseMouseScrollCallback(double xoffset, double yoffset)
    {
        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            mouseScrollEvents_.emplace_back(xoffset, yoffset);
#endif

#ifndef VISCOM_CLIENTGUI
            ImGui_ImplGlfwGL3_ScrollCallback(xoffset, yoffset);
            if (ImGui::GetIO().WantCaptureMouse) return;
#endif

            appNodeImpl_->MouseScrollCallback(xoffset, yoffset);
        }
    }

    void ApplicationNodeInternal::BaseDataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID)
    {
        appNodeImpl_->DataTransferCallback(receivedData, receivedLength, packageID, clientID);
    }

    void ApplicationNodeInternal::BaseDataAcknowledgeCallback(int packageID, int clientID)
    {
        appNodeImpl_->DataAcknowledgeCallback(packageID, clientID);
    }

    void ApplicationNodeInternal::BaseDataTransferStatusCallback(bool connected, int clientID)
    {
        appNodeImpl_->DataTransferStatusCallback(connected, clientID);
    }

    // ReSharper restore CppMemberFunctionMayBeConst

    void ApplicationNodeInternal::addTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (engine_->isMaster()) {
#ifdef WITH_TUIO
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->AddTuioCursor(tcur);
#endif
        }
    }

    void ApplicationNodeInternal::updateTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (engine_->isMaster()) {
#ifdef WITH_TUIO
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->UpdateTuioCursor(tcur);
#endif
        }
    }

    void ApplicationNodeInternal::removeTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (engine_->isMaster()) {
#ifdef WITH_TUIO
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->RemoveTuioCursor(tcur);
#endif
        }
    }

    void ApplicationNodeInternal::SetCursorInputMode(int mode)
    {
        if (engine_->isMaster()) {
            glfwSetInputMode(engine_->getWindowPtr(0)->getWindowHandle(), GLFW_CURSOR, mode);
        }
    }

    glm::dvec2 ApplicationNodeInternal::ConvertInputCoordinates(double x, double y)
    {
        glm::dvec2 result{ x, static_cast<double>(viewportScreen_[0].size_.y) - y };
        result += viewportScreen_[0].position_;
        result /= viewportScreen_[0].size_;
        result.y = 1.0 - result.y;
        return result;
    }

    void ApplicationNodeInternal::BaseEncodeData()
    {
#ifdef VISCOM_SYNCINPUT
        sgct::SharedData::instance()->writeVector(&keyboardEventsSynced_);
        sgct::SharedData::instance()->writeVector(&charEventsSynced_);
        sgct::SharedData::instance()->writeVector(&mouseButtonEventsSynced_);
        sgct::SharedData::instance()->writeVector(&mousePosEventsSynced_);
        sgct::SharedData::instance()->writeVector(&mouseScrollEventsSynced_);
#endif
        sgct::SharedData::instance()->writeObj(&syncInfoSynced_);
        appNodeImpl_->EncodeData();
    }

    void ApplicationNodeInternal::BaseDecodeData()
    {
#ifdef VISCOM_SYNCINPUT
        sgct::SharedData::instance()->readVector(&keyboardEventsSynced_);
        sgct::SharedData::instance()->readVector(&charEventsSynced_);
        sgct::SharedData::instance()->readVector(&mouseButtonEventsSynced_);
        sgct::SharedData::instance()->readVector(&mousePosEventsSynced_);
        sgct::SharedData::instance()->readVector(&mouseScrollEventsSynced_);
#endif
        sgct::SharedData::instance()->readObj(&syncInfoSynced_);
        appNodeImpl_->DecodeData();
    }

    void ApplicationNodeInternal::BaseEncodeDataStatic()
    {
        std::lock_guard<std::mutex> lock{ instanceMutex_ };
        if (instance_) instance_->BaseEncodeData();
    }

    void ApplicationNodeInternal::BaseDecodeDataStatic()
    {
        std::lock_guard<std::mutex> lock{ instanceMutex_ };
        if (instance_) instance_->BaseDecodeData();
    }

    void ApplicationNodeInternal::Terminate() const
    {
        engine_->terminate();
    }

    std::vector<FrameBuffer> ApplicationNodeInternal::CreateOffscreenBuffers(const FrameBufferDescriptor & fboDesc) const
    {
        std::vector<FrameBuffer> result;
        for (std::size_t i = 0; i < viewportScreen_.size(); ++i) {
            const auto& fboSize = viewportQuadSize_[i];
            result.emplace_back(fboSize.x, fboSize.y, fboDesc);
            LOG(DBUG) << "Offscreen FBO VP Pos: " << 0.0f << ", " << 0.0f;
            LOG(DBUG) << "Offscreen FBO VP Size: " << GetViewportQuadSize(i).x << ", " << GetViewportQuadSize(i).y;
            result.back().SetStandardViewport(0, 0, GetViewportQuadSize(i).x, GetViewportQuadSize(i).y);
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

#ifndef VISCOM_LOCAL_ONLY
    void ApplicationNodeInternal::loadProperties()
    {
        tinyxml2::XMLDocument doc;
        OpenCVParserHelper::LoadXMLDocument("Program properties", config_.programProperties_, doc);

        startNode_ = OpenCVParserHelper::ParseText<unsigned int>(doc.FirstChildElement("opencv_storage")->FirstChildElement("startNode"));
    }

    unsigned int ApplicationNodeInternal::GetGlobalProjectorId(int nodeId, int windowId) const
    {
        if (static_cast<unsigned int>(nodeId) >= startNode_) {
            unsigned int current_projector = 0;
            for (int i = startNode_; i < sgct_core::ClusterManager::instance()->getNumberOfNodes(); i++) {
                auto currNode = sgct_core::ClusterManager::instance()->getNodePtr(i);
                for (auto j = 0; j < currNode->getNumberOfWindows(); j++) {
                    if (i == nodeId && j == windowId) return current_projector;

                    current_projector += 1;
                }
            }
        }
        return 0;
    }
#endif
}
