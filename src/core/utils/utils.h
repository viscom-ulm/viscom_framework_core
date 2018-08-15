/**
 * @file   utils.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.com>
 * @date   2017.05.23
 *
 * @brief  Declaration of some utility functions.
 */

#pragma once

#include <cstring>
#include <string>
#include <future>
#include <vector>

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

        /**
         * Removes whitespaces on string beginning and end
         * @param s the string from which to trim
         * @param w the whitespace characters which should be removed (default: <space> and <tab>)
         * @return the string without whitespaces
         */
        static std::string trim(const std::string& s, const std::string& w = " \t") {
            const auto strBegin = s.find_first_not_of(w);
            if (std::string::npos == strBegin) {
                return ""; // empty string or just whitespaces.
            }

            const auto strEnd = s.find_last_not_of(w);
            const auto strRange = strEnd - strBegin + 1;

            return s.substr(strBegin, strRange);
        }

        static std::vector<std::string> split(const std::string &s, char delim) {
            std::stringstream ss(s);
            std::string item;
            std::vector<std::string> tokens;
            while (getline(ss, item, delim)) {
                tokens.push_back(item);
            }
            return tokens;
        }

        static void memcpyfaster(void* dest, const void* src, std::size_t size) {
            std::size_t offset = 0;
            std::size_t stride = 4096;

            while (offset < size)
            {
                if ((size - offset) < stride)
                    stride = size - offset;

                memcpy(reinterpret_cast<char*>(dest) + offset, reinterpret_cast<const char*>(src) + offset, stride);
                offset += stride;
            }
        }
    }
}
