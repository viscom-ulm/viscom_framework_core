/**
 * @file   WorkerNodeCalibratedInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.06.15
 *
 * @brief  Implementation of the ApplicationNodeInternal for calibrated workers.
 */

#define GLM_SWIZZLE

#include "core/main.h"
#include <sgct.h>
#include "WorkerNodeCalibratedInternal.h"
#include "core/OpenCVParserHelper.h"
#include <fstream>
#include <imgui.h>
#include <experimental/filesystem>
#include "core/open_gl.h"
#include "core/app/ApplicationNodeBase.h"

namespace viscom {

    WorkerNodeCalibratedInternal::WorkerNodeCalibratedInternal(FrameworkInternal& fwInternal) :
        WorkerNodeLocalInternal{ fwInternal }
    {
        InitOffscreenBuffers();
    }

    WorkerNodeCalibratedInternal::~WorkerNodeCalibratedInternal()
    {
        if (vaoProjectorQuads_ != 0) glDeleteVertexArrays(0, &vaoProjectorQuads_);
        vaoProjectorQuads_ = 0;
        if (vboProjectorQuads_ != 0) glDeleteBuffers(0, &vboProjectorQuads_);
        vboProjectorQuads_ = 0;

        if (!alphaTextures_.empty()) glDeleteTextures(static_cast<GLsizei>(alphaTextures_.size()), alphaTextures_.data());
        alphaTextures_.clear();
    }

    void WorkerNodeCalibratedInternal::InitOffscreenBuffers()
    {
        LOG(DBUG) << "Initializing calibration data.";
        // init shaders
        calibrationProgram_ = GetFramework().GetGPUProgramManager().GetResource("calibrationRendering", std::vector<std::string>{ "calibrationRendering.vert", "calibrationRendering.frag" });
        calibrationAlphaTexLoc_ = calibrationProgram_->getUniformLocation("alphaTex");
        calibrationSceneTexLoc_ = calibrationProgram_->getUniformLocation("tex");

        LOG(DBUG) << "Loading projector data.";
        tinyxml2::XMLDocument doc;
        OpenCVParserHelper::LoadXMLDocument("Projector data", GetFramework().GetConfig().projectorData_, doc);
        std::experimental::filesystem::path projectorDataPath(GetFramework().GetConfig().projectorData_);
        auto alphaTexturePath = projectorDataPath.parent_path();

        LOG(DBUG) << "Initializing viewports.";
        auto slaveId = sgct_core::ClusterManager::instance()->getThisNodeId();
        auto numWindows = sgct_core::ClusterManager::instance()->getThisNodePtr()->getNumberOfWindows();
        projectorViewport_.resize(numWindows);
        sceneFBOs_.reserve(numWindows);
        alphaTextures_.resize(numWindows, 0);

        glGenTextures(static_cast<GLsizei>(numWindows), alphaTextures_.data());

        for (auto i = 0U; i < numWindows; ++i) {
            LOG(DBUG) << "Initializing viewport: " << i;
            projectorViewport_[i] = GetFramework().GetViewportScreen(i);
            auto projectorSize = GetFramework().GetViewportScreen(i).size_;

            auto projectorNo = GetFramework().GetGlobalProjectorId(slaveId, i);
            auto quadCornersName = FWConfiguration::CALIBRATION_QUAD_CORNERS_NAME + std::to_string(projectorNo);
            auto quadTexCoordsName = FWConfiguration::CALIBRATION_QUAD_TEX_COORDS_NAME + std::to_string(projectorNo);
            auto resolutionScalingName = FWConfiguration::CALIBRATION_QUAD_RESOLUTION_SCALING_NAME + std::to_string(projectorNo);
            auto viewportName = FWConfiguration::CALIBRATION_VIEWPORT_NAME + std::to_string(projectorNo);
            auto texAlphaFilename = alphaTexturePath.string() + "/" + FWConfiguration::CALIBRATION_ALPHA_TEXTURE_NAME + std::to_string(projectorNo) + ".bin";

            auto screenQuadCoords = OpenCVParserHelper::ParseVector3f(doc.FirstChildElement("opencv_storage")->FirstChildElement(quadCornersName.c_str()));
            auto screenQuadTexCoords = OpenCVParserHelper::ParseVector3f(doc.FirstChildElement("opencv_storage")->FirstChildElement(quadTexCoordsName.c_str()));
            auto resolutionScalingV = OpenCVParserHelper::ParseVectorf(doc.FirstChildElement("opencv_storage")->FirstChildElement(resolutionScalingName.c_str()));
            glm::vec2 resolutionScaling(resolutionScalingV[0], resolutionScalingV[1]);
            auto viewportv2 = OpenCVParserHelper::ParseVector2f(doc.FirstChildElement("opencv_storage")->FirstChildElement(viewportName.c_str()));
            std::vector<glm::vec3> viewport;
            for (const auto& v : viewportv2) viewport.emplace_back(v, 0.0f);
            for (auto j = 0U; j < screenQuadCoords.size(); ++j) quadCoordsProjector_.emplace_back(screenQuadCoords[j], screenQuadTexCoords[j]);

            GetFramework().GetEngine()->getWindowPtr(i)->getViewport(0)->getProjectionPlane()->setCoordinate(sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner::LowerLeft, viewport[3]);
            GetFramework().GetEngine()->getWindowPtr(i)->getViewport(0)->getProjectionPlane()->setCoordinate(sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner::UpperLeft, viewport[0]);
            GetFramework().GetEngine()->getWindowPtr(i)->getViewport(0)->getProjectionPlane()->setCoordinate(sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner::UpperRight, viewport[1]);


            auto fboSize = glm::ivec2(glm::ceil(glm::vec2(projectorSize) * resolutionScaling));
            glm::vec3 vpSize(GetFramework().GetConfig().nearPlaneSize_.x, GetFramework().GetConfig().nearPlaneSize_.y, 1.0f);
            auto totalScreenSize = (glm::vec2(fboSize) * 2.0f * vpSize.xy) / (viewport[1] - viewport[3]).xy;
            GetFramework().GetViewportScreen(i).position_ = glm::ivec2(glm::floor(((viewport[3] + vpSize) / (2.0f * vpSize)).xy * totalScreenSize));
            GetFramework().GetViewportScreen(i).size_ = glm::uvec2(glm::floor(totalScreenSize));
            GetFramework().GetViewportQuadSize(i) = fboSize;
            GetFramework().GetViewportScaling(i) = totalScreenSize / GetFramework().GetConfig().virtualScreenSize_;

            CreateProjectorFBO(i, fboSize);

            LOG(DBUG) << "VP Pos: " << projectorViewport_[i].position_.x << ", " << projectorViewport_[i].position_.y;
            LOG(DBUG) << "VP Size: " << GetFramework().GetViewportQuadSize(i).x << ", " << GetFramework().GetViewportQuadSize(i).y;
            sceneFBOs_[i].SetStandardViewport(projectorViewport_[i].position_.x, projectorViewport_[i].position_.y, GetFramework().GetViewportQuadSize(i).x, GetFramework().GetViewportQuadSize(i).y);
            GetFramework().GetFramebuffer(i).SetStandardViewport(projectorViewport_[i].position_.x, projectorViewport_[i].position_.y, projectorViewport_[i].size_.x, projectorViewport_[i].size_.y);

            {
                std::ifstream texAlphaFile(texAlphaFilename, std::ios::binary);
                glm::u32vec2 textureSize;
                std::vector<float> texAlphaData(projectorSize.x * projectorSize.y);

                texAlphaFile.read(reinterpret_cast<char*>(&textureSize), sizeof(textureSize));
                assert(textureSize.x == projectorSize.x && textureSize.y == projectorSize.y);
                texAlphaFile.read(reinterpret_cast<char*>(texAlphaData.data()), sizeof(float) * texAlphaData.size());

                glBindTexture(GL_TEXTURE_2D, alphaTextures_[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, projectorSize.x, projectorSize.y, 0, GL_RED, GL_FLOAT, texAlphaData.data());
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }


            glBindTexture(GL_TEXTURE_2D, 0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }

        LOG(DBUG) << "Creating VBOs.";
        glGenBuffers(1, &vboProjectorQuads_);
        glBindBuffer(GL_ARRAY_BUFFER, vboProjectorQuads_);
        glBufferData(GL_ARRAY_BUFFER, quadCoordsProjector_.size() * sizeof(CalbrationProjectorQuadVertex), quadCoordsProjector_.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &vaoProjectorQuads_);
        glBindVertexArray(vaoProjectorQuads_);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CalbrationProjectorQuadVertex), reinterpret_cast<GLvoid*>(offsetof(CalbrationProjectorQuadVertex, position_)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CalbrationProjectorQuadVertex), reinterpret_cast<GLvoid*>(offsetof(CalbrationProjectorQuadVertex, texCoords_)));
        glBindVertexArray(0);

        LOG(DBUG) << "Calibration Initialized.";
    }


