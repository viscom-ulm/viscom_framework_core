/**
 * @file   SlaveNodeHelper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.21
 *
 * @brief  Helper header for using the internal SlaveNodeInternal class only if calibration is needed.
 */

#pragma once

#include "app/ApplicationNodeImplementation.h"

#ifndef VISCOM_LOCAL_ONLY
#include "core/SlaveNodeInternal.h"
#else
namespace viscom {
    using SlaveNodeInternal = ApplicationNodeImplementation;
}
#endif
