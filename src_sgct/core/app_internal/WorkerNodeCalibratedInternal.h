/**
 * @file   WorkerNodeCalibratedInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.06.15
 *
 * @brief  Declaration of the ApplicationNodeInternal for calibrated workers.
 */

#pragma once

#include "core/app_internal/WorkerNodeLocalInternal.h"
#include "core/CalibrationVertices.h"

namespace viscom {

    class WorkerNodeCalibratedInternal : public WorkerNodeLocalInternal
    {
    public:
        explicit WorkerNodeCalibratedInternal(FrameworkInternal& fwInternal);
        virtual ~WorkerNodeCalibratedInternal() override;

        void InitOpenGL() override;
        void DrawFrame(FrameBuffer& fbo) override;
        void Draw2D(FrameBuffer& fbo) override;
        void CleanUp() override;

    private:
        void CreateProjectorFBO(size_t windowId, const glm::ivec2& fboSize);


        /** Holds the viewport for rendering directly to the projector. */
        std::vector<Viewport> projectorViewport_;
        /** Holds the vertex coordinates for the screen aligned quad to render to (in projector space). */
        std::vector<CalbrationProjectorQuadVertex> quadCoordsProjector_;


        /** Holds the shader program for applying the calibration to a rendered scene. */
        std::shared_ptr<GPUProgram> calibrationProgram_;
        /** Holds the location of the use alpha test flag. */
        GLint calibrationAlphaTexLoc_ = -1;
        /** Holds the location of the use alpha test flag. */
        GLint calibrationSceneTexLoc_ = -1;

        /** Holds the vertex buffer for the projector quads. */
        GLuint vboProjectorQuads_ = 0;
        /** Holds the vertex array object for the projector quads. */
        GLuint vaoProjectorQuads_ = 0;
        /** Holds the frame buffers for rendering the scene into. */
        std::vector<FrameBuffer> sceneFBOs_;
        /** Holds the alpha textures. */
        std::vector<GLuint> alphaTextures_;
        
        void ParseTrackingFrame();
        glm::vec3 GetController0Pos();
        glm::vec3 GetController0Zvec();
        glm::vec3 GetController1Pos();
        glm::vec3 GetController1Zvec();
        glm::vec3 GetTrackerPos();
        glm::vec3 GetTrackerZvec();
        glm::quat GetController0Rot();
        glm::quat GetController1Rot();
        glm::quat GetTrackerRot();
        glm::vec2 GetDisplayPointerPosition(bool useLeftController);
        void InitialiseDisplay(bool useLeftController);
        bool GetDisplayInitialised();
        void SetDisplayNotInitialised();
        bool GetDisplayInitByFloor();
        void SetDisplayInitByFloor(bool b);
        void PollAndParseNextEvent();
        void PollAndParseEvents();
        std::vector<std::string> OutputDevices();
        float* GetDisplayEdges();
        bool GetVrInitSuccess();
        std::vector<std::string> GetController0Buttons();
        std::vector<std::string> GetController1Buttons();
    };
}