    void WorkerNodeCalibratedInternal::DrawFrame(FrameBuffer& fbo)
    {
        auto windowId = GetFramework().GetEngine()->getCurrentWindowPtr()->getId();

        GetFramework().GetEngine()->getCurrentWindowPtr()->getFBOPtr()->unBind();

        ClearBuffer(sceneFBOs_[windowId]);

        WorkerNodeLocalInternal::DrawFrame(sceneFBOs_[windowId]);
    }

    void WorkerNodeCalibratedInternal::Draw2D(FrameBuffer& fbo)
    {
        auto windowId = GetFramework().GetEngine()->getCurrentWindowPtr()->getId();
        WorkerNodeLocalInternal::Draw2D(sceneFBOs_[windowId]);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        fbo.DrawToFBO([windowId, this]() {
            GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
            GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
            GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);

            glDisable(GL_SCISSOR_TEST);
            glDisable(GL_STENCIL_TEST);
            glDisable(GL_DEPTH_TEST);

            // Draw off screen texture to screen
            {
                glUseProgram(calibrationProgram_->getProgramId());

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, sceneFBOs_[windowId].GetTextures()[0]);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, alphaTextures_[windowId]);

                glUniform1i(calibrationSceneTexLoc_, 0);
                glUniform1i(calibrationAlphaTexLoc_, 1);

                glBindVertexArray(vaoProjectorQuads_);
                glDrawArrays(GL_TRIANGLE_FAN, 4 * windowId, 4);
                glBindVertexArray(0);
            }

            if (last_enable_depth_test) glEnable(GL_DEPTH_TEST);
            if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST);
            if (last_enable_stencil_test) glEnable(GL_STENCIL_TEST);
        });
    }

    void WorkerNodeCalibratedInternal::CreateProjectorFBO(size_t windowId, const glm::ivec2& fboSize)
    {
        FrameBufferDescriptor fbDesc;
        fbDesc.texDesc_.emplace_back(GL_RGBA8, GL_TEXTURE_2D);
        fbDesc.rbDesc_.emplace_back(GL_DEPTH_COMPONENT32);
        sceneFBOs_.emplace_back(fboSize.x, fboSize.y, fbDesc);
        glBindTexture(GL_TEXTURE_2D, sceneFBOs_[windowId].GetTextures()[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}
