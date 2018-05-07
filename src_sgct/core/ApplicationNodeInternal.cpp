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
#include "core/imgui/imgui_impl_glfw_gl3.h"
#include "external/tinyxml2.h"
#include "core/utils/utils.h"
#include <imgui.h>
#include <sgct.h>

#ifndef VISCOM_LOCAL_ONLY
#include "core/OpenCVParserHelper.h"
#endif

namespace viscom {

    enum class InternalTransferType : std::uint8_t {
        ResourceTransfer,
        ResourceReleaseTransfer,
        ResourceRequest
    };

    enum class InternalTransferTypeLarge : std::uint16_t {
        UserData = std::numeric_limits<std::uint16_t>::max()
    };

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
        sgct::Engine::RunMode rm = sgct::Engine::Default_Mode;
        if (config_.openglProfile_ == "3.3") rm = sgct::Engine::OpenGL_3_3_Core_Profile;
        else if (config_.openglProfile_ == "4.0") rm = sgct::Engine::OpenGL_4_0_Core_Profile;
        else if (config_.openglProfile_ == "4.1") rm = sgct::Engine::OpenGL_4_1_Core_Profile;
        else if (config_.openglProfile_ == "4.2") rm = sgct::Engine::OpenGL_4_2_Core_Profile;
        else if (config_.openglProfile_ == "4.3") rm = sgct::Engine::OpenGL_4_3_Core_Profile;
        else if (config_.openglProfile_ == "4.4") rm = sgct::Engine::OpenGL_4_4_Core_Profile;
        else if (config_.openglProfile_ == "4.5") rm = sgct::Engine::OpenGL_4_5_Core_Profile;

        if (!engine_->init(rm))
        {
            LOG(FATAL) << "Failed to create SGCT engine.";
            throw std::runtime_error("Failed to create SGCT engine.");
        }

        {
            std::lock_guard<std::mutex> lock{ instanceMutex_ };
            assert(instance_ == nullptr);
            instance_ = this;
            initialized_ = true;
        }

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

        appNodeImpl_->PreWindow();
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

        for (std::size_t wId = 0; wId < numWindows; ++wId) {
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

            glm::vec2 totalScreenSize = glm::ceil((vpTotalSize / vpLocalSize) * glm::vec2(projectorSize));

            viewportScreen_[wId].position_ = ((vpLocalLowerLeft + GetConfig().nearPlaneSize_) / vpTotalSize) * totalScreenSize;
            viewportScreen_[wId].size_ = glm::ivec2(totalScreenSize);
            viewportQuadSize_[wId] = projectorSize;
            viewportScaling_[wId] = totalScreenSize / config_.virtualScreenSize_;

            glm::vec2 relPosScale = 1.0f / glm::vec2(viewportQuadSize_[wId]);
            glm::vec2 scaledRelPos = (glm::vec2(viewportScreen_[wId].position_) / glm::vec2(viewportScreen_[wId].size_)) * relPosScale;
            
            glm::mat4 glbToLcMatrix = glm::mat4{ 1.0f };
            // correct local matrix:
            // xlocal = xglobal*totalScreenSize - viewportScreen_[wId].position_
            // xlocal = xglobal*(xPixelSizeQuad / xRelSizeQuad) - ((xRelPosQuad*xPixelSizeQuad) / xRelSizeQuad)
            glbToLcMatrix[0][0] = totalScreenSize.x;
            glbToLcMatrix[1][1] = totalScreenSize.y;
            glbToLcMatrix[3][0] = -static_cast<float>(viewportScreen_[wId].position_.x);
            glbToLcMatrix[3][1] = -static_cast<float>(viewportScreen_[wId].position_.y);
            camHelper_.SetLocalCoordMatrix(wId, glbToLcMatrix, glm::vec2(projectorSize));
        }

        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfwGL3_Init(GetEngine()->getCurrentWindowPtr()->getWindowHandle(), !GetEngine()->isMaster() && SHOW_CLIENT_MOUSE_CURSOR);
        else if (GetEngine()->isMaster()) ImGui_ImplGlfwGL3_Init(GetEngine()->getCurrentWindowPtr()->getWindowHandle(), !GetEngine()->isMaster() && SHOW_CLIENT_MOUSE_CURSOR);

        FullscreenQuad::InitializeStatic();
        RequestSharedResources();
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

