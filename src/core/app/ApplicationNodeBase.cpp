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
#include <openvr.h>
#include <iostream>
#include <fstream>

namespace viscom {

    ApplicationNodeBase::ApplicationNodeBase(ApplicationNodeInternal* appNode) :
        appNode_{ appNode },
        framework_{ &appNode_->GetFramework() }
    {
        vr::EVRInitError peError;
        // VRApplication_Scene (starts SteamVR no proper data) VRApplication_Overlay (starts SteamVR no SteamVRHome)  VRApplication_Background (doesn't start SteamVR uses SteamVRHome)
        m_pHMD = vr::VR_Init(&peError, vr::EVRApplicationType::VRApplication_Background);
        if (peError == vr::VRInitError_None) {
            vrInitSucc = true;
            //TODO fix output to imgui 
            //OutputDevices();
        }
    }

    //ApplicationNodeBase::~ApplicationNodeBase() = default;
    ApplicationNodeBase::~ApplicationNodeBase() {
        vr::VR_Shutdown();
    }

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

    float *ApplicationNodeBase::VrGetPosition(const float hmdMatrix[3][4])
    {
        float vector[3];
        vector[0] = hmdMatrix[0][3];
        vector[1] = hmdMatrix[1][3];
        vector[2] = hmdMatrix[2][3];
        return vector;
    }

    double *ApplicationNodeBase::VrGetRotation(const float matrix[3][4]) 
    {
        double q[4];

        q[0] = sqrt(fmax(0, 1 + matrix[0][0] + matrix[1][1] + matrix[2][2])) / 2;
        q[1] = sqrt(fmax(0, 1 + matrix[0][0] - matrix[1][1] - matrix[2][2])) / 2;
        q[2] = sqrt(fmax(0, 1 - matrix[0][0] + matrix[1][1] - matrix[2][2])) / 2;
        q[3] = sqrt(fmax(0, 1 - matrix[0][0] - matrix[1][1] + matrix[2][2])) / 2;
        q[1] = copysign(q[1], matrix[2][1] - matrix[1][2]);
        q[2] = copysign(q[2], matrix[0][2] - matrix[2][0]);
        q[3] = copysign(q[3], matrix[1][0] - matrix[0][1]);
        return q;
    }

    float *ApplicationNodeBase::GetZVector(const float matrix[3][4]) {
        float vector[3];
        vector[0] = matrix[0][2];
        vector[1] = matrix[1][2];
        vector[2] = matrix[2][2];
        return vector;
    }

    float *ApplicationNodeBase::GetDisplayPosVector(const float position[3],const float zvector[3],const float display_lowerLeftCorner[3],const float display_upperLeftCorner[3],const float display_lowerRightCorner[3]) {
        float d1[3] = {display_lowerLeftCorner[0], display_lowerLeftCorner[1],display_lowerLeftCorner[2]};
        float d2[3] = { display_upperLeftCorner[0] - display_lowerLeftCorner[0], display_upperLeftCorner[1] - display_lowerLeftCorner[1], display_upperLeftCorner[2] - display_lowerLeftCorner[2] };
        float d3[3] = { display_lowerRightCorner[0] - display_lowerLeftCorner[0], display_lowerRightCorner[1] - display_lowerLeftCorner[1], display_lowerRightCorner[2] - display_lowerLeftCorner[2] };
        float result[2];

        result[1] = (position[0] * zvector[1] * d3[0] * zvector[2] - position[0] * zvector[1] * d3[2] * zvector[0] - position[1] * zvector[0] * d3[0] * zvector[2] + position[1] * zvector[0] * d3[2] * zvector[0] - d1[0] * zvector[1] * d3[0] * zvector[2] + d1[0] * zvector[1] * d3[2] * zvector[0] + d1[1] * zvector[0] * d3[0] * zvector[2] - d1[1] * zvector[0] * d3[2] * zvector[0] - position[0] * zvector[2] * d3[0] * zvector[1] + position[0] * zvector[2] * d3[1] * zvector[0] + position[2] * zvector[0] * d3[0] * zvector[1] - position[2] * zvector[0] * d3[1] * zvector[0] + d1[0] * zvector[2] * d3[0] * zvector[1] - d1[0] * zvector[2] * d3[1] * zvector[0] - d1[2] * zvector[0] * d3[0] * zvector[1] + d1[2] * zvector[0] * d3[1] * zvector[0]) / (d2[0] * zvector[1] * d3[0] * zvector[2] - d2[0] * zvector[1] * d3[2] * zvector[0] - d2[1] * zvector[0] * d3[0] * zvector[2] + d2[1] * zvector[0] * d3[2] * zvector[0] - d2[0] * zvector[2] * d3[0] * zvector[1] + d2[0] * zvector[2] * d3[1] * zvector[0] + d2[2] * zvector[0] * d3[0] * zvector[1] - d2[2] * zvector[0] * d3[1] * zvector[0]);
        result[0] = (position[0] * zvector[1] * d2[0] * zvector[2] - position[0] * zvector[1] * d2[2] * zvector[0] - position[1] * zvector[0] * d2[0] * zvector[2] + position[1] * zvector[0] * d2[2] * zvector[0] - d1[0] * zvector[1] * d2[0] * zvector[2] + d1[0] * zvector[1] * d2[2] * zvector[0] + d1[1] * zvector[0] * d2[0] * zvector[2] - d1[1] * zvector[0] * d2[2] * zvector[0] - position[0] * zvector[2] * d2[0] * zvector[1] + position[0] * zvector[2] * d2[1] * zvector[0] + position[2] * zvector[0] * d2[0] * zvector[1] - position[2] * zvector[0] * d2[1] * zvector[0] + d1[0] * zvector[2] * d2[0] * zvector[1] - d1[0] * zvector[2] * d2[1] * zvector[0] - d1[2] * zvector[0] * d2[0] * zvector[1] + d1[2] * zvector[0] * d2[1] * zvector[0]) / (d3[0] * zvector[1] * d2[0] * zvector[2] - d3[0] * zvector[1] * d2[2] * zvector[0] - d3[1] * zvector[0] * d2[0] * zvector[2] + d3[1] * zvector[0] * d2[2] * zvector[0] - d3[0] * zvector[2] * d2[0] * zvector[1] + d3[0] * zvector[2] * d2[1] * zvector[0] + d3[2] * zvector[0] * d2[0] * zvector[1] - d3[2] * zvector[0] * d2[1] * zvector[0]);
        //TODO
        /*midDisplayPos[0] = d1[0] + 0.5f * d2[0] + 0.5f * d3[0];
        midDisplayPos[1] = d1[1] + 0.5f * d2[1] + 0.5f * d3[1];
        midDisplayPos[2] = d1[2] + 0.5f * d2[2] + 0.5f * d3[2];*/
        return result;
    }

