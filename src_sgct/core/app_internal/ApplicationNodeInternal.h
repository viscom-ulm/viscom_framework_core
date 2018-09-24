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

    class ApplicationNodeInternal
    {
    public:
        ApplicationNodeInternal(FrameworkInternal& fwInternal);
        ApplicationNodeInternal(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal(ApplicationNodeInternal&&) = delete;
        ApplicationNodeInternal& operator=(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal& operator=(ApplicationNodeInternal&&) = delete;
        virtual ~ApplicationNodeInternal();

        [[deprecated("All initialization should be moved to the constructor in the future.")]]
        virtual void PreWindow();
        virtual void InitImplementation() = 0;
        virtual void PreSync();
        virtual void PostSync();
        virtual void ClearBuffer(FrameBuffer& fbo);
        virtual void DrawFrame(FrameBuffer& fbo);
        virtual void Draw2D(FrameBuffer& fbo);
        virtual void PostDraw();
        [[deprecated("All initialization should be moved to the destructor in the future.")]]
        virtual void CleanUp();
        void DataTransfer(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID);
        void DataAcknowledge(std::uint16_t packageID, int clientID);
        void DataTransferStatus(bool connected, int clientID);
        void EncodeData();
        void DecodeData();

        virtual void KeyboardCallback(int key, int scancode, int action, int mods);
        virtual void CharCallback(unsigned int character, int mods);
        virtual void MouseButtonCallback(int button, int action);
        virtual void MousePosCallback(double x, double y);
        virtual void MouseScrollCallback(double xoffset, double yoffset);
        virtual void AddTuioCursor(TUIO::TuioCursor *tcur);
        virtual void UpdateTuioCursor(TUIO::TuioCursor *tcur);
        virtual void RemoveTuioCursor(TUIO::TuioCursor *tcur);

        double GetCurrentAppTime() const { return syncInfoLocal_.currentTime_; }
        double GetElapsedTime() const { return elapsedTime_; }

        FrameworkInternal & GetFramework() { return fwInternal_; }

    protected:
        void SetApplicationNode(std::unique_ptr<ApplicationNodeBase> appNodeImpl) { appNodeImpl_ = std::move(appNodeImpl); }
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

        struct ResourceData {
            ResourceType type_ = ResourceType::All_Resources;
            std::string name_;
            std::vector<std::uint8_t> data_;

            bool operator==(const ResourceData& other) const { return type_ == other.type_ && name_ == other.name_; }
        };

        /** Synchronized resources to be created at next possible time. */
        std::vector<ResourceData> creatableResources_;
        /** The mutex for creatable resources. */
        std::mutex creatableResourceMutex_;
    };
}
