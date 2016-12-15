/**
 * @file   SceneMeshNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Implementation of a (sub-) mesh node in a scene.
 */

#include "SceneMeshNode.h"
#include <assimp/scene.h>
#include "SubMesh.h"
#include <core/math/transforms.h>


namespace viscom {

    static void CopyAiMatrixToGLM(const aiMatrix4x4& from, glm::mat4 &to)
    {
        to[0][0] = from.a1; to[1][0] = from.a2;
        to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2;
        to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2;
        to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2;
        to[2][3] = from.d3; to[3][3] = from.d4;
    }

    SceneMeshNode::SceneMeshNode(aiNode* node, const SceneMeshNode* parent, const std::vector<SubMesh>& meshes) :
        nodeName_(node->mName.C_Str()),
        parent_(parent)
    {
        aabb_.minmax[0] = glm::vec3(std::numeric_limits<float>::infinity()); aabb_.minmax[1] = glm::vec3(-std::numeric_limits<float>::infinity());
        CopyAiMatrixToGLM(node->mTransformation, localTransform_);
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) meshes_.push_back(&meshes[i]);
        for (unsigned int i = 0; i < node->mNumChildren; ++i) children_.push_back(std::make_unique<SceneMeshNode>(node->mChildren[i], this, meshes));

        for (const auto& mesh : meshes_) {
            auto meshAABB = math::transformAABB(mesh->GetLocalAABB(), localTransform_);
            aabb_.minmax[0] = glm::min(aabb_.minmax[0], meshAABB.minmax[0]);
            aabb_.minmax[1] = glm::max(aabb_.minmax[1], meshAABB.minmax[1]);
        }

        for (const auto& child : children_) {
            math::AABB3<float> nodeAABB;
            child->GetBoundingBox(nodeAABB, localTransform_);
            aabb_.minmax[0] = glm::min(aabb_.minmax[0], nodeAABB.minmax[0]);
            aabb_.minmax[1] = glm::max(aabb_.minmax[1], nodeAABB.minmax[1]);
        }
    }

    SceneMeshNode::SceneMeshNode(const SceneMeshNode& rhs) :
        nodeName_(rhs.nodeName_),
        meshes_(rhs.meshes_),
        localTransform_(rhs.localTransform_),
        aabb_(rhs.aabb_),
        parent_(rhs.parent_)
    {
        children_.resize(rhs.children_.size());
        for (auto i = 0; i < children_.size(); ++i) children_[i] = std::make_unique<SceneMeshNode>(*rhs.children_[i]);
    }

    SceneMeshNode::SceneMeshNode(SceneMeshNode&& rhs) :
        nodeName_(std::move(rhs.nodeName_)),
        children_(std::move(rhs.children_)),
        meshes_(std::move(rhs.meshes_)),
        localTransform_(std::move(rhs.localTransform_)),
        aabb_(std::move(rhs.aabb_)),
        parent_(std::move(rhs.parent_))
    {

    }

    SceneMeshNode& SceneMeshNode::operator=(const SceneMeshNode& rhs)
    {
        if (this != &rhs) {
            auto tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    SceneMeshNode SceneMeshNode::operator=(SceneMeshNode&& rhs)
    {
        if (this != &rhs) {
            this->~SceneMeshNode();
            nodeName_ = std::move(rhs.nodeName_);
            children_ = std::move(rhs.children_);
            meshes_ = std::move(rhs.meshes_);
            localTransform_ = std::move(rhs.localTransform_);
            aabb_ = std::move(rhs.aabb_);
            parent_ = std::move(rhs.parent_);
        }
        return *this;
    }

    SceneMeshNode::~SceneMeshNode() = default;

    void SceneMeshNode::GetBoundingBox(math::AABB3<float>& aabb, const glm::mat4& transform) const
    {
        aabb = math::transformAABB(aabb_, transform);
    }

}
