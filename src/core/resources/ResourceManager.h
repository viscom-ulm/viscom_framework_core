/**
 * @file   ResourceManager.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Contains the base class for all resource managers.
 */

#pragma once

#include "main.h"
#include <unordered_map>

namespace viscom {
    class ApplicationNodeInternal;

    struct resource_loading_error
    {
        resource_loading_error(const std::string& resId, const std::string& errorDesc) : resId_{resId}, errorDescription_{errorDesc} {}
        /** Holds the resource id. */
        std::string resId_;
        /** Holds the error description. */
        std::string errorDescription_;
    };

    /**
     * @brief  Base class for all resource managers.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.03
     */
    template<typename rType>
    class ResourceManager
    {
    protected:
        /** The resource managers resource type. */
        using ResourceType = rType;
        /** The resource map type. */
        using ResourceMap = std::unordered_map<std::string, std::weak_ptr<rType>>;
        /** The type of this base class. */
        using ResourceManagerBase = ResourceManager<rType>;

    public:
        /** Constructor for resource managers. */
        explicit ResourceManager(ApplicationNodeInternal* node) : appNode_{ node } {}

        /** Copy constructor. */
        ResourceManager(const ResourceManager& rhs) : ResourceManager(rhs.appNode_)
        {
            for (const auto& res : rhs.resources_) {
                resources_.emplace(res.first, std::weak_ptr<ResourceType>());
            }
        }

        /** Copy assignment operator. */
        ResourceManager& operator=(const ResourceManager& rhs)
        {
            ResourceManager tmp{ rhs };
            std::swap(*this, tmp);
            return *this;
        }

        /** Default move constructor. */
        ResourceManager(ResourceManager&& rhs) noexcept : resources_(std::move(rhs.resources_)), appNode_(rhs.appNode_) {}
        /** Default move assignment operator. */
        ResourceManager& operator=(ResourceManager&& rhs) noexcept
        {
            if (this != &rhs) {
                resources_ = std::move(rhs.resources_);
                appNode_ = rhs.appNode_;
            }
            return *this;
        }

        /** Default destructor. */
        virtual ~ResourceManager() = default;

        /**
         * Gets a resource from the manager.
         * @param resId the resources id
         * @return the resource as a shared pointer
         */
        template<typename... Args>
        std::shared_ptr<ResourceType> GetResource(const std::string& resId, Args&&... args)
        {
            std::weak_ptr<ResourceType> wpResource;
            try {
                wpResource = resources_.at(resId);
            }
            catch (std::out_of_range e) {
                LOG(INFO) << "No resource with id \"" << resId << "\" found. Creating new one.";
            }
            if (wpResource.expired()) {
                std::shared_ptr<ResourceType> spResource(nullptr);
                LoadResource(resId, spResource, std::forward<Args>(args)...);
                wpResource = spResource;
                resources_.insert(std::move(std::make_pair(resId, wpResource)));
                return std::move(spResource);
            }
            return wpResource.lock();
        }

        /**
         * Checks if the resource manager contains this resource.
         * @param resId the resources id
         * @return whether the manager contains the resource or not.
         */
        bool HasResource(const std::string& resId) const
        {
            auto rit = resources_.find(resId);
            return (rit != resources_.end()) && !rit->expired();
        }


    protected:
        template<typename... Args>
        void LoadResource(const std::string& resId, std::shared_ptr<ResourceType>& spResource, Args&&... args)
        {
            try {
                spResource = std::make_shared<rType>(resId, appNode_, std::forward<Args>(args)...);
            }
            catch (const resource_loading_error& loadingError) {
                LOG(INFO) << "Error while loading resource \"" << resId << "\"." << std::endl
                    << "Description: " << loadingError.errorDescription_;
                throw;
            }
        }

        /**
         *  Sets the resource with a given name to a new value.
         *  @param resourceName the name of the resource.
         *  @param resource the new resource.
         *  @return a pointer to the new resource.
         */
        std::shared_ptr<ResourceType> SetResource(const std::string& resourceName, std::shared_ptr<ResourceType>&& resource)
        {
            resources_[resourceName] = std::move(resource);
            return resources_[resourceName].lock();
        }

        /** Holds the resources managed. */
        ResourceMap resources_;
        /** Holds the application base. */
        ApplicationNodeInternal* appNode_;
    };
}
