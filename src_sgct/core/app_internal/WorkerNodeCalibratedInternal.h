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
        
        virtual void ParseTrackingFrame() override;
        virtual glm::vec3 GetController0Pos() override;
        virtual glm::vec3 GetController0Zvec() override;
        virtual glm::vec3 GetController1Pos() override;
        virtual glm::vec3 GetController1Zvec() override;
        virtual glm::vec3 GetTrackerPos() override;
        virtual glm::vec3 GetTrackerZvec() override;
        virtual glm::quat GetController0Rot() override;
        virtual glm::quat GetController1Rot() override;
        virtual glm::quat GetTrackerRot() override;
        virtual glm::vec2 GetDisplayPosition(bool useLeftController) override;
        virtual void InitialiseDisplay(bool useLeftController) override;
        virtual bool GetDisplayInitialised() override;
        virtual void SetDisplayNotInitialised() override;
        virtual bool GetDisplayInitByFloor() override;
        virtual void SetDisplayInitByFloor(bool b) override;
        virtual void PollAndParseNextEvent() override;
        virtual void PollAndParseEvents() override;
        virtual std::vector<std::string> OutputDevices() override;
        virtual float* GetDisplayEdges() override;
        virtual bool GetVrInitSuccess() override;
        virtual std::vector<std::string> GetController0Buttons() override;
        virtual std::vector<std::string> GetController1Buttons() override;
    };
}
