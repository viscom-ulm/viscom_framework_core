/**
 * @file   FrameworkInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the internal framework class for the VISCOM lab cluster.
 */

#include "FrameworkInternal.h"
#include "external/tinyxml2.h"
#include "core/utils/utils.h"
#include <imgui.h>
#include <sgct.h>
#include "sgct_wrapper.h"
#include "core/app_internal/CoordinatorNodeInternal.h"
#include "core/app_internal/WorkerNodeInternal.h"
#include "core/app/ApplicationNodeBase.h"

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

    FrameworkInternal* FrameworkInternal::instance_{ nullptr };
    std::mutex FrameworkInternal::instanceMutex_{ };

    FrameworkInternal::FrameworkInternal(FWConfiguration&& config, std::unique_ptr<sgct::Engine> engine) :
        config_( std::move(config) ),
        engine_{ std::move(engine) },
        camHelper_{ engine_.get() },
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


    FrameworkInternal::~FrameworkInternal() = default;

    void FrameworkInternal::InitNode(InitNodeFunc coordinatorNodeFactory, InitNodeFunc workerNodeFactory)
    {
        coordinatorNodeFactory_ = std::move(coordinatorNodeFactory);
        workerNodeFactory_ = std::move(workerNodeFactory);

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

        sgct::SharedData::instance()->setEncodeFunction(BaseEncodeDataStatic);
        sgct::SharedData::instance()->setDecodeFunction(BaseDecodeDataStatic);
    }

    void FrameworkInternal::Render() const
    {
        engine_->render();
    }

    void FrameworkInternal::BasePreWindow()
    {
    }

    void FrameworkInternal::BaseInitOpenGL()
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

            auto vpLocalLowerLeftA = sgct_wrapper::GetProjectionPlaneCoordinate(window, 0, sgct_core::SGCTProjectionPlane::LowerLeft);
            glm::vec2 vpLocalLowerLeft{ vpLocalLowerLeftA[0], vpLocalLowerLeftA[1] };
            auto vpLocalUpperLeftA = sgct_wrapper::GetProjectionPlaneCoordinate(window, 0, sgct_core::SGCTProjectionPlane::UpperLeft);
            glm::vec2 vpLocalUpperLeft{ vpLocalUpperLeftA[0], vpLocalUpperLeftA[1] };
            auto vpLocalUpperRightA = sgct_wrapper::GetProjectionPlaneCoordinate(window, 0, sgct_core::SGCTProjectionPlane::UpperRight);
            glm::vec2 vpLocalUpperRight{ vpLocalUpperRightA[0], vpLocalUpperRightA[1] };
            // glm::vec2 vpLocalLowerLeft = glm::vec2(window->getViewport(0)->getProjectionPlane()->getCoordinate(sgct_core::SGCTProjectionPlane::LowerLeft));
            // glm::vec2 vpLocalUpperLeft = glm::vec2(window->getViewport(0)->getProjectionPlane()->getCoordinate(sgct_core::SGCTProjectionPlane::UpperLeft));
            // glm::vec2 vpLocalUpperRight = glm::vec2(window->getViewport(0)->getProjectionPlane()->getCoordinate(sgct_core::SGCTProjectionPlane::UpperRight));
            glm::vec2 vpLocalSize = vpLocalUpperRight - vpLocalLowerLeft;
            glm::vec2 vpTotalSize = 2.0f * GetConfig().nearPlaneSize_;

            glm::vec2 totalScreenSize = glm::ceil((vpTotalSize / vpLocalSize) * glm::vec2(projectorSize));

            viewportScreen_[wId].position_ = ((vpLocalLowerLeft + GetConfig().nearPlaneSize_) / vpTotalSize) * totalScreenSize;
            viewportScreen_[wId].size_ = glm::ivec2(totalScreenSize);
            viewportQuadSize_[wId] = projectorSize;
            viewportScaling_[wId] = totalScreenSize / config_.virtualScreenSize_;

            // glm::vec2 relPosScale = 1.0f / glm::vec2(viewportQuadSize_[wId]);
            // glm::vec2 scaledRelPos = (glm::vec2(viewportScreen_[wId].position_) / glm::vec2(viewportScreen_[wId].size_)) * relPosScale;
            
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

        FullscreenQuad::InitializeStatic();
        RequestSharedResources();

        if (engine_->isMaster()) appNodeInternal_ = std::make_unique<CoordinatorNodeInternal>(*this);
        else appNodeInternal_ = std::make_unique<WorkerNodeInternal>(*this);
