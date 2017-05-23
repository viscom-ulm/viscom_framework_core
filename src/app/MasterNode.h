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
        explicit MasterNode(ApplicationNodeInternal* appNode);
        virtual ~MasterNode() override;

        void InitOpenGL() override;
        void PreSync() override;
        void DrawFrame(FrameBuffer& fbo) override;
        void Draw2D(FrameBuffer& fbo) override;
        void CleanUp() override;

        bool KeyboardCallback(int key, int scancode, int action, int mods) override;
        bool CharCallback(unsigned int character, int mods) override;
        bool MouseButtonCallback(int button, int action) override;
        bool MousePosCallback(double x, double y) override;
        bool MouseScrollCallback(double xoffset, double yoffset) override;

    };
}
