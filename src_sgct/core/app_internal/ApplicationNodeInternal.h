/**
 * @file   ApplicationNodeInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declares a base class for all application nodes in the VISCOM lab cluster.
 */

#pragma once

#include "core/main.h"
#ifdef VISCOM_SYNCINPUT
#include "core/InputWrapper.h"
#endif
#include "core/FrameworkInternal.h"
#include "sgct/SharedDataTypes.h"

#include "core/app/OpenVRControllerIfc.h"

namespace viscom::tuio {
    class TuioInputWrapper;
}

namespace viscom {

    class ApplicationNodeBase;

    struct InternalSyncedInfo {
        double currentTime_ = 0.0;
        glm::vec3 cameraPosition_;
        glm::quat cameraOrientation_;
        glm::mat4 pickMatrix_;
    };

    class ApplicationNodeInternal : public ovr::OpenVRControllerIfc
    {
    public:
        /**
         *  Constructor method.
         *  @param fwInternal the FrameworkInternal object to create the ApplicationNodeInternal with.
         */
        ApplicationNodeInternal(FrameworkInternal& fwInternal);
        ApplicationNodeInternal(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal(ApplicationNodeInternal&&) = delete;
        ApplicationNodeInternal& operator=(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal& operator=(ApplicationNodeInternal&&) = delete;
        virtual ~ApplicationNodeInternal();

        /** Called before a window is created. */
        [[deprecated("All initialization should be moved to the constructor in the future.")]]
        virtual void PreWindow();
        /** Called in the beginning to initialize the implementation and create the window and OpenGL context. */
        virtual void InitImplementation() = 0;
        /** Called before each synchronization in each frame to prepare for it. */
        virtual void PreSync();
        /** Called after each synchronization in each frame to update local information based on the sync. */
        virtual void PostSync();
        /**
         *  This method is called once each frame to clear any frame buffer.
         *  @see ApplicationNodeBase::ClearBuffer.
         */
        virtual void ClearBuffer(FrameBuffer& fbo);
        /**
         *  This method is called once each frame to draw on any frame buffer.
         *  @see ApplicationNodeBase::DrawFrame.
         */
        virtual void DrawFrame(FrameBuffer& fbo);
        /**
         *  This method is called once each frame to render the GUI and 2D elements.
         *  @see ApplicationNodeBase::Draw2D.
         */
        virtual void Draw2D(FrameBuffer& fbo);
        /** Called each frame after everything has been drawn. */
        virtual void PostDraw();
        /**
         *  This method is called when exiting the application.
         *  @see ApplicationNodeBase::CleanUp.
         */
        [[deprecated("All initialization should be moved to the destructor in the future.")]]
        virtual void CleanUp();
        /**
         *  Called when receiving a message from another node.
         *  @see ApplicationNodeBase::DataTransferCallback.
         */
        void DataTransfer(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID);
        /**
         *  Called when successfully sending a message to another node.
         *  @see ApplicationNodeBase::DataAcknowledgeCallback.
         */
        void DataAcknowledge(std::uint16_t packageID, int clientID);
        /**
         *  Called when the connection status changes.
         *  @see ApplicationNodeBase::DataTransferStatusCallback.
         */
        void DataTransferStatus(bool connected, int clientID);
        /** Synchronizes the framework by encoding and sending all synchronized data to the other nodes. */
        void EncodeData();
        /** Synchronizes the framework by receiving and decoding synchronized data from other nodes. */
        void DecodeData();

        /**
         *  This method is called once each frame to handle keyboard input.
         *  @see ApplicationNodeBase::KeyboardCallback.
         */
        virtual void KeyboardCallback(int key, int scancode, int action, int mods);
        /**
         *  This method is called once each frame to handle keyboard character input.
         *  @see ApplicationNodeBase::CharCallback.
         */
        virtual void CharCallback(unsigned int character, int mods);
        /**
         *  This method is called once each frame to handle mouse button input.
         *  @see ApplicationNodeBase::MouseButtonCallback.
         */
        virtual void MouseButtonCallback(int button, int action);
        /**
         *  This method is called once each frame to handle the cursor position.
         *  @see ApplicationNodeBase::MousePosCallback.
         */
        virtual void MousePosCallback(double x, double y);
        /**
         *  This method is called once each frame to handle changes in cursor position.
         *  @see ApplicationNodeBase::MouseScrollCallback.
         */
        virtual void MouseScrollCallback(double xoffset, double yoffset);
        /**
         *  Called for touch screens to add a cursor.
         *  @see ApplicationNodeBase::AddTuioCursor.
         */
        virtual void AddTuioCursor(TUIO::TuioCursor *tcur);
        /**
         *  Called each frame for touch screens to update a cursor.
         *  @see ApplicationNodeBase::UpdateTuioCursor.
         */
        virtual void UpdateTuioCursor(TUIO::TuioCursor *tcur);
        /**
         *  Called for touch screens to remove a cursor.
         *  @see ApplicationNodeBase::RemoveTuioCursor.
         */
        virtual void RemoveTuioCursor(TUIO::TuioCursor *tcur);

        virtual bool InitialiseVR() override;
        virtual bool InitialiseDisplayVR() override;
        virtual bool CalibrateVR(ovr::CalibrateMethod method) override;
        virtual const std::vector<ovr::DeviceInfo>& GetConnectedDevices() const override;
        virtual const glm::vec3& GetControllerPosition(std::uint32_t trackedDeviceId) const override;
        virtual const glm::vec3& GetControllerDirection(std::uint32_t trackedDeviceId) const override;
        virtual const glm::quat& GetControllerOrientation(std::uint32_t trackedDeviceId) const override;
        virtual const glm::vec2& GetDisplayPointerPosition(std::uint32_t trackedDeviceId) const override;

        virtual bool ControllerButtonPressedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues) override;
        virtual bool ControllerButtonTouchedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues) override;
        virtual bool ControllerButtonPressReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues) override;
        virtual bool ControllerButtonTouchReleasedCallback(std::uint32_t trackedDeviceId, std::size_t buttonid, const glm::vec2& axisvalues) override;

