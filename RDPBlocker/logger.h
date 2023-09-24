#ifndef __SPDLOG_LOGGER_H__
#define __SPDLOG_LOGGER_H__

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "application_exit_code.h"

// 全局 logger
extern std::shared_ptr<spdlog::logger> g_logger;

// 初始化logger
void initialize_global_logger(std::shared_ptr<spdlog::logger>& logger);

#endif  // __SPDLOG_LOGGER_H__
