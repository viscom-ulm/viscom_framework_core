/**
 * @file   Resource.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Declaration of the base class for all managed resources.
 */

#pragma once

#include "core/main.h"

namespace viscom {

    class ApplicationNodeInternal;

    class Resource
    {
    public:
        Resource(const std::string& resourceId, ResourceTransferType type, ApplicationNodeInternal* appNode, bool synchronize = false);
        Resource(const Resource&);
        Resource& operator=(const Resource&);
        Resource(Resource&&) noexcept;
        Resource& operator=(Resource&&) noexcept;
        virtual ~Resource();

        const std::string& GetId() const { return id_; }
        virtual void Load() = 0;
        virtual void Reload() = 0;
        virtual void Unload() = 0;

        static std::string FindResourceLocation(const std::string& localFilename, const ApplicationNodeInternal* appNode, const std::string& resourceId = "_no_resource_");

    protected:
        const ApplicationNodeInternal* GetAppNode() const { return appNode_; }
        ApplicationNodeInternal* GetAppNode() { return appNode_; }
        std::string FindResourceLocation(const std::string& localFilename) const;

    private:
        /** Holds the resources id. */
        std::string id_;
        /** Holds the resources type. */
        ResourceTransferType type_;
        /** Holds the application object for dependencies. */
        ApplicationNodeInternal* appNode_;
        /** Is this resource synchronized. */
        bool synchronized_;
    };
}
