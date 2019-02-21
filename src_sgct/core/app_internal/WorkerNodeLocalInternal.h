/**
 * @file   WorkerNodeLocalInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.06.15
 *
 * @brief  Declaration of the ApplicationNodeInternal for the general workers.
 */

#pragma once

#include "core/app_internal/ApplicationNodeInternal.h"
#include "core/CalibrationVertices.h"

namespace viscom {

    class WorkerNodeLocalInternal : public ApplicationNodeInternal
    {
    public:
        /**
         *  Constructor method.
         *  @param appNode the ApplicationNodeInternal object to create the ApplicationNodeBase with.
         */
        explicit WorkerNodeLocalInternal(FrameworkInternal& fwInternal);
        virtual ~WorkerNodeLocalInternal() override;

        /** Called in the beginning to initialize the implementation and create the window and OpenGL context. */
        virtual void InitImplementation() override;
        /** Called after each synchronization in each frame to update local information based on the sync. */
        void PostSync() override;
        /**
         *  This method is called once each frame to render the GUI and 2D elements.
         *  @see ApplicationNodeBase::Draw2D.
         */
        void Draw2D(FrameBuffer& fbo) override;

        /**
         *  This method is called once each frame to handle keyboard input.
         *  @see ApplicationNodeBase::KeyboardCallback.
         */
        virtual void KeyboardCallback(int key, int scancode, int action, int mods) override;
        /**
         *  This method is called once each frame to handle keyboard character input.
         *  @see ApplicationNodeBase::CharCallback.
         */
        virtual void CharCallback(unsigned int character, int mods) override;
        /**
         *  This method is called once each frame to handle mouse button input.
         *  @see ApplicationNodeBase::MouseButtonCallback.
         */
        virtual void MouseButtonCallback(int button, int action) override;
        /**
         *  This method is called once each frame to handle the cursor position.
         *  @see ApplicationNodeBase::MousePosCallback.
         */
        virtual void MousePosCallback(double x, double y) override;
        /**
         *  This method is called once each frame to handle changes in cursor position.
         *  @see ApplicationNodeBase::MouseScrollCallback.
         */
        virtual void MouseScrollCallback(double xoffset, double yoffset) override;

        /**
         *  Called for touch screens to add a cursor.
         *  @see ApplicationNodeBase::AddTuioCursor.
         */
        virtual void AddTuioCursor(TUIO::TuioCursor* tcur) override;
        /**
         *  Called each frame for touch screens to update a cursor.
         *  @see ApplicationNodeBase::UpdateTuioCursor.
         */
        virtual void UpdateTuioCursor(TUIO::TuioCursor* tcur) override;
        /**
         *  Called for touch screens to remove a cursor.
         *  @see ApplicationNodeBase::RemoveTuioCursor.
         */
        virtual void RemoveTuioCursor(TUIO::TuioCursor* tcur) override;

        bool InitialiseVR() override;
        bool InitialiseDisplayVR() override;
        bool CalibrateVR(CalibrateMethod method) override;
        const std::vector<DeviceInfo>& GetConnectedDevices() override;
        const glm::vec3& GetControllerPosition(size_t trackedDeviceId) override;
        const glm::vec3& GetControllerZVector(size_t trackedDeviceId) override;
        const glm::quat& GetControllerRotation(size_t trackedDeviceId) override;
        const glm::vec2& GetDisplayPointerPosition(size_t trackedDeviceId) override;

        void ControllerButtonPressedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues) override;
        void ControllerButtonTouchedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues) override;
        void ControllerButtonUnpressedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues) override;
        void ControllerButtonUntouchedCallback(size_t trackedDeviceId, size_t buttonid, glm::vec2 axisvalues) override;

        void GetControllerButtonState(size_t trackedDeviceId, size_t buttonid, glm::vec2& axisvalues, ButtonState& buttonstate) override;

         void ParseTrackingFrame();
         glm::vec3 GetController0Pos();
         glm::vec3 GetController0Zvec();
         glm::vec3 GetController1Pos();
         glm::vec3 GetController1Zvec();
         glm::vec3 GetTrackerPos();
         glm::vec3 GetTrackerZvec() ;
         glm::quat GetController0Rot() ;
         glm::quat GetController1Rot() ;
         glm::quat GetTrackerRot() ;
         glm::vec2 GetDisplayPointerPosition(bool useLeftController) ;
         void InitialiseDisplay(bool useLeftController) ;
         bool GetDisplayInitialised() ;
         void SetDisplayNotInitialised() ;
         bool GetDisplayInitByFloor() ;
         void SetDisplayInitByFloor(bool b) ;
         void PollAndParseNextEvent() ;
         void PollAndParseEvents() ;
         std::vector<std::string> OutputDevices() ;
         float* GetDisplayEdges() ;
         bool GetVrInitSuccess() ;
         std::vector<std::string> GetController0Buttons() ;
         std::vector<std::string> GetController1Buttons() ;



    };
}