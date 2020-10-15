/**
 * @file   filesink.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2020.04.23
 *
 * @brief  Declaration of a spdlog file sink that rotates files after program restart.
 */

#pragma once

#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/base_sink.h>

#include <memory>
#include <mutex>

namespace viscom::spdlog::sinks {

    template<typename Mutex> class rotating_open_file_sink final : public ::spdlog::sinks::base_sink<Mutex>
    {
    public:
        rotating_open_file_sink(::spdlog::filename_t base_filename, std::size_t max_files);
        static ::spdlog::filename_t calc_filename(const ::spdlog::filename_t& filename, std::size_t index);
        ::spdlog::filename_t filename();

    protected:
        void sink_it_(const ::spdlog::details::log_msg& msg) override;
        void flush_() override;

    private:
        void rotate_();
        bool rename_file_(const ::spdlog::filename_t& src_filename, const ::spdlog::filename_t& target_filename);

        ::spdlog::filename_t base_filename_;
        std::size_t max_files_;
        ::spdlog::details::file_helper file_helper_;
    };

    using rotating_open_file_sink_mt = rotating_open_file_sink<std::mutex>;
    using rotating_open_file_sink_st = rotating_open_file_sink<::spdlog::details::null_mutex>;

    template<typename Factory = ::spdlog::synchronous_factory>
    inline std::shared_ptr<::spdlog::logger>
    rotating_open_logger_mt(const std::string& logger_name, const ::spdlog::filename_t& filename, std::size_t max_files)
    {
        return Factory::template create<rotating_open_file_sink_mt>(logger_name, filename, max_files);
    }

    template<typename Factory = ::spdlog::synchronous_factory>
    inline std::shared_ptr<::spdlog::logger>
    rotating_open_logger_st(const std::string& logger_name, const ::spdlog::filename_t& filename, std::size_t max_files)
    {
        return Factory::template create<rotating_open_file_sink_st>(logger_name, filename, max_files);
    }
}
