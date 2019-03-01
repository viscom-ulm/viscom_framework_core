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

#include "MeshRenderable.h"

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
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_->GetIndexBuffer());
        DrawNode(modelMatrix, mesh_->GetRootNode(), overrideBump);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void MeshRenderable::DrawNode(const glm::mat4& modelMatrix, const SceneMeshNode* node, bool overrideBump) const
    {
        if (!node->HasMeshes()) return;

        auto localMatrix = modelMatrix * node->GetLocalTransform();
        for (std::size_t i = 0; i < node->GetNumberOfSubMeshes(); ++i) {
            const auto* submesh = &mesh_->GetSubMeshes()[node->GetSubMeshID(i)];
            DrawSubMesh(localMatrix, submesh, overrideBump);
        }
        for (std::size_t i = 0; i < node->GetNumberOfNodes(); ++i) DrawNode(localMatrix, node->GetChild(i), overrideBump);
    }

    void MeshRenderable::DrawSubMesh(const glm::mat4& modelMatrix, const SubMesh* subMesh, bool overrideBump) const
    {
        if (subMesh->GetNumberOfIndices() == 0) return;

        glUniformMatrix4fv(uniformLocations_[0], 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix3fv(uniformLocations_[1], 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(modelMatrix))));

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
            reinterpret_cast<char*>(static_cast<std::size_t>(subMesh->GetIndexOffset()) * sizeof(unsigned int)));
    }
}
