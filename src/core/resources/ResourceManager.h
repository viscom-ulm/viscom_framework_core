/**
 * @file   ResourceManager.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Contains the base class for all resource managers.
 */

#pragma once

#include "core/main.h"
#include <unordered_map>
#include <mutex>
#include <optional>

#include "core/gfx/Texture.h"

namespace viscom {

    class FrameworkInternal;

    struct resource_loading_error
    {
        resource_loading_error(const std::string& resId, const std::string& errorDesc) : resId_{resId}, errorDescription_{errorDesc} {}
        /** Holds the resource id. */
        std::string resId_;
        /** Holds the error description. */
        std::string errorDescription_;
    };

    class BaseResourceManager
    {
    public:
        /** Constructor for resource managers. */
        explicit BaseResourceManager(FrameworkInternal* node) : appNode_{ node } {}
        /** Default destructor. */
        virtual ~BaseResourceManager() = default;

    protected:
        void RequestSharedResource(std::string_view name, ResourceType type);
        void TransferResourceToNode(std::string_view name, const void* data, std::size_t length, ResourceType type, std::size_t nodeIndex);

        /** Holds the application base. */
        FrameworkInternal* appNode_;
    };

    /**
     * @brief  Base class for all resource managers.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.03
     */
    template<typename rType>
    class ResourceManager : public BaseResourceManager
    {
    protected:
        /** The resource managers resource type. */
        using ResourceType = rType;
        /** The resource map type. */
        using ResourceMap = std::unordered_map<std::string, std::weak_ptr<rType>>;
        /** The synchronized resource map type. */
        using SyncedResourceMap = std::unordered_map<std::string, std::shared_ptr<rType>>;
        /** The type of this base class. */
        using ResourceManagerBase = ResourceManager<rType>;

    public:
        /** Constructor for resource managers. */
        explicit ResourceManager(FrameworkInternal* node) : BaseResourceManager{ node } {}

        /** Copy constructor. */
        ResourceManager(const ResourceManager& rhs) : BaseResourceManager{ rhs }
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
        ResourceManager(ResourceManager&& rhs) noexcept : BaseResourceManager{ std::move(rhs) }, resources_(std::move(rhs.resources_)) {}
        /** Default move assignment operator. */
        ResourceManager& operator=(ResourceManager&& rhs) noexcept
        {
            BaseResourceManager::operator=(std::move(rhs));
            resources_ = std::move(rhs.resources_);
            return *this;
        }

        /**
         * Gets a resource from the manager.
         * @param resId the resources id
         * @return the resource as a shared pointer
         */
        template<typename... Args>
        std::shared_ptr<ResourceType> GetResource(const std::string& resId, Args&&... args)
        {
            std::lock_guard<std::mutex> accessLock{ mtx_ };
            auto resPtr = GetResourceInternal(resId, false, std::forward<Args>(args)...);
            resPtr->LoadResource();
            return resPtr;
        }


        /**
         * Gets a synchronized resource from the manager.
         * @param resId the resources id
         * @return the resource as a shared pointer
         */
        template<typename... Args>
        std::shared_ptr<ResourceType> GetSynchronizedResource(const std::string& resId, Args&&... args)
        {
            if constexpr (!USE_SGCT) return GetResource(resId, std::forward<Args>(args)...);

            std::lock_guard<std::mutex> syncAccessLock{ syncMtx_ };
            std::lock_guard<std::mutex> accessLock{ mtx_ };

            auto rit = syncedResources_.find(resId);
            if (rit != syncedResources_.end()) {
                if (!rit->second->IsInitialized()) rit->second->Initialize(std::forward<Args>(args)...);
                return rit->second;
            }
            else {
                auto resPtr = GetResourceInternal(resId, true, std::forward<Args>(args)...);
                syncedResources_[resId] = resPtr;
                resPtr->LoadResource();
                return resPtr;
            }
        }


        /**
         * Checks if the resource manager contains this resource.
         * @param resId the resources id
         * @return whether the manager contains the resource or not.
         */
        bool HasResource(const std::string& resId) const
        {
            std::lock_guard<std::mutex> accessLock{ mtx_ };
            auto rit = resources_.find(resId);
            return (rit != resources_.end()) && !rit->second.expired();
        }

