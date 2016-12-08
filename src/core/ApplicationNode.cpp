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

namespace viscom {

    ApplicationNode* ApplicationNode::instance_ = nullptr;

    ApplicationNode::ApplicationNode(FWConfiguration&& config, std::unique_ptr<sgct::Engine> engine) :
        config_( std::move(config) ),
        engine_{ std::move(engine) },
        startNode_{ 0 },
        masterSocketPort_{ "27772" },
        currentTimeSynced_{ 0.0 },
        currentTime_{ 0.0 }
    {
        loadProperties();
        engine_->setInitOGLFunction([app = this]() { app->BaseInitOpenGL(); });
        engine_->setPreSyncFunction([app = this](){ app->BasePreSync(); });
        engine_->setPostSyncPreDrawFunction([app = this]() { app->PostSyncFunction(); });
        engine_->setDrawFunction([app = this]() { app->BaseDrawFrame(); });
        engine_->setCleanUpFunction([app = this](){ app->BaseCleanUp(); });

        /*

        void setPreWindowFunction(sgct_cppxeleven::function<void(void)> fn);
        void setClearBufferFunction(sgct_cppxeleven::function<void(void)> fn);
        void setDraw2DFunction(sgct_cppxeleven::function<void(void)> fn);
        void setPostDrawFunction(sgct_cppxeleven::function<void(void)> fn);

        void setKeyboardCallbackFunction(sgct_cppxeleven::function<void(int, int, int, int)> fn); //arguments: int key, int scancode, int action, int mods
        void setCharCallbackFunction(sgct_cppxeleven::function<void(unsigned int, int)> fn); //arguments: unsigned int unicode character, int mods
        void setMouseButtonCallbackFunction(sgct_cppxeleven::function<void(int, int)> fn); //arguments: int button, int action
        void setMousePosCallbackFunction(sgct_cppxeleven::function<void(double, double)> fn); //arguments: double x, double y
        void setMouseScrollCallbackFunction(sgct_cppxeleven::function<void(double, double)> fn); //arguments: double xoffset, double yoffset
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

    void ApplicationNode::BaseInitOpenGL()
    {
        if (engine_->isMaster()) appNodeImpl_ = std::make_unique<MasterNode>(this);
        else appNodeImpl_ = std::make_unique<SlaveNode>(this);
        appNodeImpl_->InitOpenGL();
    }

    void ApplicationNode::BasePreSync()
    {
        if (engine_->isMaster()) currentTimeSynced_.setVal(sgct::Engine::getTime());
        appNodeImpl_->PreSync();
    }

    void ApplicationNode::PostSyncFunction()
    {
        appNodeImpl_->UpdateSyncedInfo();
        auto lastTime = currentTime_;
        currentTime_ = currentTimeSynced_.getVal();
        auto elapsed = currentTimeSynced_ - lastTime;
        appNodeImpl_->UpdateFrame(currentTime_, elapsed);
    }

    void ApplicationNode::BaseDrawFrame() const
    {
        appNodeImpl_->DrawFrame();
    }

    void ApplicationNode::BaseCleanUp() const
    {
        instance_ = nullptr;
        appNodeImpl_->CleanUp();
    }

    void ApplicationNode::BaseEncodeData()
    {
        sgct::SharedData::instance()->writeDouble(&currentTimeSynced_);
        appNodeImpl_->EncodeData();
    }

    void ApplicationNode::BaseDecodeData()
    {
        sgct::SharedData::instance()->readDouble(&currentTimeSynced_);
        appNodeImpl_->DecodeData();
    }

    void ApplicationNode::BaseEncodeDataStatic()
    {
        if (instance_) instance_->BaseEncodeData();
    }

    void ApplicationNode::BaseDecodeDataStatic()
    {
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
                sgct_core::SGCTNode * currNode = sgct_core::ClusterManager::instance()->getNodePtr(i);
                for (int j = 0; j < currNode->getNumberOfWindows(); j++) {
                    if (i == nodeId && j == windowId) return current_projector;

                    current_projector += 1;
                }
            }
        }
        return 0;
    }
}
