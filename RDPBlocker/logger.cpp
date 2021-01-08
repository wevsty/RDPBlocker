#include "logger.h"

std::shared_ptr<spdlog::logger> g_logger = NULL;

void init_logger()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);

    auto max_size = 1024 * 1024 * 1024;
    auto max_files = 3;
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logger.txt", max_size, max_files);
    rotating_sink->set_level(spdlog::level::debug);

    //spdlog::logger logger("global_logger", { console_sink, rotating_sink });
    auto logger_ptr = std::make_shared<spdlog::logger>(
        spdlog::logger("global_logger", { console_sink, rotating_sink })
        );
    
    logger_ptr->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
    logger_ptr->set_level(spdlog::level::info);
    #ifdef DEBUG
    logger_ptr->set_level(spdlog::level::debug);
    #endif // DEBUG
    logger_ptr->flush_on(spdlog::level::info);
    spdlog::set_default_logger(logger_ptr);

    g_logger = logger_ptr;
}
