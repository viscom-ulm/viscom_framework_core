/**
 * @file   MeshRenderable.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Implementation of a mesh renderable class.
 */


#include "SceneMeshNode.h"
#include "SubMesh.h"
#include "core/gfx/Material.h"
#include "core/gfx/Texture.h"
#include "core/open_gl.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "AnimMeshRenderable.h"

namespace viscom {

    /**
     * Constructor.
     * @param renderMesh the Mesh to use for rendering.
     * @param program the program used for rendering.
     */
    AnimMeshRenderable::AnimMeshRenderable(const Mesh* renderMesh, GLuint vBuffer, GPUProgram* program) :
        mesh_(renderMesh),
        vbo_(vBuffer),
        vao_(0),
        drawProgram_(program)
    {
    }

    /**
     * Destructor.
     */
    AnimMeshRenderable::~AnimMeshRenderable()
    {
        if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
        if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }

    /**
     * Move constructor.
     * @param orig the original object
     */
    AnimMeshRenderable::AnimMeshRenderable(AnimMeshRenderable&& orig) noexcept :
        mesh_(orig.mesh_),
        vbo_(orig.vbo_),
        vao_(orig.vao_),
        drawProgram_(orig.drawProgram_),
        uniformLocations_(std::move(orig.uniformLocations_))
    {
        orig.mesh_ = nullptr;
        orig.vbo_ = 0;
        orig.vao_ = 0;
        orig.drawProgram_ = nullptr;
    }

    /**
     * Move assignment operator.
     * @param orig the original object
     */
    AnimMeshRenderable& AnimMeshRenderable::operator=(AnimMeshRenderable&& orig) noexcept
    {
        if (this != &orig) {
            this->~AnimMeshRenderable();
            mesh_ = orig.mesh_;
            vbo_ = orig.vbo_;
            vao_ = orig.vao_;
            drawProgram_ = orig.drawProgram_;
            uniformLocations_ = std::move(orig.uniformLocations_);
            orig.mesh_ = nullptr;
            orig.vbo_ = 0;
            orig.vao_ = 0;
            orig.drawProgram_ = nullptr;
        }
        return *this;
    }

    void AnimMeshRenderable::DrawAnimated(const glm::mat4& modelMatrix, int animID, double animTime, bool overrideBump) const
    {
        std::array<glm::mat4, 128> bonePoses_;
        std::array<glm::mat4, 128> skinned_;

        const auto& invBindPoseMatrices = mesh_->GetInverseBindPoseMatrices();

        for (auto i = 0U; i < mesh_->GetNodes().size(); ++i) {
            std::size_t boneIndex = mesh_->GetNodes()[i]->GetBoneIndex();

            bonePoses_[i] = (boneIndex == -1) ? glm::mat4{ 1.0f } : mesh_->GetAnimation(animID)->ComputePoseAtTime(boneIndex, static_cast<Time>(animTime));
        }

        ComputeGlobalBonePose(mesh_->GetRootNode(), bonePoses_);

        for (const auto& node : mesh_->GetNodes()) {
            if (node->GetBoneIndex() == -1) {
                continue;
            }
            skinned_[node->GetBoneIndex()] =
                bonePoses_[node->GetNodeIndex()] * invBindPoseMatrices[node->GetBoneIndex()];
        }

        glUseProgram(drawProgram_->getProgramId());
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_->GetIndexBuffer());
        glUniformMatrix4fv(uniformLocations_[5], static_cast<GLsizei>(skinned_.size()), GL_FALSE, glm::value_ptr(*skinned_.data()));
        DrawNodeAnimated(modelMatrix, bonePoses_, mesh_->GetRootNode(), overrideBump);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void AnimMeshRenderable::DrawNodeAnimated(const glm::mat4& modelMatrix, const std::array<glm::mat4, 128>& bonePoses_, const SceneMeshNode* node, bool overrideBump) const
    {
        auto localMatrix = node->GetLocalTransform() * modelMatrix;
        for (std::size_t i = 0; i < node->GetNumberOfSubMeshes(); ++i) {
            const auto* submesh = &mesh_->GetSubMeshes()[node->GetSubMeshID(i)];
            DrawSubMeshAnimated(localMatrix, bonePoses_[node->GetNodeIndex()], submesh, overrideBump);
        }
        for (std::size_t i = 0; i < node->GetNumberOfNodes(); ++i) DrawNodeAnimated(localMatrix, bonePoses_, node->GetChild(i), overrideBump);
    }

    void AnimMeshRenderable::DrawSubMeshAnimated(const glm::mat4& modelMatrix, const glm::mat4& bonePose_, const SubMesh* subMesh, bool overrideBump) const
    {
        if (subMesh->GetNumberOfIndices() == 0) return;

        glUniformMatrix4fv(uniformLocations_[0], 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix3fv(uniformLocations_[1], 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(modelMatrix))));
        glUniformMatrix4fv(uniformLocations_[6], 1, GL_FALSE, glm::value_ptr(glm::inverse(bonePose_)));

        auto mat = mesh_->GetMaterial(subMesh->GetMaterialIndex());
        auto matTex = mesh_->GetMaterialTexture(subMesh->GetMaterialIndex());
        if (matTex->diffuseTex && uniformLocations_.size() > 2) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, matTex->diffuseTex->getTextureId());
            glUniform1i(uniformLocations_[2], 0);
        }
        if (matTex->bumpTex && uniformLocations_.size() > 3) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, matTex->bumpTex->getTextureId());
            glUniform1i(uniformLocations_[3], 1);
            if (!overrideBump) glUniform1f(uniformLocations_[4], mat->bumpMultiplier);
        }

        glDrawElements(GL_TRIANGLES, subMesh->GetNumberOfIndices(), GL_UNSIGNED_INT,
            (static_cast<char*> (nullptr)) + (static_cast<std::size_t>(subMesh->GetIndexOffset()) * sizeof(unsigned int)));
    }

    void AnimMeshRenderable::ComputeGlobalBonePose(const SceneMeshNode* node, std::array<glm::mat4, 128>& bonePoses_) const
    {
        if (node->GetParent()) {
            bonePoses_[node->GetNodeIndex()] = bonePoses_[node->GetParent()->GetNodeIndex()] * bonePoses_[node->GetNodeIndex()];
        }

        for (auto i = 0U; i < node->GetNumberOfNodes(); ++i) {
            ComputeGlobalBonePose(node->GetChild(i), bonePoses_);
        }
    }
}
