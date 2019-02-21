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

    class FrameworkInternal;
    struct Material;
    struct MaterialTextures;
    class SceneMeshNode;
    class Texture;
    class TextureManager;

    /**
     * Class defining a mesh.
     */
    class Mesh final : public Resource
    {
    public:
        Mesh(const std::string& meshFilename, FrameworkInternal* node, bool synchronize = false);
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&) noexcept = delete;
        Mesh& operator=(Mesh&&) noexcept = delete;
        virtual ~Mesh() noexcept override;

        /**
         *  Initializes the mesh by setting the force generating normals flag.
         *  @param forceGenNormals force generating normals.
         */
        void Initialize(bool forceGenNormals = false);

        /**
         *  Accessor to the meshes sub-meshes. This can be used to render more complicated meshes (with multiple sets
         *  of texture coordinates).
         */
        std::vector<SubMesh>& GetSubMeshes() noexcept { return subMeshes_; }
        /** Const accessor to the meshes sub-meshes. */
        const std::vector<SubMesh>& GetSubMeshes() const noexcept { return subMeshes_; }
        /** Accessor to the nodes of the mesh. */
        const std::vector<const SceneMeshNode*>& GetNodes() const noexcept { return nodes_; }
        /** Returns the root node of the mesh. */
        const SceneMeshNode* GetRootNode() const noexcept { return rootNode_.get(); }

        /** Returns the vertices used by the mesh and all sub-meshes. */
        const std::vector<glm::vec3>& GetVertices() const noexcept { return vertices_; }
        /** Returns the normals used by the mesh and all sub-meshes. */
        const std::vector<glm::vec3>& GetNormals() const noexcept { return normals_; }
        /** Returns the number of texture coords used by the mesh and all sub-meshes. */
        std::size_t GetNumTexCoords() const { return texCoords_.size(); }
        /** Returns the texture coords used by the mesh and all sub-meshes. */
        const std::vector<glm::vec3>& GetTexCoords(size_t i) const noexcept { return texCoords_[i]; }
        /** Returns the tangents used by the mesh and all sub-meshes. */
        const std::vector<glm::vec3>& GetTangents() const noexcept { return tangents_; }
        /** Returns the binormals used by the mesh and all sub-meshes. */
        const std::vector<glm::vec3>& GetBinormals() const noexcept { return binormals_; }
        /** Returns the colors of the vertices used by the mesh and all sub-meshes. */
        const std::vector<glm::vec4>& GetColors(size_t i) const noexcept { return colors_[i]; }
        /** Returns the indices of the influencing bones for each vertex. */
        const std::vector<glm::uvec4>& GetBoneIndices() const noexcept { return boneOffsetMatrixIndices_; }
        /** Returns the weights of the influencing bones for each vertex. */
        const std::vector<glm::vec4>& GetBoneWeights() const noexcept { return boneWeights_; }
        /** Returns the integer vectors to be used as indices for each vertex. */
        const std::vector<glm::uvec4>& GetIndexVectors(size_t i) const noexcept { return indexVectors_[i]; }

        /** Returns all the indices used by the sub-meshes. */
        const std::vector<unsigned int>& GetIndices() const noexcept { return indices_; }
        /** Returns the OpenGL index buffer. */
        GLuint GetIndexBuffer() const noexcept { return indexBuffer_; }

        /**
         *  Returns an animation of the mesh.
         *  @param animationIndex index of the animation to return.
         */
        const Animation* GetAnimation(std::size_t animationIndex = 0) const { return &animations_[animationIndex]; }
        /**
         *  Returns a material of the mesh.
         *  @param materialIndex index of the material to return.
         */
        const Material* GetMaterial(std::size_t materialIndex) const { return &materials_[materialIndex]; }
        /**
         *  Returns a material texture of the mesh.
         *  @param materialIndex index of the material to return the texture from.
         */
        const MaterialTextures* GetMaterialTexture(std::size_t materialIndex) const { return &materialTextures_[materialIndex]; }

        /** Returns the offset matrices for all bones. */
        const std::vector<glm::mat4>& GetInverseBindPoseMatrices() const noexcept { return inverseBindPoseMatrices_; }
        /** Returns the AABB for all bones. */
        const std::vector<math::AABB3<float>>& GetBoneBoundingBoxes() const noexcept { return boneBoundingBoxes_; }
        /**
         *  Returns the parent bone of any given bone.
         *  @param boneIndex index of the child bone.
         */
        std::size_t GetParentBone(std::size_t boneIndex) const { return boneParent_[boneIndex]; }
        /** Returns the number of bones used by the mesh. */
        std::size_t GetNumberOfBones() const noexcept { return inverseBindPoseMatrices_.size(); }

        /** Returns the global inverse matrix of the mesh. */
        glm::mat4 GetGlobalInverse() const { return globalInverse_; }

        /** Returns the meshes filename (and path). */
        std::string GetFilename() const;

    protected:
        /**
         *  Loads the mesh data from file.
         *  @param data vector for optional data.
         */
        virtual void Load(std::optional<std::vector<std::uint8_t>>& data) override;
        /**
         *  Loads the mesh data from memory.
         *  @param data pointer to the mesh data.
         *  @param size size of the mesh data.
         */
        virtual void LoadFromMemory(const void* data, std::size_t size) override;

    private:
        /** Defines the type of the VersionableSerializer for the mesh class. */
        using VersionableSerializerType = serializeHelper::VersionableSerializer<'V', 'M', 'E', 'S', 1000>;

        /**
         *  Loads a texture from the texture manager.
         *  @param relFilename the relative path of the file.
         *  @param node the node holding the texture manager.
         */
        std::shared_ptr<const Texture> LoadTexture(const std::string& relFilename, FrameworkInternal* node) const;
        /**
         *  Loads a mesh using Assimp, converts the mesh into the frameworks format and saves it.
         *  @param filename the path to the file.
         *  @param binFilename the path for saving the frameworks mesh format.
         *  @param node the framework.
         */
        void LoadAssimpMeshFromFile(const std::string& filename, const std::string& binFilename, FrameworkInternal* node);
        /**
         *  Converts an Assimp scene into the frameworks mesh format.
         *  @param scene the Assimp scene.
         *  @param node the framework.
         */
        void LoadAssimpMesh(const aiScene* scene, FrameworkInternal* node);
        /**
         *  Writes the mesh to file.
         *  @param filename the path of the file to write to.
         */
        void Save(const std::string& filename) const;
        /**
         *  Writes the mesh data to stream.
         *  @param ofs the stream to write to.
         */
        void Write(std::ostream& ofs) const;
        /**
         *  Loads a mesh of the frameworks format from file.
         *  @param filename the path of the original file to check if the bin file is still up to date.
         *  @param binFilename the path of the file to read from.
         *  @param node the framework.
         */
        bool Load(const std::string& filename, const std::string& binFilename, FrameworkInternal* node);
        /**
         *  Reads the mesh data from stream.
         *  @param ifs the stream to read from.
         *  @param node the framework.
         */
        bool Read(std::istream& ifs, FrameworkInternal* node);

        void ParseBoneHierarchy(const std::map<std::string, unsigned int>& bones, const aiNode* node,
            std::size_t parent);

        /** Generates AABB for all bones. */
        void GenerateBoneBoundingBoxes();
        /** Flattens all hierarchies. */
        void FlattenHierarchies();

        /** Filename of this mesh. */
        std::string filename_;
        /** Force generating normals. */
        bool forceGenNormals_ = false;

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
        glm::mat4 globalInverse_ = glm::mat4{ 1.0f };
        /** AABB for all bones */
        std::vector<math::AABB3<float>> boneBoundingBoxes_;

        /** Holds the OpenGL index buffer. */
        GLuint indexBuffer_;
    };
}
