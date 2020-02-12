/**
 * @file   CoordinatorNodeInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.06.15
 *
 * @brief  Declaration of the internal coordinator node.
 */

#pragma once

#include "core/app_internal/ApplicationNodeInternal.h"
#include "core/app/OpenVRController.h"

namespace viscom {

    struct VrSyncedInfo {
        glm::vec2 displayPosLeftController_ = glm::vec2{ 0.0f };
        glm::vec2 displayPosRightController_ = glm::vec2{ 0.0f };
    };

    
    class CoordinatorNodeInternal : public ovr::OpenVRController, public ApplicationNodeInternal
    {
    public:
        /**
         *  Constructor method.
         *  @param fwInternal the FrameworkInternal object to create the coordinator node with.
         */
        CoordinatorNodeInternal(FrameworkInternal& fwInternal);
        virtual ~CoordinatorNodeInternal() override;

        /** Called in the beginning to initialize the implementation and create the window and OpenGL context. */
        virtual void InitImplementation() override;
        /** Called before each synchronization in each frame to prepare for it. */
        virtual void PreSync() override;
        /**
         *  This method is called once each frame to render the GUI and 2D elements.
         *  @see ApplicationNodeBase::Draw2D.
         */
        virtual void Draw2D(FrameBuffer& fbo) override;

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

        void EncodeData();
        void DecodeData();

        bool ControllerButtonPressedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override {
            return ApplicationNodeInternal::ControllerButtonPressedCallback(trackedDeviceId, buttonid);
        }

        bool ControllerButtonTouchedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override {
            return ApplicationNodeInternal::ControllerButtonTouchedCallback(trackedDeviceId, buttonid);
        }

        bool ControllerButtonPressReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override {
            return ApplicationNodeInternal::ControllerButtonPressReleasedCallback(trackedDeviceId, buttonid);
        }

        bool ControllerButtonTouchReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid) override {
            return ApplicationNodeInternal::ControllerButtonTouchReleasedCallback(trackedDeviceId, buttonid);
        }

        bool InitialiseVR() override {
            return ovr::OpenVRController::InitialiseVR();
        }

        bool InitialiseDisplayVR() override {
            return ovr::OpenVRController::InitialiseDisplayVR();
        }

        bool CalibrateVR(ovr::CalibrateMethod method) override {
            return ovr::OpenVRController::CalibrateVR(method);
        }

        const std::vector<ovr::DeviceInfo>& GetConnectedDevices() const override {
            return ovr::OpenVRController::GetConnectedDevices();
        }

        const glm::vec3& GetControllerPosition(std::uint32_t trackedDeviceId) const override {
            return ovr::OpenVRController::GetControllerPosition(trackedDeviceId);
        }

        const glm::vec3& GetControllerDirection(std::uint32_t trackedDeviceId) const override {
            return ovr::OpenVRController::GetControllerDirection(trackedDeviceId);
        }

        const glm::quat& GetControllerOrientation(std::uint32_t trackedDeviceId) const override {
            return ovr::OpenVRController::GetControllerOrientation(trackedDeviceId);
        }

        const glm::vec2& GetDisplayPointerPosition(std::uint32_t trackedDeviceId) const override {
            return ovr::OpenVRController::GetDisplayPointerPosition(trackedDeviceId);
        }

        void GetControllerButtonState(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2& axisvalues, ovr::ButtonState& buttonstate) const override {
            return ovr::OpenVRController::GetControllerButtonState(trackedDeviceId, buttonid, axisvalues, buttonstate);
        }

        std::vector<std::string> OutputDevices() const override {
            return ovr::OpenVRController::OutputDevices();
        }

    protected:
        /** Holds the synchronized object (local). */
        VrSyncedInfo vrInfoLocal_;
        /** Holds the synchronized object (synced). */
        sgct::SharedObject<VrSyncedInfo> vrInfoSynced_;

    private:
#ifdef VISCOM_SYNCINPUT
        /** Holds the vector with keyboard events. */
        std::vector<KeyboardEvent> keyboardEvents_;
        /** Holds the vector with character events. */
        std::vector<CharEvent> charEvents_;
        /** Holds the vector with mouse button events. */
        std::vector<MouseButtonEvent> mouseButtonEvents_;
        /** Holds the vector with mouse position events. */
        std::vector<MousePosEvent> mousePosEvents_;
        /** Holds the vector with mouse scroll events. */
        std::vector<MouseScrollEvent> mouseScrollEvents_;
#endif
    };
}
