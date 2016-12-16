/**
 * @file   MeshRenderable.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Implementation of a mesh renderable class.
 */

#include "MeshRenderable.h"
#include <glm/gtc/matrix_inverse.hpp>
#include "SceneMeshNode.h"
#include "SubMesh.h"
#include "core/gfx/Material.h"
#include "core/gfx/Texture.h"

namespace viscom {

    /**
     * Constructor.
     * @param renderMesh the Mesh to use for rendering.
     * @param program the program used for rendering.
     */
    MeshRenderable::MeshRenderable(const Mesh* renderMesh, GLuint vBuffer, GPUProgram* program) :
        mesh_(renderMesh),
        vbo_(vBuffer),
        vao_(0),
        drawProgram_(program)
    {
    }

    /**
     * Destructor.
     */
    MeshRenderable::~MeshRenderable()
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
    MeshRenderable::MeshRenderable(MeshRenderable&& orig) noexcept :
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
    MeshRenderable& MeshRenderable::operator=(MeshRenderable&& orig) noexcept
    {
        if (this != &orig) {
            this->~MeshRenderable();
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

    void MeshRenderable::Draw(const glm::mat4& modelMatrix, bool overrideBump) const
    {
        glUseProgram(drawProgram_->getProgramId());
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        DrawNode(modelMatrix, mesh_->GetRootNode(), overrideBump);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void MeshRenderable::DrawNode(const glm::mat4& modelMatrix, const SceneMeshNode* node, bool overrideBump) const
    {
        auto localMatrix = node->GetLocalTransform() * modelMatrix;
        for (unsigned int i = 0; i < node->GetNumMeshes(); ++i) DrawSubMesh(localMatrix, node->GetMesh(i), overrideBump);
        for (unsigned int i = 0; i < node->GetNumNodes(); ++i) DrawNode(localMatrix, node->GetChild(i), overrideBump);
    }

    void MeshRenderable::DrawSubMesh(const glm::mat4& modelMatrix, const SubMesh* subMesh, bool overrideBump) const
    {
        glUniformMatrix4fv(uniformLocations_[0], 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix3fv(uniformLocations_[1], 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(modelMatrix))));

        if (subMesh->GetMaterial()->diffuseTex && uniformLocations_.size() != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, subMesh->GetMaterial()->diffuseTex->getTextureId());
            glUniform1ui(uniformLocations_[2], 0);
        }
        if (subMesh->GetMaterial()->bumpTex && uniformLocations_.size() >= 2) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, subMesh->GetMaterial()->bumpTex->getTextureId());
            glUniform1ui(uniformLocations_[3], 1);
            if (!overrideBump) glUniform1f(uniformLocations_[4], subMesh->GetMaterial()->bumpMultiplier);
        }

        glDrawElements(GL_TRIANGLES, subMesh->GetNumberOfIndices(), GL_UNSIGNED_INT,
            (static_cast<char*> (nullptr)) + (subMesh->GetIndexOffset() * sizeof(unsigned int)));
    }
}
