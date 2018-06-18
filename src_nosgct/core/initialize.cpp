/**
 * @file   initialize.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2017.05.23
 *
 * @brief  Implementation of functions needed to initialize the application.
 */

#include "core/initialize.h"
#include "core/main.h"
#include "core/FrameworkInternal.h"

namespace viscom {

    std::unique_ptr<FrameworkInternal> Application_Init(const FWConfiguration& config,
        InitNodeFunc coordinatorNodeFactory,
        InitNodeFunc workerNodeFactory) {
        auto internalConfig = config;
        auto node = std::make_unique<viscom::FrameworkInternal>(std::move(internalConfig),
            std::move(coordinatorNodeFactory), std::move(workerNodeFactory));

        return node;
    }
}
