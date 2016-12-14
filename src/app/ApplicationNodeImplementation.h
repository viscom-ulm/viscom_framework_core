/**
 * @file   ApplicationNodeImplementation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Declaration of the application node implementation common for master and slave nodes.
 */

#pragma once

#include <sgct/Engine.h>
#include "core/ApplicationNode.h"

namespace viscom {

    class ApplicationNodeImplementation
    {
    public:
        explicit ApplicationNodeImplementation(ApplicationNode* appNode);
        ApplicationNodeImplementation(const ApplicationNodeImplementation&) = delete;
        ApplicationNodeImplementation(ApplicationNodeImplementation&&) = delete;
        ApplicationNodeImplementation& operator=(const ApplicationNodeImplementation&) = delete;
        ApplicationNodeImplementation& operator=(ApplicationNodeImplementation&&) = delete;
        virtual ~ApplicationNodeImplementation();

        virtual void PreWindow();
        virtual void InitOpenGL();
        virtual void PreSync();
        virtual void UpdateSyncedInfo();
        virtual void UpdateFrame(double currentTime, double elapsedTime);
        virtual void ClearBuffer();
        virtual void DrawFrame();
        virtual void Draw2D();
        virtual void PostDraw();
        virtual void CleanUp();

        virtual void KeyboardCallback(int key, int scancode, int action, int mods);
        virtual void CharCallback(unsigned int character, int mods);
        virtual void MouseButtonCallback(int button, int action);
        virtual void MousePosCallback(double x, double y);
        virtual void MouseScrollCallback(double xoffset, double yoffset);

        virtual void EncodeData();
        virtual void DecodeData();

    protected:
        sgct::Engine* GetEngine() const { return appNode_->GetEngine(); }
        const FWConfiguration& GetConfig() const { return appNode_->GetConfig(); }
        unsigned int GetGlobalProjectorId(int nodeId, int windowId) const { return appNode_->GetGlobalProjectorId(nodeId, windowId); }

        const std::pair<glm::ivec2, glm::ivec2>& GetViewportScreen(size_t windowId) const { return appNode_->GetViewportScreen(windowId); }
        std::pair<glm::ivec2, glm::ivec2>& GetViewportScreen(size_t windowId) { return appNode_->GetViewportScreen(windowId); }
        const glm::ivec2& GetViewportQuadSize(size_t windowId) const { return appNode_->GetViewportQuadSize(windowId); }
        glm::ivec2& GetViewportQuadSize(size_t windowId) { return appNode_->GetViewportQuadSize(windowId); }
        const glm::vec2& GetViewportScaling(size_t windowId) const { return appNode_->GetViewportScaling(windowId); }
        glm::vec2& GetViewportScaling(size_t windowId) { return appNode_->GetViewportScaling(windowId); }

        double GetCurrentAppTime() const { return appNode_->GetCurrentAppTime(); }
        double GetElapsedTime() const { return appNode_->GetElapsedTime(); }

    private:
        /** Holds the application node. */
        ApplicationNode* appNode_;

        /** Holds the shader program for drawing the background. */
        sgct::ShaderProgram backgroundProgram_;
        /** Holds the location of the MVP matrix. */
        GLint backgroundMVPLoc_ = -1;

        /** Holds the shader program for drawing the foreground triangle. */
        sgct::ShaderProgram triangleProgram_;
        /** Holds the location of the MVP matrix. */
        GLint triangleMVPLoc_ = -1;

        /** Holds the number of vertices of the background grid. */
        unsigned int numBackgroundVertices_ = 0;
        /** Holds the vertex buffer for the background grid. */
        GLuint vboBackgroundGrid_ = 0;
        /** Holds the vertex array object for the background grid. */
        GLuint vaoBackgroundGrid_ = 0;

        glm::mat4 triangleModelMatrix_;
    };
}
