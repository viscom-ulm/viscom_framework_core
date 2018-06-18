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
    };
}
