/**
 * @file   MasterNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the master application node.
 */

#include "MasterNode.h"
#include "ColorCalib.h"
#include "core/OpenCVParserHelper.h"

namespace viscom {

    MasterNode::MasterNode(ApplicationNode* appNode) :
        ApplicationNodeImplementation{ appNode }
    {
    }


    MasterNode::~MasterNode() = default;


    void MasterNode::InitOpenGL()
    {
        tinyxml2::XMLDocument doc;
        OpenCVParserHelper::LoadXMLDocument("Projector data", GetConfig().projectorData_, doc);

        auto slaveId = sgct_core::ClusterManager::instance()->getThisNodeId();
        auto numWindows = sgct_core::ClusterManager::instance()->getThisNodePtr()->getNumberOfWindows();
        resolutionScaling_.resize(numWindows);
        quadCorners_.resize(numWindows);


        std::vector<std::vector<cv::Point2f>> quadCornersCV_(numWindows);

        for (auto i = 0U; i < numWindows; ++i) {
            auto projectorNo = pro_cal::getProjectorNo(slaveId, i);
            auto quadCornersName = "quad_corners" + std::to_string(projectorNo);
            quadCorners_[i] = OpenCVParserHelper::ParseVector2f(doc.FirstChildElement("opencv_storage")->FirstChildElement(quadCornersName.c_str()));
        }
    }

    void MasterNode::PreSync()
    {
        ApplicationNodeImplementation::PreSync();
    }
}
