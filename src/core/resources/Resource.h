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
        Resource(const std::string& resourceId, ResourceType type, ApplicationNodeInternal* appNode, bool synchronize = false);
        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;
        Resource(Resource&&) noexcept = delete;
        Resource& operator=(Resource&&) noexcept = delete;
        virtual ~Resource();

        const std::string& GetId() const { return id_; }
        ResourceType GetType() const { return type_; }
        bool IsInitialized() const { return initialized_; }
        bool IsLoaded() const { return loaded_; }
        const std::vector<std::uint8_t>& GetData() const { return data_; }

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
        const ResourceType type_;
        /** Is this resource synchronized. */
        const bool synchronized_;

        /** Is the resource initialized (i.e. has the 'Initialize' method been called). */
        bool initialized_ = false;
        /** is the resource loaded (i.e. has the 'Load' method been called). */
        bool loaded_ = false;
        /** In a synchronized resource on the master node the resources memory representation is stored here. */
        std::vector<std::uint8_t> data_;

        /** Holds the application object for dependencies. */
        ApplicationNodeInternal* appNode_;
    };
}
