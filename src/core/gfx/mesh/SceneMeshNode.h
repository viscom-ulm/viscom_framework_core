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

    class SceneMeshNode
    {
    public:
        SceneMeshNode() noexcept;
        SceneMeshNode(aiNode* node, const SceneMeshNode* parent, const std::map<std::string, unsigned int>& boneMap);
        SceneMeshNode(const SceneMeshNode& rhs);
        SceneMeshNode& operator=(const SceneMeshNode& rhs);
        SceneMeshNode(SceneMeshNode&& rhs) noexcept;
        SceneMeshNode operator=(SceneMeshNode&& rhs) noexcept;
        ~SceneMeshNode() noexcept;

        glm::mat4 GetLocalTransform() const noexcept { return localTransform_; }
        std::size_t GetNumberOfNodes() const noexcept { return children_.size(); }
        const SceneMeshNode* GetChild(std::size_t index) const noexcept { return children_[index].get(); }
        std::size_t GetNumberOfSubMeshes() const noexcept { return subMeshIds_.size(); }
        std::size_t GetSubMeshID(std::size_t index) const noexcept { return subMeshIds_[index]; }
        const SceneMeshNode* GetParent() const noexcept { return parent_; }
        const std::string& GetName() const noexcept { return nodeName_; }
        int GetBoneIndex() const noexcept { return boneIndex_; }
        unsigned int GetNodeIndex() const noexcept { return nodeIndex_; }
        const math::AABB3<float>& GetBoundingBox() const noexcept { return aabb_; }
        const std::vector<math::AABB3<float>>& GetSubMeshBoundingBoxes() const { return subMeshBoundingBoxes_; }
        bool IsBoundingBoxValid() const noexcept { return boundingBoxValid_; }

        void FlattenNodeTree(std::vector<const SceneMeshNode*>& nodes);
        bool GenerateBoundingBoxes(const Mesh& mesh);

        [[deprecated("Do we even need this anymore?")]]
        void GetBoundingBox(math::AABB3<float>& aabb, const glm::mat4& transform) const;

        void Write(std::ostream& ofs);
        bool Read(std::istream& ifs, std::unordered_map<std::uint64_t, SceneMeshNode*>& nodes);

    private:
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
