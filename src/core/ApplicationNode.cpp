/**
 * @file   ApplicationNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the base application node base class.
 */

#include "ApplicationNode.h"
#include "external/tinyxml2.h"
#include "app/MasterNode.h"
#include "app/SlaveNode.h"
#include "OpenCVParserHelper.h"
#include <imgui.h>
#include "core/imgui/imgui_impl_glfw_gl3.h"

namespace viscom {

    ApplicationNode* ApplicationNode::instance_{ nullptr };
    std::mutex ApplicationNode::instanceMutex_{ };

    ApplicationNode::ApplicationNode(FWConfiguration&& config, std::unique_ptr<sgct::Engine> engine) :
        config_( std::move(config) ),
        engine_{ std::move(engine) },
        startNode_{ 0 },
        masterSocketPort_{ "27772" },
        currentTimeSynced_{ 0.0 },
        currentTime_{ 0.0 }
    {
        loadProperties();
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


    ApplicationNode::~ApplicationNode() = default;

    void ApplicationNode::InitNode()
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

    void ApplicationNode::Render() const
    {
        engine_->render();
    }

    void ApplicationNode::BasePreWindow()
    {
        if (engine_->isMaster()) appNodeImpl_ = std::make_unique<MasterNode>(this);
        else appNodeImpl_ = std::make_unique<SlaveNode>(this);
    }

    void ApplicationNode::BaseInitOpenGL()
    {
        auto numWindows = sgct_core::ClusterManager::instance()->getThisNodePtr()->getNumberOfWindows();
        viewport_.resize(numWindows, std::make_pair(glm::ivec2(0), glm::ivec2()));
        viewportOrigin_.resize(numWindows, glm::ivec2(0));
        viewportScaling_.resize(numWindows, glm::vec2(1.0f));
        viewportSize_.resize(numWindows, glm::ivec2(0));

        for (size_t wId = 0; wId < numWindows; ++wId) {
            glm::ivec2 projectorSize;
            auto window = GetEngine()->getWindowPtr(wId);
            window->getFinalFBODimensions(projectorSize.x, projectorSize.y);
            viewport_[wId].second = projectorSize;
            viewportOrigin_[wId] = glm::ivec2(0);
            viewportScaling_[wId] = glm::vec2(projectorSize) / glm::vec2(1920.0f, 1080.0f);
            viewportSize_[wId] = projectorSize;
        }

        ImGui_ImplGlfwGL3_Init(GetEngine()->getCurrentWindowPtr()->getWindowHandle(), !GetEngine()->isMaster());

        appNodeImpl_->InitOpenGL();
    }

    void ApplicationNode::BasePreSync()
    {
        if (engine_->isMaster()) {
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

            currentTimeSynced_.setVal(sgct::Engine::getTime());
        }
        appNodeImpl_->PreSync();
    }

    void ApplicationNode::PostSyncFunction()
    {
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

        auto lastTime = currentTime_;
        currentTime_ = currentTimeSynced_.getVal();
        appNodeImpl_->UpdateSyncedInfo();

        auto elapsed = currentTimeSynced_ - lastTime;
        appNodeImpl_->UpdateFrame(currentTime_, elapsed);
    }

    void ApplicationNode::BaseClearBuffer() const
    {
        appNodeImpl_->ClearBuffer();
    }

    void ApplicationNode::BaseDrawFrame() const
    {
        appNodeImpl_->DrawFrame();
    }

    void ApplicationNode::BaseDraw2D() const
    {
        appNodeImpl_->Draw2D();
    }

    void ApplicationNode::BasePostDraw() const
    {
        appNodeImpl_->PostDraw();
    }

    void ApplicationNode::BaseCleanUp() const
    {
        std::lock_guard<std::mutex> lock{ instanceMutex_ };
        instance_ = nullptr;
        ImGui_ImplGlfwGL3_Shutdown();
        appNodeImpl_->CleanUp();
    }

    void ApplicationNode::BaseKeyboardCallback(int key, int scancode, int action, int mods)
    {
        if (engine_->isMaster()) keyboardEvents_.emplace_back(key, scancode, action, mods);
        appNodeImpl_->KeyboardCallback(key, scancode, action, mods);
    }

    void ApplicationNode::BaseCharCallback(unsigned int character, int mods)
    {
        if (engine_->isMaster()) charEvents_.emplace_back(character, mods);
        appNodeImpl_->CharCallback(character, mods);
    }

    void ApplicationNode::BaseMouseButtonCallback(int button, int action)
    {
        if (engine_->isMaster()) mouseButtonEvents_.emplace_back(button, action);
        appNodeImpl_->MouseButtonCallback(button, action);
    }

    void ApplicationNode::BaseMousePosCallback(double x, double y)
    {
        x /= static_cast<double>(viewport_[0].second.x);
        y /= static_cast<double>(viewport_[0].second.y);
        if (engine_->isMaster()) {
            mousePosEvents_.emplace_back(x, y);
            appNodeImpl_->MousePosCallback(x, y);
        }
    }

    void ApplicationNode::BaseMouseScrollCallback(double xoffset, double yoffset)
    {
        if (engine_->isMaster()) mouseScrollEvents_.emplace_back(xoffset, yoffset);
        appNodeImpl_->MouseScrollCallback(xoffset, yoffset);
    }

    void ApplicationNode::BaseEncodeData()
    {
        sgct::SharedData::instance()->writeVector(&keyboardEventsSynced_);
        sgct::SharedData::instance()->writeVector(&charEventsSynced_);
        sgct::SharedData::instance()->writeVector(&mouseButtonEventsSynced_);
        sgct::SharedData::instance()->writeVector(&mousePosEventsSynced_);
        sgct::SharedData::instance()->writeVector(&mouseScrollEventsSynced_);
        sgct::SharedData::instance()->writeDouble(&currentTimeSynced_);
        appNodeImpl_->EncodeData();
    }

    void ApplicationNode::BaseDecodeData()
    {
        sgct::SharedData::instance()->readVector(&keyboardEventsSynced_);
        sgct::SharedData::instance()->readVector(&charEventsSynced_);
        sgct::SharedData::instance()->readVector(&mouseButtonEventsSynced_);
        sgct::SharedData::instance()->readVector(&mousePosEventsSynced_);
        sgct::SharedData::instance()->readVector(&mouseScrollEventsSynced_);
        sgct::SharedData::instance()->readDouble(&currentTimeSynced_);
        appNodeImpl_->DecodeData();
    }

    void ApplicationNode::BaseEncodeDataStatic()
    {
        std::lock_guard<std::mutex> lock{ instanceMutex_ };
        if (instance_) instance_->BaseEncodeData();
    }

    void ApplicationNode::BaseDecodeDataStatic()
    {
        std::lock_guard<std::mutex> lock{ instanceMutex_ };
        if (instance_) instance_->BaseDecodeData();
    }

    void ApplicationNode::loadProperties()
    {
        tinyxml2::XMLDocument doc;
        OpenCVParserHelper::LoadXMLDocument("Program properties", config_.programProperties_, doc);

        startNode_ = OpenCVParserHelper::ParseText<unsigned int>(doc.FirstChildElement("opencv_storage")->FirstChildElement("startNode"));
        masterSocketPort_ = OpenCVParserHelper::ParseTextString(doc.FirstChildElement("opencv_storage")->FirstChildElement("masterSocketPort"));
    }

    unsigned int ApplicationNode::GetGlobalProjectorId(int nodeId, int windowId) const
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
}
