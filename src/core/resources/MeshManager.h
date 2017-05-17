/**
 * @file   MeshManager.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Declaration of the texture manager.
 */

#pragma once

#include "ResourceManager.h"
#include "core/gfx/mesh/Mesh.h"

namespace viscom {

    class MeshManager final : public ResourceManager<Mesh>
    {
    public:
        explicit MeshManager(ApplicationNode* node);
        MeshManager(const MeshManager&);
        MeshManager& operator=(const MeshManager&);
        MeshManager(MeshManager&&) noexcept;
        MeshManager& operator=(MeshManager&&) noexcept;
        virtual ~MeshManager() override;
    };
}
