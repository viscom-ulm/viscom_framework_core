/**
 * @file   initialize.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2017.05.23
 *
 * @brief  Implementation of functions needed to initialize the application.
 */

#include "core/initialize.h"
#include "core/main.h"
#include "core/ApplicationNodeInternal.h"

namespace viscom {

    std::unique_ptr<ApplicationNodeInternal> Application_Init(const FWConfiguration& config) {
        auto internalConfig = config;
        auto node = std::make_unique<viscom::ApplicationNodeInternal>(std::move(internalConfig));

        return node;
    }
}