        /** Releases a shared resource (it is not deleted if the shared pointers to it are used elsewhere). */
        void ReleaseSharedResource(const std::string& resId)
        {
            std::lock_guard<std::mutex> accessLock{ syncMtx_ };
            auto rit = syncedResources_.find(resId);
            if (rit != syncedResources_.end()) rit = syncedResources_.erase(rit);
        }

        template<typename... Args>
        void CreateSharedResource(const std::string& resId, const void* data, std::size_t size, Args&&... args)
        {
            std::lock_guard<std::mutex> accessLock{ mtx_ };

            auto rit = syncedResources_.find(resId);
            if (rit != syncedResources_.end()) {
                rit->second->LoadResource(data, size);
            }
            else {
                auto res = GetResourceInternal(resId, true, std::forward<Args>(args)...);
                res->LoadResource(data, size);
                syncedResources_[resId] = res;
            }
        }

        /**
         *  Adds the resource with the specified id to the list of resources to wait for.
         *  @param resId the resource id.
         */
        void WaitForResource(const std::string& resId)
        {
            waitedResources_.push_back(syncedResources_[resId]);
        }

        bool ProcessResourceWaitList()
        {
            if (waitedResources_.empty()) return false;
            std::vector<std::shared_ptr<ResourceType>> newWaitedResources;
            for (auto& waitedResource : waitedResources_) if (!waitedResource->IsLoaded()) {
                waitedResource->IncreaseLoadCounter();
                if (waitedResource->GetLoadCounter() > 10) RequestSharedResource(waitedResource->GetId(), waitedResource->GetType());
                newWaitedResources.emplace_back(std::move(waitedResource));
            }

            waitedResources_ = std::move(newWaitedResources);
            return !waitedResources_.empty();
        }

        void SynchronizeAllResourcesToNode(int clientID)
        {
            std::lock_guard<std::mutex> syncAccessLock{ syncMtx_ };

            for (const auto& res : syncedResources_) {
                TransferResourceToNode(res.first, res.second->GetData().data(), res.second->GetData().size(), res.second->GetType(), clientID);
            }
        }

        void SynchronizeResourceToNode(const std::string& resId, int clientID)
        {
            std::lock_guard<std::mutex> syncAccessLock{ syncMtx_ };

            auto rit = syncedResources_.find(resId);
            if (rit != syncedResources_.end()) {
                TransferResourceToNode(rit->first, rit->second->GetData().data(), rit->second->GetData().size(), rit->second->GetType(), clientID);
            }
            else LOG(WARNING) << "Requested resource does not exist: " << resId;
        }

    protected:
        template<typename... Args>
        std::shared_ptr<ResourceType> GetResourceInternal(const std::string& resId, bool synchronized, Args&&... args)
        {
            std::weak_ptr<ResourceType> wpResource;
            try {
                wpResource = resources_.at(resId);
            }
            catch (std::out_of_range&) {
                LOG(INFO) << "No resource with id \"" << resId << "\" found. Creating new one.";
            }
            if (wpResource.expired()) {
                std::shared_ptr<ResourceType> spResource(nullptr);
                LoadResource(resId, synchronized, spResource, std::forward<Args>(args)...);
                wpResource = spResource;
                resources_.insert(std::move(std::make_pair(resId, wpResource)));
                return std::move(spResource);
            }
            return wpResource.lock();
        }

        template<typename... Args>
        void LoadResource(const std::string& resId, bool synchronized, std::shared_ptr<ResourceType>& spResource, Args&&... args)
        {
            try {
                spResource = std::make_shared<rType>(resId, appNode_, synchronized);
                spResource->Initialize(std::forward<Args>(args)...);
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
        /** Holds a map of synchronized resources (to make sure they are not deleted). */
        SyncedResourceMap syncedResources_;
        /** Holds a list of resources to wait for. */
        std::vector<std::shared_ptr<rType>> waitedResources_;
        /** Holds a mutex to the resources. */
        mutable std::mutex mtx_;
        /** Holds a mutex to the synced resources. */
        std::mutex syncMtx_;
    };
}
