#ifndef __SPDLOG_LOGGER__
#define __SPDLOG_LOGGER__

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "application_exit_code.h"

// 全局 logger
extern std::shared_ptr<spdlog::logger> g_logger;

class LoggerConfig
{
    public:
    const std::string m_pattern;
    spdlog::level::level_enum m_level;

    LoggerConfig();
    ~LoggerConfig();

    void set_level(const std::string& level);
    std::string get_level() const;

    void apply();
};

// 初始化logger
void initialize_logger();

#endif  // __SPDLOG_LOGGER__
