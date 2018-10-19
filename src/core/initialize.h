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

    /**
    *  Initializes the application by creating a FrameworkInternal object.
    *  @param config the frameworks configuration object.
    *  @param coordinatorNodeFactory the class of the coordinator to be used.
    *  @param workerNodeFactory the class of the worker to be used.
    */
    std::unique_ptr<FrameworkInternal> Application_Init(const FWConfiguration& config,
        InitNodeFunc coordinatorNodeFactory,
        InitNodeFunc workerNodeFactory);

}
