/**
 * @file   MasterNode.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declaration of the ApplicationNodeImplementation for the master node.
 */

#pragma once

#include "../app/ApplicationNodeImplementation.h"

namespace viscom {

    class MasterNode final : public ApplicationNodeImplementation
    {
    public:
        explicit MasterNode(ApplicationNode* appNode);
        virtual ~MasterNode() override;

        void InitOpenGL() override;
        void PreSync() override;
        void DrawFrame(FrameBuffer& fbo) override;
        void Draw2D(FrameBuffer& fbo) override;
        void CleanUp() override;

        void KeyboardCallback(int key, int scancode, int action, int mods) override;
        void CharCallback(unsigned int character, int mods) override;
        void MouseButtonCallback(int button, int action) override;
        void MousePosCallback(double x, double y) override;
        void MouseScrollCallback(double xoffset, double yoffset) override;

    };
}
