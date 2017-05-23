/**
 * @file   filesink.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.10.28
 *
 * @brief  Implementation of a derivation of FileSink for g3log without timestamp.
 * This file is heavily based on the FileSink by KjellKod.cc.
 */

#include "filesink.h"
#include "filesinkhelper.inl"
#include <cassert>

namespace vku {

    std::string createLogFileName(const std::string& _log_prefix_backup, const std::string& logger_id, bool useTimestamp)
    {
        auto file_name = internal::createLogFileName(_log_prefix_backup, logger_id);
        if (!useTimestamp) {
            file_name = _log_prefix_backup;
            if (logger_id != "") file_name += "." + logger_id;
            file_name += ".log";
        }
        return file_name;
    }

    FileSink::FileSink(const std::string &log_prefix, const std::string &log_directory, bool useTimestamp, const std::string& logger_id)
        : useTimestamp_{ useTimestamp },
        _log_file_with_path(log_directory)
        , _log_prefix_backup(log_prefix)
        , _outptr(new std::ofstream)
    {
        _log_prefix_backup = internal::prefixSanityFix(log_prefix);
        if (!internal::isValidFilename(_log_prefix_backup)) {
            std::cerr << "g3log: forced abort due to illegal log prefix [" << log_prefix << "]" << std::endl;
            abort();
        }

        auto file_name = createLogFileName(_log_prefix_backup, logger_id, useTimestamp_);
        _log_file_with_path = internal::pathSanityFix(_log_file_with_path, file_name);
        _outptr = internal::createLogFile(_log_file_with_path);

        if (!_outptr) {
            std::cerr << "Cannot write log file to location, attempting current directory" << std::endl;
            _log_file_with_path = "./" + file_name;
            _outptr = internal::createLogFile(_log_file_with_path);
        }
        assert(_outptr && "cannot open log file at startup");
        addLogFileHeader();
    }


    FileSink::~FileSink() {
        std::string exit_msg{ "\ng3log g3FileSink shutdown at: " };
        exit_msg.append(g3::localtime_formatted(g3::systemtime_now(), g3::internal::time_formatted));
        filestream() << exit_msg << std::flush;

        exit_msg.append({ "\nLog file at: [" }).append(_log_file_with_path).append({ "]\n\n" });
        std::cerr << exit_msg << std::flush;
    }

    // The actual log receiving function
    void FileSink::fileWrite(g3::LogMessageMover message) const
    {
        auto& out(filestream());
        out << message.get().toString() << std::flush;
    }

    std::string FileSink::changeLogFile(const std::string &directory, const std::string &logger_id) {

        auto now = g3::systemtime_now();
        auto now_formatted = g3::localtime_formatted(now, { g3::internal::date_formatted + " " + g3::internal::time_formatted });

        auto file_name = createLogFileName(_log_prefix_backup, logger_id, useTimestamp_);
        auto prospect_log = directory + file_name;
        auto log_stream = internal::createLogFile(prospect_log);
        if (nullptr == log_stream) {
            filestream() << "\n" << now_formatted << " Unable to change log file. Illegal filename or busy? Unsuccessful log name was: " << prospect_log;
            return{}; // no success
        }

        addLogFileHeader();
        std::ostringstream ss_change;
        ss_change << "\n\tChanging log file from : " << _log_file_with_path;
        ss_change << "\n\tto new location: " << prospect_log << "\n";
        filestream() << now_formatted << ss_change.str();
        ss_change.str("");

        auto old_log = _log_file_with_path;
        _log_file_with_path = prospect_log;
        _outptr = std::move(log_stream);
        ss_change << "\n\tNew log file. The previous log file was at: ";
        ss_change << old_log;
        filestream() << now_formatted << ss_change.str();
        return _log_file_with_path;
    }
    std::string FileSink::fileName() const
    {
        return _log_file_with_path;
    }
    void FileSink::addLogFileHeader() const
    {
        filestream() << internal::header();
    }

}
