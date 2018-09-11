/**
 * @file   Resource.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Implementation of the resource base class.
 */

#include "Resource.h"
#include "core/FrameworkInternal.h"
#include "core/utils/utils.h"

#ifdef VISCOM_USE_SGCT
#include <sgct.h>
#endif

namespace viscom {
    /**
     * Constructor.
     * @param resourceId the resource id to use
     */
    Resource::Resource(const std::string& resourceId, ResourceType type, FrameworkInternal* appNode, bool synchronize) :
        id_{ resourceId },
        type_{ type },
        appNode_{ appNode },
        synchronized_{ synchronize }
    {
    }

    Resource::~Resource()
    {
        if (synchronized_) appNode_->TransferReleaseResource(id_, type_);
    }

    void Resource::LoadResource()
    {
        if (synchronized_) {
            if (appNode_->IsMaster()) {
                std::optional<std::vector<std::uint8_t>> optData(std::vector<std::uint8_t>{});
                Load(optData);
                data_.swap(*optData);

                appNode_->TransferResource(id_, data_.data(), data_.size(), type_);
                loadCounter_ = -1;
            }
            else if (!IsLoaded()) appNode_->WaitForResource(id_, type_);
        }
        else {
            std::optional<std::vector<std::uint8_t>> optData(std::vector<std::uint8_t>{});
            Load(optData);
            loadCounter_ = -1;
        }
    }

    void Resource::LoadResource(const void* data, std::size_t size)
    {
        LoadFromMemory(data, size);
        loadCounter_ = -1;
    }

    std::string Resource::FindResourceLocation(const std::string& localFilename, const FrameworkInternal* appNode, const std::string& resourceId)
    {
        for (const auto& dir : appNode->GetConfig().resourceSearchPaths_) {
            auto filename = dir + "/" + localFilename;
            if (dir.empty()) filename = localFilename;
            if (utils::file_exists(filename)) return filename;
        }

        LOG(WARNING) << "Cannot find local resource file \"" << localFilename.c_str() << "\".";
        throw resource_loading_error(resourceId, "Cannot find local resource file (" + localFilename + ").");
    }

    std::string Resource::FindResourceLocation(const std::string & localFilename) const
    {
        return FindResourceLocation(localFilename, appNode_, id_);
    }
}
