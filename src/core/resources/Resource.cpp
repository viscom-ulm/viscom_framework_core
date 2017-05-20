/**
 * @file   Resource.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Implementation of the resource base class.
 */

#include "Resource.h"

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
}
