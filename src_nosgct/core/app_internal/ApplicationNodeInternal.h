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
        /**
        *  Constructor method.
        *  @param fwInternal the FrameworkInternal object to create the ApplicationNodeInternal with.
        */
        ApplicationNodeInternal(FrameworkInternal& fwInternal);
        ApplicationNodeInternal(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal(ApplicationNodeInternal&&) = delete;
        ApplicationNodeInternal& operator=(const ApplicationNodeInternal&) = delete;
        ApplicationNodeInternal& operator=(ApplicationNodeInternal&&) = delete;
        virtual ~ApplicationNodeInternal() override;

        /** Called before a window is created. */
        virtual void PreWindow();
        /** Called after the OpenGL context is created. Here OpenGL objects can be initialized. */
        virtual void InitOpenGL();
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
        void addTuioCursor(TUIO::TuioCursor *tcur) override;
        /**
         *  Called each frame for touch screens to update a cursor.
         *  @see ApplicationNodeBase::UpdateTuioCursor.
         */
        void updateTuioCursor(TUIO::TuioCursor *tcur) override;
        /**
         *  Called for touch screens to remove a cursor.
         *  @see ApplicationNodeBase::RemoveTuioCursor.
         */
        void removeTuioCursor(TUIO::TuioCursor *tcur) override;

        /** Returns the current application time. */
        double GetCurrentAppTime() const { return currentTime_; }
        /** Returns the time elapsed since the last frame. */
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
