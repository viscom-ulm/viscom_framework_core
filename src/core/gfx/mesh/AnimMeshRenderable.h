/**
 * @file   MeshRenderable.h
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

namespace viscom {

    class Mesh;

    /**
     *  Contains the current animation state of an animated object.
     */
    class AnimationState final
    {
    public:
        AnimationState(const Mesh* mesh, const SubAnimationMapping& mappings, std::size_t startingAnimationIndex = 0, bool isRepeating = true);

        void Play(double currentTime);
        /**
         *  Pauses the animation.
         *  @param currentTime the current timestamp.
         */
        void Pause(double currentTime) { isPlaying_ = false; pauseTime_ = static_cast<float>(currentTime); }
        /** Stops the animation. */
        void Stop() { isPlaying_ = false; currentPlayTime_ = 0.0f; pauseTime_ = 0.0f; }

        /** Accessor for animation speed. */
        float GetSpeed() const { return animationPlaybackSpeed_.at(animationIndex_); }
        /** Accessor for animation frames per second. */
        float GetFramesPerSecond() const { return animations_.at(animationIndex_).GetFramesPerSecond(); }
        /** Accessor for animation duration. */
        float GetDuration() const { return animations_.at(animationIndex_).GetDuration(); }
        /** Accessor for animation time. */
        float GetTime() const { return currentPlayTime_; }

        /** Checks whether the animation is playing. */
        bool IsPlaying() const { return isPlaying_; }
        /** Checks whether the animation is repeating. */
        bool IsRepeating() const { return isRepeating_; }

        bool UpdateTime(double currentTime);
        void ComputeAnimationsFinalBonePoses();

        /**
         *  Sets the current animation speed.
         *  @param speed the new animation speed.
         */
        void SetSpeed(float speed) { animationPlaybackSpeed_[animationIndex_] = speed; }
        /**
         *  Sets whether the animation should be repeated.
         *  @param repeat whether the animation should be repeated.
         */
        void SetRepeat(bool repeat) { isRepeating_ = repeat; }

        /**
         *  Returns the current animation index.
         *  @return the current animation index.
         */
        std::size_t GetCurrentAnimationIndex() const { return animationIndex_; }
        /**
         *  Sets the current animation index.
         *  @param animationIndex the new current animation index.
         */
        void SetCurrentAnimationIndex(std::size_t animationIndex) { animationIndex_ = animationIndex; }

        /** Returns the global bone pose for a node. */
        const glm::mat4& GetGlobalBonePose(std::size_t index) const { return globalBonePoses_[index]; }
        /** Returns the local bone pose for a node. */
        const glm::mat4& GetLocalBonePose(std::size_t index) const { return localBonePoses_[index]; }
        /** Returns the skinning matrices. */
        const std::vector<glm::mat4>& GetSkinningMatrices() const { return skinned_; }

    private:
        void ComputeGlobalBonePose(const SceneMeshNode* node);

        /** Holds the mesh to render. */
        const Mesh* mesh_;
        /** The current animation id. **/
        std::size_t animationIndex_;

        /** Holds the playback speed for each AnimationState. */
        std::vector<float> animationPlaybackSpeed_;
        /** Holds the actual animation for each AnimationState. */
        std::vector<Animation> animations_;


        /** Is the animation playing. */
        bool isPlaying_ = false;
        /** Is the animation repeating. */
        bool isRepeating_ = true;

        /** The starting time of the animation. */
        float startTime_ = 0.0f;
        /** The starting time of the current pause of the animation. */
        float pauseTime_ = 0.0f;
        /** The starting playback time of the animation. */
        float currentPlayTime_ = 0.0f;

        /** The local bone poses. */
        std::vector<glm::mat4> localBonePoses_;
        /** The global bone poses. */
        std::vector<glm::mat4> globalBonePoses_;
        /** The skinning matrices used for rendering. */
        std::vector<glm::mat4> skinned_;
    };

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
