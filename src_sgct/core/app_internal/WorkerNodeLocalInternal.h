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
         glm::vec2 GetDisplayPosition(bool useLeftController) ;
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
