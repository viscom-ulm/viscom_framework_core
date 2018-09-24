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

    class FrameworkInternal;
    struct FWConfiguration;

    /** Base class for all resources. */
    class Resource
    {
    public:
        Resource(const std::string& resourceId, ResourceType type, FrameworkInternal* appNode, bool synchronize = false);
        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;
        Resource(Resource&&) noexcept = delete;
        Resource& operator=(Resource&&) noexcept = delete;
        virtual ~Resource();

        /** Returns the resources id. */
        const std::string& GetId() const { return id_; }
        /** Returns the resources type. */
        ResourceType GetType() const { return type_; }
        /** Checks if the resource has been initialized. */
        bool IsInitialized() const { return initialized_; }
        /** Checks if the resource has been loaded. */
        bool IsLoaded() const { return loadCounter_ == -1; }
        /** Returns the resource load counter. */
        int GetLoadCounter() const { return loadCounter_; }
        /** Increases the resource load counter by one. */
        void IncreaseLoadCounter() { loadCounter_ += 1; }
        /** Resets the resource load counter to zero. */
        void ResetLoadCounter() { loadCounter_ = 0; }
        /** Return the data of the resource memory representation. */
        const std::vector<std::uint8_t>& GetData() const { return data_; }

        /** Manages the resource data loading from file for shared and local resources handling both coordinator and worker nodes. */
        void LoadResource();
        /** Loads the resource data from memory. */
        void LoadResource(const void* data, std::size_t size);

        /**
         *  Returns the location of the file with the specified file name.
         *  @param localFilename the file name to search for.
         *  @param config the framework configuration holding the search paths.
         *  @param resourceId the resource id.
         */
        static std::string FindResourceLocation(const std::string& localFilename, const FWConfiguration* config, const std::string& resourceId = "_no_resource_");
        /**
         *  Returns the location of the file with the specified file name.
         *  @param localFilename the file name to search for.
         *  @param appNode the application node holding the framework configuration to retrieve the search paths.
         *  @param resourceId the resource id.
         */
        static std::string FindResourceLocation(const std::string& localFilename, const FrameworkInternal* appNode, const std::string& resourceId = "_no_resource_");

    protected:
        /** Returns the application object. */
        const FrameworkInternal* GetAppNode() const { return appNode_; }
        /** Returns the application object. */
        FrameworkInternal* GetAppNode() { return appNode_; }

        /**
         *  Returns the location of the file with the specified file name.
         *  @param localFilename the file name to search for.
         */
        std::string FindResourceLocation(const std::string& localFilename) const;

        /**
         *  Loads the resource data from file.
         *  @param data vector for optional data.
         */
        virtual void Load(std::optional<std::vector<std::uint8_t>>& data) = 0;
        /**
         *  Loads the resource data from memory.
         *  @param data pointer to the resource data.
         *  @param size size of the resource data.
         */
        virtual void LoadFromMemory(const void* data, std::size_t size) = 0;
        /** Initializes the resource by setting the initialized flag. */
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
        int loadCounter_ = 0;
        /** In a synchronized resource on the master node the resources memory representation is stored here. */
        std::vector<std::uint8_t> data_;

        /** Holds the application object for dependencies. */
        FrameworkInternal* appNode_;
    };
}
