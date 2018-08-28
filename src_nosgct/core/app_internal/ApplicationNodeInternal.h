/**
 * @file   ApplicationNodeInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declares a base class for all application nodes in the VISCOM lab cluster.
 */

#pragma once

#include "core/main.h"
#include "core/TuioInputWrapper.h"
#include "core/FrameworkInternal.h"

namespace viscom {

    class ApplicationNodeBase;

    struct InternalSyncedInfo {
        double currentTime_ = 0.0;
        glm::vec3 cameraPosition_;
        glm::quat cameraOrientation_;
        glm::mat4 pickMatrix_;
    };

    class ApplicationNodeInternal : public viscom::tuio::TuioInputWrapper
    {
    public:
        ApplicationNodeInternal(FrameworkInternal& fwInternal);
        ApplicationNodeInternal(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal(ApplicationNodeInternal&&) = delete;
        ApplicationNodeInternal& operator=(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal& operator=(ApplicationNodeInternal&&) = delete;
        virtual ~ApplicationNodeInternal() override;

        virtual void PreWindow();
        virtual void InitOpenGL();
        virtual void PreSync();
        virtual void PostSync();
        virtual void ClearBuffer(FrameBuffer& fbo);
        virtual void DrawFrame(FrameBuffer& fbo);
        virtual void Draw2D(FrameBuffer& fbo);
        virtual void PostDraw();
        virtual void CleanUp();
        void DataTransfer(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID);
        void DataAcknowledge(std::uint16_t packageID, int clientID);
        void DataTransferStatus(bool connected, int clientID);

        virtual void KeyboardCallback(int key, int scancode, int action, int mods);
        virtual void CharCallback(unsigned int character, int mods);
        virtual void MouseButtonCallback(int button, int action);
        virtual void MousePosCallback(double x, double y);
        virtual void MouseScrollCallback(double xoffset, double yoffset);

        void addTuioCursor(TUIO::TuioCursor *tcur) override;
        void updateTuioCursor(TUIO::TuioCursor *tcur) override;
        void removeTuioCursor(TUIO::TuioCursor *tcur) override;

        double GetCurrentAppTime() const { return currentTime_; }
        double GetElapsedTime() const { return elapsedTime_; }

        FrameworkInternal & GetFramework() { return fwInternal_; }
        
        virtual void ParseTrackingFrame() override;
        virtual glm::vec3 GetController0Pos() override;
        virtual glm::vec3 GetController0Zvec() override;
        virtual glm::vec3 GetController1Pos() override;
        virtual glm::vec3 GetController1Zvec() override;
        virtual glm::vec3 GetTrackerPos() override;
        virtual glm::vec3 GetTrackerZvec() override;
        virtual glm::quat GetController0Rot() override;
        virtual glm::quat GetController1Rot() override;
        virtual glm::quat GetTrackerRot() override;
        virtual glm::vec2 GetDisplayPosition(bool useleftcontroller) override;
        virtual void InitialiseDisplay(bool useLeftController) override;
        virtual bool GetDisplayInitialised() override;
        virtual void SetDisplayNotInitialised() override;
        virtual bool GetDisplayInitByFloor() override;
        virtual void SetDisplayInitByFloor(bool b) override;
        virtual void PollAndParseNextEvent() override;
        virtual void PollAndParseEvents() override;        
        virtual float* GetDisplayEdges() override;
        virtual bool GetVrInitSuccess() override;
        virtual std::vector<std::string> OutputDevices() override;
        virtual std::vector<std::string> GetController0Buttons() override;
        virtual std::vector<std::string> GetController1Buttons() override;

    protected:
        void SetApplicationNode(std::unique_ptr<ApplicationNodeBase> appNodeImpl) { appNodeImpl_ = std::move(appNodeImpl); }

    private:
        /** The internal framework class. */
        FrameworkInternal& fwInternal_;
        /** Holds the application node implementation. */
        std::unique_ptr<ApplicationNodeBase> appNodeImpl_;

        /** Holds the current application time. */
        double currentTime_;
        /** Holds the time elapsed since the last frame. */
        double elapsedTime_;
        
        float * GetPosition(const float hmdMatrix[3][4]);
        double * GetRotation(const float matrix[3][4]);
        float * GetZVector(const float matrix[3][4]);
        float * GetDisplayPosVector(glm::vec3 pos, glm::vec3 zvector, const float display_lowerLeftCorner[3], const float display_upperLeftCorner[3], const float display_lowerRightCorner[3]);
        void InitDisplay(glm::vec3 dpos);
        void InitDisplayFloor(glm::vec3 cpos, glm::vec3 cz);
        void InitDisplayFromFile();
        void WriteInitDisplayToFile();
        bool ProcessVREvent(const vr::VREvent_t & event);
        void HandleSCGT(glm::vec3 pos, glm::quat q);

        vr::IVRSystem *m_pHMD;
        bool vrInitSucc = false;
        glm::vec3 controller0pos;
        glm::vec3 controller0zvec;
        glm::vec3 controller1pos;
        glm::vec3 controller1zvec;
        glm::vec3 trackerpos;
        glm::vec3 trackerzvec;
        glm::quat controller0rot;
        glm::quat controller1rot;
        glm::quat trackerrot;
        float midDisplayPos[3] = { 0.0f,0.0f,0.0f };
        float displayEdges[3][3] = { { -1.7f, -0.2f, -3.0f },{ -1.7f, 1.5f, -3.0f },{ 1.8f, -0.28f, -3.0f } };
        bool initDisplay = true;
        bool displayllset = false;
        bool displayulset = false;
        bool displaylrset = false;
        bool initfloor = true;
        bool useleftcontroller;
        std::vector<std::string> controller0buttons;
        std::vector<std::string> controller1buttons;
    };
}
