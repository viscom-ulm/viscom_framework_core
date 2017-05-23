/**
 * @file   initialize.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2017.05.23
 *
 * @brief  Functions needed to initialize the application.
 */

#pragma once

#include <memory>


namespace viscom {

    class ApplicationNodeInternal;
    struct FWConfiguration;

    std::unique_ptr<ApplicationNodeInternal> Application_Init(const FWConfiguration&);

}
