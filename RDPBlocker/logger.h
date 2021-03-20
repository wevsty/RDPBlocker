#ifndef __SPDLOG_LOGGER__
#define __SPDLOG_LOGGER__

#include <algorithm>
#include <string>
#include <chrono>
#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "self_exit_code.h"

extern std::shared_ptr<spdlog::logger> g_logger;

class logger_options
{
public:
	std::string filename;
	int max_size;
	int max_files;
	spdlog::level::level_enum level;

	logger_options();
	~logger_options();
	void set_level_string(const std::string& level);
	std::string get_level_string() const;
};

void init_global_logger(const logger_options& options);

#endif // __SPDLOG_LOGGER__