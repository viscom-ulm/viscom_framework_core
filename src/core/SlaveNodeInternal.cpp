/**
 * @file   SlaveNodeInternal.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the slave application node.
 */

#define GLM_SWIZZLE

#include "SlaveNodeInternal.h"
#include "core/OpenCVParserHelper.h"
#include <fstream>
#include <imgui.h>

namespace viscom {

    SlaveNodeInternal::SlaveNodeInternal(ApplicationNodeInternal* appNode) :
        ApplicationNodeImplementation{ appNode },
        useAlphaTransition_{ false }
    {
        loadProperties();
    }


    SlaveNodeInternal::~SlaveNodeInternal() = default;


    void SlaveNodeInternal::InitOpenGL()
    {
        ApplicationNodeImplementation::InitOpenGL();

        // init shaders
        calibrationProgram_ = GetApplication()->GetGPUProgramManager().GetResource("calibrationRendering", std::initializer_list<std::string>{ "calibrationRendering.vert", "calibrationRendering.frag" });
        calibrationUseAlphaTestLoc_ = calibrationProgram_->getUniformLocation("withAlphaTrans");
        calibrationAlphaTexLoc_ = calibrationProgram_->getUniformLocation("alphaTrans");
        calibrationAlphaOverlapTexLoc_ = calibrationProgram_->getUniformLocation("alphaOverlap");
        calibrationColorLookupTexLoc_ = calibrationProgram_->getUniformLocation("colorLookup");
        calibrationSceneTexLoc_ = calibrationProgram_->getUniformLocation("tex");
        calibrationResolutionLoc_ = calibrationProgram_->getUniformLocation("resolution");

        tinyxml2::XMLDocument doc;
        OpenCVParserHelper::LoadXMLDocument("Projector data", GetConfig().projectorData_, doc);

        auto slaveId = sgct_core::ClusterManager::instance()->getThisNodeId();
        auto numWindows = sgct_core::ClusterManager::instance()->getThisNodePtr()->getNumberOfWindows();
        projectorViewport_.resize(numWindows);
        sceneFBOs_.reserve(numWindows);
        alphaTextures_.resize(numWindows, 0);
        if (useAlphaTransition_) alphaTransTextures_.resize(numWindows, 0);
        colorLookUpTableTextures_.resize(numWindows, 0);

        glGenTextures(static_cast<GLsizei>(numWindows), alphaTextures_.data());
        if (useAlphaTransition_) glGenTextures(static_cast<GLsizei>(numWindows), alphaTransTextures_.data());
        glGenTextures(static_cast<GLsizei>(numWindows), colorLookUpTableTextures_.data());

        for (auto i = 0U; i < numWindows; ++i) {
            projectorViewport_[i] = GetViewportScreen(i);
            auto projectorSize = GetViewportScreen(i).size_;

            auto projectorNo = GetGlobalProjectorId(slaveId, i);
            auto quadCornersName = "screenQuadCoords" + std::to_string(projectorNo);
            auto quadTexCoordsName = "screenQuadTexCoords" + std::to_string(projectorNo);
            auto resolutionScalingName = "resolutionScaling" + std::to_string(projectorNo);
            auto viewportName = "viewport" + std::to_string(projectorNo);
            auto texAlphaName = "texAlphaFile" + std::to_string(projectorNo);
            auto texAlphaTransName = "texAlphaTransFile" + std::to_string(projectorNo);
            auto colorLUTName = "colorLUTFile" + std::to_string(projectorNo);

            auto screenQuadCoords = OpenCVParserHelper::ParseVector3f(doc.FirstChildElement("opencv_storage")->FirstChildElement(quadCornersName.c_str()));
            auto screenQuadTexCoords = OpenCVParserHelper::ParseVector3f(doc.FirstChildElement("opencv_storage")->FirstChildElement(quadTexCoordsName.c_str()));
            auto resolutionScaling = OpenCVParserHelper::Parse2f(doc.FirstChildElement("opencv_storage")->FirstChildElement(resolutionScalingName.c_str()));
            auto viewport = OpenCVParserHelper::ParseVector3f(doc.FirstChildElement("opencv_storage")->FirstChildElement(viewportName.c_str()));
            for (auto j = 0U; j < screenQuadCoords.size(); ++j) quadCoordsProjector_.emplace_back(screenQuadCoords[j], screenQuadTexCoords[j]);

            GetEngine().SetProjectionPlaneCoordinate(i, 0, sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner::LowerLeft, viewport[0]);
            GetEngine().SetProjectionPlaneCoordinate(i, 0, sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner::UpperLeft, viewport[1]);
            GetEngine().SetProjectionPlaneCoordinate(i, 0, sgct_core::SGCTProjectionPlane::ProjectionPlaneCorner::UpperRight, viewport[2]);


            auto fboSize = glm::ivec2(glm::ceil(glm::vec2(projectorSize) * resolutionScaling));
            glm::vec3 vpSize(1.7777777777777f, 1.0f, 1.0f);
            auto totalScreenSize = (glm::vec2(fboSize) * 2.0f * vpSize.xy) / (viewport[2] - viewport[0]).xy;
            GetViewportScreen(i).position_ = glm::ivec2(glm::floor(((viewport[0] + vpSize) / (2.0f * vpSize)).xy * totalScreenSize));
            GetViewportScreen(i).size_ = glm::uvec2(glm::floor(totalScreenSize));
            GetViewportQuadSize(i) = fboSize;
            GetViewportScaling(i) = totalScreenSize / GetConfig().virtualScreenSize_;

            CreateProjectorFBO(i, fboSize);

            sceneFBOs_[i].SetStandardViewport(projectorViewport_[i].position_.x, projectorViewport_[i].position_.y, GetViewportQuadSize(i).x, GetViewportQuadSize(i).y);
            GetApplication()->GetFramebuffer(i).SetStandardViewport(projectorViewport_[i].position_.x, projectorViewport_[i].position_.y, projectorViewport_[i].size_.x, projectorViewport_[i].size_.y);

            auto texAlphaFilename = GetConfig().baseDirectory_ + "data/" + GetConfig().viscomConfigName_ + "/" + doc.FirstChildElement("opencv_storage")->FirstChildElement(texAlphaName.c_str())->GetText();
            auto texAlphaTransFilename = GetConfig().baseDirectory_ + "data/" + GetConfig().viscomConfigName_ + "/" + doc.FirstChildElement("opencv_storage")->FirstChildElement(texAlphaTransName.c_str())->GetText();
            auto colorLUTFilename = GetConfig().baseDirectory_ + "data/" + GetConfig().viscomConfigName_ + "/" + doc.FirstChildElement("opencv_storage")->FirstChildElement(colorLUTName.c_str())->GetText();

            {
                std::ifstream texAlphaFile(texAlphaFilename, std::ios::binary);
                std::vector<glm::vec4> texAlphaData(projectorSize.x * projectorSize.y);
                texAlphaFile.read(reinterpret_cast<char*>(texAlphaData.data()), sizeof(glm::vec4) * texAlphaData.size());

                glBindTexture(GL_TEXTURE_2D, alphaTextures_[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, projectorSize.x, projectorSize.y, 0, GL_RGBA, GL_FLOAT, texAlphaData.data());
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            if (useAlphaTransition_) {
                std::ifstream texAlphaTransFile(texAlphaTransFilename, std::ios::binary);
                std::vector<glm::vec4> texAlphaTransData(projectorSize.x * projectorSize.y);
                texAlphaTransFile.read(reinterpret_cast<char*>(texAlphaTransData.data()), sizeof(glm::vec4) * texAlphaTransData.size());

                glBindTexture(GL_TEXTURE_2D, alphaTransTextures_[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, projectorSize.x, projectorSize.y, 0, GL_RGBA, GL_FLOAT, texAlphaTransData.data());
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            {
                std::ifstream colorLUTFile(colorLUTFilename, std::ios::binary);
                std::vector<glm::vec3> colorLUTData(colorCalibrationCellCount_ * colorCalibrationCellCount_ * colorCalibrationValueCount_);
                colorLUTFile.read(reinterpret_cast<char*>(colorLUTData.data()), sizeof(glm::vec3) * colorLUTData.size());

                glBindTexture(GL_TEXTURE_2D_ARRAY, colorLookUpTableTextures_[i]);
                glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB16F, colorCalibrationCellCount_, colorCalibrationCellCount_, colorCalibrationValueCount_, 0, GL_RGB, GL_FLOAT, colorLUTData.data());
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }


            glBindTexture(GL_TEXTURE_2D, 0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }

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
    }


    void SlaveNodeInternal::DrawFrame(FrameBuffer& fbo)
    {
        auto windowId = GetEngine().GetCurrentWindowId();

        GetEngine().UnbindCurrentWindowFBO();

        ClearBuffer(sceneFBOs_[windowId]);

        ApplicationNodeImplementation::DrawFrame(sceneFBOs_[windowId]);
    }

    void SlaveNodeInternal::Draw2D(FrameBuffer& fbo)
    {
        auto windowId = GetEngine().GetCurrentWindowId();
        ApplicationNodeImplementation::Draw2D(sceneFBOs_[windowId]);

#ifdef VISCOM_CLIENTGUI
        sceneFBOs_[windowId].DrawToFBO([]() {
            ImGui::Render();
        });
#endif

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
                if (useAlphaTransition_) {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, alphaTransTextures_[windowId]);
                }
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, alphaTextures_[windowId]);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D_ARRAY, colorLookUpTableTextures_[windowId]);

                glUniform1i(calibrationSceneTexLoc_, 0);
                glUniform2f(calibrationResolutionLoc_, static_cast<float>(projectorViewport_[windowId].size_.x), static_cast<float>(projectorViewport_[windowId].size_.y));
                glUniform1i(calibrationUseAlphaTestLoc_, useAlphaTransition_ ? 1 : 0);
                if (useAlphaTransition_) glUniform1i(calibrationAlphaTexLoc_, 1);
                glUniform1i(calibrationAlphaOverlapTexLoc_, 2);
                glUniform1i(calibrationColorLookupTexLoc_, 3);

                glBindVertexArray(vaoProjectorQuads_);
                glDrawArrays(GL_TRIANGLE_FAN, 4 * windowId, 4);
                glBindVertexArray(0);
            }

            if (last_enable_depth_test) glEnable(GL_DEPTH_TEST);
            if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST);
            if (last_enable_stencil_test) glEnable(GL_STENCIL_TEST);
        });
    }


    void SlaveNodeInternal::CreateProjectorFBO(size_t windowId, const glm::ivec2& fboSize)
    {
        FrameBufferDescriptor fbDesc;
        fbDesc.texDesc_.emplace_back(GL_RGBA32F, GL_TEXTURE_2D);
        fbDesc.rbDesc_.emplace_back(GL_DEPTH_COMPONENT32);
        sceneFBOs_.emplace_back(fboSize.x, fboSize.y, fbDesc);
        glBindTexture(GL_TEXTURE_2D, sceneFBOs_[windowId].GetTextures()[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }


    void SlaveNodeInternal::loadProperties()
    {
        tinyxml2::XMLDocument doc;
        OpenCVParserHelper::LoadXMLDocument("Program properties", GetConfig().programProperties_, doc);

        useAlphaTransition_ = OpenCVParserHelper::ParseText<bool>(doc.FirstChildElement("opencv_storage")->FirstChildElement("alphaTransition"));
        colorCalibrationCellCount_ = OpenCVParserHelper::ParseText<unsigned int>(doc.FirstChildElement("opencv_storage")->FirstChildElement("ccCellCount"));
        colorCalibrationValueCount_ = OpenCVParserHelper::ParseText<unsigned int>(doc.FirstChildElement("opencv_storage")->FirstChildElement("ccValueCount"));
    }


    void SlaveNodeInternal::CleanUp()
    {
        if (vaoProjectorQuads_ != 0) glDeleteVertexArrays(0, &vaoProjectorQuads_);
        vaoProjectorQuads_ = 0;
        if (vboProjectorQuads_ != 0) glDeleteBuffers(0, &vboProjectorQuads_);
        vboProjectorQuads_ = 0;

        if (!alphaTextures_.empty()) glDeleteTextures(static_cast<GLsizei>(alphaTextures_.size()), alphaTextures_.data());
        alphaTextures_.clear();
        if (!alphaTransTextures_.empty()) glDeleteTextures(static_cast<GLsizei>(alphaTransTextures_.size()), alphaTransTextures_.data());
        alphaTransTextures_.clear();
        if (!colorLookUpTableTextures_.empty()) glDeleteTextures(static_cast<GLsizei>(colorLookUpTableTextures_.size()), colorLookUpTableTextures_.data());
        colorLookUpTableTextures_.clear();

        ApplicationNodeImplementation::CleanUp();
    }
}
