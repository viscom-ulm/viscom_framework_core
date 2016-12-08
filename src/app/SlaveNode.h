/**
 * @file   SlaveNode.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declaration of the ApplicationNodeImplementation for the slave node.
 */

#pragma once

#include "app/ApplicationNodeImplementation.h"
#include "core/CalibrationVertices.h"

namespace viscom {

    class SlaveNode final : public ApplicationNodeImplementation
    {
    public:
        explicit SlaveNode(ApplicationNode* appNode);
        ~SlaveNode();

        void InitOpenGL() override;
        void DrawFrame() override;
        void CleanUp() override;

    private:
        void loadProperties();
        void CreateProjectorFBO(size_t windowId, const glm::ivec2& projectorSize);

        /** Holds whether to use alpha transition for blending. */
        bool useAlphaTransition_;
        /** Holds the cell count for color calibration. */
        unsigned int colorCalibrationCellCount_;
        /** Holds the value count for color calibration. */
        unsigned int colorCalibrationValueCount_;

        /** Holds the resolution scaling for each quad. */
        std::vector<glm::vec2> resolutionScaling_;
        /** Holds the vertex coordinates for the screen aligned quad to render to (in projector space). */
        std::vector<CalbrationProjectorQuadVertex> quadCoordsProjector_;


        /** Holds the shader program for applying the calibration to a rendered scene. */
        sgct::ShaderProgram calibrationProgram_;
        /** Holds the location of the use alpha test flag. */
        GLint calibrationUseAlphaTestLoc_ = -1;
        /** Holds the location of the use alpha test flag. */
        GLint calibrationAlphaTexLoc_ = -1;
        /** Holds the location of the use alpha test flag. */
        GLint calibrationAlphaOverlapTexLoc_ = -1;
        /** Holds the location of the use alpha test flag. */
        GLint calibrationColorLookupTexLoc_ = -1;
        /** Holds the location of the use alpha test flag. */
        GLint calibrationSceneTexLoc_ = -1;
        /** Holds the location of the use alpha test flag. */
        GLint calibrationResolutionLoc_ = -1;

        /** Holds the vertex buffer for the projector quads. */
        GLuint vboProjectorQuads_ = 0;
        /** Holds the vertex array object for the projector quads. */
        GLuint vaoProjectorQuads_ = 0;
        /** Holds the frame buffers for rendering the scene into. */
        std::vector<GLuint> sceneFBOs_;
        /** Holds the frame buffer textures for rendering the scene into. */
        std::vector<GLuint> sceneFBOTextures_;
        /** Holds the frame buffer depth buffers for rendering the scene into. */
        std::vector<GLuint> sceneFBODepthBuffers_;
        /** Holds the alpha textures. */
        std::vector<GLuint> alphaTextures_;
        /** Holds the alpha trans textures. */
        std::vector<GLuint> alphaTransTextures_;
        /** Holds the color lookup tables textures. */
        std::vector<GLuint> colorLookUpTableTextures_;
    };
}