            glm::vec2 relProjectorPos = glm::vec2(viewportScreen_[0].position_) / glm::vec2(viewportScreen_[0].size_);
            glm::vec2 relQuadSize = glm::vec2(viewportQuadSize_[0]) / glm::vec2(viewportScreen_[0].size_);
            glm::vec2 relProjectorSize = 1.0f / relQuadSize;

            syncInfoLocal_.pickMatrix_ = glm::mat4{ 1.0f };
            syncInfoLocal_.pickMatrix_[0][0] = 2.0f * relProjectorSize.x;
            syncInfoLocal_.pickMatrix_[1][1] = -2.0f * relProjectorSize.y;
            syncInfoLocal_.pickMatrix_[3][0] = (-2.0f * relProjectorPos.x * relProjectorSize.x) - 1.0f;
            syncInfoLocal_.pickMatrix_[3][1] = (-2.0f * relProjectorPos.y * relProjectorSize.y) + 1.0f;
            syncInfoLocal_.pickMatrix_[3][2] = 1.0f;
            syncInfoLocal_.pickMatrix_[3][3] = 1.0f;
            syncInfoLocal_.pickMatrix_ = glm::inverse(camHelper_.GetCentralViewPerspectiveMatrix()) * syncInfoLocal_.pickMatrix_;

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
        camHelper_.SetPickMatrix(syncInfoLocal_.pickMatrix_);

        elapsedTime_ = syncInfoLocal_.currentTime_ - lastFrameTime_;

