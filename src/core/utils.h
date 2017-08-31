/**
 * @file   utils.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.23
 *
 * @brief  Declaration of some utility functions.
 */

#pragma once

#include <string>
#include <future>

namespace viscom {

    namespace utils {

        /**
         *  Checks if a string end with another.
         *  @param s the string to check.
         *  @param e the ending to check for.
         */
        static bool endsWith(const std::string& s, const std::string& e) {
            if (s.length() >= e.length()) {
                return (0 == s.compare(s.length() - e.length(), e.length(), e));
            }
            else {
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
            }
            else {
                return false;
            }
        }

        static bool file_exists(const std::string& name) {
            if (FILE *file = fopen(name.c_str(), "r")) {
                fclose(file);
                return true;
            }
            else {
                return false;
            }
        }

        template<typename R>
        bool is_ready(std::future<R> const& f) {
            return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }

        static std::string trim(const std::string& str, const std::string& whitespace = " \t") {
            const auto strBegin = str.find_first_not_of(whitespace);
            if (strBegin == std::string::npos)
                return ""; // no content

            const auto strEnd = str.find_last_not_of(whitespace);
            const auto strRange = strEnd - strBegin + 1;

            return str.substr(strBegin, strRange);
        }
    }
}
