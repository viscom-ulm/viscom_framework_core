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
        ~MasterNode();

        void InitOpenGL() override;

        void PreSync() override;

    private:
        /** Holds the resolution scaling for each window. */
        std::vector<glm::vec2> resolutionScaling_;
        /** Holds the quad corners for each window. */
        std::vector<std::vector<glm::vec2>> quadCorners_;
    };
}
