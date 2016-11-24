/**
 * @file   main.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.24
 *
 * @brief  Variables, definitions and includes used throughout the framework.
 */

#pragma once

// ReSharper disable CppUnusedIncludeDirective
#include <string>
#include <array>
// ReSharper restore CppUnusedIncludeDirective

struct FWConfiguration
{
    std::string baseDirectory_;
    std::string programProperties_;
    std::string sgctConfig_;
    std::string projectorData_;
    std::string colorCalibrationData_;
    std::string sgctLocal_;
};
