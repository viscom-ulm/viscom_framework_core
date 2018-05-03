/**
 * @file   Resource.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Declaration of the base class for all managed resources.
 */

#pragma once

#include "core/main.h"
#include <optional>

namespace viscom {

    class ApplicationNodeInternal;

    class Resource
    {
    public:
        Resource(const std::string& resourceId, ResourceTransferType type, ApplicationNodeInternal* appNode, bool synchronize = false);
        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;
        Resource(Resource&&) noexcept = delete;
        Resource& operator=(Resource&&) noexcept = delete;
        virtual ~Resource();

        const std::string& GetId() const { return id_; }
        bool IsInitialized() const { return initialized_; }
        bool IsLoaded() const { return loaded_; }

        // resource loading:
        // - with valid parameters: non-synced or synced on master
        // - without valid parameters: synced on slave
        // - from memory: synced on slave
        void LoadResource();
        void LoadResource(const void* data, std::size_t size);

        static std::string FindResourceLocation(const std::string& localFilename, const ApplicationNodeInternal* appNode, const std::string& resourceId = "_no_resource_");

    protected:
        const ApplicationNodeInternal* GetAppNode() const { return appNode_; }
        ApplicationNodeInternal* GetAppNode() { return appNode_; }

        std::string FindResourceLocation(const std::string& localFilename) const;

        virtual void Load(std::optional<std::vector<std::uint8_t>>& data) = 0;
        virtual void LoadFromMemory(const void* data, std::size_t size) = 0;
        void InitializeFinished() { initialized_ = true; }

    private:
        /** Holds the resources id. */
        const std::string id_;
        /** Holds the resources type. */
        const ResourceTransferType type_;
        /** Is this resource synchronized. */
        const bool synchronized_;

        /** Is the resource initialized (i.e. has the 'Initialize' method been called). */
        bool initialized_ = false;
        /** is the resource loaded (i.e. has the 'Load' method been called). */
        bool loaded_ = false;

        /** Holds the application object for dependencies. */
        ApplicationNodeInternal* appNode_;
    };

    template<typename RType>
    struct NetworkSharePolicy {
        void LoadResource(RType& resource);
        void ReloadResource(RType& resource);
        void DeleteResource(RType& resource);
        // if on master:
        // - load resource locally
        // - transfer memory representation to slaves

        // if on slave:
        // - initialize id
        // - add id to list of uninitialized resources

        // if new resource comes in .. create...
    };
}
