/**
 * @file   SlaveNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the slave application node.
 */

#define GLM_SWIZZLE

#include "SlaveNode.h"
#include "ColorCalib.h"
#include "core/OpenCVParserHelper.h"

namespace viscom {

    SlaveNode::SlaveNode(ApplicationNode* appNode) :
        ApplicationNodeImplementation{ appNode },
        useAlphaTransition_{ false }
    {
        loadProperties();
    }


    SlaveNode::~SlaveNode() = default;


    void SlaveNode::InitOpenGL()
    {
        const auto SHADER_PATH = GetConfig().baseDirectory_ + "/shader/";
        if (!sgct::ShaderManager::instance()->shaderProgramExists("calibrationRendering")) {
            sgct::ShaderManager::instance()->addShaderProgram(calibrationProgram_, "calibrationRendering", SHADER_PATH + "calibrationRendering.vert", SHADER_PATH + "calibrationRendering.frag");
            sgct::ShaderManager::instance()->bindShaderProgram(calibrationProgram_);
            sgct::ShaderManager::instance()->unBindShaderProgram();
        } else calibrationProgram_ = sgct::ShaderManager::instance()->getShaderProgram("calibrationRendering");

        calibrationUseAlphaTestLoc_ = calibrationProgram_.getUniformLocation("withAlphaTrans");
        calibrationAlphaTexLoc_ = calibrationProgram_.getUniformLocation("alphaTrans");
        calibrationAlphaOverlapTexLoc_ = calibrationProgram_.getUniformLocation("alphaOverlap");
        calibrationColorLookupTexLoc_ = calibrationProgram_.getUniformLocation("colorLookup");
        calibrationSceneTexLoc_ = calibrationProgram_.getUniformLocation("tex");
        calibrationResolutionLoc_ = calibrationProgram_.getUniformLocation("resolution");

        tinyxml2::XMLDocument doc;
        OpenCVParserHelper::LoadXMLDocument("Projector data", GetConfig().projectorData_, doc);

        auto slaveId = sgct_core::ClusterManager::instance()->getThisNodeId();
        auto numWindows = sgct_core::ClusterManager::instance()->getThisNodePtr()->getNumberOfWindows();
        resolutionScaling_.resize(numWindows, glm::vec2(1.0f));
        quadCorners_.resize(numWindows);
        quadTexCoords_.resize(numWindows);
        quadCoordsProjector_.resize(numWindows);
        sceneFBOs_.resize(numWindows, 0);
        sceneFBOTextures_.resize(numWindows, 0);
        sceneFBODepthBuffers_.resize(numWindows, 0);

        glGenFramebuffers(numWindows, sceneFBOs_.data());
        glGenRenderbuffers(numWindows, sceneFBODepthBuffers_.data());
        glGenTextures(numWindows, sceneFBOTextures_.data());

        for (auto i = 0U; i < numWindows; ++i) {
            auto projectorNo = pro_cal::getProjectorNo(slaveId, i);
            auto quadCornersName = "quad_corners" + std::to_string(projectorNo);
            auto quadTexCoordsName = "TexCoordinates" + std::to_string(projectorNo);

            quadCorners_[i] = OpenCVParserHelper::ParseVector2f(doc.FirstChildElement("opencv_storage")->FirstChildElement(quadCornersName.c_str()));
            quadTexCoords_[i] = OpenCVParserHelper::ParseVector3f(doc.FirstChildElement("opencv_storage")->FirstChildElement(quadTexCoordsName.c_str()));

            CreateProjectorQuadVBO(i);
            CreateProjectorFBO(i);
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

    void SlaveNode::DrawFrame()
    {
        auto window = GetEngine()->getCurrentWindowPtr();
        auto windowId = window->getId();
        std::array<int, 4> viewport;
        window->getCurrentViewportPixelCoords(viewport[0], viewport[1], viewport[2], viewport[3]);

        window->getFBOPtr()->unBind();

        // Draw scene to off screen texture.
        {
            glViewport(viewport[0], viewport[1], static_cast<GLsizei>(viewport[2] * resolutionScaling_[windowId].x), static_cast<GLsizei>(viewport[3] * resolutionScaling_[windowId].y));

            glBindFramebuffer(GL_FRAMEBUFFER, sceneFBOs_[windowId]);
            GLenum drawBuffers = GL_COLOR_ATTACHMENT0;
            glDrawBuffers(1, &drawBuffers);

            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            ApplicationNodeImplementation::DrawFrame();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_DEPTH_TEST);

        // Draw off screen texture to screen
        {
            window->getFBOPtr()->bind();
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

            sgct::ShaderManager::instance()->bindShaderProgram(calibrationProgram_);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sceneFBOTextures_[windowId]);
            if (useAlphaTransition_) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, tex_AlphaTrans[windowId]);
            }
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, tex_Alpha[windowId]);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D_ARRAY, tex_ColorLookup[windowId]);

            glUniform1i(calibrationSceneTexLoc_, 0);
            glUniform2f(calibrationResolutionLoc_, static_cast<float>(viewport[2]), static_cast<float>(viewport[3]));
            glUniform1i(calibrationUseAlphaTestLoc_, useAlphaTransition_ ? 1 : 0);
            if (useAlphaTransition_) glUniform1i(calibrationAlphaTexLoc_, 1);
            glUniform1i(calibrationAlphaOverlapTexLoc_, 2);
            glUniform1i(calibrationColorLookupTexLoc_, 3);

            glBindVertexArray(vaoProjectorQuads_);
            glDrawArrays(GL_TRIANGLE_FAN, 4 * windowId, 4);
            glBindVertexArray(0);
        }
    }

    void SlaveNode::CreateProjectorQuadVBO(size_t windowId)
    {
        quadCoordsProjector_.emplace_back(glm::vec3(quadCorners_[windowId][3] * 2.0f - 1.0f, 0.0f), quadTexCoords_[windowId][0]);
        quadCoordsProjector_.emplace_back(glm::vec3(quadCorners_[windowId][2] * 2.0f - 1.0f, 0.0f), quadTexCoords_[windowId][1]);
        quadCoordsProjector_.emplace_back(glm::vec3(quadCorners_[windowId][1] * 2.0f - 1.0f, 0.0f), quadTexCoords_[windowId][2]);
        quadCoordsProjector_.emplace_back(glm::vec3(quadCorners_[windowId][0] * 2.0f - 1.0f, 0.0f), quadTexCoords_[windowId][3]);

        glm::vec2 minCoords = quadCoordsProjector_[4 * windowId].position_.xy;
        glm::vec2 maxCoords = quadCoordsProjector_[4 * windowId].position_.xy;
        for (auto i = 1U; i < 4U; ++i) {
            minCoords = glm::min(minCoords, glm::vec2(quadCoordsProjector_[4 * windowId + i].position_.xy));
            minCoords = glm::max(maxCoords, glm::vec2(quadCoordsProjector_[4 * windowId + i].position_.xy));
        }
        resolutionScaling_[windowId] = (maxCoords - minCoords) * 1.1f;
    }

    void SlaveNode::CreateProjectorFBO(size_t windowId)
    {
        glm::ivec2 fboSize;
        auto window = GetEngine()->getWindowPtr(windowId);
        window->getFinalFBODimensions(fboSize.x, fboSize.y);

        fboSize = glm::ivec2(glm::ceil(glm::vec2(fboSize) * resolutionScaling_[windowId]));

        glBindFramebuffer(GL_FRAMEBUFFER, sceneFBOs_[windowId]);
        glBindTexture(GL_TEXTURE_2D, sceneFBOTextures_[windowId]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, fboSize.x, fboSize.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneFBOTextures_[windowId], 0);

        glBindRenderbuffer(GL_RENDERBUFFER, sceneFBODepthBuffers_[windowId]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, fboSize.x, fboSize.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneFBODepthBuffers_[windowId]);

        GLenum drawBuffers = GL_COLOR_ATTACHMENT0;
        glDrawBuffers(1, &drawBuffers);

        auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error("Could not create frame buffer.");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void SlaveNode::loadProperties()
    {
        tinyxml2::XMLDocument doc;
        OpenCVParserHelper::LoadXMLDocument("Program properties", GetConfig().programProperties_, doc);

        useAlphaTransition_ = OpenCVParserHelper::ParseText<bool>(doc.FirstChildElement("opencv_storage")->FirstChildElement("alphaTransition"));
    }
}
