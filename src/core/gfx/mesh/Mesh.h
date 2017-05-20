/**
 * @file   Mesh.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Declaration of a mesh class.
 */

#pragma once

#include "main.h"
#include <sgct.h>
#include "core/resources/Resource.h"
#include "SubMesh.h"

namespace viscom {

    class ApplicationNodeInternal;
    struct Material;
    class SceneMeshNode;
    class Texture;

    /** The vertex object used in these examples. */
    struct MeshVertex {
        /** The vertex position. */
        glm::vec4 position;
        /** The vertex normal. */
        glm::vec3 normal;
        /** The vertex texture coordinate. */
        glm::vec2 textureCoordinate;
    };

    /**
     * Helper class for loading an OpenGL texture from file.
     */
    class Mesh final : public Resource
    {
    public:
        Mesh(const std::string& meshFilename, ApplicationNodeInternal* node);
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&) noexcept;
        Mesh& operator=(Mesh&&) noexcept;
        virtual ~Mesh() override;

        /**
         *  Accessor to the meshes sub-meshes. This can be used to render more complicated meshes (with multiple sets
         *  of texture coordinates).
         */
        std::vector<SubMesh>& GetSubMeshes() { return subMeshes_; }
        /** Const accessor to the meshes sub-meshes. */
        const std::vector<SubMesh>& GetSubMeshes() const { return subMeshes_; }
        const SceneMeshNode* GetRootNode() const { return rootNode_.get(); }

        const std::vector<glm::vec3>& GetVertices() const { return vertices_; }
        const std::vector<glm::vec3>& GetNormals() const { return normals_; }
        const std::vector<glm::vec3>& GetTexCoords(size_t i) const { return texCoords_[i]; }
        const std::vector<glm::vec3>& GetTangents() const { return tangents_; }
        const std::vector<glm::vec3>& GetBinormals() const { return binormals_; }
        const std::vector<glm::vec4>& GetColors(size_t i) const { return colors_[i]; }
        const std::vector<unsigned int>& GetIndices() const { return indices_; }
        GLuint GetIndexBuffer() const { return indexBuffer_; }

    private:
        std::shared_ptr<const Texture> loadTexture(const std::string& relFilename, ApplicationNodeInternal* node) const;


        /** Holds all the single points used by the mesh (and its sub-meshes) as points or in vertices. */
        std::vector<glm::vec3> vertices_;
        /** Holds all the single normals used by the mesh (and its sub-meshes). */
        std::vector<glm::vec3> normals_;
        /** Holds all the single texture coordinates used by the mesh (and its sub-meshes). */
        std::vector<std::vector<glm::vec3>> texCoords_;
        /** Holds all the single tangents used by the mesh (and its sub-meshes). */
        std::vector<glm::vec3> tangents_;
        /** Holds all the single binormals used by the mesh (and its sub-meshes). */
        std::vector<glm::vec3> binormals_;
        /** Holds all the single colors used by the mesh (and its sub-meshes). */
        std::vector<std::vector<glm::vec4>> colors_;
        /** Holds all the indices used by the sub-meshes. */
        std::vector<unsigned int> indices_;

        /** Holds all materials of the mesh. */
        std::vector<Material> materials_;

        /** Holds all the meshes sub-meshes. */
        std::vector<SubMesh> subMeshes_;

        /** The root scene node. */
        std::unique_ptr<SceneMeshNode> rootNode_;

        /** Holds the OpenGL index buffer. */
        GLuint indexBuffer_;
    };
}
