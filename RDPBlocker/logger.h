#ifndef __SPDLOG_LOGGER__
#define __SPDLOG_LOGGER__

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// 全局 logger
extern std::shared_ptr<spdlog::logger> g_logger;

class logger_options
{
    public:
    spdlog::level::level_enum level;

    logger_options();
    ~logger_options();
    void set_level_string(const std::string& level);
    std::string get_level_string() const;
};

#endif  // __SPDLOG_LOGGER__