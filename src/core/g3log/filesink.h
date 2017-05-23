/**
 * @file   filesink.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.10.28
 *
 * @brief  Declaration of a derivation of FileSink for g3log without timestamp.
 * This file is heavily based on the FileSink by KjellKod.cc.
 */

#pragma once

#include <memory>

#include "g3log/logmessage.hpp"
namespace vku {

    class FileSink final {
    public:
        FileSink(const std::string &log_prefix, const std::string &log_directory, bool useTimestamp = false, const std::string &logger_id = "g3log");
        ~FileSink();

        void fileWrite(g3::LogMessageMover message) const;
        std::string changeLogFile(const std::string &directory, const std::string &logger_id);
        std::string fileName() const;


    private:
        bool useTimestamp_;
        std::string _log_file_with_path;
        std::string _log_prefix_backup; // needed in case of future log file changes of directory
        std::unique_ptr<std::ofstream> _outptr;

        void addLogFileHeader() const;
        std::ofstream &filestream() const
        {
            return *(_outptr.get());
        }


        FileSink &operator=(const FileSink &) = delete;
        FileSink(const FileSink &other) = delete;

    };
}
