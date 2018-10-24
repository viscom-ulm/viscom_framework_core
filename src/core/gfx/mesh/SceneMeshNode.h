/**
 * @file   SceneMeshNode.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Declaration of a (sub-) mesh node in a scene.
 */

#pragma once


#include "core/main.h"
#include "core/utils/serializationHelper.h"
#include <core/math/math.h>
#include <unordered_map>

struct aiNode;

namespace viscom {

    class SubMesh;
    class Mesh;

    /** The class representing a node in the scene and holding the local meshes. */
    class SceneMeshNode
    {
    public:
        SceneMeshNode() noexcept;
        /**
         *  Constructor method.
         *  @param node the ASSIMP node holding the node information.
         *  @param parent the parent node.
         *  @param boneMap bones of the mesh to retrieve the bone index from.
         */
        SceneMeshNode(aiNode* node, const SceneMeshNode* parent, const std::map<std::string, unsigned int>& boneMap);
        /**
         *  Constructor method.
         *  @param rhs SceneMeshNode to copy all information from.
         */
        SceneMeshNode(const SceneMeshNode& rhs);
        /**
         *  Constructor method.
         *  @param rhs SceneMeshNode to copy all information from.
         */
        SceneMeshNode& operator=(const SceneMeshNode& rhs);
        /**
         *  Constructor method.
         *  @param rhs SceneMeshNode to copy all information from.
         */
        SceneMeshNode(SceneMeshNode&& rhs) noexcept;
        /**
         *  Constructor method.
         *  @param rhs SceneMeshNode to copy all information from.
         */
        SceneMeshNode operator=(SceneMeshNode&& rhs) noexcept;
        ~SceneMeshNode() noexcept;

        /** Returns the local transformation matrix. */
        glm::mat4 GetLocalTransform() const noexcept { return localTransform_; }
        /** Returns the number of children nodes. */
        std::size_t GetNumberOfNodes() const noexcept { return children_.size(); }
        /**
         *  Returns a child node.
         *  @param index the index of the child node.
         */
        const SceneMeshNode* GetChild(std::size_t index) const noexcept { return children_[index].get(); }
        /** Returns the number of sub meshes of the node. */
        std::size_t GetNumberOfSubMeshes() const noexcept { return subMeshIds_.size(); }
        /**
         *  Returns a sub-mesh.
         *  @param index the index of the sub mesh.
         */
        std::size_t GetSubMeshID(std::size_t index) const noexcept { return subMeshIds_[index]; }
        /** Returns the nodes parent. */
        const SceneMeshNode* GetParent() const noexcept { return parent_; }
        /** Returns the node name. */
        const std::string& GetName() const noexcept { return nodeName_; }
        /** Returns the bone index. */
        int GetBoneIndex() const noexcept { return boneIndex_; }
        /** Returns the node index. */
        unsigned int GetNodeIndex() const noexcept { return nodeIndex_; }
        /** Returns the nodes local AABB. */
        const math::AABB3<float>& GetBoundingBox() const noexcept { return aabb_; }
        /** Returns AABB for all sub meshes of the node. */
        const std::vector<math::AABB3<float>>& GetSubMeshBoundingBoxes() const { return subMeshBoundingBoxes_; }
        /** Returns if the AABB is valid. */
        bool IsBoundingBoxValid() const noexcept { return boundingBoxValid_; }

        /**
         *  Flattens the tree of the node and its child nodes to a vector.
         *  @param nodes list of nodes that will contain the flattened tree.
         */
        void FlattenNodeTree(std::vector<const SceneMeshNode*>& nodes);
        /**
         *  Generates the AABB for the mesh and all sub meshes.
         *  @param mesh the mesh to generate the bounding box to.
         */
        bool GenerateBoundingBoxes(const Mesh& mesh);

        /**
         *  Transforms a AABB with a 4x4 matrix.
         *  @param aabb the bounding box to be transformed.
         *  @param transform the 4x4 transformation matrix.
         */
        [[deprecated("Do we even need this anymore?")]]
        void GetBoundingBox(math::AABB3<float>& aabb, const glm::mat4& transform) const;

        /**
         *  Writes the node and all its children to a stream.
         *  @param ofs stream to write to.
         */
        void Write(std::ostream& ofs);
        /**
         *  Reads the node and all its children from a stream.
         *  @param ifs stream to read from.
         *  @param nodes the node that will be the parent to the read node.
         */
        bool Read(std::istream& ifs, std::unordered_map<std::uint64_t, SceneMeshNode*>& nodes);

    private:
        /** Defines the type of the VersionableSerializer for the SceneMeshNode class. */
        using VersionableSerializerType = serializeHelper::VersionableSerializer<'V', 'S', 'M', 'N', 1000>;

        /** The nodes name. */
        std::string nodeName_;
        /** The nodes children. */
        std::vector<std::unique_ptr<SceneMeshNode>> children_;
        /** Meshes in this node. */
        std::vector<std::size_t> subMeshIds_;
        /** The local transformation matrix. */
        glm::mat4 localTransform_;
        /** The nodes parent. */
        const SceneMeshNode* parent_;
        /** Bone index. */
        int boneIndex_;
        /** Node index. */
        unsigned int nodeIndex_;
        /** The nodes local AABB. */
        math::AABB3<float> aabb_;
        /** Bounding boxes for this nodes sub meshes. */
        std::vector<math::AABB3<float>> subMeshBoundingBoxes_;
        /** Flag if the bounding box is valid. */
        bool boundingBoxValid_ = false;
    };
}
