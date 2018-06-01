/**
 * @file   Mesh.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Declaration of a mesh class.
 */

#pragma once

#include "Animation.h"
#include "SubMesh.h"
#include "core/gfx/Material.h"
#include "core/main.h"
#include "core/math/aabb.h"
#include "core/open_gl_fwd.h"
#include "core/resources/Resource.h"
#include "core/utils/serializationHelper.h"

struct aiNode;

namespace viscom {

    class ApplicationNodeInternal;
    struct Material;
    struct MaterialTextures;
    class SceneMeshNode;
    class Texture;
    class TextureManager;

    /**
     * Helper class for loading an OpenGL texture from file.
     */
    class Mesh final : public Resource
    {
    public:
        Mesh(const std::string& meshFilename, ApplicationNodeInternal* node, bool synchronize = false);
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&) noexcept = delete;
        Mesh& operator=(Mesh&&) noexcept = delete;
        virtual ~Mesh() noexcept override;

        void Initialize();

        /**
         *  Accessor to the meshes sub-meshes. This can be used to render more complicated meshes (with multiple sets
         *  of texture coordinates).
         */
        std::vector<SubMesh>& GetSubMeshes() noexcept { return subMeshes_; }
        /** Const accessor to the meshes sub-meshes. */
        const std::vector<SubMesh>& GetSubMeshes() const noexcept { return subMeshes_; }
        const std::vector<const SceneMeshNode*>& GetNodes() const noexcept { return nodes_; }
        const SceneMeshNode* GetRootNode() const noexcept { return rootNode_.get(); }

        const std::vector<glm::vec3>& GetVertices() const noexcept { return vertices_; }
        const std::vector<glm::vec3>& GetNormals() const noexcept { return normals_; }
        std::size_t GetNumTexCoords() const { return texCoords_.size(); }
        const std::vector<glm::vec3>& GetTexCoords(size_t i) const noexcept { return texCoords_[i]; }
        const std::vector<glm::vec3>& GetTangents() const noexcept { return tangents_; }
        const std::vector<glm::vec3>& GetBinormals() const noexcept { return binormals_; }
        const std::vector<glm::vec4>& GetColors(size_t i) const noexcept { return colors_[i]; }
        const std::vector<glm::uvec4>& GetBoneIndices() const noexcept { return boneOffsetMatrixIndices_; }
        const std::vector<glm::vec4>& GetBoneWeights() const noexcept { return boneWeights_; }
        const std::vector<glm::uvec4>& GetIndexVectors(size_t i) const noexcept { return indexVectors_[i]; }

        const std::vector<unsigned int>& GetIndices() const noexcept { return indices_; }
        GLuint GetIndexBuffer() const noexcept { return indexBuffer_; }

        const Animation* GetAnimation(std::size_t animationIndex = 0) const { return &animations_[animationIndex]; }
        const Material* GetMaterial(std::size_t materialIndex) const { return &materials_[materialIndex]; }
        const MaterialTextures* GetMaterialTexture(std::size_t materialIndex) const { return &materialTextures_[materialIndex]; }

        const std::vector<glm::mat4>& GetInverseBindPoseMatrices() const noexcept { return inverseBindPoseMatrices_; }
        const std::vector<math::AABB3<float>>& GetBoneBoundingBoxes() const noexcept { return boneBoundingBoxes_; }
        std::size_t GetParentBone(std::size_t boneIndex) const { return boneParent_[boneIndex]; }
        std::size_t GetNumberOfBones() const noexcept { return inverseBindPoseMatrices_.size(); }

        glm::mat4 GetGlobalInverse() const { return globalInverse_; }

    protected:
        virtual void Load(std::optional<std::vector<std::uint8_t>>& data) override;
        virtual void LoadFromMemory(const void* data, std::size_t size) override;

    private:
        using VersionableSerializerType = serializeHelper::VersionableSerializer<'V', 'M', 'E', 'S', 1000>;

        std::shared_ptr<const Texture> LoadTexture(const std::string& relFilename, ApplicationNodeInternal* node) const;
        void LoadAssimpMeshFromFile(const std::string& filename, const std::string& binFilename, ApplicationNodeInternal* node);
        void LoadAssimpMesh(const aiScene* scene, ApplicationNodeInternal* node);
        void Save(const std::string& filename) const;
        void Write(std::ostream& ofs) const;
        bool Load(const std::string& filename, const std::string& binFilename, ApplicationNodeInternal* node);
        bool Read(std::istream& ifs, TextureManager& texMan);

        void ParseBoneHierarchy(const std::map<std::string, unsigned int>& bones, const aiNode* node,
            std::size_t parent, glm::mat4 parentMatrix);

        void GenerateBoneBoundingBoxes();

        /** Filename of this mesh. */
        std::string filename_;

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
        /** The indices to bones influencing this vertex (corresponds to boneWeights_). */
        std::vector<glm::uvec4> boneOffsetMatrixIndices_;
        /** Weights, how strong a vertex is influenced by the matrix of the bone. */
        std::vector<glm::vec4> boneWeights_;
        /** Holds integer vectors to be used as indices (similar to boneOffsetMatrixIndices_ but more general). */
        std::vector<std::vector<glm::uvec4>> indexVectors_;

        /** Offset matrices for each bone. */
        std::vector<glm::mat4> inverseBindPoseMatrices_;
        /**
        *  Parent of a bone. Stores the parent for each bone in
        *  boneOffsetMatrices_
        */
        std::vector<std::size_t> boneParent_;

        /** Holds all the indices used by the sub-meshes. */
        std::vector<unsigned int> indices_;

        /** Holds all materials of the mesh. */
        std::vector<Material> materials_;
        /** Holds all materials of the mesh. */
        std::vector<MaterialTextures> materialTextures_;
        /** Holds all the meshes sub-meshes. */
        std::vector<SubMesh> subMeshes_;
        /** Nodes in this mesh. */
        std::vector<const SceneMeshNode*> nodes_;
        /** Animations of this mesh */
        std::vector<Animation> animations_;

        /** The root scene node. */
        std::unique_ptr<SceneMeshNode> rootNode_;
        /** The global inverse of this mesh. */
        glm::mat4 globalInverse_;
        /** AABB for all bones */
        std::vector<math::AABB3<float>> boneBoundingBoxes_;

        /** Holds the OpenGL index buffer. */
        GLuint indexBuffer_;
    };
}
