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

    static const std::string resourceBasePath = "resources/";

    struct FWConfiguration
    {
        std::string baseDirectory_;
        std::string viscomConfigName_;
        std::string programProperties_;
        std::string sgctConfig_;
        std::string projectorData_;
        std::string sgctLocal_;
        bool sgctSlave_ = false;
        glm::vec2 virtualScreenSize_;
    };


    namespace utils {

        /**
         *  Checks if a string end with another.
         *  @param s the string to check.
         *  @param e the ending to check for.
         */
        static bool endsWith(const std::string& s, const std::string& e) {
            if (s.length() >= e.length()) {
                return (0 == s.compare(s.length() - e.length(), e.length(), e));
            } else {
                return false;
            }
        }

        /**
         *  Checks if a string starts with another.
         *  @param s the string to check.
         *  @param e the beginning to check for.
         */
        static bool beginsWith(const std::string& s, const std::string& b) {
            if (s.length() >= b.length()) {
                return (0 == s.compare(0, b.length(), b));
            } else {
                return false;
            }
        }
    }
}