PUSH_DISABLE_DEPRECATED_WARNINGS
        appNodeInternal_->PreWindow();
POP_WARNINGS
        appNodeInternal_->InitImplementation();
    }

    void FrameworkInternal::BasePreSync()
    {
        appNodeInternal_->PreSync();
    }

    void FrameworkInternal::PostSyncFunction()
    {
        appNodeInternal_->PostSync();
    }

    void FrameworkInternal::BaseClearBuffer()
    {
        if (applicationHalted_) return;
        appNodeInternal_->ClearBuffer(framebuffers_[GetEngine()->getCurrentWindowIndex()]);
    }

    void FrameworkInternal::BaseDrawFrame()
    {
        if (applicationHalted_) return;
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        appNodeInternal_->DrawFrame(framebuffers_[GetEngine()->getCurrentWindowIndex()]);
    }

    void FrameworkInternal::BaseDraw2D()
    {
        if (applicationHalted_) return;
        appNodeInternal_->Draw2D(framebuffers_[GetEngine()->getCurrentWindowIndex()]);
    }

    void FrameworkInternal::BasePostDraw()
    {
        if (applicationHalted_) return;
        appNodeInternal_->PostDraw();
    }

    void FrameworkInternal::BaseCleanUp()
    {
PUSH_DISABLE_DEPRECATED_WARNINGS
        appNodeInternal_->CleanUp();
POP_WARNINGS
        appNodeInternal_ = nullptr;

        std::lock_guard<std::mutex> lock{ instanceMutex_ };
        instance_ = nullptr;

        initialized_ = false;
    }

    bool FrameworkInternal::IsMouseButtonPressed(int button) const noexcept
    {
        return mousePressedState_[button];
    }

    bool FrameworkInternal::IsKeyPressed(int key) const noexcept
    {
        return keyPressedState_[key];
    }

    void FrameworkInternal::BaseKeyboardCallback(int key, int scancode, int action, int mods)
    {
        if (!initialized_ || !appNodeInternal_) return;
        keyPressedState_[key] = (action == GLFW_RELEASE) ? false : true;

        appNodeInternal_->KeyboardCallback(key, scancode, action, mods);
    }

    void FrameworkInternal::BaseCharCallback(unsigned int character, int mods)
    {
        if (!initialized_ || !appNodeInternal_) return;

        appNodeInternal_->CharCallback(character, mods);
    }

    void FrameworkInternal::BaseMouseButtonCallback(int button, int action)
    {
        if (!initialized_ || !appNodeInternal_) return;
        mousePressedState_[button] = (action == GLFW_RELEASE) ? false : true;

        appNodeInternal_->MouseButtonCallback(button, action);
    }

    void FrameworkInternal::BaseMousePosCallback(double x, double y)
    {
        if (!initialized_ || !appNodeInternal_) return;
        auto mousePos = ConvertInputCoordinatesLocalToGlobal(glm::dvec2(x, y));
        mousePosition_ = glm::vec2(mousePos.x, mousePos.y);
        mousePositionNormalized_.x = (2.0f * mousePosition_.x - 1.0f);
        mousePositionNormalized_.y = -(2.0f * mousePosition_.y - 1.0f);

        appNodeInternal_->MousePosCallback(mousePos.x, mousePos.y);
    }

    void FrameworkInternal::BaseMouseScrollCallback(double xoffset, double yoffset)
    {
        if (!initialized_ || !appNodeInternal_) return;
        appNodeInternal_->MouseScrollCallback(xoffset, yoffset);
    }

    void FrameworkInternal::BaseDataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID)
    {
        if (!initialized_ || !appNodeInternal_) return;
        auto splitID = reinterpret_cast<std::uint16_t*>(&packageID);

        if (splitID[0] == static_cast<std::uint16_t>(InternalTransferTypeLarge::UserData)) {
            appNodeInternal_->DataTransfer(receivedData, receivedLength, splitID[1], clientID);
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

    void FrameworkInternal::BaseDataAcknowledgeCallback(int packageID, int clientID)
    {
        if (!initialized_ || !appNodeInternal_) return;
        auto splitID = reinterpret_cast<std::uint16_t*>(&packageID);

        if (splitID[0] == static_cast<std::uint16_t>(-1)) appNodeInternal_->DataAcknowledge(splitID[1], clientID);
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

    void FrameworkInternal::BaseDataTransferStatusCallback(bool connected, int clientID)
    {
        if (!initialized_ || !appNodeInternal_) return;
        appNodeInternal_->DataTransferStatus(connected, clientID);
    }

    void FrameworkInternal::BaseEncodeDataStatic()
    {
        std::lock_guard<std::mutex> lock{ instanceMutex_ };
        if (instance_) instance_->BaseEncodeData();
    }

    void FrameworkInternal::BaseDecodeDataStatic()
    {
        std::lock_guard<std::mutex> lock{ instanceMutex_ };
        if (instance_) instance_->BaseDecodeData();
    }

    void FrameworkInternal::BaseEncodeData()
    {
        appNodeInternal_->EncodeData();
    }

    void FrameworkInternal::BaseDecodeData()
    {
        appNodeInternal_->DecodeData();
    }

    void FrameworkInternal::SetCursorInputMode(int mode)
    {
        if (engine_->isMaster()) {
            glfwSetInputMode(engine_->getWindowPtr(0)->getWindowHandle(), GLFW_CURSOR, mode);
        }
    }

    glm::dvec2 FrameworkInternal::ConvertInputCoordinatesLocalToGlobal(const glm::dvec2& p)
    {
        glm::dvec2 result{ p.x, static_cast<double>(viewportScreen_[0].size_.y) - p.y };
        result += viewportScreen_[0].position_;
        result /= viewportScreen_[0].size_;
        result.y = 1.0 - result.y;
        return result;
    }

    void FrameworkInternal::TransferDataToNode(const void* data, std::size_t length, std::uint16_t packageId, std::size_t nodeIndex) const
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
        else if (nodeIndex != 0) {
            LOG(WARNING) << "SGCT does not allow inter-node connections (nodeIndex: " << nodeIndex << ").";
        }
        engine_->transferDataToNode(data, static_cast<int>(length), completePackageId, nodeIndex);
    }

    void FrameworkInternal::TransferData(const void* data, std::size_t length, std::uint16_t packageId) const
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
        if (!engine_->isMaster()) {
            LOG(WARNING) << "SGCT does not allow inter-node connections (all nodes).";
        }
        engine_->transferDataBetweenNodes(data, static_cast<int>(length), completePackageId);
    }

    void FrameworkInternal::TransferResource(std::string_view name, const void* data, std::size_t length, ResourceType type)
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
            if (!engine_->isMaster()) {
                LOG(WARNING) << "SGCT does not allow inter-node connections (all nodes).";
            }
            engine_->transferDataBetweenNodes(transferedData.data(), static_cast<int>(transferedData.size()), completePackageId);
        }
    }

    void FrameworkInternal::TransferResourceToNode(std::string_view name, const void* data, std::size_t length, ResourceType type, std::size_t nodeIndex)
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
            else if (nodeIndex != 0) {
                LOG(WARNING) << "SGCT does not allow inter-node connections (nodeIndex: " << nodeIndex << ").";
            }
            engine_->transferDataToNode(transferedData.data(), static_cast<int>(transferedData.size()), completePackageId, nodeIndex);
        }
    }

    void FrameworkInternal::TransferReleaseResource(std::string_view name, ResourceType type)
    {
        if (!initialized_) return;
        if (engine_->isMaster()) {
            auto completePackageId = MakePackageID(static_cast<std::uint8_t>(InternalTransferType::ResourceReleaseTransfer), static_cast<std::uint8_t>(type), 0);
            engine_->transferDataBetweenNodes(name.data(), static_cast<int>(name.length()), completePackageId);
        }
    }

    void FrameworkInternal::RequestSharedResources()
    {
        if (!engine_->isMaster()) {
            auto completePackageId = MakePackageID(static_cast<std::uint8_t>(InternalTransferType::ResourceRequest), static_cast<std::uint8_t>(ResourceType::All_Resources), 0);
            int tmp = 0;
            engine_->transferDataToNode(&tmp, sizeof(int), completePackageId, 0);
        }
    }

    void FrameworkInternal::RequestSharedResource(std::string_view name, ResourceType type)
    {
        if (!initialized_) return;
        if (!engine_->isMaster()) {
            auto completePackageId = MakePackageID(static_cast<std::uint8_t>(InternalTransferType::ResourceRequest), static_cast<std::uint8_t>(type), 0);
            engine_->transferDataBetweenNodes(name.data(), static_cast<int>(name.length()), completePackageId);
        }
    }

    void FrameworkInternal::WaitForResource(const std::string& name, ResourceType type)
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

    bool FrameworkInternal::IsCoordinator() const
    {
        return engine_->isMaster();
    }

    std::size_t FrameworkInternal::GetCurrentWindowID() const
    {
        return static_cast<std::size_t>(engine_->getCurrentWindowPtr()->getId());
    }

    GLFWwindow* FrameworkInternal::GetCurrentNodeMainWindow() const
    {
        return engine_->getThisNodePtr(0)->getWindowPtr(0)->getWindowHandle();
    }

    void FrameworkInternal::Terminate() const
    {
        engine_->terminate();
    }

    std::vector<FrameBuffer> FrameworkInternal::CreateOffscreenBuffers(const FrameBufferDescriptor & fboDesc, int sizeDivisor) const
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

    const FrameBuffer* FrameworkInternal::SelectOffscreenBuffer(const std::vector<FrameBuffer>& offscreenBuffers) const
    {
        return &offscreenBuffers[engine_->getCurrentWindowPtr()->getId()];
    }

    std::unique_ptr<FullscreenQuad> FrameworkInternal::CreateFullscreenQuad(const std::string& fragmentShader)
    {
        return std::make_unique<FullscreenQuad>(fragmentShader, this);
    }

    void FrameworkInternal::ReleaseSynchronizedResource(ResourceType type, std::string_view name)
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

    void FrameworkInternal::CreateSynchronizedResource(ResourceType type, const void* data, std::size_t length)
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

    void FrameworkInternal::CreateSynchronizedResources()
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

    void FrameworkInternal::SendResourcesToNode(ResourceType type, const void* data, std::size_t length, int clientID)
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

    int FrameworkInternal::MakePackageID(std::uint8_t internalType, std::uint8_t internalPID, std::uint16_t userPID)
    {
        int completePackageId = 0;
        auto splitId = reinterpret_cast<std::uint8_t*>(&completePackageId);
        splitId[0] = internalType;
        splitId[1] = internalPID;
        reinterpret_cast<std::uint16_t*>(&completePackageId)[1] = userPID;

        return completePackageId;
    }

#ifndef VISCOM_LOCAL_ONLY
    void FrameworkInternal::loadProperties()
    {
        tinyxml2::XMLDocument doc;
        OpenCVParserHelper::LoadXMLDocument("Program properties", config_.programProperties_, doc);

        startNode_ = OpenCVParserHelper::ParseText<unsigned int>(doc.FirstChildElement("opencv_storage")->FirstChildElement("startNode"));
    }

    unsigned int FrameworkInternal::GetGlobalProjectorId(int nodeId, int windowId) const
    {
        if (static_cast<unsigned int>(nodeId) >= startNode_) {
            unsigned int current_projector = 0;
            for (std::size_t i = startNode_; i < sgct_core::ClusterManager::instance()->getNumberOfNodes(); i++) {
                auto currNode = sgct_core::ClusterManager::instance()->getNodePtr(i);
                for (std::size_t j = 0; j < currNode->getNumberOfWindows(); j++) {
                    if (i == static_cast<std::size_t>(nodeId) && j == static_cast<std::size_t>(windowId)) return current_projector;

                    current_projector += 1;
                }
            }
        }
        return 0;
    }
#endif
}
