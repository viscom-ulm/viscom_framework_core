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
#include <vector>
#include <glm/glm.hpp>

#include <g3log/g3log.hpp>
// ReSharper restore CppUnusedIncludeDirective

namespace viscom {

    struct FWConfiguration
    {
        std::string baseDirectory_;
        std::string viscomConfigName_;
        std::string programProperties_;
        std::string sgctConfig_;
        std::string projectorData_;
        std::string sgctLocal_;
    };
}