        CreateSynchronizedResources();
        applicationHalted_ = false;
        applicationHalted_ = applicationHalted_ || GetGPUProgramManager().ProcessResourceWaitList();
        applicationHalted_ = applicationHalted_ || GetTextureManager().ProcessResourceWaitList();
        applicationHalted_ = applicationHalted_ || GetMeshManager().ProcessResourceWaitList();
        if (applicationHalted_) return;
        appNodeImpl_->UpdateFrame(syncInfoLocal_.currentTime_, elapsedTime_);
    }

    void ApplicationNodeInternal::BaseClearBuffer()
    {
        if (applicationHalted_) return;
        appNodeImpl_->ClearBuffer(framebuffers_[GetEngine()->getCurrentWindowIndex()]);
    }

    void ApplicationNodeInternal::BaseDrawFrame()
    {
        if (applicationHalted_) return;
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        appNodeImpl_->DrawFrame(framebuffers_[GetEngine()->getCurrentWindowIndex()]);
    }

    void ApplicationNodeInternal::BaseDraw2D()
    {
        if (applicationHalted_) return;
        auto window = GetEngine()->getCurrentWindowPtr();

        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfwGL3_NewFrame(-GetViewportScreen(window->getId()).position_, GetViewportQuadSize(window->getId()), GetViewportScreen(window->getId()).size_, GetViewportScaling(window->getId()), GetCurrentAppTime(), GetElapsedTime());
        else if (engine_->isMaster()) ImGui_ImplGlfwGL3_NewFrame(-GetViewportScreen(window->getId()).position_, GetViewportQuadSize(window->getId()), GetViewportScreen(window->getId()).size_, GetViewportScaling(window->getId()), GetCurrentAppTime(), GetElapsedTime());

        auto& fbo = framebuffers_[GetEngine()->getCurrentWindowIndex()];
        appNodeImpl_->Draw2D(fbo);

        fbo.DrawToFBO([this]() {
            // ImGui::Render for slaves is called in SlaveNodeInternal...
            if (engine_->isMaster()) ImGui::Render();
        });
    }

    void ApplicationNodeInternal::BasePostDraw()
    {
        if (applicationHalted_) return;
        appNodeImpl_->PostDraw();
        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfwGL3_FinishAllFrames();
        else if (engine_->isMaster()) ImGui_ImplGlfwGL3_FinishAllFrames();
    }

    void ApplicationNodeInternal::BaseCleanUp()
    {
        std::lock_guard<std::mutex> lock{ instanceMutex_ };
        instance_ = nullptr;

        if constexpr (SHOW_CLIENT_GUI) ImGui_ImplGlfwGL3_Shutdown();
        else if (GetEngine()->isMaster()) ImGui_ImplGlfwGL3_Shutdown();
        appNodeImpl_->CleanUp();
        initialized_ = false;
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
        if (!initialized_) return;
        keyPressedState_[key] = (action == GLFW_RELEASE) ? false : true;

        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            keyboardEvents_.emplace_back(key, scancode, action, mods);
#endif

            if constexpr (!SHOW_CLIENT_GUI) {
                ImGui_ImplGlfwGL3_KeyCallback(key, scancode, action, mods);
                if (ImGui::GetIO().WantCaptureKeyboard) return;
            }

            appNodeImpl_->KeyboardCallback(key, scancode, action, mods);
        }
    }

    void ApplicationNodeInternal::BaseCharCallback(unsigned int character, int mods)
    {
        if (!initialized_) return;
        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            charEvents_.emplace_back(character, mods);
#endif

            if constexpr (!SHOW_CLIENT_GUI) {
                ImGui_ImplGlfwGL3_CharCallback(character);
                if (ImGui::GetIO().WantCaptureKeyboard) return;
            }

            appNodeImpl_->CharCallback(character, mods);
        }
    }

    void ApplicationNodeInternal::BaseMouseButtonCallback(int button, int action)
    {
        if (!initialized_) return;
        mousePressedState_[button] = (action == GLFW_RELEASE) ? false : true;

        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            mouseButtonEvents_.emplace_back(button, action);
#endif

            if constexpr (!SHOW_CLIENT_GUI) {
                ImGui_ImplGlfwGL3_MouseButtonCallback(button, action, 0);
                if (ImGui::GetIO().WantCaptureMouse) return;
            }

            appNodeImpl_->MouseButtonCallback(button, action);
        }
    }

    void ApplicationNodeInternal::BaseMousePosCallback(double x, double y)
    {
        if (!initialized_) return;
        auto mousePos = glm::dvec2(x, y);
        if (engine_->isMaster()) {
            mousePos = ConvertInputCoordinatesLocalToGlobal(mousePos);

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

            if constexpr (!SHOW_CLIENT_GUI) {
                ImGui_ImplGlfwGL3_MousePositionCallback(mousePos.x, mousePos.y);
                if (ImGui::GetIO().WantCaptureMouse) return;
            }

            appNodeImpl_->MousePosCallback(mousePos.x, mousePos.y);
        }
    }

    void ApplicationNodeInternal::BaseMouseScrollCallback(double xoffset, double yoffset)
    {
        if (!initialized_) return;
        if (engine_->isMaster()) {
#ifdef VISCOM_SYNCINPUT
            mouseScrollEvents_.emplace_back(xoffset, yoffset);
#endif

            if constexpr (!SHOW_CLIENT_GUI) {
                ImGui_ImplGlfwGL3_ScrollCallback(xoffset, yoffset);
                if (ImGui::GetIO().WantCaptureMouse) return;
            }

            appNodeImpl_->MouseScrollCallback(xoffset, yoffset);
        }
    }

    void ApplicationNodeInternal::BaseDataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID)
    {
        if (!initialized_) return;
        auto splitID = reinterpret_cast<std::uint16_t*>(&packageID);

        if (splitID[0] == static_cast<std::uint16_t>(InternalTransferTypeLarge::UserData)) {
            appNodeImpl_->DataTransferCallback(receivedData, receivedLength, splitID[1], clientID);
            return;
        }
        auto internalID = reinterpret_cast<std::uint8_t*>(&splitID[0]);
        switch (static_cast<InternalTransferType>(internalID[0])) {
        case InternalTransferType::ResourceTransfer:
            CreateSynchronizedResource(static_cast<ResourceType>(internalID[1]), receivedData, receivedLength);
            break;
        case InternalTransferType::ResourceReleaseTransfer:
            ReleaseSynchronizedResource(static_cast<ResourceType>(internalID[1]), std::string_view(reinterpret_cast<char*>(receivedData), receivedLength));
            break;
        case InternalTransferType::ResourceRequest:
            SendResourcesToNode(static_cast<ResourceType>(internalID[1]), receivedData, receivedLength, clientID);
            break;
        default:
            LOG(WARNING) << "Unknown InternalTransferType: " << internalID[0];
            break;
        }
    }

    void ApplicationNodeInternal::BaseDataAcknowledgeCallback(int packageID, int clientID)
    {
        if (!initialized_) return;
        auto splitID = reinterpret_cast<std::uint16_t*>(&packageID);

        if (splitID[0] == -1) appNodeImpl_->DataAcknowledgeCallback(splitID[1], clientID);
        auto internalID = reinterpret_cast<std::uint8_t*>(&splitID[0]);
        switch (static_cast<InternalTransferType>(internalID[0])) {
        case InternalTransferType::ResourceTransfer:
            break;
        case InternalTransferType::ResourceReleaseTransfer:
            break;
        case InternalTransferType::ResourceRequest:
            break;
        default:
            LOG(WARNING) << "Unknown InternalTransferType: " << internalID[0];
            break;
        }
    }

    void ApplicationNodeInternal::BaseDataTransferStatusCallback(bool connected, int clientID)
    {
        if (!initialized_) return;
        appNodeImpl_->DataTransferStatusCallback(connected, clientID);
    }

    // ReSharper restore CppMemberFunctionMayBeConst

    void ApplicationNodeInternal::addTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (!initialized_) return;
        if (engine_->isMaster()) {
#ifdef VISCOM_USE_TUIO
            auto tPoint = tcur->getPosition();
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->AddTuioCursor(tcur);
#endif
        }
    }

    void ApplicationNodeInternal::updateTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (!initialized_) return;
        if (engine_->isMaster()) {
#ifdef VISCOM_USE_TUIO
            auto tPoint = tcur->getPosition();
            // TODO: TUIO events will not be synced currently. [5/27/2017 Sebastian Maisch]
            appNodeImpl_->UpdateTuioCursor(tcur);
#endif
        }
    }

    void ApplicationNodeInternal::removeTuioCursor(TUIO::TuioCursor* tcur)
    {
        if (!initialized_) return;
        if (engine_->isMaster()) {
#ifdef VISCOM_USE_TUIO
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

    glm::dvec2 ApplicationNodeInternal::ConvertInputCoordinatesLocalToGlobal(const glm::dvec2& p)
    {
        glm::dvec2 result{ p.x, static_cast<double>(viewportScreen_[0].size_.y) - p.y };
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

    void ApplicationNodeInternal::TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex)
    {
        int completePackageId = 0;
        auto splitId = reinterpret_cast<std::uint16_t*>(&completePackageId);
        splitId[0] = static_cast<std::uint16_t>(InternalTransferTypeLarge::UserData);
        splitId[1] = packageId;
        // So SGCT seems to have different node indices for the data connections than the actual node indices.
        // Basically: data connection indices are successive numbers starting at 0 but leaving out the current node.
        // As connections only exist between masters and slaves (no direct inter-node connection possible, thanks documentation, not)
        // we can just reduce the node index by 1 when on master ...
        // and check that we do not connect to other nodes on slaves.
        if (engine_->isMaster()) nodeIndex -= 1;
        else if (nodeIndex != 0) LOG(WARNING) << "SGCT does not allow inter-node connections (nodeIndex: " << nodeIndex << ").";
        engine_->transferDataToNode(data, static_cast<int>(length), completePackageId, nodeIndex);
    }

    void ApplicationNodeInternal::TransferData(const void* data, std::size_t length, std::uint16_t packageId)
    {
        int completePackageId = 0;
        auto splitId = reinterpret_cast<std::uint16_t*>(&completePackageId);
        splitId[0] = static_cast<std::uint16_t>(InternalTransferTypeLarge::UserData);
        splitId[1] = packageId;
        // So SGCT seems to have different node indices for the data connections than the actual node indices.
        // Basically: data connection indices are successive numbers starting at 0 but leaving out the current node.
        // As connections only exist between masters and slaves (no direct inter-node connection possible, thanks documentation, not)
        // we can just reduce the node index by 1 when on master ...
        // and check that we do not connect to other nodes on slaves.
        if (!engine_->isMaster()) LOG(WARNING) << "SGCT does not allow inter-node connections (all nodes).";
        engine_->transferDataBetweenNodes(data, static_cast<int>(length), completePackageId);
    }

    void ApplicationNodeInternal::TransferResource(std::string_view name, const void* data, std::size_t length, ResourceType type)
    {
        if (engine_->isMaster()) {
            int completePackageId = 0;
            auto splitId = reinterpret_cast<std::uint8_t*>(&completePackageId);
            splitId[0] = static_cast<std::uint8_t>(InternalTransferType::ResourceTransfer);
            splitId[1] = static_cast<std::uint8_t>(type);

            std::vector<std::uint8_t> transferedData(sizeof(std::size_t) + name.length() + length);
            *reinterpret_cast<std::size_t*>(&transferedData[0]) = name.length();
            memcpy(&transferedData[0] + sizeof(std::size_t), name.data(), name.length());
            utils::memcpyfaster(&transferedData[0] + sizeof(std::size_t) + name.length(), data, length);
            // So SGCT seems to have different node indices for the data connections than the actual node indices.
            // Basically: data connection indices are successive numbers starting at 0 but leaving out the current node.
            // As connections only exist between masters and slaves (no direct inter-node connection possible, thanks documentation, not)
            // we can just reduce the node index by 1 when on master ...
            // and check that we do not connect to other nodes on slaves.
            if (!engine_->isMaster()) LOG(WARNING) << "SGCT does not allow inter-node connections (all nodes).";
            engine_->transferDataBetweenNodes(transferedData.data(), static_cast<int>(transferedData.size()), completePackageId);
        }
    }

    void ApplicationNodeInternal::TransferResourceToNode(std::string_view name, const void* data, std::size_t length, ResourceType type, std::size_t nodeIndex)
    {
        if (engine_->isMaster()) {
            auto completePackageId = MakePackageID(static_cast<std::uint8_t>(InternalTransferType::ResourceTransfer), static_cast<std::uint8_t>(type), 0);

            std::vector<std::uint8_t> transferedData(sizeof(std::size_t) + name.length() + length);
            *reinterpret_cast<std::size_t*>(&transferedData[0]) = name.length();
            memcpy(&transferedData[0] + sizeof(std::size_t), name.data(), name.length());
            utils::memcpyfaster(&transferedData[0] + sizeof(std::size_t) + name.length(), data, length);
            // So SGCT seems to have different node indices for the data connections than the actual node indices.
            // Basically: data connection indices are successive numbers starting at 0 but leaving out the current node.
            // As connections only exist between masters and slaves (no direct inter-node connection possible, thanks documentation, not)
            // we can just reduce the node index by 1 when on master ...
            // and check that we do not connect to other nodes on slaves.
            if (engine_->isMaster()) nodeIndex -= 1;
            else if (nodeIndex != 0) LOG(WARNING) << "SGCT does not allow inter-node connections (nodeIndex: " << nodeIndex << ").";
            engine_->transferDataToNode(transferedData.data(), static_cast<int>(transferedData.size()), completePackageId, nodeIndex);
        }
    }

    void ApplicationNodeInternal::TransferReleaseResource(std::string_view name, ResourceType type)
    {
        if (!initialized_) return;
        if (engine_->isMaster()) {
            auto completePackageId = MakePackageID(static_cast<std::uint8_t>(InternalTransferType::ResourceReleaseTransfer), static_cast<std::uint8_t>(type), 0);
            engine_->transferDataBetweenNodes(name.data(), static_cast<int>(name.length()), completePackageId);
        }
    }

    void ApplicationNodeInternal::RequestSharedResources()
    {
        if (!engine_->isMaster()) {
            auto completePackageId = MakePackageID(static_cast<std::uint8_t>(InternalTransferType::ResourceRequest), static_cast<std::uint8_t>(ResourceType::All_Resources), 0);
            int tmp = 0;
            engine_->transferDataToNode(&tmp, sizeof(int), completePackageId, 0);
        }
    }

    void ApplicationNodeInternal::WaitForResource(const std::string& name, ResourceType type)
    {
        switch (type) {
        case ResourceType::GPUProgram:
            gpuProgramManager_.WaitForResource(name);
            break;
        case ResourceType::Mesh:
            meshManager_.WaitForResource(name);
            break;
        case ResourceType::Texture:
            textureManager_.WaitForResource(name);
            break;
        default:
            LOG(WARNING) << "Unknown ResourceTransferType: " << static_cast<std::uint8_t>(type);
            break;
        }
    }

    bool ApplicationNodeInternal::IsMaster() const
    {
        return engine_->isMaster();
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

    std::vector<FrameBuffer> ApplicationNodeInternal::CreateOffscreenBuffers(const FrameBufferDescriptor & fboDesc, int sizeDivisor) const
    {
        std::vector<FrameBuffer> result;
        for (std::size_t i = 0; i < viewportScreen_.size(); ++i) {
            const auto& fboSize = viewportQuadSize_[i] / sizeDivisor;
            result.emplace_back(fboSize.x, fboSize.y, fboDesc);
            LOG(DBUG) << "Offscreen FBO VP Pos: " << 0.0f << ", " << 0.0f;
            LOG(DBUG) << "Offscreen FBO VP Size: " << fboSize.x << ", " << fboSize.y;
            result.back().SetStandardViewport(0, 0, fboSize.x, fboSize.y);
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

    void ApplicationNodeInternal::ReleaseSynchronizedResource(ResourceType type, std::string_view name)
    {
        switch (type) {
        case ResourceType::GPUProgram:
            gpuProgramManager_.ReleaseSharedResource(std::string(name));
            break;
        case ResourceType::Mesh:
            meshManager_.ReleaseSharedResource(std::string(name));
            break;
        case ResourceType::Texture:
            textureManager_.ReleaseSharedResource(std::string(name));
            break;
        default:
            LOG(WARNING) << "Unknown ResourceTransferType: " << static_cast<std::uint8_t>(type);
            break;
        }
    }

    void ApplicationNodeInternal::CreateSynchronizedResource(ResourceType type, const void* data, std::size_t length)
    {
        auto resourceNamePtr = reinterpret_cast<const std::size_t*>(data);
        std::string resourceName;
        resourceName.resize(resourceNamePtr[0]);
        memcpy(resourceName.data(), &resourceNamePtr[1], resourceNamePtr[0]);
        auto dataPtr = reinterpret_cast<const std::uint8_t*>(&resourceNamePtr[1]) + resourceNamePtr[0];
        auto dataLength = length - sizeof(std::size_t) - resourceNamePtr[0];


        ResourceData key;
        key.type_ = type;
        key.name_ = resourceName;
        auto rit = std::find(creatableResources_.begin(), creatableResources_.end(), key);
        if (rit != creatableResources_.end()) {
            LOG(WARNING) << "Resource already synchronized: " << resourceName << "(" << type << ")";
        }
        else {
            creatableResources_.emplace_back(std::move(key));
            creatableResources_.back().data_.resize(dataLength);
            utils::memcpyfaster(creatableResources_.back().data_.data(), dataPtr, dataLength);
        }
    }

    void ApplicationNodeInternal::CreateSynchronizedResources()
    {
        std::lock_guard<std::mutex> accessLock{ creatableResourceMutex_ };
        for (const auto& res : creatableResources_) {
            auto dataPtr = res.data_.data();
            auto dataLength = res.data_.size();

            switch (res.type_) {
            case ResourceType::GPUProgram:
                gpuProgramManager_.CreateSharedResource(res.name_, dataPtr, dataLength, std::vector<std::string>());
                break;
            case ResourceType::Mesh:
                meshManager_.CreateSharedResource(res.name_, dataPtr, dataLength);
                break;
            case ResourceType::Texture:
                textureManager_.CreateSharedResource(res.name_, dataPtr, dataLength);
                break;
            default:
                LOG(WARNING) << "Unknown ResourceTransferType: " << static_cast<std::uint8_t>(res.type_);
                break;
            }
        }
        creatableResources_.clear();
    }

    void ApplicationNodeInternal::SendResourcesToNode(ResourceType type, const void* data, std::size_t length, int clientID)
    {
        if (!engine_->isMaster()) return;

        if (type == ResourceType::All_Resources) {
            textureManager_.SynchronizeAllResourcesToNode(clientID);
            meshManager_.SynchronizeAllResourcesToNode(clientID);
            gpuProgramManager_.SynchronizeAllResourcesToNode(clientID);
        }
        else {
            std::string name(reinterpret_cast<const char*>(data), length);
            switch (type)
            {
            case viscom::ResourceType::Texture:
                textureManager_.SynchronizeResourceToNode(name, clientID);
                break;
            case viscom::ResourceType::Mesh:
                meshManager_.SynchronizeResourceToNode(name, clientID);
                break;
            case viscom::ResourceType::GPUProgram:
                gpuProgramManager_.SynchronizeResourceToNode(name, clientID);
                break;
            default:
                LOG(WARNING) << "Unknown ResourceTransferType: " << static_cast<std::uint8_t>(type);
                break;
            }
        }
    }

    int ApplicationNodeInternal::MakePackageID(std::uint8_t internalType, std::uint8_t internalPID, std::uint16_t userPID)
    {
        int completePackageId = 0;
        auto splitId = reinterpret_cast<std::uint8_t*>(&completePackageId);
        splitId[0] = internalType;
        splitId[1] = internalPID;
        reinterpret_cast<std::uint16_t*>(&completePackageId)[1] = userPID;

        return completePackageId;
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