        virtual void GetControllerButtonState(std::uint32_t trackedDeviceId, std::size_t buttonid, glm::vec2& axisvalues, ovr::ButtonState& buttonstate) const override;
        virtual std::vector<std::string> OutputDevices() const override;

        /** Returns the current application time. */
        double GetCurrentAppTime() const { return syncInfoLocal_.currentTime_; }
        /** Returns the time elapsed since the last frame. */
        double GetElapsedTime() const { return elapsedTime_; }

        /** Returns the internal framework. */
        FrameworkInternal & GetFramework() { return fwInternal_; }

    protected:
        /**
         * Sets the application node implementation object.
         * @param appNodeImpl the new application node implementation.
         */
        void SetApplicationNode(std::unique_ptr<ApplicationNodeBase> appNodeImpl) { appNodeImpl_ = std::move(appNodeImpl); }
        /** Returns the application node implementation object. */
        ApplicationNodeBase* GetApplicationNode() { return appNodeImpl_.get(); }

        /** Holds the synchronized object (local). */
        InternalSyncedInfo syncInfoLocal_;
        /** Holds the synchronized object (synced). */
        sgct::SharedObject<InternalSyncedInfo> syncInfoSynced_;

#ifdef VISCOM_SYNCINPUT
        /** Holds the synchronized vector with keyboard events. */
        sgct::SharedVector<KeyboardEvent> keyboardEventsSynced_;
        /** Holds the synchronized vector with character events. */
        sgct::SharedVector<CharEvent> charEventsSynced_;
        /** Holds the synchronized vector with mouse button events. */
        sgct::SharedVector<MouseButtonEvent> mouseButtonEventsSynced_;
        /** Holds the synchronized vector with mouse position events. */
        sgct::SharedVector<MousePosEvent> mousePosEventsSynced_;
        /** Holds the synchronized vector with mouse scroll events. */
        sgct::SharedVector<MouseScrollEvent> mouseScrollEventsSynced_;
#endif

    private:
        /** The internal framework class. */
        FrameworkInternal& fwInternal_;
        /** Holds the application node implementation. */
        std::unique_ptr<ApplicationNodeBase> appNodeImpl_;
        /** The input wrapper for TUIO. */
        std::unique_ptr<tuio::TuioInputWrapper> tuio_;

        /** Holds the last frame time. */
        double lastFrameTime_ = 0.0;
        /** Holds the time elapsed since the last frame. */
        double elapsedTime_;

        /** Holds all data of a resource. */
        struct ResourceData {
            /** Holds the resource type. */
            ResourceType type_ = ResourceType::All_Resources;
            /** Holds the resource name. */
            std::string name_;
            /** Holds the resource data. */
            std::vector<std::uint8_t> data_;

            bool operator==(const ResourceData& other) const { return type_ == other.type_ && name_ == other.name_; }
        };

        /** Synchronized resources to be created at next possible time. */
        std::vector<ResourceData> creatableResources_;
        /** The mutex for creatable resources. */
        std::mutex creatableResourceMutex_;
    };
}
