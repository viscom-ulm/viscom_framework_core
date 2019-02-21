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

        void PostSync() override;
        void Draw2D(FrameBuffer& fbo) override;
        virtual void PostDraw() override;

        virtual void KeyboardCallback(int key, int scancode, int action, int mods) override;
        virtual void CharCallback(unsigned int character, int mods) override;
        virtual void MouseButtonCallback(int button, int action) override;
        virtual void MousePosCallback(double x, double y) override;
        virtual void MouseScrollCallback(double xoffset, double yoffset) override;

        virtual void addTuioCursor(TUIO::TuioCursor* tcur) override;
        virtual void updateTuioCursor(TUIO::TuioCursor* tcur) override;
        virtual void removeTuioCursor(TUIO::TuioCursor* tcur) override;

    };
}
