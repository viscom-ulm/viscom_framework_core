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
    SubMesh::SubMesh(const Mesh* mesh, const std::string& objectName, unsigned int indexOffset, unsigned int numIndices, std::size_t materialIndex) :
        objectName_(objectName),
        indexOffset_(indexOffset),
        numIndices_(numIndices),
        materialIndex_(materialIndex)
    {
        aabb_.minmax_[0] = glm::vec3(std::numeric_limits<float>::infinity()); aabb_.minmax_[1] = glm::vec3(-std::numeric_limits<float>::infinity());
        if (numIndices_ == 0) return;
        auto& vertices = mesh->GetVertices();
        auto& indices = mesh->GetIndices();
        aabb_.minmax_[0] = aabb_.minmax_[1] = vertices[indices[indexOffset_]].xyz(); //-V108
        for (auto i = indexOffset_; i < indexOffset_ + numIndices_; ++i) {
            aabb_.minmax_[0] = glm::min(aabb_.minmax_[0], vertices[indices[i]].xyz()); //-V108
            aabb_.minmax_[1] = glm::max(aabb_.minmax_[1], vertices[indices[i]].xyz()); //-V108
        }
    }

    /** Default destructor. */
    SubMesh::~SubMesh() noexcept = default;

    /** Default copy constructor. */
    SubMesh::SubMesh(const SubMesh&) = default;
    /** Default copy assignment operator. */
    SubMesh& SubMesh::operator=(const SubMesh&) = default;

    /** Default move constructor. */
    SubMesh::SubMesh(SubMesh&& rhs) noexcept = default;

    /** Default move assignment operator. */
    SubMesh& SubMesh::operator=(SubMesh&& rhs) noexcept = default;

    void SubMesh::Write(std::ostream& ofs) const
    {
        VersionableSerializerType::writeHeader(ofs);
        serializeHelper::write(ofs, objectName_);
        serializeHelper::write(ofs, indexOffset_);
        serializeHelper::write(ofs, numIndices_);
        serializeHelper::write(ofs, aabb_);
        serializeHelper::write(ofs, materialIndex_);
    }

    bool SubMesh::Read(std::istream& ifs)
    {
        bool correctHeader;
        unsigned int actualVersion;
        std::tie(correctHeader, actualVersion) = VersionableSerializerType::checkHeader(ifs);
        if (correctHeader) {
            serializeHelper::read(ifs, objectName_);
            serializeHelper::read(ifs, indexOffset_);
            serializeHelper::read(ifs, numIndices_);
            serializeHelper::read(ifs, aabb_.minmax_[0]);
            serializeHelper::read(ifs, aabb_.minmax_[1]);
            serializeHelper::read(ifs, materialIndex_);
            return true;
        }
        return false;
    }

}
