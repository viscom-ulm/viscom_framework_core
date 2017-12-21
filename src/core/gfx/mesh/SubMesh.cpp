/**
 * @file   SubMesh.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Implementation of the sub mesh class.
 */

#define GLM_SWIZZLE
#include "SubMesh.h"
#include "Mesh.h"

#undef min
#undef max

namespace viscom {

    /** Constructor. */
    SubMesh::SubMesh(const Mesh* mesh, const std::string& objectName, unsigned int indexOffset, unsigned int numIndices, Material* material) :
        objectName_(objectName),
        indexOffset_(indexOffset),
        numIndices_(numIndices),
        material_(material)
    {
        aabb_.minmax[0] = glm::vec3(std::numeric_limits<float>::infinity()); aabb_.minmax[1] = glm::vec3(-std::numeric_limits<float>::infinity());
        if (numIndices_ == 0) return;
        auto& vertices = mesh->GetVertices();
        auto& indices = mesh->GetIndices();
        aabb_.minmax[0] = aabb_.minmax[1] = vertices[indices[indexOffset_]].xyz(); //-V108
        for (auto i = indexOffset_; i < indexOffset_ + numIndices_; ++i) {
            aabb_.minmax[0] = glm::min(aabb_.minmax[0], vertices[indices[i]].xyz()); //-V108
            aabb_.minmax[1] = glm::max(aabb_.minmax[1], vertices[indices[i]].xyz()); //-V108
        }
    }

    /** Default destructor. */
    SubMesh::~SubMesh() = default;
    /** Default copy constructor. */
    SubMesh::SubMesh(const SubMesh&) = default;
    /** Default copy assignment operator. */
    SubMesh& SubMesh::operator=(const SubMesh&) = default;

    /** Default move constructor. */
    SubMesh::SubMesh(SubMesh&& rhs) noexcept :
        objectName_(std::move(rhs.objectName_)),
        indexOffset_(std::move(rhs.indexOffset_)),
        numIndices_(std::move(rhs.numIndices_)),
        aabb_(std::move(rhs.aabb_)),
        material_(std::move(rhs.material_))
    {
    }

    /** Default move assignment operator. */
    SubMesh& SubMesh::operator=(SubMesh&& rhs) noexcept
    {
        if (this != &rhs) {
            this->~SubMesh();
            objectName_ = std::move(rhs.objectName_);
            indexOffset_ = std::move(rhs.indexOffset_);
            numIndices_ = std::move(rhs.numIndices_);
            aabb_ = std::move(rhs.aabb_);
            material_ = std::move(rhs.material_);
        }
        return *this;
    }
}
