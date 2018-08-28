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
        explicit WorkerNodeLocalInternal(FrameworkInternal& fwInternal);
        virtual ~WorkerNodeLocalInternal() override;

        void PreWindow() override;
        void InitOpenGL() override;
        void PostSync() override;
        void Draw2D(FrameBuffer& fbo) override;
        virtual void CleanUp() override;

        virtual void KeyboardCallback(int key, int scancode, int action, int mods) override;
        virtual void CharCallback(unsigned int character, int mods) override;
        virtual void MouseButtonCallback(int button, int action) override;
        virtual void MousePosCallback(double x, double y) override;
        virtual void MouseScrollCallback(double xoffset, double yoffset) override;

        virtual void AddTuioCursor(TUIO::TuioCursor* tcur) override;
        virtual void UpdateTuioCursor(TUIO::TuioCursor* tcur) override;
        virtual void RemoveTuioCursor(TUIO::TuioCursor* tcur) override;

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
        virtual glm::vec2 GetDisplayPosition(bool useLeftController) override;
        virtual void InitialiseDisplay(bool useLeftController) override;
        virtual bool GetDisplayInitialised() override;
        virtual void SetDisplayNotInitialised() override;
        virtual bool GetDisplayInitByFloor() override;
        virtual void SetDisplayInitByFloor(bool b) override;
        virtual void PollAndParseNextEvent() override;
        virtual void PollAndParseEvents() override;
        virtual std::vector<std::string> OutputDevices() override;
        virtual float* GetDisplayEdges() override;
        virtual bool GetVrInitSuccess() override;
        virtual std::vector<std::string> GetController0Buttons() override;
        virtual std::vector<std::string> GetController1Buttons() override;



    };
}
