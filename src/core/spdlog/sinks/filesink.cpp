/**
 * @file   filesink.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2020.04.23
 *
 * @brief  Implementation of a spdlog file sink that rotates files after program restart.
 * A lot of code is copied from the original rotating_file_sink in spdlog.
 */

#include "core/spdlog/sinks/filesink.h"
#include <cassert>
#include <spdlog/details/os.h>

namespace viscom::spdlog::sinks {

    template<typename Mutex>
    inline rotating_open_file_sink<Mutex>::rotating_open_file_sink(::spdlog::filename_t base_filename,
                                                                   std::size_t max_files)
        : base_filename_(std::move(base_filename)), max_files_(max_files)
    {
        file_helper_.open(calc_filename(base_filename_, 0));
        auto current_size = file_helper_.size(); // expensive. called only once
        if (current_size > 0) { rotate_(); }
    }

    template<typename Mutex>
    ::spdlog::filename_t rotating_open_file_sink<Mutex>::calc_filename(const ::spdlog::filename_t& filename,
                                                                       std::size_t index)
    {
        if (index == 0u) { return filename; }
        auto [basename, ext] = ::spdlog::details::file_helper::split_by_extension(filename);
        return fmt::format(SPDLOG_FILENAME_T("{}.{}{}"), basename, index, ext);
    }

    template<typename Mutex>::spdlog::filename_t rotating_open_file_sink<Mutex>::filename()
    {
        std::lock_guard<Mutex> lock(::spdlog::sinks::base_sink<Mutex>::mutex_);
        return file_helper_.filename();
    }

    template<typename Mutex> void rotating_open_file_sink<Mutex>::sink_it_(const ::spdlog::details::log_msg& msg)
    {
        ::spdlog::memory_buf_t formatted;
        ::spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        file_helper_.write(formatted);
    }

    template<typename Mutex> void rotating_open_file_sink<Mutex>::flush_() { file_helper_.flush(); }

    template<typename Mutex> void rotating_open_file_sink<Mutex>::rotate_()
    {
        using ::spdlog::details::os::filename_to_str;
        using ::spdlog::details::os::path_exists;
        file_helper_.close();
        for (auto i = max_files_; i > 0; --i) {
            ::spdlog::filename_t src = calc_filename(base_filename_, i - 1);
            if (!path_exists(src)) { continue; }
            ::spdlog::filename_t target = calc_filename(base_filename_, i);

            if (!rename_file_(src, target)) {
                // if failed try again after a small delay.
                // this is a workaround to a windows issue, where very high rotation
                // rates can cause the rename to fail with permission denied (because of antivirus?).
                ::spdlog::details::os::sleep_for_millis(100);
                if (!rename_file_(src, target)) {
                    file_helper_.reopen(true); // truncate the log file anyway to prevent it to grow beyond its limit!
                    SPDLOG_THROW(::spdlog::spdlog_ex("rotating_file_sink: failed renaming " + filename_to_str(src)
                                                         + " to " + filename_to_str(target),
                                                     errno));
                }
            }
        }
        file_helper_.reopen(true);
    }

    template<typename Mutex>
    bool rotating_open_file_sink<Mutex>::rename_file_(const ::spdlog::filename_t& src_filename,
                                                      const ::spdlog::filename_t& target_filename)
    {
        (void)::spdlog::details::os::remove(target_filename);
        return ::spdlog::details::os::rename(src_filename, target_filename) == 0;
    }

    template class rotating_open_file_sink<std::mutex>;
    template class rotating_open_file_sink<::spdlog::details::null_mutex>;

    template std::shared_ptr<::spdlog::logger>
    rotating_open_logger_mt<::spdlog::synchronous_factory>(const std::string& logger_name,
                                                           const ::spdlog::filename_t& filename, std::size_t max_files);

    template std::shared_ptr<::spdlog::logger>
    rotating_open_logger_st<::spdlog::synchronous_factory>(const std::string& logger_name,
                                                           const ::spdlog::filename_t& filename, std::size_t max_files);
}
