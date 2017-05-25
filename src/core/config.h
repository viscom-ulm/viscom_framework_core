/**
 * @file   config.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.23
 *
 * @brief  Declaration of configuration related structs and functions.
 */

#pragma once

 // ReSharper disable CppUnusedIncludeDirective
#include <string>
#include <vector>
#include <glm/glm.hpp>
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
        bool sgctSlave_ = false;
        std::string tuioPort_;
        glm::vec2 virtualScreenSize_;
        std::vector<std::string> resourceSearchPaths_;
    };

    FWConfiguration LoadConfiguration(const std::string& configFilename);
}