    void ApplicationNodeBase::InitDisplay(float dpos[3]) {
        if (!displayllset) {
            displayEdges[0][0] = dpos[0];
            displayEdges[0][1] = dpos[1];
            displayEdges[0][2] = dpos[1];
            displayllset = true;
            return;
        }
        if (!displayulset) {
            displayEdges[1][0] = dpos[0];
            displayEdges[1][1] = dpos[1];
            displayEdges[1][2] = dpos[1];
            displayulset = true;
            return;
        }
        if (!displaylrset) {
            displayEdges[2][0] = dpos[0];
            displayEdges[2][1] = dpos[1];
            displayEdges[2][2] = dpos[1];
            displaylrset = true;
        }
        initDisplay = true;
    }

    void ApplicationNodeBase::InitDisplayFloor(float cpos[3], float cz[3]) {
        float t = (-cpos[1]) / cz[1];
        if (!displayllset) {
            displayEdges[0][0] = cpos[0] + t * cz[0];
            displayEdges[0][1] = 0.0f;
            displayEdges[0][2] = cpos[2] + t * cz[2];
            displayllset = true;
            return;
        }
        if (!displaylrset) {
            displayEdges[2][0] = cpos[0] + t * cz[0];
            displayEdges[2][1] = 0.0f;
            displayEdges[2][2] = cpos[2] + t * cz[2];
            displaylrset = true;
            return;
        }
        if (!displayulset) {
            float f = (displayEdges[0][0] * (displayEdges[2][2] - displayEdges[0][2]) - displayEdges[0][2] * (displayEdges[2][0] - displayEdges[0][0]) - cpos[0] * (displayEdges[2][2] - displayEdges[0][2]) + cpos[2] * (displayEdges[2][0] - displayEdges[0][0])) / (cz[0] * (displayEdges[2][2] - displayEdges[0][2]) - cz[2] * (displayEdges[2][0] - displayEdges[0][0]));
            displayEdges[1][0] = displayEdges[0][0]; //cpos[0] + f * cz[0];
            displayEdges[1][1] = cpos[1] + f * cz[1];
            displayEdges[1][2] = displayEdges[0][2]; //cpos[2] + f * cz[2];
            displayulset = true;
        }
        initDisplay = true;
    }

    void ApplicationNodeBase::InitDisplayFromFile() {
        std::ifstream myfile("displayEdges.txt");
        if (myfile.is_open()) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    myfile >> displayEdges[i][j];
                }
            }
        }
    }

    void ApplicationNodeBase::WriteInitDisplayToFile() {
        std::ofstream myfile;
        myfile.open("displayEdges.txt");
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                myfile << displayEdges[i][j] << " ";
            }
        }
        myfile.close();
    }

    vr::IVRSystem *ApplicationNodeBase::GetIVRSystem()
    {
        return m_pHMD;
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
}
