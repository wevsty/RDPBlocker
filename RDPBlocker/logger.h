#ifndef __SPDLOG_LOGGER__
#define __SPDLOG_LOGGER__

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

extern std::shared_ptr<spdlog::logger> g_logger;

void init_logger();

#endif // __SPDLOG_LOGGER__