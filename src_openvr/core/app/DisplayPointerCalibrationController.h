/**
 * @file   DisplayPointerCalibrationController.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2019.02.21
 *
 * @brief  Calibration class for the display pointer.
 */

#pragma once

#include "core/app/OpenVR.h"

namespace viscom::ovr {

    // calibration order is always: lower left, lower right, upper left.
    class CalibrationController
    {
    public:
        CalibrationController(CalibrateMethod method) : calibrationMethod_{ method } {}

        void DisplayCalibrationGUI() const;
        bool IsFinished() { return calibratedPositions_.size() >= 3; }
        void DoNextCalibrationStep(std::uint32_t unDevice);

        DisplayPlane GetDisplayPlane() const;

    private:
        CalibrateMethod calibrationMethod_ = CalibrateMethod::CALIBRATE_BY_POINTING;
        std::vector<glm::vec3> calibratedPositions_;
    };
}
