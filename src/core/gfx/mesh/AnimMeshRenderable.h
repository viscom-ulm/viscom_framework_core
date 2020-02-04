/**
 * @file   AnimMeshRenderable.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Declaration of a mesh renderable class.
 */

#pragma once

#include "core/main.h"
#include "Mesh.h"
#include "core/gfx/GPUProgram.h"

#include <vector>
#include "AnimationState.h"

namespace viscom {

    class Mesh;

    /**
     *  This class renders a mesh with a specific shader. The shader is assumed to have fixed uniform names:
     *  modelMatrix: the model matrix.
     *  normalMatrix: the normal matrix.
     *  diffuseTexture: the diffuse texture.
     *  bumpTexture: a bump map.
     *  bumpMultiplier: the bump multiplier.
     *
     *  NOT ALL UNIFORM LOCATIONS NEED TO BE USED!
     *
     *  The attribute names are determined by the vertex structure.
     */
    class AnimMeshRenderable
    {
    public:
        /**
         *  Creates a new mesh renderable.
         *  @param renderMesh the mesh to render.
         *  @param program the GPU program to render the mesh with.
         */
        template<class VTX> static std::unique_ptr<AnimMeshRenderable> create(const Mesh* renderMesh, GPUProgram* program);

        virtual ~AnimMeshRenderable();
        AnimMeshRenderable(const AnimMeshRenderable&) = delete;
        AnimMeshRenderable& operator=(const AnimMeshRenderable&) = delete;
        AnimMeshRenderable(AnimMeshRenderable&&) noexcept;
        AnimMeshRenderable& operator=(AnimMeshRenderable&&) noexcept;

        /**
        *  Draws the mesh of the mesh renderable with a specified animation clip and time.
        *  @param modelMatrix the model matrix to draw the mesh with.
        *  @param animState the current state of the animation.
        *  @param overrideBump flag for bump map parameters.
        */
        void DrawAnimated(const glm::mat4& modelMatrix, const AnimationState& animState, bool overrideBump = false) const;

        /**
         *  Gets the standard uniform locations when a mesh renderable is created.
         *  @param program the GPU program to bind the uniforms to.
         */
        template<class VTX> void NotifyRecompiledShader(const GPUProgram* program);

    protected:
        /** Constructor method.
         *  @param renderMesh the mesh to render.
         *  @param vBuffer the meshes vertex buffer.
         *  @param program the GPU program to be used with the mesh.
         */
        AnimMeshRenderable(const Mesh* renderMesh, GLuint vBuffer, GPUProgram* program);

        /**
         *  Draws a node and all its child nodes of the mesh.
         *  @param modelMatrix the model matrix to draw the mesh with.
        *  @param animState the current state of the animation.
         *  @param node the node to draw.
         *  @param overrideBump flag for bump map parameters.
         */
        void DrawNodeAnimated(const glm::mat4& modelMatrix, const AnimationState& animState, const SceneMeshNode* node, bool overrideBump = false) const;

    private:
        /** Holds the mesh to render. */
        const Mesh* mesh_;
        /** Holds the vertex buffer. */
        GLuint vbo_;
        /** Holds the vertex array object. */
        GLuint vao_;
        /** Holds the rendering GPU program for drawing. */
        GPUProgram* drawProgram_;
        /** Holds the standard uniform bindings. */
        std::vector<GLint> uniformLocations_;

        /**
         *  Draws a sub mesh of the mesh renderable.
         *  @param modelMatrix the model matrix to draw the sub mesh with.
         *  @param subMesh the sub mesh to be drawn.
         *  @param overrideBump flag for bump map parameters.
         */
        void DrawSubMeshAnimated(const glm::mat4& modelMatrix, const glm::mat4& bonePose_, const SubMesh* subMesh, bool overrideBump = false) const;
    };

    template <class VTX>
    std::unique_ptr<AnimMeshRenderable> AnimMeshRenderable::create(const Mesh* renderMesh, GPUProgram* program)
    {
        std::unique_ptr<AnimMeshRenderable> result{ new AnimMeshRenderable(renderMesh, VTX::CreateVertexBuffer(renderMesh), program) };
        result->NotifyRecompiledShader<VTX>(program);
        return result;
    }

    template<class VTX> void AnimMeshRenderable::NotifyRecompiledShader(const GPUProgram* program)
    {
        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_->GetIndexBuffer());
        VTX::SetVertexAttributes(program);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        uniformLocations_ = program->GetUniformLocations({ "modelMatrix", "normalMatrix", "diffuseTexture", "bumpTexture", "bumpMultiplier", "skinningMatrices", "invNodeMatrix" });
    }
}
