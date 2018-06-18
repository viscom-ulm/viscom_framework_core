/**
 * @file   WorkerNodeInternal.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2018.06.15
 *
 * @brief  Defines the internal worker node class.
 */

#pragma once

#ifndef VISCOM_LOCAL_ONLY
#include "core/app_internal/WorkerNodeCalibratedInternal.h"
namespace viscom {
    using WorkerNodeInternal = WorkerNodeCalibratedInternal;
}
#else
#include "core/app_internal/WorkerNodeLocalInternal.h"
namespace viscom {
    using WorkerNodeInternal = WorkerNodeLocalInternal;
}
#endif


