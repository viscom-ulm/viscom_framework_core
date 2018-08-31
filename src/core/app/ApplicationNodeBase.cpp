/**
 * @file   ApplicationNodeBase.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Implementation of the application node class.
 */

#include "ApplicationNodeBase.h"
#include <imgui.h>
#include "core/FrameworkInternal.h"
//#include <openvr.h>

namespace viscom {

    ApplicationNodeBase::ApplicationNodeBase(ApplicationNodeInternal* appNode) :
        appNode_{ appNode },
        framework_{ &appNode_->GetFramework() }
    {

    }

    ApplicationNodeBase::~ApplicationNodeBase() = default;
 
    void ApplicationNodeBase::PreWindow()
    {
    }

    void ApplicationNodeBase::InitOpenGL()
    {
    }

    void ApplicationNodeBase::PreSync()
    {
    }

    void ApplicationNodeBase::UpdateSyncedInfo()
    {
    }

    void ApplicationNodeBase::UpdateFrame(double, double)
    {
    }

    void ApplicationNodeBase::ClearBuffer(FrameBuffer&)
    {
    }

    void ApplicationNodeBase::DrawFrame(FrameBuffer&)
    {
    }

    void ApplicationNodeBase::Draw2D(FrameBuffer& fbo)
    {
    }

    void ApplicationNodeBase::CleanUp()
    {
    }

    bool ApplicationNodeBase::DataTransferCallback(void * receivedData, int receivedLength, std::uint16_t packageID, int clientID)
    {
        return false;
    }

    bool ApplicationNodeBase::DataAcknowledgeCallback(std::uint16_t packageID, int clientID)
    {
        return false;
    }

    bool ApplicationNodeBase::DataTransferStatusCallback(bool connected, int clientID)
    {
        return false;
    }

    bool ApplicationNodeBase::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        return false;
    }

    bool ApplicationNodeBase::CharCallback(unsigned int character, int mods)
    {
        return false;
    }

    bool ApplicationNodeBase::MouseButtonCallback(int button, int action)
    {
        return false;
    }

    bool ApplicationNodeBase::MousePosCallback(double x, double y)
    {
        return false;
    }

    bool ApplicationNodeBase::MouseScrollCallback(double xoffset, double yoffset)
    {
        return false;
    }

    bool ApplicationNodeBase::AddTuioCursor(TUIO::TuioCursor* tcur)
    {
        return false;
    }

    bool ApplicationNodeBase::UpdateTuioCursor(TUIO::TuioCursor* tcur)
    {
        return false;
    }

    bool ApplicationNodeBase::RemoveTuioCursor(TUIO::TuioCursor* tcur)
    {
        return false;
    }

    void ApplicationNodeBase::EncodeData()
    {
    }

    void ApplicationNodeBase::DecodeData()
    {
    }

    void ApplicationNodeBase::Terminate() const
    {
        framework_->Terminate();
    }

    bool ApplicationNodeBase::ControllerButtonPressedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, float posx, float posy, glm::vec3 position, glm::vec3 zvector, glm::quat rotation)
    {
        return false;
    }

    bool ApplicationNodeBase::ControllerButtonTouchedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, float posx, float posy, glm::vec3 position, glm::vec3 zvector, glm::quat rotation)
    {
        return false;
    }

    bool ApplicationNodeBase::ControllerButtonUnpressedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, float posx, float posy, glm::vec3 position, glm::vec3 zvector, glm::quat rotation)
    {
        return false;
    }

    bool ApplicationNodeBase::ControllerButtonUntouchedCallback(TrackedDeviceIdentifier trackedDevice, ControllerButtonIdentifier buttonid, float posx, float posy, glm::vec3 position, glm::vec3 zvector, glm::quat rotation)
    {
        return false;
    }

    void ApplicationNodeBase::ParseTrackingFrame()
    {
    }

    glm::vec3 ApplicationNodeBase::GetController0Pos()
    {
        return glm::vec3();
    }

    glm::vec3 ApplicationNodeBase::GetController0Zvec()
    {
        return glm::vec3();
    }

    glm::vec3 ApplicationNodeBase::GetController1Pos()
    {
        return glm::vec3();
    }

    glm::vec3 ApplicationNodeBase::GetController1Zvec()
    {
        return glm::vec3();
    }

    glm::vec3 ApplicationNodeBase::GetTrackerPos()
    {
        return glm::vec3();
    }

    glm::vec3 ApplicationNodeBase::GetTrackerZvec()
    {
        return glm::vec3();
    }

    glm::quat ApplicationNodeBase::GetController0Rot()
    {
        return glm::quat();
    }

    glm::quat ApplicationNodeBase::GetController1Rot()
    {
        return glm::quat();
    }

    glm::quat ApplicationNodeBase::GetTrackerRot()
    {
        return glm::quat();
    }

    glm::vec2 ApplicationNodeBase::GetDisplayPosition(bool useleftcontroller)
    {
        return glm::vec2();
    }

    void ApplicationNodeBase::InitialiseDisplay(bool useLeftController)
    {
    }

    bool ApplicationNodeBase::GetDisplayInitialised()
    {
        return false;
    }

    void ApplicationNodeBase::SetDisplayNotInitialised()
    {
    }

    bool ApplicationNodeBase::GetDisplayInitByFloor()
    {
        return false;
    }

    void ApplicationNodeBase::SetDisplayInitByFloor(bool b)
    {
    }

    void ApplicationNodeBase::PollAndParseNextEvent()
    {
    }

    void ApplicationNodeBase::PollAndParseEvents()
    {
    }

    std::vector<std::string> ApplicationNodeBase::OutputDevices()
    {
        return std::vector<std::string>();
    }

    float * ApplicationNodeBase::GetDisplayEdges()
    {
        return nullptr;
    }

    bool ApplicationNodeBase::GetVrInitSuccess()
    {
        return false;
    }

    std::vector<std::string> ApplicationNodeBase::GetController0Buttons()
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> ApplicationNodeBase::GetController1Buttons()
    {
        return std::vector<std::string>();
    }

}
