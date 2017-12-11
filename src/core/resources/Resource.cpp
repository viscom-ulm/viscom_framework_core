/**
 * @file   Resource.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Implementation of the resource base class.
 */

#include "Resource.h"
#include "core/ApplicationNodeInternal.h"
#include "core/utils.h"

namespace viscom {
    /**
     * Constructor.
     * @param resourceId the resource id to use
     */
    Resource::Resource(const std::string& resourceId, ApplicationNodeInternal* appNode) :
        id_{ resourceId },
        appNode_{ appNode }
    {
    }

    /** Default copy constructor. */
    Resource::Resource(const Resource&) = default;

    /** Default assignment operator. */
    Resource& Resource::operator=(const Resource&) = default;

    /** Move constructor. */
    Resource::Resource(Resource&& orig) noexcept :
        id_{ std::move(orig.id_) },
        appNode_{ orig.appNode_ }
    {
    }

    /** Move assignment operator. */
    Resource& Resource::operator=(Resource&& orig) noexcept
    {
        if (this != &orig) {
            this->~Resource();
            id_ = std::move(orig.id_);
            appNode_ = orig.appNode_;
        }
        return *this;
    }

    Resource::~Resource() = default;

    std::string Resource::FindResourceLocation(const std::string& localFilename, const ApplicationNodeInternal* appNode, const std::string& resourceId)
    {
        if (utils::file_exists(localFilename)) return localFilename;
        for (const auto& dir : appNode->GetConfig().resourceSearchPaths_) {
            auto filename = dir + "/" + localFilename;
            if (utils::file_exists(filename)) return filename;
        }

        LOG(WARNING) << "Cannot find local resource file \"" << localFilename.c_str() << "\".";
        throw resource_loading_error(resourceId, "Cannot find local resource file (" + localFilename + ").");
    }

    bool Resource::IsResource(const std::string& localFilename, const ApplicationNodeInternal* appNode)
    {
        if (utils::file_exists(localFilename)) return true;
        for (const auto& dir : appNode->GetConfig().resourceSearchPaths_) {
            const auto filename = dir + "/" + localFilename;
            if (utils::file_exists(filename)) return true;
        }
        return false;
    }

    std::string Resource::FindResourceLocation(const std::string & localFilename) const
    {
        return FindResourceLocation(localFilename, appNode_, id_);
    }
}
