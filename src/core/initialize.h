/**
 * @file   initialize.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2017.05.23
 *
 * @brief  Functions needed to initialize the application.
 */

#pragma once

#include <memory>
#include "core/main.h"


namespace viscom {

    class FrameworkInternal;
    struct FWConfiguration;

    std::unique_ptr<FrameworkInternal> Application_Init(const FWConfiguration&,
        InitNodeFunc coordinatorNodeFactory,
        InitNodeFunc workerNodeFactory);

}
