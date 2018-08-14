/**
 * @file   CoordinatorNodeInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.06.15
 *
 * @brief  Declaration of the internal coordinator node.
 */

#pragma once

#include "core/app_internal/ApplicationNodeInternal.h"
#include <openvr.h>

namespace viscom {

    class CoordinatorNodeInternal : public ApplicationNodeInternal
    {
    public:
        CoordinatorNodeInternal(FrameworkInternal& fwInternal);
        virtual ~CoordinatorNodeInternal() override;

        virtual void PreWindow() override;
        virtual void InitOpenGL() override;
        virtual void PreSync() override;
        virtual void Draw2D(FrameBuffer& fbo) override;
        virtual void CleanUp() override;

        virtual void KeyboardCallback(int key, int scancode, int action, int mods) override;
        virtual void CharCallback(unsigned int character, int mods) override;
        virtual void MouseButtonCallback(int button, int action) override;
        virtual void MousePosCallback(double x, double y) override;
        virtual void MouseScrollCallback(double xoffset, double yoffset) override;

        virtual void AddTuioCursor(TUIO::TuioCursor* tcur) override;
        virtual void UpdateTuioCursor(TUIO::TuioCursor* tcur) override;
        void ParseTrackingFrame();
        virtual void RemoveTuioCursor(TUIO::TuioCursor* tcur) override;

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

        float * GetPosition(const float hmdMatrix[3][4]);
        double * GetRotation(const float matrix[3][4]);
        float * GetZVector(const float matrix[3][4]);
        float * GetDisplayPosVector(const float position[3], const float zvector[3], const float display_lowerLeftCorner[3], const float display_upperLeftCorner[3], const float display_lowerRightCorner[3]);
        void InitDisplay(float dpos[3]);
        void InitDisplayFloor(float cpos[3], float cz[3]);
        void InitDisplayFromFile();
        void WriteInitDisplayToFile();

        void CoordinatorNodeInternal::HandleSCGT(glm::vec3 pos, glm::quat q);
        
        vr::IVRSystem *m_pHMD;
        bool vrInitSucc = false;

        vr::HmdVector3_t position;
        vr::HmdVector3_t zvector;
        vr::HmdVector2_t displayPos;
        vr::HmdVector3_t trackerPos;
        vr::HmdVector3_t midDisplayPos;
        vr::HmdVector3_t sgctTrackerPos;

        float displayEdges[3][3] = { { -1.7f, -0.2f, -3.0f },{ -1.7f, 1.5f, -3.0f },{ 1.8f, -0.28f, -3.0f } };
        bool initDisplay = true;
        bool displayllset = false;
        bool displayulset = false;
        bool displaylrset = false;
        bool initfloor = true;


    };
}
