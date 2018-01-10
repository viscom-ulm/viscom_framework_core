/**
 * @file   SubMesh.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.15
 *
 * @brief  Declaration of a sub mesh helper class.
 */

#pragma once

#include "core/main.h"
#include "core/math/primitives.h"
#include "core/utils/serializationHelper.h"

namespace viscom {

    struct Material;
    class Mesh;

    /**
     * A SubMesh is a sub group of geometry in a mesh. It does not have its own
     * vertex information but uses indices to define which vertices of the mesh are used.
     */
    class SubMesh
    {
    public:
        SubMesh() noexcept : indexOffset_{ 0 }, numIndices_{ 0 }, materialIndex_{ 0 } { aabb_.minmax_[0] = glm::vec3(std::numeric_limits<float>::infinity()); aabb_.minmax_[1] = glm::vec3(-std::numeric_limits<float>::infinity()); }
        SubMesh(const Mesh* mesh, const std::string& objectName, unsigned int indexOffset, unsigned int numIndices, std::size_t materialIndex);
        SubMesh(const SubMesh&);
        SubMesh& operator=(const SubMesh&);
        SubMesh(SubMesh&&) noexcept;
        SubMesh& operator=(SubMesh&&) noexcept;
        virtual ~SubMesh() noexcept;

        const std::string& GetName() const noexcept { return objectName_; }
        unsigned int GetIndexOffset() const noexcept { return indexOffset_; }
        unsigned int GetNumberOfIndices() const noexcept { return numIndices_; }
        unsigned int GetNumberOfTriangles() const noexcept { return numIndices_ / 3; }
        const math::AABB3<float>& GetLocalAABB() const noexcept { return aabb_; }
        std::size_t GetMaterialIndex() const noexcept { return materialIndex_; }

        void Write(std::ostream& ofs) const;
        bool Read(std::istream& ifs);

    private:
        using VersionableSerializerType = serializeHelper::VersionableSerializer<'V', 'S', 'B', 'M', 1000>;

        /** Holds the sub-meshes object name. */
        std::string objectName_;
        /** The index offset the sub-mesh starts. */
        unsigned int indexOffset_;
        /** The number of indices in the sub-mesh. */
        unsigned int numIndices_;
        /** The sub-meshes local AABB. */
        math::AABB3<float> aabb_;
        /** Index of sub-meshes material. */
        std::size_t materialIndex_;
    };
}
